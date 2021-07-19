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

from os import read
from DebugPortDriver import DebugPortDriver
import serial
import serial.tools.list_ports


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

    def send(self, packet):
        self._write(packet)

    def receive(self):
        response = self._readByte()
        return response

    def _readByte(self):
        readByte = self.__serialPort.read()
        if not readByte:
            raise Exception("DebugSerialPort._readByte(): timeout on read")
        return readByte

    def _write(self, data):
        bytesWritten = self.__serialPort.write(str(bytes(data)).encode('utf-8'))
        return bytesWritten