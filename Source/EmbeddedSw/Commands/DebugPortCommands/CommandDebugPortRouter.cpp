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

#include <new>      // for placement new

#include "CommandDebugPortRouter.hpp"
#include "Logging.hpp"


/**
 * Implementation of CommandDebugPortRouterRouter Methods
 *
 * Logging:
 * 		When logging has a new log, it requests memory via checkoutLogBufferLogging()
 * 			Logging then fills in cefLog_t with logging information
 * 		Then the log is "returned" via checkinLogBufferLogging(), and added to a queue to transmit
 * 		checkoutLogTransmitBuffer() returns a pointer of the next log to transmit
 * 		Once the transmit has been completed, the buffer is returned via checkinLogTransmitBuffer()
 *
 * CEF Proxy Command
 * 		There is one buffer that is used for CEF Proxy Commands.
 * 		The buffer transitions between various states as it progresses through CEF Command processing
 * 		The buffer is "checked out" by an API, which transfers ownership (i.e. rights to modify) to who checked out
 * 		    the buffer.
 * 		The command only exposes the part of the buffer that the checkout API is allowed to modify
 *		The command is responsible to allocating/managing all memory for the DebugPort (in current implementation)
 *		   	as this simplifies the current implementation.
 */

/**
 * Maximum number of logging cefLogPacketAscii_t that can exist in the system at one time.
 * Caution:  Logging packets can be memory hogs, to be careful how big this number is
 */
static const uint32_t maxNumLoggingPackets = 20;


//! Singleton instantiation of CommandDebugPortRouter
static CommandDebugPortRouter commandDebugPortRouterSingleton(BufferPoolBase::BufferPoolId_Logging,
	      	  	  	  	  	  	  	  	  	  	  sizeof(cefLog_t),
												  maxNumLoggingPackets);




CommandDebugPortRouter::CommandDebugPortRouter(uint32_t logBufferPoolId,
								   uint32_t numBytesPerLogEntry,
								   uint32_t maxNumLogEntries) :
					CommandBase(commandOpCodeDebugPortRouter),
					m_logPool(logBufferPoolId, numBytesPerLogEntry, maxNumLogEntries),
					m_logsToSend(maxNumLogEntries),
					m_cefCommandBuffer(&m_cefCommand, sizeof(m_cefCommand)),
					m_cefLogBufferTransmit(nullptr, 0),
					mp_cefBufferTransmit(nullptr),
				    m_cefCommandBufferState(cefCommandBufferState_bufferAvailable)
					{ }


CommandDebugPortRouter& CommandDebugPortRouter::instance()
{
	return commandDebugPortRouterSingleton;
}


bool CommandDebugPortRouter::execute(CommandBase* p_childCommand)
{
    bool commandDone = false;
    bool shouldYield = false;

    validateNullChildResponse(p_childCommand);

    while (shouldYield == false)
    {
        switch (m_commandState)
        {
            case commandStateCommandEntry:
            {
                m_commandState = commandStateExecuteTransportFunctions;
                break;
            }
            case commandStateExecuteTransportFunctions:
            {

            	// Finished with transport functions; exit, and and retry transport functions again next time
                shouldYield = true;
                break;
            }
            case commandStateCommandComplete:		// DebugPort should run forever
            default:
            {
                // If we get here, we've lost our mind.
                LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Unhandled command state {:d}", m_commandState);
                shouldYield = true;
                commandDone = true;
                break;
            }
        }
    }

    return commandDone;
}


cefLog_t* CommandDebugPortRouter::checkoutLogBufferLogging()
{
    cefLog_t* p_cefLog = (cefLog_t*)m_logPool.allocate(sizeof(cefLog_t));

	if (p_cefLog == nullptr)
	{
		// No log buffers available; they must all be backed up in the transmit queue
		// Eventually we want to thrown away some data and make room for an overflow message
		return nullptr;
	}

	return p_cefLog;
}


void CommandDebugPortRouter::checkinLogBufferLogging(cefLog_t* p_cefLog)
{
	if (p_cefLog == nullptr)
	{
		LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "A nullptr log buffer was returned!");
	}

	// The p_cefLog is assumed to have valid logging data, and now is ready to be transmitted, so add
	// it to the log to send fifo.
	if (m_logsToSend.put(p_cefLog) == false)
	{
		// Something is messed up in the system setup as there should be room to send all logs we have buffer space for
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "m_logsToSend not setup correctly in CommandDebugPortRouter");
	}

}

cefLog_t* CommandDebugPortRouter::checkoutLogTransmitBuffer()
{
	void* p_cefLog = nullptr;

	if (m_logsToSend.get(p_cefLog) == false)
	{
		// No logs to send
		return nullptr;
	}

	return (cefLog_t*)p_cefLog;
}


void CommandDebugPortRouter::checkinLogTransmitBuffer(cefLog_t* p_cefLog)
{
	// If memory is attempted to be returned to a pool that it was not allocated from, then free() with trace fatal.
	m_logPool.free(p_cefLog);
}

CefBuffer* CommandDebugPortRouter::checkoutCefCommandReceiveBuffer()
{
	if (m_cefCommandBufferState != cefCommandBufferState_bufferAvailable)
	{
		// If the buffer is not available...
		return nullptr;
	}

	// Mark the packet as checked out
	m_cefCommandBufferState = cefCommandBufferState_receivingCommand;
	return &m_cefCommandBuffer;
}


void CommandDebugPortRouter::checkinCefCommandReceiveBuffer(CefBuffer* p_cefBuffer)
{
	// Sanity Check:  Confirm the memory was checked out appropriately in the first place
	if (m_cefCommandBufferState != cefCommandBufferState_receivingCommand)
	{
		// Attempting to return a packet that was not checked out!
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Attempt to return memory to CommandDebugPortRouter in wrong state{:d}, Allowed {:d}", m_cefCommandBufferState, cefCommandBufferState_receivingCommand);
	}

	// Sanity Check:  Confirm that returning the right memory
	if (p_cefBuffer != &m_cefCommandBuffer)
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Attempt to return unknown memory to CommandDebugPortRouter 0x{:x}", p_cefBuffer);
	}

	// Sanity Check that we didn't overflow the buffer
	if (m_cefCommandBuffer.getNumberOfValidBytes() > m_cefCommandBuffer.getMaxBufferSizeInBytes())
	{
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Buffer overflow! valid={%d}, max={%d}", m_cefCommandBuffer.getNumberOfValidBytes(), m_cefCommandBuffer.getMaxBufferSizeInBytes());
	}

	// This is a valid return, so change the state of the CEF Command Buffer
	// By design, the buffer should only be returned with a CEF Receive Command
	m_cefCommandBufferState = cefCommandBufferState_commandReceived;
}


CefBuffer* CommandDebugPortRouter::checkoutCefCommandProxyProcessingBuffer()
{
	if (m_cefCommandBufferState != cefCommandBufferState_commandReceived)
	{
		return nullptr;
	}

	// Mark that the cefCommand Buffer (does not include packet header) is checked out to proxy command
	m_cefCommandBufferState = cefCommandBufferState_proxyCommandOwnsBuffer;
	return &m_cefCommandBuffer;
}


void CommandDebugPortRouter::checkinCefCommandProxyProcessingBuffer(CefBuffer* p_cefBuffer)
{
	// Sanity Check State
	if (m_cefCommandBufferState != cefCommandBufferState_proxyCommandOwnsBuffer)
	{
		// Attempting to return a packet that was not checked out!
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Attempt to return memory to CommandDebugPortRouter in wrong state{:d}, Allowed {:d}", m_cefCommandBufferState, cefCommandBufferState_proxyCommandOwnsBuffer);
	}

	// Sanity Check Address
	if (p_cefBuffer != &m_cefCommandBuffer)
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Attempt to return unknown memory 0x{:x} via returnCefCommandBufferForProxyProcessing, expected 0x{:x}", p_cefBuffer, &m_cefCommandBuffer);
	}

	// Sanity Check numBytes makes sense
	if (m_cefCommandBuffer.getNumberOfValidBytes() > m_cefCommandBuffer.getMaxBufferSizeInBytes())
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Illegal payload size={:d}, max allowed={:d}", m_cefCommandBuffer.getNumberOfValidBytes(), m_cefCommandBuffer.getMaximumBufferSizeInBytes());
	}

	m_cefCommandBufferState = cefCommandBufferState_readyToTransmit;
}


CefBuffer* CommandDebugPortRouter::checkoutCefCommandTransmitBuffer()
{
	if (m_cefCommandBufferState != cefCommandBufferState_readyToTransmit)
	{
		return nullptr;
	}

	// Note:  The number of bytes to transmit is contained in m_cefCommandBuffer.getNumberOfValidBytes()

	m_cefCommandBufferState = cefCommandBufferState_transmittingBuffer;

	return &m_cefCommandBuffer;
}


void CommandDebugPortRouter::checkinCefCommandTransmitBuffer(CefBuffer* p_cefBuffer)
{
	// Sanity Check State
	if (m_cefCommandBufferState != cefCommandBufferState_transmittingBuffer)
	{
		// Attempting to return a packet that was not checked out!
		LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "Attempt to return memory to CommandDebugPortRouter in wrong state{:d}, Allowed {:d}", m_cefCommandBufferState, cefCommandBufferState_transmittingBuffer);
	}

	// Sanity Check Address
	if (p_cefBuffer != &m_cefCommandBuffer)
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Attempt to return unknown memory to Cef Transmit Command Packet 0x{:x}, expected 0x{:x}, ", p_cefCommandResponse, &m_cefCommandPacket);
	}

	// All done with the buffer; mark buffer as being available
	m_cefCommandBufferState = cefCommandBufferState_bufferAvailable;

	// Reset the valid bytes to aid debug as the next step is to receive another command
	m_cefCommandBuffer.setNumberOfValidBytes(0);
}


CefBuffer* CommandDebugPortRouter::checkoutCefTransmitBuffer(debugPacketDataType_t& debugDataType)
{
    // Is there already a command checked out?
    if (mp_cefBufferTransmit != nullptr)
    {
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Attempt to checkout CEF Buffer to transmit when one is already checked out!");
    }

    // Initialize the return values
    debugDataType = debugPacketType_invalid;
    mp_cefBufferTransmit = nullptr;   //!<  Yes, the above nullptr check confirms this, but for defensive coding setting up anyhow.

    // Is there a Command Response waiting to be sent?
    if (m_cefCommandBufferState == cefCommandBufferState_readyToTransmit)
    {
        mp_cefBufferTransmit = checkoutCefCommandTransmitBuffer();
        if (mp_cefBufferTransmit == nullptr)
        {
            LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Programming Error:  Unexpected nullptr for CefCommand Response!");
        }
        debugDataType = debugPacketType_commandResponse;
    }
    // Is there a log waiting to be sent?
    else if (m_logsToSend.isEmpty() == false)
    {
        cefLog_t* p_cefLog = checkoutLogTransmitBuffer();
        if (p_cefLog == nullptr)
        {
            LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Programming Error:  Unexpected nullptr for CefCommand Response!");
        }

        // Need to convert to CefBuffer
        mp_cefBufferTransmit = (CefBuffer*) new ((void*)&m_cefLogBufferTransmit) CefBuffer((void*)p_cefLog, sizeof(cefLog_t));
        if (mp_cefBufferTransmit == nullptr)
        {
            LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Programming Error:  Unexpected nullptr for Log Transmit!");
        }

        // Setup the number of valid bytes in the log buffer
        mp_cefBufferTransmit->setNumberOfValidBytes(sizeof(cefLog_t));

        debugDataType = debugPacketType_loggingData;
    }

    return mp_cefBufferTransmit;
}


void CommandDebugPortRouter::checkinCefTransmitBuffer(CefBuffer* p_cefBuffer)
{
    if (p_cefBuffer == nullptr)
    {
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Programming Error:  Request to return nullptr for Log Transmit Buffer!");
    }

    // Is this the Cef Command Buffer?
    if (p_cefBuffer == &m_cefCommandBuffer)
    {
        checkinCefCommandTransmitBuffer(p_cefBuffer);
    }
    // Then this must be a log buffer
    else
    {
        cefLog_t* p_cefLog = (cefLog_t*)p_cefBuffer->getBufferStartAddress();
        checkinLogTransmitBuffer(p_cefLog);
    }
}
