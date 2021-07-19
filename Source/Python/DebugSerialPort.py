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


from DebugPortDriver import DebugPortDriver
import serial
import serial.tools.list_ports
import struct


class DebugSerialPort(DebugPortDriver):
    def __init__(self, port=None, baudRate=19200, timeout=1):
        self.__port = port
        self.__baudRate = baudRate
        self.__timeout = timeout

        self.__serialPort = None

    @property
    def port(self):
        return self.__port

    @property
    def baudRate(self):
        return self.__baudRate

    @property
    def timeout(self):
        return self.__timeout

    @staticmethod
    def getAvailablePorts():
        ports = serial.tools.list_ports.comports()
        for p in ports:
            print(p.device)

    def open(self):
        try:
            self.__serialPort = serial.Serial(port=self.__port, baudrate=self.__baudRate, timeout=self.__timeout)
        except (serial.SerialException):
            print("DebugSerialPort.open(): error")
            raise

    def close(self):
        self.__serialPort.close()

    def send(self, packet: bytes) -> int:
        bytesWritten = self.__serialPort.write(packet)
        return bytesWritten

    def receive(self) -> bytes:
        readByte = self.__serialPort.read()
        if not readByte:
            raise Exception("DebugSerialPort._readByte(): timeout on read")
        return readByte


if __name__ == '__main__':
    s = DebugSerialPort('/dev/tty3', baudRate=9600)
    s.open()
    msg = bytes("TEST", 'utf-8')
    packet = struct.pack("@4s", msg)
    s.send(packet)