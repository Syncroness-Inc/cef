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
 * Implementation of RingBufferOfVoidPointers methods
 */

#include "RingBufferOfVoidPointers.hpp"
#include "Logging.hpp"


RingBufferOfVoidPointers::RingBufferOfVoidPointers(uint32_t numPointersInRingBuffer) :
		m_head(0),
		m_tail(0),
		m_numElements(numPointersInRingBuffer + 1)	// see member variable documentation as to why this is plus 1
{
	// allocate memory
	const size_t numBytesToAllocate = m_numElements * sizeof(void*);

	mp_ringArray = (void**) malloc(numBytesToAllocate);

	if (mp_ringArray == nullptr)
	{
		// We expect to succeed, and since this is done in the constructor, have no way to return an error
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "malloc Failed for RingBufferOfVoidPointers of m_numElements={:d}", m_numElements);
	}
}

bool RingBufferOfVoidPointers::put(void* putPointer)
{
	if (isFull() == true)
	{
		// ring buffer is full, so the put failed
		return false;
	}

	// Head points to next available spot
	// Add the value to the ring buffer before incrementing the index to make put operation thread safe
	mp_ringArray[m_head] = putPointer;
	m_head = incrementToNextIndex(m_head);

	return true;
}

bool RingBufferOfVoidPointers::get(void*& getPointer)
{
	if (isEmpty() == true)
	{
		// ring buffer is empty, so the get failed
		return false;
	}

	// tail points to the the next item to remove from the buffer
	// Remove the value to the ring buffer before incrementing the index to make get operation thread safe
	getPointer = mp_ringArray[m_tail];
	m_tail = incrementToNextIndex(m_tail);

	return true;
}

uint32_t RingBufferOfVoidPointers::getCurrentNumberOfEntries()
{
	// if m_tail == m_head, then numEntries is zero as this signifies an empty array
	uint32_t numEntries = 0;

	/**
	 * The "empty" entry in the ring buffer array makes the following math work out.
	 */
	if (m_head > m_tail)
	{
		// The head is ahead of the tail, so don't need to account for the wrap
		numEntries = m_head - m_tail;
	}
	else if (m_tail > m_head)
	{
		// Tail is ahead of the head, so need to account for the wrap.
		// Figure out how many "empty" slots there are, and subtract this from number of slots
		numEntries = m_numElements - (m_tail - m_head);
	}

	return numEntries;
}


bool RingBufferOfVoidPointers::removeItem(void* p_itemToRemove)
{
	if (isEmpty() == true)
	{
		return false;
	}

	/**
	 * At this point, we know there is at least one entry in the ring buffer.
	 * Parse through the ring buffer, removing each item to see if it matches the item to be removed.
	 * If the item is not a match, then add the item back to the tail of the ring buffer and keep trying
	 * until loop through the entire ring buffer.
	 */

	uint32_t numEntries = getCurrentNumberOfEntries();
	for (uint32_t i = 0; i < numEntries; ++i)
	{
		void* p_currentItem = nullptr;
		if (get(p_currentItem) == false)
		{
			// We know there is at least one item in the ring buffer, so something is seriously broken!
			LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Failed to remove item from ring buffer when expected to find at least one item");
		}

		if (p_currentItem == p_itemToRemove)
		{
			// Found a match!  We have already removed the item from the ring buffer, so we are all done.
			return true;
		}

		// We don't have a match, add the item to the tail of the ring buffer
		if (put(p_currentItem) == false)
		{
			// We just successfully removed an item from the ring buffer, so we should be able to put it back in!
			LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Failed to add an item back into the ring buffer after just removing it!");
		}
	}

	// No match to the requested item was found if control flow reaches this point
	return false;
}
