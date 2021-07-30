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
    Object for dispositioning incoming packets to command and logging handlers.
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

        self.__packetReadThread.start()

    def send(self, command: Command):
        if self.responsePending:
            print("Cannot send, awaiting pending response")
            return False
        else:
            self.timeoutOccurred = False
            self.__sequenceNumber += 1
            command.setSequenceNumber(self.__sequenceNumber)
            self.__transport.send(command.payload())
            self.responsePending = True
            self.__lastSentCommand = command
            self.__lastSendTime = time.time()
            return True

    def _readPackets(self):
        while(True):
            currentTime = time.time()
            if self.responsePending and abs(currentTime - self.__lastSendTime) > self.responseTimeout:
                # timeout on command response
                self.responsePending = False
                self.timeoutOccurred = True
                print("Timeout occurred on command response")
            packet = self.__transport.getNextPacket()
            if packet is not None:
                self.responsePending = False
                packetType = packet.header.m_packetType
                if packetType == cefContract.debugPacketDataType.debugPacketType_commandResponse.value:
                    self._handleCommandResponse(packet)
                elif packetType == cefContract.debugPacketDataType.debugPacketType_loggingDataAscii.value:
                    #TODO log handling
                    pass
                elif packetType == cefContract.debugPacketDataType.debugPacketType_loggingDataBinary.value:
                    #TODO log handling
                    pass
                else:
                    raise Exception("Unknown packet type")
                

    def _handleCommandResponse(self, packet):
        # extract commandresponse from packet
        # check against expected length
        # extract header from commandresponse
        # check sequence number
        # check error code
        # check opCode (should match what was sent)

        payload = list(bytes(packet.payload.bytes))
        print("RECEIVED VS EXPECTED    {}  {}".format(len(payload), self.__lastSentCommand.expectedResponseLength()))
        assert(len(payload) == self.__lastSentCommand.expectedResponseLength())

        commandResponseHeader = cefContract.cefCommandHeader()
        for f in commandResponseHeader._fields_:
            numBytes = ctypes.sizeof(f[1])
            n = int.from_bytes(payload[0:numBytes], self.__endianness)
            setattr(commandResponseHeader, f[0], n)
            for b in range(numBytes):
                payload.pop(0) # remove the consumed bytes
            print("{}: {}".format(f[0], hex(getattr(commandResponseHeader, f[0])))) # uncomment for debug

        # HEADER CHECKING STUFF HERE
        if not self.__lastSentCommand.validateResponseHeader(commandResponseHeader):
            print("INVALID COMMAND RESPONSE HEADER")
            return False
        
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


        if not self.__lastSentCommand.validateResponseBody(commandResponse):
            print("INVALID COMMAND RESPONSE")
            return False
        
        print("RESPONSE SUCCESS")
        return True


if __name__ == '__main__':
    from DebugSerialPort import DebugSerialPort
    p = DebugSerialPort('/dev/ttyACM0', baudRate=115200)
    p.open()
    r = Router(p)
    testPing = CommandPing()

    r.send(testPing)
