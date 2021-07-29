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

#include "CommandDebugPortRouter.hpp"
#include "Logging.hpp"


/**
 * Implementation of CommandDebugPortRouterRouter Methods
 *
 * Logging:
 * 		When logging has a new log, it requests memory via checkoutLogBuffer()
 * 			Logging then fills in cefLogging_t with logging information
 * 		Then the log is "returned" via returnLogBuffer(), and added to a queue to transmit
 * 		checkoutLogPacket() returns a pointer to a cefLoggingPacket_t if there is a log to transmit
 * 		Once the transmit has been completed, the log is returned via returnLogPacket()
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
 * Maximum number of logging cefLoggingPacketAscii_t that can exist in the system at one time.
 * Caution:  Logging packets can be memory hogs, to be careful how big this number is
 */
static const uint32_t maxNumLoggingPackets = 2;
//static const uint32_t maxNumLoggingPackets = 20;


//! Singleton instantiation of CommandDebugPortRouter
static CommandDebugPortRouter commandDebugPortRouterSingleton(BufferPoolBase::BufferPoolId_Logging,
	      	  	  	  	  	  	  	  	  	  	  sizeof(cefLoggingPacket_t),
												  maxNumLoggingPackets);




CommandDebugPortRouter::CommandDebugPortRouter(uint32_t logBufferPoolId,
								   uint32_t numBytesPerLoggingEntry,
								   uint32_t maxNumLoggingEntries) :
					CommandBase(commandOpCodeDebugPortRouter),
					m_loggingPool(logBufferPoolId, numBytesPerLoggingEntry, maxNumLoggingEntries),
					m_logsToSend(maxNumLoggingEntries),
					m_numBytesInCefCommandResponse(0),
				    m_cefCommandPacketState(cefCommandPacketState_bufferAvailable)
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
                LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "Unhandled command state {:d}", m_commandState);
                shouldYield = true;
                commandDone = true;
                break;
            }
        }
    }

    return commandDone;
}


cefLogging_t* CommandDebugPortRouter::checkoutLogBuffer()
{
	cefLoggingPacket_t* p_loggingPacket = (cefLoggingPacket_t*)m_loggingPool.allocate(sizeof(cefLoggingPacket_t));

	if (p_loggingPacket == nullptr)
	{
		// No logging buffers available; they must all be backed up in the transmit queue
		// Eventually we want to thrown away some data and make room for an overflow message
		return nullptr;
	}

	// We want to return a pointer to the logging packet only and "hide" the packet header
	// part of the memory from this API.
	return &(p_loggingPacket->m_loggingInfo);
}


void CommandDebugPortRouter::returnLogBuffer(cefLogging_t* p_cefLogBuffer)
{
	if (p_cefLogBuffer == nullptr)
	{
		LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "A nullptr log buffer was returned!");
	}

	size_t const offsetToBaseAddr = offsetof(cefLoggingPacket_t, m_loggingInfo);

	// We were pointing to a member variable that is not the first address of the structure...so adjust accordingly
	void* p_loggingPacket = reinterpret_cast<void*>(reinterpret_cast<char*>(p_cefLogBuffer) - offsetToBaseAddr);

	// The log packet is assumed to have valid logging data, and now is ready to be transmitted, so add
	// it to the log to send fifo.
	if (m_logsToSend.put(p_loggingPacket) == false)
	{
		// Something is messed up in the system setup as there should be room to send all logs we have buffer space for
		LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "m_logsToSend not setup correctly in CommandDebugPortRouter");
	}

}


cefLoggingPacket_t* CommandDebugPortRouter::checkoutLogPacket()
{
	void* p_cefLoggingPacket = nullptr;

	if (m_logsToSend.get(p_cefLoggingPacket) == false)
	{
		// No logs to send
		return nullptr;
	}

	return (cefLoggingPacket_t*)p_cefLoggingPacket;
}


void CommandDebugPortRouter::returnLogPacket(cefLoggingPacket_t* p_cefLogBuffer)
{
	// If memory is attempted to be returned to a pool that it was not allocated from, then free() with trace fatal.
	m_loggingPool.free(p_cefLogBuffer);
}



cefCommandPacketMaximum_t* CommandDebugPortRouter::checkoutCefReceiveCommandPacket()
{
	if (m_cefCommandPacketState != cefCommandPacketState_bufferAvailable)
	{
		// If the buffer is not available...
		return nullptr;
	}

	// Mark the packet as checked out
	m_cefCommandPacketState = cefCommandPacketState_receivingPacket;
	return &m_cefCommandPacket;
}

void CommandDebugPortRouter::returnCefReceiveCommandPacket(cefCommandPacketMaximum_t* p_cefCommandPacket)
{
	// Sanity Check:  Confirm the memory was checked out appropriately in the first place
	if (m_cefCommandPacketState != cefCommandPacketState_receivingPacket)
	{
		// Attempting to return a packet that was not checked out!
		LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "Attempt to return memory to CommandDebugPortRouter in wrong state{:d}, Allowed {:d}", m_cefCommandPacketState, cefCommandPacketState_receivingPacket);
	}

	// Sanity Check:  Confirm that returning the right memory
	if (p_cefCommandPacket != &m_cefCommandPacket)
	{
		LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "Attempt to return unknown memory to CommandDebugPortRouter 0x{:x}", p_cefCommandPacket);
	}

	// This is a valid return, so change the state of the CEF Command Buffer Packet
	// By design, the buffer should only be returned with a CEF Receive Command
	m_cefCommandPacketState = cefCommandPacketState_packetReceived;
}

cefCommandMaximum_t* CommandDebugPortRouter::checkoutCefCommandBuffer()
{
	if (m_cefCommandPacketState != cefCommandPacketState_packetReceived)
	{
		return nullptr;
	}

	// Mark that the cefCommand Buffer (does not include packet header) is checked out to proxy command
	m_cefCommandPacketState = cefCommandPacketState_proxyCommandOwnsPacket;
	return ((cefCommandMaximum_t*)&m_cefCommandPacket.m_cefCommandPayload);
}

void CommandDebugPortRouter::returnCefCommandBuffer(cefCommandMaximum_t* p_cefCommandResponse, uint32_t numBytesInCommandResponse)
{
	// Sanity Check State
	if (m_cefCommandPacketState != cefCommandPacketState_proxyCommandOwnsPacket)
	{
		// Attempting to return a packet that was not checked out!
		LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "Attempt to return memory to CommandDebugPortRouter in wrong state{:d}, Allowed {:d}", m_cefCommandPacketState, cefCommandPacketState_proxyCommandOwnsPacket);
	}

	// Sanity Check Address
	if ((char*)p_cefCommandResponse != ((char*)&m_cefCommandPacket.m_cefCommandPayload))
	{
		LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "Attempt to return unknown memory 0x{:x} via returnCefCommandBuffer, expected 0x{:x}", p_cefCommandResponse, &m_cefCommandPacket.m_cefCommandPayload);
	}

	// Sanity Check numBytes makes sense
	if (numBytesInCommandResponse > sizeof(m_cefCommandPacket.m_cefCommandPayload))
	{
		LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "Illegal payload size={:d}, max allowed={:d}", numBytesInCommand, sizeof(m_cefCommandPacket.m_cefCommandPayload));
	}

	m_numBytesInCefCommandResponse = numBytesInCommandResponse;
	m_cefCommandPacketState = cefCommandPacketState_readyToTransmit;
}

cefCommandPacketMaximum_t* CommandDebugPortRouter::checkoutCefTransmitCommandPacket(uint32_t& numBytesInCommandResponse)
{
	if (m_cefCommandPacketState != cefCommandPacketState_readyToTransmit)
	{
		return nullptr;
	}

	// Update how bytes in the m_cefCommandPayload should be transmitted
	numBytesInCommandResponse = m_numBytesInCefCommandResponse;

	m_cefCommandPacketState = cefCommandPacketState_transmittingPacket;

	return &m_cefCommandPacket;
}

void CommandDebugPortRouter::returnCefTransmitCommandPacket(cefCommandPacketMaximum_t* p_cefCommandPacket)
{
	// Sanity Check State
	if (m_cefCommandPacketState != cefCommandPacketState_transmittingPacket)
	{
		// Attempting to return a packet that was not checked out!
		LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "Attempt to return memory to CommandDebugPortRouter in wrong state{:d}, Allowed {:d}", m_cefCommandPacketState, cefCommandPacketState_transmittingPacket);
	}

	// Sanity Check Address
	if (p_cefCommandPacket != &m_cefCommandPacket)
	{
		LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "Attempt to return unknown memory to Cef Transmit Command Packet 0x{:x}, expected 0x{:x}, ", p_cefCommandResponse, &m_cefCommandPacket);
	}

	// All done with the buffer; mark buffer as being available
	m_cefCommandPacketState = cefCommandPacketState_bufferAvailable;
}
