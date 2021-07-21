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
 * Implementation of CommandPool methods
 */

#include <new>		// for placement new

#include "CommandPool.hpp"
#include "RingBufferOfVoidPointers.hpp"
#include "Logging.hpp"

CommandPool::CommandPool(uint32_t maxCommandSize, uint32_t numCommands) :
m_ringBufferOfCommandMemory(numCommands),
	m_maxCommandSizeInBytes(maxCommandSize),
	m_numCommands(numCommands),
	mp_memoryPoolStart(nullptr),
	mp_memoryPoolEnd(nullptr)
{
	// Round up the number of bytes needed to an alignment boundary
	uint32_t bytesNeededForEachCommand = (m_maxCommandSizeInBytes + m_commandPoolAlignmentSizeInBytes - 1) / m_commandPoolAlignmentSizeInBytes;

	uint32_t totalNumBytesNeeded = bytesNeededForEachCommand * m_numCommands;

	mp_memoryPoolStart = (uint8_t*)malloc(totalNumBytesNeeded);
	if (mp_memoryPoolStart == nullptr)
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Failed to allocate command memory pool. MaxCommandSize={}, m_maxCommandSizeInBytes={:d}", m_maxCommandSize, m_numCommands);
	}
	mp_memoryPoolEnd = &mp_memoryPoolStart[totalNumBytesNeeded - 1];

	// Fill up the memory pool
	for (uint32_t i = 0; i < m_numCommands; ++i)
	{
		void* p_commandMemory = &mp_memoryPoolStart[i * bytesNeededForEachCommand];
		bool putResult = m_ringBufferOfCommandMemory.put(p_commandMemory);
		if (putResult == false)
		{
			// the pool was sized to accept all the command memory, so this should work
			LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Failed to put into command memory pool during constructor");
		}
	}
}


void* CommandPool::allocateCommandMemory(uint32_t commandSizeInBytes)
{
	// Sanity check that not trying to allocate a command bigger than this pool is sized for
	if (commandSizeInBytes > m_maxCommandSizeInBytes)
	{
		LOG_ERROR(Logging::LogModuleIdCefInfrastructure, "Request for command of {:d} bytes from a command pool whose max size is {:d} bytes.", commandSizeInBytes, m_maxCommandSizeInBytes);
		return nullptr;
	}

	void* p_allocatedMemory = nullptr;
	bool getResult = m_ringBufferOfCommandMemory.get(p_allocatedMemory);
	if (getResult == false)
	{
		// make sure return nullptr if failed to get memory
		p_allocatedMemory = nullptr;
	}

	return(p_allocatedMemory);
}

void CommandPool::freeCommandMemory(void* p_commandMemory)
{
	// Sanity check that memory belongs to this pool.
	if ((p_commandMemory < mp_memoryPoolStart) ||
	    (p_commandMemory > mp_memoryPoolEnd))
	{
		// attempting to return memory to the pool that is out of range
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Attempting to return out of range memory to pool. address=0x{:x}, minAddress=0x{:x}, maxAddress=0x{:x}", p_commandMemory, mp_memoryPoolStart, mp_memoryPoolEnd);
	}

	bool putResult = m_ringBufferOfCommandMemory.put(p_commandMemory);
	if (putResult == false)
	{
		// we are returning memory to a pool it was allocated from, so there should be room!
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Failed to put into command memory pool on freeCommandMemory");
	}
}

