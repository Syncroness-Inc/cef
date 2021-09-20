# ##################################################################
#\copyright COPYRIGHT AND PROPRIETARY RIGHTS NOTICES:
#
#Copyright (C) 2021, an unpublished work by Syncroness, Inc.
#All rights reserved.
#
#This material contains the valuable properties and trade secrets of
#Syncroness of Westminster, CO, United States of America
#embodying substantial creative efforts and confidential information,
#ideas and expressions, no part of which may be reproduced or
#transmitted in any form or by any means, electronic, mechanical, or
#otherwise, including photocopying and recording or in connection
#with any information storage or retrieval system, without the prior
#written permission of Syncroness.
################################################################## #


import sys
from os.path import dirname, abspath
import ctypes
from threading import Thread
import time

sys.path.append(dirname(dirname(abspath(__file__))))
from Shared import cefContract
from DebugPortDriver import DebugPortDriver
from Transport import Transport
from Commands.CommandBase import *
from Common import CefCommonDefines
from Logger import Logger

class Router:
    """
    Object for dispositioning incoming packets to command and logging handlers. Packets
    are continuously read from the transport layer queue with a separate forever loop on its own
    thread.
    """
    def __init__(self, debugPortInterface: DebugPortDriver, responseTimeoutInSeconds=5, sendTimeoutInSeconds=5):                       
        if (cefContract.structureEndiannessType == ctypes.LittleEndianStructure):
        	self.__endianness = CefCommonDefines.LITTLE_ENDIAN
        else:
            self.__endianness = CefCommonDefines.BIG_ENDIAN
            
        self.__transport = Transport(debugPortInterface, self.__endianness)
        self.__logger = Logger()
        self.__packetReadThread = Thread(target=self._readPackets)
        self.__sequenceNumber = 0
        self.__lastSentCommand = None
        self.__lastSendTime = None
        self.commandResponsePending = False
        self.responseTimeoutInSeconds = responseTimeoutInSeconds
        self.sendTimeoutInSeconds = sendTimeoutInSeconds
        self.timeoutOccurred = False
        self.commandSuccess = False

        self.__packetReadThread.start()

    def send(self, command: CommandBase):
        """
        Transmit the command request via the supplied transport layer. Only one oustanding
        command is permitted at a time - a response must be received or a timeout must occur before
        a new command may be issued. Upon sending, the current sequence number is incremented and 
        applied to the outgoing command's header.
        @param command: the full command (header and body) to be used as packet payload
        @return: False if a response has not yet been received for the current command, else True
        """
        if self.commandResponsePending:
            print("Cannot send, awaiting pending response")
            return False
        else:
            self.__sequenceNumber += 1
            command.setRequestSequenceNumber(self.__sequenceNumber)
            self.timeoutOccurred = False
            self.commandResponsePending = True
            self.commandSuccess = False
            self.__lastSentCommand = command
            self.__lastSendTime = time.time()

            # start a thread to send the command - this makes the send
            # non-blocking and allows for timeout checking
            sendThread = Thread(target=self._send, args=(command,))
            sendThread.start()
            sendThread.join(timeout=self.sendTimeoutInSeconds)
            if sendThread.isAlive():
                self.commandResponsePending = False
                self.timeoutOccurred = True
                print("Timeout occurred on command request (send)")
                return False
            
            return True

    def _send(self, command):  
        """
        Sends the command to the transport layer
        """  
        self.__transport.send(command.payload())

    def _readPackets(self):
        """
        Retrieve framed packets from the transport queue. Command responses are validated for errors, logs
        are decoded (if necessary) and saved to file.
        """
        while(True):
            # check for timeout on the current request/response transaction
            currentTime = time.time()
            if self.commandResponsePending and not self.timeoutOccurred and abs(currentTime - self.__lastSendTime) > self.responseTimeoutInSeconds:
                self.commandResponsePending = False
                self.timeoutOccurred = True
                print("Timeout occurred on command response (receive)")

            # get the next packet in the queue and handle according to type - new packet types added
            # to the CEF contract should have handling added here
            packet = self.__transport.getNextPacket()
            if packet is not None:
                packetType = packet.header.m_packetType
                if packetType == cefContract.debugPacketDataType.debugPacketType_commandResponse.value:
                    self.commandSuccess = self._handleCommandResponse(packet)
                    self.commandResponsePending = False
                    print("Got a command response")
                elif packetType == cefContract.debugPacketDataType.debugPacketType_loggingData.value:
                    print("Got a log packet")
                    self._handleLog(packet)

                else:
                    raise Exception("Unknown packet type")

    def _handleLog(self, packet):
        """
        The main message-extraction logic for incoming log packets:
        1. Extract the response (packet payload) from the packet
        2. Extract the log's header and validate
        3. Extract the log's main content
        4. Hand off to Logging object for final processing (file I/O, parsing, etc.)
        @param packet: the full response packet received from the transport layer
        @return: False if any of the response fails to validate, else True
        """

        # 1. extract payload from packet
        payload = list(bytes(packet.payload.bytes))

        # 2. extract and populate the log response header
        responseHeader = cefContract.cefCommandHeader()
        for f in responseHeader._fields_:
            numBytes = ctypes.sizeof(f[1])
            n = int.from_bytes(payload[0:numBytes], self.__endianness)
            setattr(responseHeader, f[0], n)
            for b in range(numBytes):
                payload.pop(0) # remove the consumed bytes
            #print("{}: {}".format(f[0], hex(getattr(commandResponseHeader, f[0])))) # uncomment for debug

        # 2. validate header
        if not self.__logger.validateResponseHeader(responseHeader):
            return False

        # 3. extract and populate log response body
        logResponseBody = cefContract.cefLog()
        for f in logResponseBody._fields_:
            if f[0] == 'm_header':
                continue # skip the header since we've already consumed those bytes above
            numBytes = ctypes.sizeof(f[1])
            if not issubclass(f[1], ctypes.Array):
                n = int.from_bytes(payload[0:numBytes], self.__endianness)
            else:
                l = []
                for c in range(numBytes):
                    l.append((payload[c]))
                n = bytes(bytearray(l))
            setattr(logResponseBody, f[0], n)
            for b in range(numBytes):
                payload.pop(0) # remove the consumed bytes
            #print("{}: {}".format(f[0], hex(getattr(commandResponse, f[0])))) # uncomment for debug

        # 4. process the extracted log
        self.__logger.processLogMessage(logResponseBody)

        return True

    def _handleCommandResponse(self, packet):
        """
        The main message-extraction logic for incoming command response packets:
        1. Extract the response (packet payload) from the packet
        2. Check its length against the expected length (according to the contract)
        3. Extract the command's header and validate (proper sequence number and opCode, no error codes)
        4. Extract and validate the content of the command body (received values vs expected per CEF contract)
        @param packet: the full response packet received from the transport layer
        @return: False if any part of the response does not match expected values, else True
        """

        # 1. extract payload from packet
        payload = list(bytes(packet.payload.bytes))

        # 2. check the length against the expected type of response
        expectedLength = self.__lastSentCommand.expectedResponseLength()
        if len(payload) != expectedLength:
            print("Invalid command response length - received: {}, expected: {}".format(len(payload), expectedLength))
            return False

        # 3. extract and populate the command response header
        commandResponseHeader = cefContract.cefCommandHeader()
        for f in commandResponseHeader._fields_:
            numBytes = ctypes.sizeof(f[1])
            n = int.from_bytes(payload[0:numBytes], self.__endianness)
            setattr(commandResponseHeader, f[0], n)
            for b in range(numBytes):
                payload.pop(0) # remove the consumed bytes
            #print("{}: {}".format(f[0], hex(getattr(commandResponseHeader, f[0])))) # uncomment for debug

        # 3. validate the extracted header
        if not self.__lastSentCommand.validateResponseHeader(commandResponseHeader):
            return False
        
        # 4. extract and populate command response body
        commandResponse = self.__lastSentCommand.expectedResponseType
        for f in commandResponse._fields_:
            if f[0] == 'm_header':
                continue # skip the header since we've already consumed those bytes above
            numBytes = ctypes.sizeof(f[1])
            n = int.from_bytes(payload[0:numBytes], self.__endianness)
            setattr(commandResponse, f[0], n)
            for b in range(numBytes):
                payload.pop(0) # remove the consumed bytes
            #print("{}: {}".format(f[0], hex(getattr(commandResponse, f[0])))) # uncomment for debug

        # 4. validate extracted command body
        if not self.__lastSentCommand.validateResponseBody(commandResponse):
            return False
        
        return True
