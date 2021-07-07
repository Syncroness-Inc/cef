# Debug Port

### Debug Communication

Below is an overview of debug communication between Python Test Utilities and CEF.  Communication will consist of commands, command responses, and logging data.  Only one command can be processed at a time.  Both the CEF and Python Test Utilities will have a common command document library to build and interpret commands/responses.  Communication will only be sent once, if it is not received the sender will not be aware of this and there will be no retries.  This decision was made to speed up development and can be implemented later if projects need this feature

##### Python Test Utilities - User Debug

The python test utilities provide the user debug console.  In this console the user can send command requests, receive commands, and logging information.  CEF can only accept one command at a time.  Any command the user sends while CEF is processing prior command will be “lost”.

##### CEF/Communication Layer

The communication layers job is to handle all ingoing and outgoing communication with Python Test Utilities.  It will send both logging data and command responses, and receive commands.  It is also responsible for contracting and deconstructing packet header and checksum.

* Receive

  * CEF only fulfills one command at a time.  When CEF is not in currently fulfilling a command CEF communication layer must enable receiving to be able receive next command form user debug.  Receiving will be done on interrupts in order to stay non-blocking.
  * CEF command layer is responsible for decoding the debug port packet and informing the CEF main the command.
* Transmit

  * CEF can transmit both command response and Logging information.  Command responses will take priority over logging if both are ready at the same time.  The packet for each will be the same with a different 8-bit debug packet type.
  * CEF Communication layer is responsible for packaging debug port packet header, data packet, and checksum.
  * The data will be transmitted on interrupts in order to stay non-blocking.

##### CEF Main

CEF main is responsible for allocating memory for the communication layer to send and receive packets, and packet headers.  CEF system is the “owner” of this memory.  Main is responsible for handling all commands and generating the command responses

* For the first pass of this project the communication layer will copy all communication payload and header info to its own 528 bytes of memory to make development go faster.  This must be designed in a way where it will be easy to fix this resource insufficiency.

![DebugCommunication](./docsSource/DebugCommunication.png)

### Debug Packet

Debug Port Packets (528 bytes) – The debug port packet is the communication between CEF and the user debug console.  The packet consists of header info, payload, and checksum.  Only one command can be handled at a time.  Once a command is received the debug port will drop any data received by the user until it fills the request or times out.

##### Deubg Header Packet

Debug header packet is 12 bytes to indicate the following.

* Formatting Signature - 4 bytes
* Number of Bytes in Packet - 4 bytes
* Debug type (logging, CEF command request, CEF command response) - 1 byte
* Reserve - 3 bytes

##### Debug Payload Packet

Debug payload is a maximum of 512 bytes.  Command send responses will be “decoded” by a “shared” command file between CEF system and user debug.  This will help limit the amount of data that needs to be sent.

##### Checksum

The packet will end in a 4 byte checksum.

![DebugPortPacket](./docsSource/DebugPortPacket.png)
