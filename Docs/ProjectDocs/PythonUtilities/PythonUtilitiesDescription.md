# CEF Python Utility

The Python Utility provides the developer with a console interface for interacting with a CEF-structured embedded project. At its core is a serial debug port for sending/receiving commands and logging messages. It also provides an extensible test framework for exercising components of the sytem in a command-based manner. Finally it includes functionality for interacting with the Continuous Integration system.

## Debug Port

The Python Utility implements a serial port for communication with the embedded CEF debug port, with logic for parsing CEF packets and routing them to appropriate handlers (i.e. commands and log messages). 