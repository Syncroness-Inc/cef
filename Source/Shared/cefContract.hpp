/* ******************************************************************
\copyright COPYRIGHT AND PROPRIETARY RIGHTS NOTICES:

Copyright (C) 2021, an unpublished work by Syncroness, Inc.
All rights reserved.

This material contains the valuable properties and trade secrets of
Syncroness of Westminster, CO, United States of America
embodying substantial creative efforts and confidential information,
ideas and expressions, no part of which may be reproduced or
transmitted in any form or by any means, electronic, mechanical, or
otherwise, including photocopying and recording or in connection
with any information storage or retrieval system, without the prior
written permission of Syncroness.
****************************************************************** */

/* Header guard */
#ifndef __CEF_CONTRACT_H
#define __CEF_CONTRACT_H

/* Start of c/c++ guard */
#ifdef __cplusplus
extern "C" {
#endif


/**
 * Contains definitions for shared structures between Embedded SW and Python Utilities.
 * 	For now, there MUST be a python file that exactly matches this file.
 * 	Eventually there may be a single source file from which the other is auto-generated (e.g. by 
 *  python script). For now, any changes in this file must be
 * 	added to the python file as well.  Try to keep the python and c files in roughly the same order.
 */


#include "cefMappings.hpp"



/*********************************************************************************************************************/
/******  ERROR CODES                                                                                            ******/
/*********************************************************************************************************************/
/**
 *  Error codes.  Each error code "should" only be used one time in order to aid debug.
 *  All error codes should begin with "errorCode_" and should be entered sequentially.
 *  By design, error codes are generic, and not module specific..
 *  Each error code is explicitly assigned the next available error code to aid debug (for when just the error code number is reported)
 *  A comment is discouraged for each error code to ease maintenance;
 *    instead, refer to the references in source code for the cause of the error.
 */
enum
{
	errorCode_OK 							            			        = 0, 	// No Error, Pass
	errorCode_LogFatalReturn	        									= 1,	// Should only be used in return statements immediately after LOG_FATAL.
																					//    Currently, a TRACE_FATAL statement never returns, but this is used
																					// 	  in the event some type of recovery is put in place in the future.
	errorCode_PointerIsNullptr										        = 2,	// Only should be used after LOG_FATAL nullptr check
	errorCode_reserved2												        = 3,	// to be used later for commonly used return values
	errorCode_reserved3												        = 4,
	errorCode_reserved4												        = 5,
	errorCode_reserved5												        = 6,
	errorCode_IllegalCommandState 			            			        = 7,
	errorCode_CmdPingReceiveValuesDoNotMatchExpectedValues			        = 8,
	errorCode_CmdBaseImportCefCommandOpCodeDoesNotMatchCommand		        = 9,
	errorCode_CmdBaseImportCefCommandNumBytesInCefRequestDoesNotMatch 		= 11,


	errorCode_NumApplicationErrorCodes, // Must be last entry for error checking
};

// Application error code type must fit in 16 bits
STATIC_ASSERT((errorCode_NumApplicationErrorCodes < UINT16_MAX), error_codes_must_fit_in_16_bits);
typedef uint16_t errorCode_t;



/*********************************************************************************************************************/
/******  COMMAND OPCODES                                                                                        ******/
/*********************************************************************************************************************/
/**
 *  Command OpCodes (Operation Code) supported by the Embedded Software
 *  OpCodes are not grouped by sub-module by design to simplify entry as well as for error checking the enum.
 *  Each command type must be explicitly declared to aid debug (aids lookup when debugging when only have the opCode value)
 *  A comment is discouraged for each command type to ease maintenance; refer to the references in source code
 *  	for information about the command.
 */
enum
{
    commandOpCodeNone                              		= 0,	// Illegal command type, reserved
	commandOpCodePing 	   						   		= 1,

    maxCommandOpCodeNumber, // Must be last, except for 'invalid'
    commandOpCodeInvalid                               = 0xFFFF,
};

// Command type must fit in 16 bits
STATIC_ASSERT((maxCommandOpCodeNumber < UINT16_MAX), command_type_must_fit_in_16_bits);
typedef uint16_t commandOpCode_t;



/*********************************************************************************************************************/
/******  CEF COMMAND REQUEST AND RESPONSE STRUCTURES                                                            ******/
/*********************************************************************************************************************/

/**
 * When sharing data between Python Utilities and CEF Embedded Software, the byte packing must be the same
 * between the two compilers.
 * See https://en.wikipedia.org/wiki/Data_structure_alignment for info about "packing structures" and the need for "padding"
 * when manually packing structures.
 * In short, each element of the structure must be aligned to the primitive data type of the processor. Uint_8 should "8 bit aligned",
 * uint16_t should be "16 bit aligned", uint32_t should be "32 bit aligned", and  uint64_t should be "64 bit aligned.
 * Hence, it is not enough to just use the pack pragma; we must also manually pad out each structure depending on
 * the alignment requirements of the variables.  As such, it is usually easier to group similar sized variables next to each other.
 * Each variable should specify whether it is 8bit, 16 bit, 32 bit, or 64 bit aligned relative to start of the structure
 * AFTER that variable has been used.  For example, if the first variable is a uint32_t, then that variable should be
 * commented with "32 bit aligned".  If the next variable is also a uint32_t, then that variable should be commented with 64 bit aligned.
 *
 * See the importFromCefCommand() and exportToCefCommand() functions for each command for definition of variables.
 * 		This is done rather than create a maintenance issue of essentially duplicating the documentation in two spots.
 */


/**
 * This must be the first line in the CEF COMMAND REQUEST AND RESPONSE STRUCTURES section in order to
 * set packing to byte alignment (i.e. no compiler specified "padding")
 */
#pragma pack(push, 1)

/**
 * CEF Command Header
 * Each Request and Receive command has a common header associated with it.
 * The CEF Command Header must be an increment of 8 bytes so that when the CEF command header
 * is used within a structure, the next variable in the structure can rely upon being 64 bit aligned.
 *
 * Having both the request and the response structures have a common header, then the implementation of the command packets
 * can optionally share the same memory buffer for receiving/sending commands to the debug port in memory constrained systems.
 */
typedef struct
{
	uint16_t m_commandOpCode;					// 16 bit aligned

	/**
	 * Rolling sequence number generated by Python to help ensure that the command response is indeed
	 * the correct response to the command request.  It is the responsibility of the EmbeddedSw to
	 * populate the response command's header m_commandRequestResponseSequenceNumberPython variable
	 * with the same value that was in the request command' header field.
	 * Python should validate that the response packet's m_commandRequestResponseSequenceNumberPython
	 * matches the number generated for the original request command.
	 */
	uint16_t m_commandRequestResponseSequenceNumberPython;	// 32 bit aligned

	uint32_t m_commandErrorCode;				// 64 bit aligned
	uint32_t m_commandNumBytes;					// Used to confirm python/embedded SW in sync; 32 bit aligned
	uint32_t m_padding1;						// 64 bit aligned
} cefCommandHeader_t;

/**
 * CEF Command Debug Port Header
 * Each Command Request, Command Response, and Logging Packet has a common debug header associated with it.
 * The CEF Command Debug Header must guarantee to end on a 64 bit alignment as other structures that follow
 * this header rely on it ending on a 64 bit alignment.
 */
typedef struct
{
	uint32_t 	m_framingSignature;			//32 bit aligned
	uint32_t 	m_packetPayloadChecksum;	//Checksum over the payload only, 64 bit aligned
	uint32_t 	m_payloadSize;				//Payload size in bytes, 32 bit aligned
	/**
	 * The types of packets are
	 * * Command Request
	 * * Command Response
	 * * Logging Packet
	 */
	uint8_t 	m_packetType;				//40 bit aligned
	uint8_t 	m_reserve;					//48 bit aligned
	uint16_t 	m_packetHeaderChecksum;		//checksum over the header only, 64 bit aligned
} cefCommandDebugPortHeader_t;

/**
 * CommandPing
 *		See command implementation files for variable documentation
 *
 * The ping command is also used for basic command protocol checkout, so known values
 * outlined below are shared between python and embedded software.
 */
#define CMD_PING_UINT8_REQUEST_EXPECTED_VALUE  0xA3
#define CMD_PING_UINT16_REQUEST_EXPECTED_VALUE 0x93A3
#define CMD_PING_UINT32_REQUEST_EXPECTED_VALUE 0x208461A3
#define CMD_PING_UINT64_REQUEST_EXPECTED_VALUE 0x936217995202A373

typedef struct
{
	cefCommandHeader_t m_header;				// Must be 1st entry in structure, guaranteed to be 64 bit aligned

	uint8_t		m_uint8Value;					// 8  bit aligned
	uint8_t		m_padding1;						// 16 bit aligned
	uint16_t	m_uint16Value;					// 32 bit aligned
	uint32_t 	m_testValue;					// 64 bit aligned
	uint32_t	m_uint32Value;					// 32 bit aligned
	uint32_t	m_padding2;						// 64 bit aligned
	uint64_t 	m_offsetToAddToResponse;		// 64 bit aligned
	uint64_t	m_uint64Value;					// 64 bit aligned
} cefCommandPingRequest_t;

typedef struct
{
	cefCommandHeader_t m_header;				// Must be 1st entry in structure, 64 bit aligned

	uint8_t		m_uint8Value;					// 8  bit aligned
	uint8_t		m_padding1;						// 16 bit aligned
	uint16_t	m_uint16Value;					// 32 bit aligned
	uint32_t 	m_testValue;					// 64 bit aligned
	uint32_t	m_uint32Value;					// 32 bit aligned
	uint32_t	m_padding2;						// 64 bit aligned
	uint64_t	m_uint64Value;					// 64 bit aligned
} cefCommandPingResponse_t;




/**
 * This must be the last line in the CEF COMMAND REQUEST AND RESPONSE STRUCTURES section in order to
 * restore packing to the previous value
 */
#pragma pack(pop)


/* End of c/c++ guard */
#ifdef __cplusplus
}
#endif

#endif  // end header guard
