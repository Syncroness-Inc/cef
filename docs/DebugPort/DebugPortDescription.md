# Debug Port

### Debug Communication

Below is an overview of debug communication between Python Test Utilities and CEF.  Communication will consist of commands, command responses, and logging data.  Only one command can be processed at a time.  Both the CEF and Python Test Utilities will have a common command document library to build and interpret commands/responses.  All communication must be non-blocking. 

##### Python Test Utilities - User Debug

* This is the users debug console.  This will send command requests and receive commands and logging information.
* CEF can only accept one command at a time.  Any command the user sends while CEF is processing prior command will be “lost”.

##### CEF/Communication Layer

The communication layers job is to handle all ingoing and outgoing communication with Python Test Utilities.  It will send both logging data and command responses, and receive commands.  It is also responsible for contracting and deconstructing packet header and checksum.  

* Receive

  * CEF only fulfills one command at a time.  When CEF is not in currently fulfilling a command CEF communication layer must enable receiving to be able receive next command form user debug.  Receiving will be done on interrupts in order to stay non-blocking.  Any command the user debug sends while CEF is still processing a previous command will be lost, it is up the user to only send one command at a time.
  * CEF command layer is responsible for decoding the debug port packet and informing the CEF main the command.
  * TODO – failed packet
* Transmit

  * CEF can transmit both command response and Logging information.  Command responses will take priority over logging if both are ready at the same time.  The packet for each will be the same with a different 8-bit debug packet type.
  * CEF Communication layer is responsible for packaging (debug port packet header and checksum) the packet.
  * The data must be transmitted on interrupts in order to stay non-blocking.
  * TODO – failed packet

##### CEF Main

CEF main is responsible for allocating memory for the communication layer to send and receive packets, and packet headers.  CEF system is the “owner” of this memory.  Main is responsible for handling all commands and generating the command responses

* For the first pass of this project the communication layer will copy all communication payload and header info to its own 528 bytes of memory to make development go faster.  This must be designed in a way where it will be easy to fix this resource insufficiency.


![DebugCommunication](./docsSource/DebugCommunication.png)

### Debug Packet

Debug Port Packets (528 bytes) – The debug port packet is the communication between CEF and the user debug console.  The packet consists of header info, payload, and checksum.  Only one command can be handled at a time.  Once a command is received the debug port will drop any data received by the user until it fills the request or times out (TODO- figure out time out).

##### Deubg Header Packet

Debug header packet is 12 bytes to indicate the following.

* Formatting Signature - 4 bytes
* Number of Bites in Packet - 4 bytes
* Debug type (logging, CEF command request, CEF command response) - 1 byte
* Reserve - 3 bytes

##### Debug Payload Packet

Debug payload is a maximum of 512 bytes.  Command sand responses will be “decoded” by a “shared” command file between CEF system and user debug.  This will help limit the amount of data that needs to be sent.

##### Checksum

The packet will end in a 4 byte checksum.

![DebugPortPacket](./docsSource/DebugPortPacket.png)
