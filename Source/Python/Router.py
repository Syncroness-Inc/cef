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

sys.path.append(dirname(dirname(abspath(__file__))))
from Shared import cefContract
from DebugPortDriver import DebugPortDriver
from Transport import Transport


class Command:
    pass

class CommandPing(Command):
    def __init__(self):
        self.header = cefContract.cefCommandHeader()
        self.header.m_commandOpCode = cefContract.commandOpCode.commandOpCodePing.value
        self.header.m_commandSequenceNumber = 0 # this is populated at transmit-time
        self.header.m_commandErrorCode = cefContract.errorCode.errorCode_OK.value
        self.header.m_commandNumBytes = ctypes.sizeof(cefContract.cefCommandPingRequest)

        self.request = cefContract.cefCommandPingRequest()
        self.request.m_header = self.header
        self.request.m_uint8Value = cefContract.CMD_PING_UINT8_REQUEST_EXPECTED_VALUE
        self.request.m_uint16Value = cefContract.CMD_PING_UINT16_REQUEST_EXPECTED_VALUE
        self.request.m_uint32Value = cefContract.CMD_PING_UINT32_REQUEST_EXPECTED_VALUE
        self.request.m_uint64Value = cefContract.CMD_PING_UINT64_REQUEST_EXPECTED_VALUE



class Router:
    """
    Object for dispositioning incoming packets to command and logging handlers.
    """

    def __init__(self, debugPortInterface: DebugPortDriver, endianness=Transport.BIG_ENDIAN):
        self.__transport = Transport(debugPortInterface)
        self.__endianness = endianness
        self.__packetReadThread = Thread(target=self._readPackets)
        self.sequenceNumber = 1

        self.__packetReadThread.start()

    def send(self, command):
        command.header.m_commandSequenceNumber = self.sequenceNumber
        self.sequenceNumber += 1
        self.__transport.send(command)

    def _readPackets(self):
        while(True):
            packet = self.__transport.getNextPacket()
            if packet is not None:
                packetType = packet.header.m_packetType
                if packetType == COMMAND_RESPONSE:
                    pass
                elif packetType == LOG_ASCII:
                    pass
                elif packetType == LOG_BINARY:
                    pass
                else:
                    # unknown packet type, throw an exception
                    pass


if __name__ == '__main__':
    from DebugSerialPort import DebugSerialPort
    p = DebugSerialPort('/dev/ttyACM0', baudRate=115200)
    p.open()
    r = Router(p)

    testPing = CommandPing()
    r.send(testPing)
