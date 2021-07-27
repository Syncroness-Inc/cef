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
        pass

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



def buildCommand(opCode):
    header = cefContract.cefCommandHeader()
    header.m_commandOpCode = opCode




if __name__ == '__main__':
    from DebugSerialPort import DebugSerialPort
    p = DebugSerialPort('/dev/ttyACM0', baudRate=115200)
    p.open()
    r = Router(p)

    def buildCommand():
        pingHeader = cefContract.cefCommandHeader()
        pingHeader.m_commandOpCode = cefContract.commandOpCode.commandOpCodePing.value
        pingHeader.m_commandSequenceNumber = 1
        pingHeader.m_commandErrorCode = 0
        pingHeader.m_commandNumBytes = 48

        pingRequest = cefContract.cefCommandPingRequest()
        pingRequest.m_header = pingHeader
        pingRequest.m_uint8Value = 1
        pingRequest.m_uint16Value = 2
        pingRequest.m_uint32Value = 3
        pingRequest.m_uint64Value = 4

        class CefCommand(ctypes.Structure):
            _fields_ = [('header', cefContract.cefCommandHeader), ('payload', cefContract.cefCommandPingRequest)]

        command = CefCommand()
        command.header = pingHeader
        command.payload = pingRequest

        return bytes(command)

    c = buildCommand()
    r.send(c)
    response = None
    while True:
        resp = t.getNextPacket()
        if resp is not None:
            print(resp)