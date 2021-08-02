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
from Commands import *


class Router:
    """
    Object for dispositioning incoming packets to command and logging handlers. Packets
    are continuously read from the transport layer queue with a separate forever loop on its own
    thread.
    """
    def __init__(self, debugPortInterface: DebugPortDriver, endianness=Transport.BIG_ENDIAN, responseTimeout=5):
        self.__transport = Transport(debugPortInterface)
        self.__endianness = endianness
        self.__packetReadThread = Thread(target=self._readPackets)
        self.__sequenceNumber = 0
        self.__lastSentCommand = None
        self.__lastSendTime = None
        self.responsePending = False
        self.responseTimeout = responseTimeout # in seconds
        self.timeoutOccurred = False
        self.commandSuccess = False

        self.__packetReadThread.start()

    def send(self, command: Command):
        """
        Transmit the command request via the supplied transport layer. Only one oustanding
        command is permitted at a time - a response must be received or a timeout must occur before
        a new command may be issued. Upon sending, the current sequence number is incremented and 
        applied to the outgoing command's header.
        @param command: the full command (header and body) to be used as packet payload
        @return: False if a response has not yet been received for the current command, else True
        """
        if self.responsePending:
            print("Cannot send, awaiting pending response")
            return False
        else:
            self.__sequenceNumber += 1
            command.setSequenceNumber(self.__sequenceNumber)

            self.timeoutOccurred = False
            self.responsePending = True
            self.commandSuccess = False
            self.__lastSentCommand = command
            self.__lastSendTime = time.time()
            self.__transport.send(command.payload())

            return True

    def _readPackets(self):
        """
        Retrieve framed packets from the transport queue. Command responses are validated for errors, logs
        are decoded (if necessary) and saved to file.
        """
        while(True):
            # check for timeout on the current request/response transaction
            currentTime = time.time()
            if self.responsePending and abs(currentTime - self.__lastSendTime) > self.responseTimeout:
                self.responsePending = False
                self.timeoutOccurred = True
                print("Timeout occurred on command response")

            # get the next packet in the queue and handle according to type - new packet types added
            # to the CEF contract should have handling added here
            packet = self.__transport.getNextPacket()
            if packet is not None:
                packetType = packet.header.m_packetType
                if packetType == cefContract.debugPacketDataType.debugPacketType_commandResponse.value:
                    self.commandSuccess = self._handleCommandResponse(packet)
                    self.responsePending = False
                elif packetType == cefContract.debugPacketDataType.debugPacketType_loggingDataAscii.value:
                    #TODO log handling
                    pass
                elif packetType == cefContract.debugPacketDataType.debugPacketType_loggingDataBinary.value:
                    #TODO log handling
                    pass
                else:
                    raise Exception("Unknown packet type")

    def _handleCommandResponse(self, packet):
        """
        The main validation logic for incoming command responses:
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
            print("{}: {}".format(f[0], hex(getattr(commandResponseHeader, f[0])))) # uncomment for debug

        # 3. validate the extracted header
        if not self.__lastSentCommand.validateResponseHeader(commandResponseHeader):
            return False
        
        # 4. extract and populate command response body
        commandResponse = self.__lastSentCommand.expectedResponseType()
        for f in commandResponse._fields_:
            if f[0] == 'm_header':
                continue # skip the header since we've already consumed those bytes above
            numBytes = ctypes.sizeof(f[1])
            n = int.from_bytes(payload[0:numBytes], self.__endianness)
            setattr(commandResponse, f[0], n)
            for b in range(numBytes):
                payload.pop(0) # remove the consumed bytes
            print("{}: {}".format(f[0], hex(getattr(commandResponse, f[0])))) # uncomment for debug

        # 4. validate extracted command body
        if not self.__lastSentCommand.validateResponseBody(commandResponse):
            return False
        
        return True
