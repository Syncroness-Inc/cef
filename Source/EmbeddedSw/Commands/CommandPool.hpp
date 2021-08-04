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
 */

#include "cefMappings.hpp"
#include "BufferPoolBase.hpp"

class CommandPool : public BufferPoolBase
{
	public:
		/**
		 * Constructor
		 *
		 * @param commandPooolId  Unique pool id for each command
		 * @param maxCommandSize  The maximum command size in bytes to be allocated from this pool
		 * @param numCommands	  The maximum number of commands that can be allocated at one time for this pool
		 */
		CommandPool(uint32_t commandPoolId, uint32_t maxCommandSizeInBytes, uint32_t numCommands);

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

};

#endif  // end header guard

