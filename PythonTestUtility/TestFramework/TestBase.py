# TBD HEADER

from DebugPort.DebugPort import DebugPort, CommandStatus


class TestBaseException(Exception):
    def __init__(self, msg, command):
        super().__init__(msg)
        self.command = command


class TestBase:
    """
    Base class for the CEF Test Framework
    """    

    def __init__(self):
        self.debugPort = DebugPort()

    def sendCommand(self, command):
        statusCode = self.debugPort.send(command)
        if statusCode != CommandStatus.SUCCESS:
            raise TestBaseException("Bad status code {}".format(statusCode), command)

