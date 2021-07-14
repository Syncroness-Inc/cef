# TBD HEADER

import serial


class SerialInterface:

    def __init__(self, port='/dev/ttyUSB0', baud=19200):
        self.serial = serial.Serial()
        self.serial.port = port
        self.serial.baudrate = baud
    
    def open(self):
        if self.serial.is_open:
            pass
        else:
            self.serial.open()

    def close(self):
        self.port.close()

    def read(self, numbytes):
        return self.port.read(numbytes)

    def write(self, data):
        self.port.write(data)