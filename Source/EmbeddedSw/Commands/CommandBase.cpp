/*******************************************************************
@copyright COPYRIGHT AND PROPRIETARY RIGHTS NOTICES:

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


/**
 * Implementation of CommandBase functions
 * Note:  Pure virtual functions can cause the compiler/linker to pull in
 * an excess amount of code.  So, pure virtual functions should be implemented to
 * fail at run time instead (not ideal, but the least bad option).
 */

#include "CommandBase.hpp"
#include "Logging.hpp"

// Initialize the rolling sequence number that will make each object instantiation have a quasi unique id
CommandBase::commandSequenceNumber_t CommandBase::m_rollingCommandSequenceNumber = 0;


bool CommandBase::execute(CommandBase* p_childCommand)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class execute() called, supposed to be implemented in derived class",
	        0, 0, 0);
	return true;
}

errorCode_t CommandBase::importFromCefCommand(void* p_cefCommand, uint32_t actualNumBytesReceived)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class importFromCefCommand() called, supposed to be implemented in derived class",
	        0, 0, 0);
	return errorCode_LogFatalReturn;
}

errorCode_t CommandBase::importFromCefCommandBase(cefCommandHeader_t* p_cefCommandHeader,
                                                  uint32_t numBytesInCefRequestCommand,
                                                  uint32_t actualNumBytesReceived)
{
	if (p_cefCommandHeader == nullptr)
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class importFromCefCommand called with nullptr",
		        0, 0, 0);
		return errorCode_PointerIsNullptr;
	}

	// Sanity Check that have the right opcode
	if (p_cefCommandHeader->m_commandOpCode != m_commandOpCode)
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class importFromCefCommand cefOpCode(0x{:X}) != expected OpCode (0x{:X})",
		        p_cefCommandHeader->m_commandOpCode, m_commandOpCode, 0);
		return errorCode_CmdBaseImportCefCommandOpCodeDoesNotMatchCommand;
	}

	// Sanity Check that python utilities and embedded software are in synch
	if (p_cefCommandHeader->m_commandNumBytes != numBytesInCefRequestCommand)
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure,
		        "Base class importFromCefCommand m_commandNumBytes({:d}) != expected numBytesInCefRequestCommand ({:d})",
		        p_cefCommandHeader->m_commandNumBytes, numBytesInCefRequestCommand, 0);
		return errorCode_CmdBaseImportCefCommandNumBytesInCefRequestDoesNotMatch;
	}

	// How many bytes did we actually receive for this command?
	if (actualNumBytesReceived != numBytesInCefRequestCommand)
	{
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure,
                "Base class importFromCefCommand actualNumBytesReceived({:d}) != expected numBytesInCefRequestCommand ({%d})",
                actualNumBytesReceived, numBytesInCefRequestCommand, 0);
	}

	return errorCode_OK;
}

errorCode_t CommandBase::exportToCefCommand(void* p_cefCommand)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class exportToCefCommand() called, supposed to be implemented in derived class",
	        0, 0, 0);
	return errorCode_LogFatalReturn;
}

errorCode_t CommandBase::exportToCefCommandBase(cefCommandHeader_t* p_cefCommandHeader, uint32_t numBytesInCefResponseCommand)
{
	if (p_cefCommandHeader == nullptr)
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class importFromCefCommand called with nullptr",
		        0, 0, 0);
		return errorCode_PointerIsNullptr;
	}

	p_cefCommandHeader->m_commandOpCode = m_commandOpCode;

	// Add number of bytes in CefResponseCommand to Python Utilities can do a sanity check
	p_cefCommandHeader->m_commandNumBytes = numBytesInCefResponseCommand;

	p_cefCommandHeader->m_commandErrorCode = m_commandErrorCode;

	return errorCode_OK;
}

void CommandBase::validateNullChildResponse(CommandBase* p_childCommand)
{
	// See comments in function header in .hpp file

	// @todo  Once determine how handle emptying logging queue when log_fatal issued, then add additional
	// debug print here (i.e. printInfo() for both this object and the child object) to aid debug...

    if (p_childCommand != nullptr)
    {
    	LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "Unexpected child response received (0x{:x}, 0x(:x})",
    	        (uint64_t)p_childCommand, 0, 0);
    }
}


