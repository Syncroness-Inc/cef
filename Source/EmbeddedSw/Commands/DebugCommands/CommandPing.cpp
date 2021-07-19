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

#include "CommandPing.hpp"
#include "Logging.hpp"

/**
 * Implementation of CommandPing Methods
 * See notes in CommandPing.h for the use model of the ping command
 */

bool CommandPing::execute(void* p_childCommand)
{
    bool commandDone = false;
    bool shouldYield = false;
    (void) p_childCommand;   // avoids compiler warning of unused variable since a Ping command doesn't have a child command

    while (shouldYield == false)
    {
        switch (m_commandState)
        {
            case commandStateCommandEntry:
            {
            	// Print out an info statement as it helps indicate to console we are "connected"
                LOG_INFO(Logging::LogModuleIdCefDebugCommands, "Ping!  m_testValue = 0x{:X}, m_offsetToAddToResponse = 0x{:X}", m_testValue, m_offsetToAddToResponse);
                m_commandState = commandStateCheckRequest;
                break;
            }
            case commandStateCheckRequest:
            {
				// Compare the received value to the expected value to make sure infrastructure working properly
				if ((m_request.m_uint8Value  != CMD_PING_UINT8_REQUEST_EXPECTED_VALUE) ||
				    (m_request.m_uint16Value != CMD_PING_UINT16_REQUEST_EXPECTED_VALUE) ||
				    (m_request.m_uint32Value != CMD_PING_UINT32_REQUEST_EXPECTED_VALUE) ||
				    (m_request.m_uint64Value != CMD_PING_UINT64_REQUEST_EXPECTED_VALUE))
				{
					m_commandErrorCode = errorCode_CmdPingReceiveValuesDoNotMatchExpectedValues;
				}

				// Error or not, go ahead and generate the response fields
				m_commandState = commandStateGenerateResponse;
                break;
            }
            case commandStateGenerateResponse:
            {
            	// The response is expected to be the received value plus and offset
				m_response.m_uint8Value  = m_request.m_uint8Value  + m_request.m_offsetToAddToResponse;
				m_response.m_uint16Value = m_request.m_uint16Value + m_request.m_offsetToAddToResponse;
				m_response.m_uint32Value = m_request.m_uint32Value + m_request.m_offsetToAddToResponse;
				m_response.m_uint64Value = m_request.m_uint64Value + m_request.m_offsetToAddToResponse;

				// All done with command, exit out of here
				m_commandState = commandStateCommandComplete;
                break;
            }
            case commandStateCommandComplete:
            {
                shouldYield = true;
                commandDone = true;
                break;
            }
            default:
            {
                // If we get here, we've lost our mind.
                LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "Unhandled command state %d", m_commandState);
                shouldYield = true;
                commandDone = true;
                break;
            }
        }
    }

    return commandDone;
}


errorCode_t CommandPing::importFromCefCommand(void* p_cefCommand)
{
	// Help avoid cut/paste errors by only having one place the actual command type is defined for the import function
	typedef cefCommandPingRequest_t cefCommand_t;

	if (p_cefCommand == nullptr)
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "p_cefCommand is a nullptr");
		return errorCode_PointerIsNullptr;
	}

	cefCommand_t* p_cef = (cefCommand_t*)p_cefCommand;

	// From the CEF Command's header parameters, update Command Base parameters
	importFromCefCommandBase(&(p_cef->m_header), (uint32_t)sizeof(cefCommand_t));

	// Update all the Ping request parameters from the CEF Command request parameters
	m_request.m_testValue = p_cef->m_testValue;
	m_request.m_offsetToAddToResponse = p_cef->m_offsetToAddToResponse;
	m_request.m_uint8Value = p_cef->m_uint8Value;
	m_request.m_uint16Value = p_cef->m_uint16Value;
	m_request.m_uint32Value = p_cef->m_uint32Value;
	m_request.m_uint64Value = p_cef->m_uint64Value;

	return errorCode_OK;
}


errorCode_t CommandPing::exportToCefCommand(void* p_cefCommand)
{
	// Help avoid cut/paste errors by only having one place the actual command type is defined for the import function
	typedef cefCommandPingResponse_t cefCommand_t;

	if (p_cefCommand == nullptr)
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "exportToCefCommand called with nullptr");
		return errorCode_PointerIsNullptr;
	}

	cefCommand_t* p_cef = (cefCommand_t*)p_cefCommand;

	// From the Command Base, update the CEF Command's header parameters
	exportToCefCommandBase(&(p_cef->m_header), sizeof(cefCommand_t));

	// Update the CEF Command response parameters from the Ping response parameters
	p_cef->m_testValue   = m_response.m_testValue;
	p_cef->m_uint8Value  = m_response.m_uint8Value;
	p_cef->m_uint16Value = m_response.m_uint16Value;
	p_cef->m_uint32Value = m_response.m_uint32Value;
	p_cef->m_uint64Value = m_response.m_uint64Value;

	return errorCode_OK;
}