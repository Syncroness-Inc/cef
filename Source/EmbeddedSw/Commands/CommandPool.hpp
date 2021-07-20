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
#ifndef __COMMAND_POOL_H
#define __COMMAND_POOL_H


/**
 * The constructor allocates enough memory for the command pool (on the proper alignment) given
 * the number of commands and the maximum command size.
 *
 * A linked list is then used to manage a "queue" of "command memory" that can be allocated.
 *
 * In order to use the linked list pointers inherent in all commands (and thereby avoid needing
 * to consume more memory as well as avoiding a maintenance issue of sizing a queue properly) all the memory
 * pool item must have a base class of CommandBase.  For robustness, whenever memory is returned to the pool,
 * a "placement new" of CommandBase is re-applied just in case the memory was corrupted after it was checked out.
 */

#include "cefMappings.hpp"
#include "RingBufferOfVoidPointers.hpp"

class CommandPool
{
	public:
		//! Constructor
		CommandPool(uint32_t maxCommandSize, uint32_t numCommands);

		/**
		 * Allocate command memory from the pool
		 *
		 * @param commandSizeInBytes	how many bytes in the command to be allocated
		 *
		 * @return	nullptr if no command could be allocated, a valid pointer otherwise
		 */
		void* allocateCommandMemory(uint32_t commandSizeInBytes);

		/**
		 * Returns command to the pool.
		 * 		A fatal error will occur if the memory being returned is not part of the command pool's memory
		 *
		 *@param p_commandMemory	pointer to memory to return to the pool
		 */
		void freeCommandMemory(void* p_commandMemory);


	private:
		// Align the memory for each command.  For now, align to a void* pointer should be the alignment
		// requirement for structures as well.
		static const uint32_t m_commandPoolAlignmentSizeInBytes = sizeof(void*);

		//!
		RingBufferOfVoidPointers m_ringBufferOfCommandMemory;

		//! Maximum command size in bytes that can be allocated from this pool
		uint32_t m_maxCommandSizeInBytes;

		//! number of commands that can be allocated from this pool at one time
		uint32_t m_numCommands;;

		//! pointer to the start of the memory allocated for this pool
		uint8_t* mp_memoryPoolStart;

		//! pointer to the last byte of memory in this pool
		uint8_t* mp_memoryPoolEnd;
};

#endif  // end header guard

