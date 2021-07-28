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
 * Implementation of BufferPoolBase methods
 */


#include <new>		// for placement new

#include "BufferPoolBase.hpp"
#include "Logging.hpp"

BufferPoolBase::BufferPoolBase(uint32_t bufferPoolId, uint32_t maxBufferSize, uint32_t numBuffers) :
	m_bufferPoolId(bufferPoolId),
	m_ringBufferOfBufferMemory(numBuffers),
	m_maxBufferSizeInBytes(maxBufferSize),
	m_numBuffers(numBuffers),
	mp_memoryPoolStart(nullptr),
	mp_memoryPoolEnd(nullptr)
{
	// Round up the number of bytes needed to an alignment boundary
	uint32_t bytesNeededForEachBuffer = \
			((m_maxBufferSizeInBytes + m_bufferPoolAlignmentSizeInBytes - 1) / m_bufferPoolAlignmentSizeInBytes) * m_bufferPoolAlignmentSizeInBytes;

	uint32_t totalNumBytesNeeded = bytesNeededForEachBuffer * m_numBuffers;

	mp_memoryPoolStart = (uint8_t*)malloc(totalNumBytesNeeded);
	if (mp_memoryPoolStart == nullptr)
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Failed to allocate buffer memory pool. MaxBufferSize={}, m_maxBufferSizeInBytes={:d}, BufferPoolId={:d}", m_maxBufferSize, m_numBuffers, m_bufferPoolId);
	}
	mp_memoryPoolEnd = &mp_memoryPoolStart[totalNumBytesNeeded - 1];

	// Fill up the memory pool
	for (uint32_t i = 0; i < m_numBuffers; ++i)
	{
		void* p_bufferMemory = &mp_memoryPoolStart[i * bytesNeededForEachBuffer];
		bool putResult = m_ringBufferOfBufferMemory.put(p_bufferMemory);
		if (putResult == false)
		{
			// the pool was sized to accept all the buffer memory, so this should work
			LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Failed to put into buffer memory pool during constructor, BufferPoolId={:d}", m_bufferPoolId);
		}
	}
}


void* BufferPoolBase::allocate(uint32_t bufferSizeInBytes)
{
	// Sanity check that not trying to allocate a buffer bigger than this pool is sized for
	if (bufferSizeInBytes > m_maxBufferSizeInBytes)
	{
		LOG_ERROR(Logging::LogModuleIdCefInfrastructure, "Request for buffer of {:d} bytes from a buffer pool whose max size is {:d} bytes.  bufferPoolId={:d}", bufferSizeInBytes, m_maxBufferSizeInBytes, m_bufferPoolId);
		return nullptr;
	}

	void* p_allocatedMemory = nullptr;
	bool getResult = m_ringBufferOfBufferMemory.get(p_allocatedMemory);
	if (getResult == false)
	{
		// make sure return nullptr if failed to get memory
		p_allocatedMemory = nullptr;
	}

	return(p_allocatedMemory);
}

void BufferPoolBase::free(void* p_bufferMemory)
{
	// Sanity check that memory belongs to this pool.
	if ((p_bufferMemory < mp_memoryPoolStart) ||
	    (p_bufferMemory > mp_memoryPoolEnd))
	{
		// attempting to return memory to the pool that is out of range
		LOG_ERROR(Logging::LogModuleIdCefInfrastructure, "Attempting to return out of range memory to pool.  bufferPoolId={d}", bufferPoolId);
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Attempting to return out of range memory to pool. address=0x{:x}, minAddress=0x{:x}, maxAddress=0x{:x}", p_bufferMemory, mp_memoryPoolStart, mp_memoryPoolEnd);

	}

	bool putResult = m_ringBufferOfBufferMemory.put(p_bufferMemory);
	if (putResult == false)
	{
		// we are returning memory to a pool it was allocated from, so there should be room!
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Failed to put into buffer memory pool id = {:d} on free",m_memoryPoolId);
	}
}

