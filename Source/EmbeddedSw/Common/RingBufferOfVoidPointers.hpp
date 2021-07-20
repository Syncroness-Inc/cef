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
/* Header guard */
#ifndef __RING_BUFFER_OF_VOID_POINTERS_H
#define __RING_BUFFER_OF_VOID_POINTERS_H


/**
 * Ring Buffer of void* pointers that is thread, but not cross core, safe.
 * Data Sync barriers are not used to make this ring buffer cross core safe,
 * but within a core, this object can be used as "Single Producer, Single Consumer" queue.
 *
 * Note:  This class uses malloc, so objects of this class MUST be created only at system startup
 * when malloc is allowed to be used.
 *
 * The ring buffer is stored as an array, with one extra element that is empty per the design
 * pattern of SPSC (Single Producers Single Consumer) queues.
 */


#include "cefMappings.hpp"


class RingBufferOfVoidPointers
{
    public:
        //! Constructor
		RingBufferOfVoidPointers(uint32_t numPointersInRingBuffer);


        /**
         * Checks if the ring buffer is empty.
         *
         * @return	true if the ring buffer is empty
         */
		bool isEmpty()
		{
			return(m_head == m_tail);
		}

        /**
         * Checks if the ring buffer is full
         *
         * @return	true if the ring buffer is full
         */
		bool isFull()
		{
			return(incrementToNextIndex(m_head) == m_tail);
		}

        /**
         * Adds/Puts a value to the ring buffer
         *
         *@param putPointer	The pointer to add to the ring buffer
         *
         * @return	true if successfully added pointer to ring buffer; false if ring buffer was full
         */
		bool put(void* putPointer);

        /**
         * Gets the next pointer from the ring buffer
         *
         *@param value	(reference) the value removed from the ring buffer
         *
         * @return	true if successfully removed a pointer from the ring buffer; false if ring buffer was empty
         */
		bool get(void*& getPointer);

		/**
		 * Returns the maximum number of elements that can be added to the ring buffer
		 *
		 * @return maximum number of elements
		 */
		uint32_t getMaximumNumberOfElements()
		{
			// There is one empty array space in the ring buffer, so need to subtract 1 from m_numElements to account for this
			return (m_numElements - 1);
		}

		/**
		 * Returns the number of elements currently in the ring buffer
		 * 		Note:  This method is neither thread nor multi-core safe
		 *
		 * @param number of elements currently in the ring buffer
		 */
		uint32_t getCurrentNumberOfEntries();

		/**
		 * Removes the void* pointer/item from the ring buffer
		 * 		Note:  There is an assumption that each address in the ring buffer is unique!
		 * 		Note:  This method is neither thread nor multi-core safe
		 *
		 * @param p_itemToRemove		item/pointer to remove from the list
		 *
		 * @return true if item was removed from the list; false if item wasn't in the list
		*/
		bool removeItem(void* p_itemToRemove);


    private:
        /**
         * Increments to the next index in the ring buffer
         *
         * @param index to increment (either head or tail pointer)
         *
         * @return	incremented value
         */
		uint32_t incrementToNextIndex(uint32_t index)
		{
			return((++index) % m_numElements);
		}


		/**
		 * The head points to the next available spot/index in the array.
		 * The tail points to the oldest index in the array (i.e. the next element to be removed)
		 * There is one extra storage element in the array to simplify the logic.
		 * When head = tail, then both indexes point to an empty element
		 */
		uint32_t m_head;
		uint32_t m_tail;

		/**
		 * The storage size of the array needs to be increased by 1 over the template value N to
		 * account for the one extra storage element in the array that is used to simplify the implementation.
		 */
		const uint32_t m_numElements;

		//! Pointer to the start of the ring array of void*; must be set during constructor
		void** mp_ringArray;
};



#endif  // end header guard
