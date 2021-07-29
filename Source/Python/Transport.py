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


class Transport:
    """
    Object for packetizing outgoing commands and framing incoming data with bi-endian support.
    The class runs a separate thread for capturing all incoming data from the port.
    The debug port interface must be defined and supplied by the application.
    Framed packets are placed in a queue for the application to retrieve from.
    """

    BIG_ENDIAN = 'big'
    LITTLE_ENDIAN = 'little'

    PAYLOAD_HEADER_SIZE_BYTES = ctypes.sizeof(cefContract.cefCommandDebugPortHeader())
    PAYLOAD_MAX_SIZE_BYTES = cefContract.DEBUG_PORT_MAX_PACKET_SIZE_BYTES - PAYLOAD_HEADER_SIZE_BYTES

    def __init__(self, debugPortInterface: DebugPortDriver, endianness=BIG_ENDIAN):
        self.__debugPort = debugPortInterface
        self.__endianness = endianness
        self.__readThread = Thread(target=self._readLoop)
        self.__readBuffer = []
        self.__packetQueue = []

        self.__readThread.start()

    @staticmethod
    def calculateChecksum(data: bytes) -> int:
        """
        This simple checksum adds all bytes in the input
        @param data: the data to compute the checksum over, as a byte array
        @return byteSum: the sum of all bytes in the input data
        """
        byteSum = 0
        for byte in data:
            byteSum += byte
        # print("CHECKSUM: {}".format(byteSum)) # uncomment for debug
        return byteSum

    @staticmethod
    def framingSignatureToBytes(endianness=BIG_ENDIAN):
        """
        Helper method to translate the CEF Framing Signature from a 32-bit hex value
        to an iterable list of integers
        @param endianness: the byte order to apply during conversion
        """
        signature = []
        #TODO: make this size-agnostic instead of 32-bit?
        if endianness == Transport.LITTLE_ENDIAN:
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

    def getNextPacket(self):
        """
        Accessor for received packets
        @return: the first packet in the queue of received packets
        """
        if len(self.__packetQueue) > 0:
            return self.__packetQueue.pop(0)
        else:
            return None

    def send(self, payload: bytes):
        """
        Transmitter for outgoing data
        @param payload: data to be packetized and sent to the target
        """
        packet = self._buildPacket(payload)
        self.__debugPort.send(packet)

    def _readLoop(self):
        """
        Forever loop for reading incoming bytes, with the following sequence:
        1. Look for framing signature
        2. Receive remaining header bytes
        3. Validate received header against received checksum
        4. Check expected payload size and receive corresponding bytes
        5. Validate payload checksum against received checksum
        6. Put packet in the receiving queue
        """

        # retrieve the framing signature from the CEF contract and convert to an 
        # iterable for byte-by-byte incoming detection
        framingSignature = self.framingSignatureToBytes(self.__endianness)

        # start the loop
        while(True):
            # 1. look for framing signature
            signatureFound = False
            self.__readBuffer = []
            while not signatureFound:
                i = 0
                # while all bytes of the framing signature, in order, are not yet received
                while i < len(framingSignature):
                    byte = self.__debugPort.receive()
                    # if a byte matches our current position in the framing signature, 
                    # save it and increment, else start over at the first byte
                    if int.from_bytes(byte, self.__endianness) == framingSignature[i]:
                        self.__readBuffer.append(byte)
                        i += 1
                    else:
                        # reset the read buffer and start over
                        self.__readBuffer = []
                        i = 0
                signatureFound = True
            
            # 2. receive remaining header bytes and populate a structure with them
            for i in range(self.PAYLOAD_HEADER_SIZE_BYTES - len(framingSignature)):
                byte = self.__debugPort.receive()
                self.__readBuffer.append(byte)
            # print("HEADER FOUND, LEN:{}\n{}\n".format(len(self.__readBuffer),self.__readBuffer)) # uncomment for debug
            packetHeader = cefContract.cefCommandDebugPortHeader()

            # populate header fields from the buffer
            for f in packetHeader._fields_:
                numBytes = ctypes.sizeof(f[1])
                n = int.from_bytes(b''.join(self.__readBuffer[0:numBytes]), self.__endianness)
                setattr(packetHeader, f[0], n)
                for b in range(numBytes):
                    self.__readBuffer.pop(0) # remove the consumed bytes

            # 3. validate received header against received checksum
            headerCopy = copy.deepcopy(packetHeader)
            headerCopy.m_packetHeaderChecksum = 0
            headerChecksum = self.calculateChecksum(bytes(headerCopy))
            if headerChecksum != packetHeader.m_packetHeaderChecksum:
                #TODO: raise an exception here
                print("PACKET FRAMING HEADER CHECKSUM FAILURE: {} != {}".format(headerChecksum, packetHeader.m_packetHeaderChecksum))

            # 4. check expected payload size and receive corresponding bytes
            self.__readBuffer = []
            for b in range(packetHeader.m_payloadSize):
                self.__readBuffer.append(self.__debugPort.receive())

            # 5. validate payload checksum against received checksum
            payloadChecksum = self.calculateChecksum(b''.join(self.__readBuffer))
            if payloadChecksum != packetHeader.m_packetPayloadChecksum:
                #TODO: raise an exception here
                print("PACKET FRAMING PAYLOAD CHECKSUM FAILURE: {} != {}".format(payloadChecksum, packetHeader.m_packetPayloadChecksum))
            
            # 6. put packet in the receiving queue
            packet = self._buildPacket(b''.join(self.__readBuffer), packetHeader)
            self.__packetQueue.append(packet)

    def _buildPacket(self, payload: bytes, packetHeader=None):
        """
        Helper function for incoming and outgoing packet assembly, combines header and payload
        @param payload: the outgoing data in bytes
        @param packetHeader: header to be used - reception should supply this argument while transmission 
        should omit it
        @return packet: the final, full packet with header and payload and associated checksums
        """
        assert(len(payload) <= self.PAYLOAD_MAX_SIZE_BYTES)

        if packetHeader is None:
            packetHeader = cefContract.cefCommandDebugPortHeader()
            packetHeader.m_framingSignature = cefContract.DEBUG_PACKET_UINT32_FRAMING_SIGNATURE
            packetHeader.m_packetPayloadChecksum = self.calculateChecksum(payload)
            packetHeader.m_payloadSize = len(payload)
            packetHeader.m_packetType = cefContract.debugPacketDataType.debugPacketType_commandRequest.value # Outgoing packets are always this type
            packetHeader.m_reserve = 0
            tmpChecksum = self.calculateChecksum(bytes(packetHeader))
            packetHeader.m_packetHeaderChecksum = tmpChecksum

        # defer structure definitions until now because the payload field size is variable
        class CefPayload(ctypes.Structure):
            # _pack_ = 1 # uncomment and edit this value if there are boundary alignment issues        
            _fields_ = [('bytes', ctypes.c_byte * packetHeader.m_payloadSize)]

        class CefPacket(ctypes.Structure):
            # _pack_ = 1 # uncomment and edit this value if there are boundary alignment issues
            _fields_ = [('header', cefContract.cefCommandDebugPortHeader), ('payload', CefPayload)]

        packetPayload  = CefPayload()
        packet = CefPacket()

        # populate the fields
        ctypes.memmove(packetPayload.bytes, payload, packetHeader.m_payloadSize)
        packet.header = packetHeader
        packet.payload = packetPayload
        return packet
