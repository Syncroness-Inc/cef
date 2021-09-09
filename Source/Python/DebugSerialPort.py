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


import serial
import serial.tools.list_ports

from DebugPortDriver import DebugPortDriver


class DebugSerialPort(DebugPortDriver):
    """ 
    Concrete implementation of the DebugPortDriver using serial port semantics.
    The device/port must be provided. 
    """

    def __init__(self, port, baudRate=19200, timeout=None):
        self.__port = port
        self.__baudRate = baudRate
        self.__timeout = timeout
        self.__serialPort = None

        self.bytesRx = 0


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
        """ 
        Print the system's available serial ports 
        """
        ports = serial.tools.list_ports.comports()
        for p in ports:
            print(p.device)

    def open(self):
        """ 
        Instantiate a serial port and open with desired baudrate and timeout in seconds 
        """
        try:
            self.__serialPort = serial.Serial(port=self.__port, baudrate=self.__baudRate, timeout=self.__timeout)
            self.__serialPort.flush() # clear any leftover buffer data on open so it doesn't get read on the next receive
        except (serial.SerialException):
            print("DebugSerialPort.open(): error")
            raise

    def close(self):
        """ 
        Close the serial port immediately 
        """
        self.__serialPort.close()

    def send(self, data: bytes) -> int:
        """ 
        Writes the packet to the serial port
        @param data: bytearray to be written
        @return bytesWritten: number of bytes successfully written
        """
        bytesWritten = self.__serialPort.write(data)
        return bytesWritten

    def receive(self) -> bytes:
        """ 
        Read a single byte from the serial port. This will block forever, it
        is the responsibility of the application to apply threading/timeout logic
        """
        readByte = self.__serialPort.read()

        #if readByte:
        self.bytesRx = self.bytesRx + 1
        tempByte = int.from_bytes(readByte,"big",signed=False)
        #print("bytes: {}      got byte 0x{:02x}".format(self.bytesRx, tempByte))

        return readByte
