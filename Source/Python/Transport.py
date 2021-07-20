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


PACKET_HEADER_SIZE_BYTES = 16
PAYLOAD_MAX_SIZE_BYTES = 512


class Transport:
    """
    Object for packetizing outgoing commands and framing incoming
    data
    """
    def __init__(self, debugPortInterface: DebugPortDriver):
        self.__debugPort = debugPortInterface
        self.__readThread = Thread(target=self.readLoop)
        self.__readThread.start()
        self.__readBuffer = []

    def readLoop(self):
        while(True):
            byte = self.__debugPort.receive()
            print(byte)
        


def send(packet):
    pass

def buildPacket(payload, packetType):
    # Check the payload for valid byte length and calculate checksum
    payloadSize = len(payload)
    assert payloadSize <= PAYLOAD_MAX_SIZE_BYTES
    payloadChecksum = calculateChecksum(payload)

    # Create a structure with deferred field size since it is variable
    class CefPayload(ctypes.Structure):
        pass

    CefPayload._fields_ = [('bytes', ctypes.c_byte * payloadSize)]
    packetPayload  = CefPayload()
    ctypes.memmove(packetPayload.bytes, payload, payloadSize)

    class CefPacket(ctypes.Structure):
        _pack_ = 32
        _fields_ = [
            ('header', cefContract.cefCommandDebugPortHeader),
            ('payload', CefPayload)
        ]

    # Construct the packet header
    packetHeader = cefContract.cefCommandDebugPortHeader()
    packetHeader.m_framingSignature = 0xFFFFFFFF
    packetHeader.m_packetPayloadChecksum = payloadChecksum
    packetHeader.m_payloadSize = payloadSize
    packetHeader.m_packetType = packetType
    packetHeader.m_reserve = 0xAA
    packetHeader.m_packetHeaderChecksum = 0

    # Calculate header checksum
    tmpChecksum = calculateChecksum(bytes(packetHeader))
    # ctypes.memmove(packetHeader.m_packetHeaderChecksum, tmpChecksum)
    packetHeader.m_packetHeaderChecksum = 0xFF

    # Now combine header and payload into full packet
    packet = CefPacket()
    packet.header = packetHeader
    packet.payload = packetPayload

    return packet, packetHeader, packetPayload

def calculateChecksum(data):
    byteData = bytes(data)
    s = 0
    for i in byteData:
        binary = bin(i)
        for b in binary:
            if b == '1':
                s += 1
    print("CHECKSUM: {}".format(s))
    return s


if __name__ == '__main__':
    payload = b'\x11' * 512
    # payload = bytes("TEST STRING", 'utf-8')
    packet, header, payload = buildPacket(payload, 0x00)
    print(ctypes.sizeof(packet))
    import serial
    s = serial.Serial(port='/dev/tty3', baudrate=9600, timeout=1)
    s.write(str(bytes(packet)).encode('utf-8'))
    print(bytes(header))

    from DebugSerialPort import DebugSerialPort
    p = DebugSerialPort('/dev/ttyACM0', baudRate=115200)
    p.open()

    t = Transport(p)