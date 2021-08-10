# How to create a CEF Command

This "how to guide" is a brief overview of how to create a CEF command.

A "command" as outlined elsewhere in the CEF documentation, is a structured design methodology for performing work within the CEF Embedded software.

A "CEF command" is a command has the following attributes:

* triggered from the Python Utilities
* the CEF command request is passed through the Debug Port infrastructure
* instantiated and executed as if the command was generated internally to the Embedded Software
* returns the results of the command via the Debug Port infrastructure to Python Utilities
* Python Utilities may combine multiple CEF commands to conduct a desired task/test
* Only one CEF command can exist in the system at one time (done for simplicity)

To create a "command", a subset of the following steps is used.  The "Ping" command is a good design pattern to follow.

## CEF Contract

The Embedded software and Python Utilities need a "contract" to communicate how a command is to be setup.  This is done via the "cefContract" files.  This "contract" is currently in two files, `cefContract.hpp` and `cefContract.py`.  Eventually there will only be one file that is the master, and the other is auto generated.  For now, the information has to be duplicated between the two files.  There are some sanity checks to make sure the two files are in synch (e.g. checking the size of structures), but it is inherently a manual operation.

### Information to be entered in CEF Contract to create a CEF command

1. OpCode
2. ErrorCodes (that may be returned by the command)
3. CEF Command Request and Response structures.  These structures will typically shadow the Embedded Software object's request and response field.  As the Embedded Software's command may rely on objects inside the request , and the contract only supports fundamental data types, a Cef Command request/response structure is necessary.  Be sure to pay attention to the "packing and alignment" notes in cefContract.hpp.  It is the responsibility of the Python Utilities to handle "endianess".

The above information needs to entered in both `cefContract.hpp` and `cefContract.py`.  It is typically easier to enter the data in `cefContract.hpp` and then copy/paste/modify it in `cefContract.py`.

## Python Utilities

The following is the minimum operation that needs to be implemented in Python Utilities.  Python Utilities has a common infrastructure that

* Sends the command request via the Debug Port to the CEF Embedded Software
* Waits for a command response
* Validates the command response
* Returns the response to the calling function

The Ping command implements additional features that are optional.

### Information in Python Utilities

1. Create a CEF command object file, similar to PingCommand.py.  The required sections are `def __init__(self):`,  and `buildCommand()`.  Within `buildCommand()`, the following code is required.

   ```
           # build the header
           self.header.m_commandSequenceNumber = 0 # this is populated at transmit-time
           self.header.m_commandErrorCode = cefContract.errorCode.errorCode_OK.value
           self.header.m_commandOpCode = cefContract.commandOpCode.commandOpCodePing.value
           self.header.m_commandNumBytes = ctypes.sizeof(cefContract.cefCommandPingRequest)

           # build the body
           self.request = cefContract.cefCommandPingRequest()
           self.request.m_header = self.header
   ```

   Other sections of the Ping code are optional.  Typically, there will be a request and response field for most commands.  The request and response fields can either be filled out by the object itself, or the the fields can be setup within the command's wrapper function in `TestUtility.py`
2. In `TestUtility.py` a test command like `def ping(self, printResults=True):` must be implemented.  This allows the command to be executed from the python command line, or embedded in other Python Utilities methods.  Typically the response fields are used to build a dictionary that is returned and used by python calling routines.

## Embedded Software

There is a common infrastructure in the Embedded Software that handles

* receiving the CEF command request
* creating the command
* importing the CEF command information into the command
* executing the command
* exporting the command results back to the CEF command response
* sending the CEF command response back to Python Utilities

### Information in Embedded Software

1. In `CommandGenerator.cpp` add the command to the appropriate "command pool".  The size of the command is used to appropriately size the memory pool that the command will be instantiated from.
2. In`CommandGenerator.cpp`, `CommandGenerator::allocateCommand` add a case statement into the switch statement following the same design pattern as `case commandOpCodePing:`.  This is necessary to instantiate the command.
3. Create a `.cpp` and `.hpp` file similar to `CommandPing.cpp/hpp` .  All commands must have an `execute()`, `importFromCefCommand()`, and `exportToCefCommand()`.  `importFromCefCommand` must call `importFromCefCommandBase()`.  `exportToCefCommand` must call `exportToCefCommandBase`

The "import" method is used to translate the information provided by Python Utilities into the command infrastructure.  Using this translation method allows the Embedded Software to implement the command in whatever manner is optimal for the Embedded Software, without being constrained by the cefContract limitations.
