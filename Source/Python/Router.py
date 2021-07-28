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
from abc import ABC, abstractmethod


sys.path.append(dirname(dirname(abspath(__file__))))
from Shared import cefContract
from DebugPortDriver import DebugPortDriver
from Transport import Transport


class Command(ABC):
    def __init__(self):
        self.header = cefContract.cefCommandHeader()
        self.header.m_commandSequenceNumber = 0 # this is populated at transmit-time
        self.header.m_commandErrorCode = cefContract.errorCode.errorCode_OK.value
        self.request = None
        self.expectedResponse = None

    @abstractmethod
    def buildCommand(self):
        """
        Populate the command header and request body according to the CEF contract. This is
        done in a method (instead of the subclass init) so that this base class can have at least
        one abstract method (without one, it could be instantiated at runtime - Python has 
        'loose' enforcement for abstract classes)
        """
        pass

    def __len__(self):
        return len(bytes(self.request))

    def payload(self):
        return bytes(self.request)

    def setSequenceNumber(self, sequenceNumber):
        self.header.m_commandSequenceNumber = sequenceNumber

    def expectedResponseLength(self):
        return len(bytes(self.expectedResponse))


class CommandPing(Command):
    def __init__(self):
        super().__init__()
        self.buildCommand()

    def buildCommand(self):
        self.header.m_commandOpCode = cefContract.commandOpCode.commandOpCodePing.value
        self.header.m_commandNumBytes = ctypes.sizeof(cefContract.cefCommandPingRequest)
        self.request = cefContract.cefCommandPingRequest()
        self.request.m_header = self.header
        self.request.m_uint8Value = cefContract.CMD_PING_UINT8_REQUEST_EXPECTED_VALUE
        self.request.m_uint16Value = cefContract.CMD_PING_UINT16_REQUEST_EXPECTED_VALUE
        self.request.m_uint32Value = cefContract.CMD_PING_UINT32_REQUEST_EXPECTED_VALUE
        self.request.m_uint64Value = cefContract.CMD_PING_UINT64_REQUEST_EXPECTED_VALUE
        self.expectedResponse = cefContract.cefCommandPingResponse()


class Router:
    """
    Object for dispositioning incoming packets to command and logging handlers.
    """

    def __init__(self, debugPortInterface: DebugPortDriver, endianness=Transport.BIG_ENDIAN):
        self.__transport = Transport(debugPortInterface)
        self.__endianness = endianness
        self.__packetReadThread = Thread(target=self._readPackets)
        self.__sequenceNumber = 0
        self.__lastSentCommand = None
        self.responsePending = False

        self.__packetReadThread.start()

    def send(self, command: Command):
        if self.responsePending:
            print("Cannot send, awaiting pending response")
            return False
        else:
            self.__sequenceNumber += 1
            command.setSequenceNumber(self.__sequenceNumber)
            self.__transport.send(command.payload())
            self.responsePending = True
            self.__lastSentCommand = command
            return True

    def _readPackets(self):
        while(True):
            packet = self.__transport.getNextPacket()
            if packet is not None:
                packetType = packet.header.m_packetType
                if packetType == cefContract.debugPacketDataType.debugPacketType_commandResponse.value:
                    self._handleCommandResponse(packet)
                elif packetType == cefContract.debugPacketDataType.debugPacketType_loggingDataAscii.value:
                    pass
                elif packetType == cefContract.debugPacketDataType.debugPacketType_loggingDataBinary.value:
                    pass
                else:
                    # unknown packet type, throw an exception
                    pass

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

        commandHeader = cefContract.cefCommandHeader()
        for f in commandHeader._fields_:
            numBytes = ctypes.sizeof(f[1])
            n = int.from_bytes(payload[0:numBytes], self.__endianness)
            setattr(commandHeader, f[0], n)
            for b in range(numBytes):
                payload.pop(0) # remove the consumed bytes
            print("FIELD {}, VALUE {}".format(f[0], getattr(commandHeader, f[0])))



if __name__ == '__main__':
    from DebugSerialPort import DebugSerialPort
    p = DebugSerialPort('/dev/ttyACM0', baudRate=115200)
    p.open()
    r = Router(p)

    testPing = CommandPing()
    r.send(testPing)
