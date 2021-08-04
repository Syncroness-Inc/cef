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


from Router import Router
from Commands.PingCommand import CommandPing


class Base:
    """
    Base class for creating test modules to interact with the embedded target
    """
    def __init__(self, interface):
        self.__router = Router(interface)

    def send(self, command):
        self.__router.send(command)
        #TODO: raise exception if send fails

    def receive(self):
        while self.__router.commandResponsePending and not self.__router.timeoutOccurred:
            pass
        if self.__router.timeoutOccurred:
            return False
        else:
            if not self.__router.commandSuccess:
                return False
            else:
                return True


class Diag(Base):
    """
    Basic test module for issuing Ping commands to the target to test communications
    """
    def __init__(self, interface):
        super().__init__(interface)

    def ping(self, var1=0, var2=0, var3=0):
        ping = CommandPing()

        #TODO: combine these into a single execute()
        self.send(ping)
        result = self.receive()


        if result:
            print("Ping Success!")
        else:
            print("Ping Fail!")


if __name__ == '__main__':
    from DebugSerialPort import DebugSerialPort
    p = DebugSerialPort('/dev/ttyACM0', baudRate=115200)
    # p = DebugSerialPort('/dev/tty3', baudRate=9600)
    p.open()
    d = Diag(p)
    d.ping()
