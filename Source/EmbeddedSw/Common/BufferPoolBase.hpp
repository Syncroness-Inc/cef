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
#ifndef __BUFFER_POOL_BASE_H
#define __BUFFER_POOL_BASE_H


/**
 * The constructor allocates enough memory for the buffer pool (on the proper alignment) given
 * the number of buffers and the maximum buffer size.
 *
 * A ring buffer is used to manage a "queue" of "chunks of buffer memory" that can be allocated/freed.
 */

#include "cefMappings.hpp"
#include "RingBufferOfVoidPointers.hpp"

class BufferPoolBase
{
	public:
		/**
		 * Constructor
		 *
		 * @param bufferPoolId			Id from BufferPoolBase used to aid debug
		 * @param maxBufferSizeInBytes 	The maximum buffer size in bytes to be allocated from the pool
		 * @param numBuffers			The number of buffers in the pool
		 * */
		BufferPoolBase(uint32_t bufferPoolId, uint32_t maxBufferSizeInBytes, uint32_t numBuffers);

		//! Enum used to aid debug; each pool should have a unique pool id
		enum
		{
			BufferPoolId_DebugCommandPool,
			BufferPoolId_Logging
		};

		/**
		 * Allocate buffer memory from the pool
		 *
		 * @param bufferSizeInBytes	how many bytes in each buffer that can be allocated
		 *
		 * @return	nullptr if no buffer could be allocated, a valid pointer otherwise
		 */
		void* allocate(uint32_t bufferSizeInBytes);

		/**
		 * Returns a buffer to the pool.
		 * 		A fatal error will occur if the memory being returned is not part of the buffer pool's memory
		 *
		 *@param p_bufferMemory	pointer to that start of the memory to return to the pool
		 */
		void free(void* p_bufferMemory);


	private:
		// Align the memory for each buffer.  For now, align to a void* pointer as void* should be the alignment
		// requirement for structures as well.
		static const uint32_t m_bufferPoolAlignmentSizeInBytes = sizeof(void*);

		//! Id of buffer pool (used to help debug memory allocation/free issues)
		uint32_t m_bufferPoolId;

		//! Ring Buffer of pointers to "chunks of memory" that can be allocated for buffer memory
		RingBufferOfVoidPointers m_ringBufferOfBufferMemory;

		//! Maximum buffer size in bytes that can be allocated from this pool
		uint32_t m_maxBufferSizeInBytes;

		//! number of buffers that can be allocated from this pool at one time
		uint32_t m_numBuffers;

		//! pointer to the start of the memory allocated for this pool
		uint8_t* mp_memoryPoolStart;

		//! pointer to the last byte of memory in this pool
		uint8_t* mp_memoryPoolEnd;
};

#endif  // end header guard

