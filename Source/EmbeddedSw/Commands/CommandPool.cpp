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
#include "BufferPoolBase.hpp"
#include "Logging.hpp"

CommandPool::CommandPool(uint32_t commandPoolId, uint32_t maxCommandSizeInBytes, uint32_t numCommands) :
	BufferPoolBase(commandPoolId, maxCommandSizeInBytes, numCommands)
{

}

void* CommandPool::allocateCommandMemory(uint32_t commandSizeInBytes)
{
	return(allocate(commandSizeInBytes));
}

void CommandPool::freeCommandMemory(void* p_commandMemory)
{
	free(p_commandMemory);
}

