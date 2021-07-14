# HEADER

import struct
import ctypes
from Shared import cefContract



PACKET_HEADER_SIZE_BYTES = 16
PAYLOAD_MAX_SIZE_BYTES = 512

def buildPacket(payload):
    payloadSize = len(payload)
    assert payloadSize <= PAYLOAD_MAX_SIZE_BYTES
  
    packetChecksum = calculateChecksum(payload)

    packetFormatString = buildPacketFormatString()

    packet = struct.pack(
        packetFormatString,
        # framing signature,
        # payload checkum
        # payload size
        # packet type
        # reserve
        # packet checksum
        # payload
    )


def buildPacketFormatString():
    return "!IIIBBH"

def calculateChecksum(data):
    pass