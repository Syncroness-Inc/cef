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

import ctypes
import cefContract


PACKET_HEADER_SIZE_BYTES = 16
PAYLOAD_MAX_SIZE_BYTES = 512


def send():
    pass

def buildPacket(payload, packetType):
    # Check the payload for valid byte length and calculate checksum
    payloadSize = len(payload)
    print("Payload size {}".format(payloadSize))
    assert payloadSize <= PAYLOAD_MAX_SIZE_BYTES
    payloadChecksum = _calculateChecksum(payload)

    # Create a structure with deferred field size since it is variable
    class CefPayload(ctypes.Structure):
        pass

    CefPayload._fields_ = [('bytes', ctypes.c_uint8 * payloadSize)]
    packetPayload  = CefPayload()

    class CefPacket(ctypes.Structure):
        _pack_ = 32
        _fields_ = [
            ('header', cefContract.cefCommandDebugPortHeader),
            ('payload', CefPayload)
        ]

    # Construct the packet header
    packetHeader = cefContract.cefCommandDebugPortHeader()
    packetHeader.m_framingSignature = 0
    packetHeader.m_packetPayloadChecksum = payloadChecksum
    packetHeader.m_payloadSize = payloadSize
    packetHeader.m_packetType = packetType
    packetHeader.m_reserve = 0
    packetHeader.m_packetHeaderChecksum = 0

    # Calculate header checksum
    tmpChecksum = _calculateChecksum(packetHeader)
    packetHeader.m_packetHeaderChecksum = tmpChecksum

    # Now combine header and payload into full packet
    packet = CefPacket()
    packet.header = packetHeader
    packet.payload = packetPayload

    return packet, packetHeader, packetPayload

def _calculateChecksum(data):
    return 0


if __name__ == '__main__':
    payload = b'\x00' * 512
    payload = "TEST STRING"
    packet, header, payload = buildPacket(payload, 0x00)
    print(packet.header.m_framingSignature)
    print(ctypes.sizeof(packet))

    import serial
    s = serial.Serial('/dev/ttyS0', timeout=1)
    s.open()
    s.write(packet)