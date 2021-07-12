# CEF Python Utility

The Python Utility provides the developer with a console interface for interacting with a CEF-structured target. At its core is a serial debug port for sending/receiving commands and logging messages. It also provides an extensible test framework for exercising components of the sytem in a command-based manner. Finally it will include functionality for interacting with the Continuous Integration system.

### Target communication

On launch the user is placed in a console environment. From here the user can interact with the target (hardware or simulator) via commands. For this, the Utility implements a serial port for communication with the embedded CEF debug port, with logic for parsing CEF packets and routing them to appropriate handlers (i.e. commands and log messages). 