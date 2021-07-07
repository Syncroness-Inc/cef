# TBD HEADER

import struct


class DebugPacketTypes:

    LOGGING = 1
    COMMAND_REQUEST = 2
    COMMAND_RESPONSE = 3


class CefPacket:
    """
    Defines the structure for Debug Port data transfer
    """

    HEADER_SIZE = 12
    PAYLOAD_SIZE = 512
    CHECKSUM_SIZE = 4
    PACKET_SIZE = HEADER_SIZE + PAYLOAD_SIZE + CHECKSUM_SIZE
    

    def __init__(self, interface = None):
        self.interface = interface
        self.framingSignature = b'0000'

    def buildPacket(self, payload, packetType):
        assert len(payload) == self.PAYLOAD_SIZE
        checksum = self.calculateChecksum(payload)
        packet = struct.pack(
                    '!IIB3c512sI',
                    int(self.framingSignature),
                    int(self.PAYLOAD_SIZE),
                    int(packetType),
                    b'0',
                    b'0',
                    b'0',
                    payload,
                    int(checksum)
                )

        assert len(packet) == self.PACKET_SIZE
        return packet

    def calculateChecksum(self, data):
        return b'0000'
    
if __name__ == '__main__':
    b = Base()
    p = b'\x00'*512
    pack = b.buildPacket(p, DebugPacketTypes.LOGGING)
    print(pack)
    print(len(pack))