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

"""
Contains definitions for shared structures between Embedded SW and Python Utilities.
For now, there MUST be a C++ header file that exactly matches this file.
Eventually there may be a single source file from which the other is auto-generated (e.g. by 
python script). For now, any changes in this file must be added to the C++ file as well.  
Try to keep the python and c files in roughly the same order.
"""

import ctypes
from enum import Enum, auto


# NOTE: Choose one of the following and comment out the other depending on
# your target's architecture
#structureEndiannessType = ctypes.BigEndianStructure
structureEndiannessType = ctypes.LittleEndianStructure

#####################################################################################################################
######  ERROR CODES                                                                                            ######
#####################################################################################################################

class errorCode(Enum):
    """
    Error codes.  Each error code "should" only be used one time in order to aid debug.
    All error codes should begin with "errorCode_" and should be entered sequentially.
    By design, error codes are generic, and not module specific..
    Each error code is explicitly assigned the next available error code to aid debug (for when just the error code number is reported)
    A comment is discouraged for each error code to ease maintenance;
    instead, refer to the references in source code for the cause of the error.
    """
    errorCode_OK                                                                = 0
    errorCode_LogFatalReturn                                                    = 1
    errorCode_PointerIsNullptr                                                  = 2
    errorCode_reserved2                                                         = 3
    errorCode_reserved3                                                         = 4
    errorCode_reserved4                                                         = 5
    errorCode_reserved5                                                         = 6
    errorCode_IllegalCommandState                                               = 7
    errorCode_CmdPingReceiveValuesDoNotMatchExpectedValues                      = 8
    errorCode_CmdBaseImportCefCommandOpCodeDoesNotMatchCommand                  = 9
    errorCode_CmdBaseImportCefCommandNumBytesInCefRequestDoesNotMatch           = 10
    errorCode_debugPortErrorCodeNone                  			   				= 11
    errorCode_debugPortErrorCodeParity 	   				   						= 12
    errorCode_debugPortErrorCodeNoise						   					= 13
    errorCode_debugPortErrorCodeFrame						   					= 14
    errorCode_debugPortErrorCodeOverrun					   						= 15
    errorCode_debugPortErrorCodeUnknown					                        = 16
    errorCode_RequestedCefProxyCommandNotAllocatable                            = 17
    errorCode_BufferValidBytesExceedsBufferSize     							= 18
    errorCode_UnableToCreateLoggingSpace										= 19
    errorCode_LoggingCalledRecursively              							= 20
    errorCode_TraceFatalEncountered											    = 21
    errorCode_debugPortTransportPacketHeaderChecksumMismatch 					= 22
    errorCode_debugPortTransportPayloadChecksumMismatch 						= 23
    errorCode_debugPortTransportBufferNotBigEnoughForPayload 					= 24
	    
    errorCode_NumApplicationErrorCodes                                          = auto()


#####################################################################################################################/
######  COMMAND OPCODES                                                                                        ######/
#####################################################################################################################/

class commandOpCode(Enum):
    """
    Command OpCodes (Operation Code) supported by the Embedded Software
    OpCodes are not grouped by sub-module by design to simplify entry as well as for error checking the enum.
    Each command type must be explicitly declared to aid debug (aids lookup when debugging when only have the opCode value)
    A comment is discouraged for each command type to ease maintenance; refer to the references in source code
    for information about the command.
    """
    commandOpCodeNone               = 0
    commandOpCodePing               = 1
    commandOpCodeDebugPortRouter    = 2
    commandOpCodeCefCommandProxy    = 3

    maxCommandOpCodeNumber          = auto()
    commandOpCodeInvalid            = 0xFFFF


#####################################################################################################################
######  CEF COMMAND REQUEST AND RESPONSE STRUCTURES                                                            ######
#####################################################################################################################

"""
When sharing data between Python Utilities and CEF Embedded Software, the byte packing must be the same
between the two compilers.
See https://en.wikipedia.org/wiki/Data_structure_alignment for info about "packing structures" and the need for "padding"
when manually packing structures.
In short, each element of the structure must be aligned to the primitive data type of the processor. Uint_8 should "8 bit aligned",
uint16_t should be "16 bit aligned", uint32_t should be "32 bit aligned", and  uint64_t should be "64 bit aligned.
Hence, it is not enough to just use the pack pragma; we must also manually pad out each structure depending on
the alignment requirements of the variables.  As such, it is usually easier to group similar sized variables next to each other.
Each variable should specify whether it is 8bit, 16 bit, 32 bit, or 64 bit aligned relative to start of the structure
AFTER that variable has been used.  For example, if the first variable is a uint32_t, then that variable should be
commented with "32 bit aligned".  If the next variable is also a uint32_t, then that variable should be commented with 64 bit aligned.

See the importFromCefCommand() and exportToCefCommand() functions for each command for definition of variables.
    This is done rather than create a maintenance issue of essentially duplicating the documentation in two spots.
"""

"""
Debug Port Framing Signature
  * This is the 32 bit framing signature to send debug packets to/from CEF and Python Utilities
  * Every Byte MUST be unique
  * The framing signature is specified as a byte array to make the signature endianess agnostic
"""
debugPacketFramingSignature = [0x43, 0x45, 0x46, 0x53];
numElementsInDebugPacketFramingSignature = len(debugPacketFramingSignature);


class debugPacketDataType(Enum):
    """
    Debug Port Packet Data Type - the debug port expect the following types of packets
    - command request
    - command response
    - logging data
    """
    debugPacketType_commandRequest                          = 0
    debugPacketType_commandResponse                         = 1
    debugPacketType_loggingData                             = 2

    debugPacketType_invalid                                 = 0xff


class cefCommandHeader(structureEndiannessType):
    """
    CEF Command Header
    Each Request and Receive command has a common header associated with it.
    The CEF Command Header must be an increment of 8 bytes so that when the CEF command header
    is used within a structure, the next variable in the structure can rely upon being 64 bit aligned.

    Having both the request and the response structures have a common header, then the implementation of the command packets
    can optionally share the same memory buffer for receiving/sending commands to the debug port in memory constrained systems.
    """
    _fields_ = [
        ('m_commandOpCode', ctypes.c_uint16),

        # Rolling sequence number generated by Python to help ensure that the command response is indeed
        # the correct response to the command request.  It is the responsibility of the EmbeddedSw to
        # populate the response command's header m_commandRequestResponseSequenceNumberPython variable
        # with the same value that was in the request command' header field.
        # Python should validate that the response packet's m_commandRequestResponseSequenceNumberPython
        # matches the number generated for the original request command.

        ('m_commandSequenceNumber', ctypes.c_uint16),
        ('m_commandErrorCode', ctypes.c_uint32),
        ('m_commandNumBytes', ctypes.c_uint32),
        ('m_padding1', ctypes.c_uint32)
    ]


class cefCommandDebugPortHeader(structureEndiannessType):
    """
    CEF Command Debug Port Header
    Each Command Request, Command Response, and Logging Packet has a common debug header associated with it.
    The CEF Command Debug Header must guarantee to end on a 64 bit alignment as other structures that follow
    this header rely on it ending on a 64 bit alignment.
    """
    _fields_ = [
        ('m_framingSignature', ctypes.c_uint8 * numElementsInDebugPacketFramingSignature),
        ('m_packetPayloadChecksum', ctypes.c_uint32),
        ('m_payloadSize', ctypes.c_uint32),

        # The types of packets are defined in class debugPacketDataType
        ('m_packetType', ctypes.c_uint8),
        ('m_reserve', ctypes.c_uint8),
        ('m_packetHeaderChecksum', ctypes.c_uint16)
    ]
  
  

""" 
CommandPing constants
"""
CMD_PING_UINT8_REQUEST_EXPECTED_VALUE   = 0xA3
CMD_PING_UINT16_REQUEST_EXPECTED_VALUE  = 0x93A3
CMD_PING_UINT32_REQUEST_EXPECTED_VALUE  = 0x208461A3
CMD_PING_UINT64_REQUEST_EXPECTED_VALUE  = 0x936217995202A373


class cefCommandPingRequest(structureEndiannessType):
    """
    CommandPing
        See command implementation files for variable documentation

    The ping command is also used for basic command protocol checkout, so known values
    outlined below are shared between python and embedded software.
    """
    _fields_ = [
        ('m_header', cefCommandHeader),

        ('m_uint8Value', ctypes.c_uint8),
        ('m_padding1', ctypes.c_uint8),
        ('m_uint16Value', ctypes.c_uint16),
        ('m_testValue', ctypes.c_uint32),
        ('m_uint32Value', ctypes.c_uint32),
        ('m_padding2', ctypes.c_uint32),
        ('m_offsetToAddToResponse', ctypes.c_uint64),
        ('m_uint64Value', ctypes.c_uint64)
    ]


class cefCommandPingResponse(structureEndiannessType):
    """
    CommandPing
        See command implementation files for variable documentation

    The ping command is also used for basic command protocol checkout, so known values
    outlined below are shared between python and embedded software.
    """
    _fields_ = [
        ('m_header', cefCommandHeader),

        ('m_uint8Value', ctypes.c_uint8),
        ('m_padding1', ctypes.c_uint8),
        ('m_uint16Value', ctypes.c_uint16),
        ('m_testValue', ctypes.c_uint32),
        ('m_uint32Value', ctypes.c_uint32),
        ('m_padding2', ctypes.c_uint32),
        ('m_uint64Value', ctypes.c_uint64)
    ]
    

#####################################################################################################################
######  LOGGING                                                                                                ######
#####################################################################################################################

###### Logging types
class logType(Enum):
    """
    Logging levels (note, 'FATAL' is mapped to 'CRITICAL' in python)
    """
    logTypeDebug                            = 0
    logTypeInfo                             = 1
    logTypeWarning                          = 2
    logTypeError                            = 3
    logTypeFatal                            = 4

######  Logging Constants
#Converts logging uint64_t nano second value/count into seconds
LOGGING_UINT64_NSEC_TO_SECONDS = 1000000000

LOGGING_ASCII_LOG_STRING_MAX_NUM_CHARACTERS = 128
LOGGING_ASCII_FILENAME_NUM_CHARACTERS = 40

class cefLog(structureEndiannessType):
    """
    Logging Structure (passes a string (rather than a string hash) in log packet
    """
    _fields_ = [
        ('m_header', cefCommandHeader),    
    
        ('m_logVariable1', ctypes.c_uint64),
        ('m_logVariable2', ctypes.c_uint64),
        ('m_logVariable3', ctypes.c_uint64),
        ('m_timeStamp', ctypes.c_uint64),
        ('m_logString', ctypes.c_char * LOGGING_ASCII_LOG_STRING_MAX_NUM_CHARACTERS),
        ('m_fileName', ctypes.c_char * LOGGING_ASCII_FILENAME_NUM_CHARACTERS),
        ('m_fileLineNumber', ctypes.c_uint32),
        ('m_logSequenceNumber', ctypes.c_uint16),
        ('m_moduleId', ctypes.c_uint8),
        ('m_logType', ctypes.c_uint8),    
    ]
 
 
 
#####################################################################################################################
######  DEBUG PORT CONSTANTS THAT RELY ON PREVIOUSLY DEFINED CLASSES                                           ######
#####################################################################################################################
 
 
"""
Maximum number of bytes in the application layer payload.
In other words, the total number of bytes the application layer would request the
transport layer to received/send.
This number does NOT include Transport layer headers.
"""
DEBUG_PORT_MAX_APPLICATION_PAYLOAD_COMMAND = ctypes.sizeof(cefCommandHeader) + 512
DEBUG_PORT_MAX_APPLICATION_PAYLOAD_LOG = ctypes.sizeof(cefLog)
DEBUG_PORT_MAX_APPLICATION_PAYLOAD = DEBUG_PORT_MAX_APPLICATION_PAYLOAD_COMMAND \
		if DEBUG_PORT_MAX_APPLICATION_PAYLOAD_COMMAND > DEBUG_PORT_MAX_APPLICATION_PAYLOAD_LOG \
		else DEBUG_PORT_MAX_APPLICATION_PAYLOAD_LOG

"""
Debug Port Packet Size
  * Max number of bytes in a debug port packet
"""
DEBUG_PORT_MAX_PACKET_SIZE_BYTES = ctypes.sizeof(cefCommandDebugPortHeader) + DEBUG_PORT_MAX_APPLICATION_PAYLOAD
 
