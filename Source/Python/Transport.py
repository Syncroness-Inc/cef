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
import copy
from threading import Thread

sys.path.append(dirname(dirname(abspath(__file__))))
from Shared import cefContract
from DebugPortDriver import DebugPortDriver


PACKET_HEADER_SIZE_BYTES = 16
PAYLOAD_MAX_SIZE_BYTES = 512

BIG_ENDIAN = 'big'
LITTLE_ENDIAN = 'little'


class Transport:
    """
    Object for packetizing outgoing commands and framing incoming
    data
    """

    # Create a structure with deferred field size since it is variable
    class CefPayload(ctypes.Structure):
        pass

    def __init__(self, debugPortInterface: DebugPortDriver, endianness=BIG_ENDIAN):
        self.__debugPort = debugPortInterface
        self.__endianness = endianness
        self.__readThread = Thread(target=self.readLoop)
        self.__readThread.start()
        self.__readBuffer = []
        self.__signatureFound = False


    def readLoop(self):
        # Look for framing signature
        # Get the rest of the header
        # Validate header checksum
        # Check payload size
        # Get rest of payload bytes
        # Validate payload checksum
        # Check packet type
        # Hand-off to router

        #TODO: make this size-agnostic instead of 4-byte?
        framingSignature = framingSignatureToBytes(self.__endianness)
        while(True):
            # look for framing signature
            self.__signatureFound = False
            self.__readBuffer = []
            while not self.__signatureFound:
                i = 0
                while i < 4:
                    byte = self.__debugPort.receive()
                    if int.from_bytes(byte, self.__endianness) == framingSignature[i]:
                        self.__readBuffer.append(byte)
                        i += 1
                    else:
                        # reset the read buffer and start over
                        self.__readBuffer = []
                        i = 0
                self.__signatureFound = True
            
            # get the rest of the header bytes and populate the struct
            for i in range(PACKET_HEADER_SIZE_BYTES - 4):
                byte = self.__debugPort.receive()
                self.__readBuffer.append(byte)
            print("HEADER FOUND, LEN:{}\n{}\n".format(len(self.__readBuffer),self.__readBuffer)) 
            packetHeader = cefContract.cefCommandDebugPortHeader()
            for f in packetHeader._fields_:
                numBytes = ctypes.sizeof(f[1])
                bitsLeft = 8*numBytes - 8
                i = 0
                for b in range(numBytes):
                    i = i | (int.from_bytes(self.__readBuffer[0], self.__endianness) << (bitsLeft - (b*8)))
                    self.__readBuffer.pop(0)
                setattr(packetHeader, f[0], i)
                bitsLeft -= 8*numBytes

            # calculate header checksum and compare to the received one
            headerCopy = copy.deepcopy(packetHeader)
            headerCopy.m_packetHeaderChecksum = 0
            headerChecksum = calculateChecksum(bytes(headerCopy))
            if headerChecksum != packetHeader.m_packetHeaderChecksum:
                #TODO: raise an exception here
                print("PACKET FRAMING CHECKSUM FAILURE: {} != {}".format(headerChecksum, packetHeader.m_packetHeaderChecksum))

            # check payload size and receive the outstanding bytes
            self.__readBuffer = []
            for b in range(packetHeader.m_payloadSize):
                self.__readBuffer.append(self.__debugPort.receive())

            # calculate payload checksum and compare to the received one

            # self.CefPayload._fields_ = [('bytes', ctypes.c_byte * packetHeader.m_payloadSize)]
            # packetPayload  = self.CefPayload()
            # ctypes.memmove(packetPayload.bytes, bytearray(self.__readBuffer), packetHeader.m_payloadSize)
            payloadChecksum = calculateChecksum(b''.join(self.__readBuffer))
            if payloadChecksum != packetHeader.m_packetPayloadChecksum:
                #TODO: raise an exception here
                print("PACKET FRAMING PAYLOAD CHECKSUM FAILURE")
                pass

            


def framingSignatureToBytes(endianness=BIG_ENDIAN):
    signature = []
    #TODO: make this size-agnostic instead of 32-bit?
    if endianness == LITTLE_ENDIAN:
        signature.append(cefContract.DEBUG_PACKET_UINT32_FRAMING_SIGNATURE & 0x000000FF)
        signature.append((cefContract.DEBUG_PACKET_UINT32_FRAMING_SIGNATURE & 0x0000FF00) >> 8)
        signature.append((cefContract.DEBUG_PACKET_UINT32_FRAMING_SIGNATURE & 0x00FF0000) >> 16)
        signature.append((cefContract.DEBUG_PACKET_UINT32_FRAMING_SIGNATURE & 0xFF000000) >> 24)
    else:
        signature.append((cefContract.DEBUG_PACKET_UINT32_FRAMING_SIGNATURE >> 24) & 0xFF)
        signature.append((cefContract.DEBUG_PACKET_UINT32_FRAMING_SIGNATURE >> 16) & 0xFF)
        signature.append((cefContract.DEBUG_PACKET_UINT32_FRAMING_SIGNATURE >> 8) & 0xFF)
        signature.append((cefContract.DEBUG_PACKET_UINT32_FRAMING_SIGNATURE >> 0) & 0xFF)
    return signature    


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
    packetHeader.m_framingSignature = cefContract.DEBUG_PACKET_UINT32_FRAMING_SIGNATURE
    packetHeader.m_packetPayloadChecksum = payloadChecksum
    packetHeader.m_payloadSize = payloadSize
    packetHeader.m_packetType = packetType
    packetHeader.m_reserve = 0x00
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

def calculateChecksum(data: bytes) -> int:
    s = 0
    for i in data:
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