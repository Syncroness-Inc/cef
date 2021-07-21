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
 * Implementation of CommandGenerator methods
 *
 * More sophisticated algorithms for allocating commands can be implemented, but to start with
 * the allocation is based on first come, first serve for a given opcode.
 * There can be multiple pools, so a first level of fairness could be implementing a pool that is
 * dedicated to certain opcodes.  For example, all debug commands can use one pool, and application commands
 * can use another pool.  This prevents debug commands from gobbling up all the application command pool memory.
 * Conversely, it ensures there is at least one debug command that can always be run regardless what
 * is happening with the application commands.
 *
 * Commands are allocated from a "pool" of command memory.  A pool is a series of "chunks" of memory.
 * Each "chunk" is big enough to instantiate any command in the pool.
 * To allocate a command
 * 		1. Determine which pool should be used from the opcode
 * 		2. Confirm there isn't a programming error by checking that the "chunk" of memory allocated is big
 * 		   enough to instantiate the command.  This helps avoid memory corruption issues that would be difficult to
 * 		   track down.
 * 		3. Do a "placement new" using the appropriate class to create the object
 *
 * 		If a command cannot be allocated, then a nullptr is returned.  Future implementations could consider
 * 		adding query methods to the class as to why the command could not be allocated.
 *
 * 	What pool the command was allocated from is stored in the command object.  To "free" the command, the pool is retrieved from
 * 	the object.  A sanity check is made when freeing the command that the command is not in use elsewhere by asking CommandBase
 * 	if there is any indication that that command may still be in use.
 */

#include <new>		// for placement new

#include "CommandGenerator.hpp"
#include "Logging.hpp"

/* All command classes in the system that are allocated by the CommandGenerator need to be included here */
#include "CommandPing.hpp"


/*
 * Templates to calculate the size of the command pools at compile time.
 */
template <typename T>
static constexpr T static_max(T a, T b) {
    return a < b ? b : a;
}

template <typename T, typename... Ts>
static constexpr T static_max(T a, Ts... bs) {
    return static_max(a, static_max(bs...));
}

template <typename... Ts>
constexpr size_t max_sizeof() {
    return static_max(sizeof(Ts)...);
};


//! Singleton declaration of the CommandGenerator
static CommandGenerator commandGeneratorSingleton;


//******************************************** Debug Command Pool **********************************************//
/**
 * Calculate the maximum number of bytes needed to allocate a command for all the commands using this pool.
 * 		Each command that is to be allocated from this pool must be added to the expression below.
 */
static constexpr size_t debugCommandPoolMaxClassSizeInBytes = max_sizeof<
		CommandPing,
		CommandPing
		>();

//! Number of commands in the debug command pool (be sure to add all pool counts into m_totalNumberOfCommandGeneratorCommands
CommandPool CommandGenerator::m_debugCommandPool(debugCommandPoolMaxClassSizeInBytes,
												 CommandGenerator::m_numDebugCommandPoolEntries);


//******************************************** Application Command Pool **********************************************//
// Add application specific command pool(s) here



/*
 * Template used to allocate a command and do a "placement new"
 */
template <class T>
static CommandBase* generateCommand(CommandPool& commandPool)
{
	void* p_commandMemory = commandPool.allocateCommandMemory(sizeof(T));
	/**
	 * If couldn't allocate the command, exit and let a high level routine handle what to do next.
	 * There is a limited amount of memory, so a task may have to wait until another command has finished
	 * before getting allocated a command.  If failure to allocate happens often, then consider increasing the
	 * number of commands in the command pool, or moving the opcode to a new/different command pool.
	 *   */
	if (p_commandMemory == nullptr)
	{
		return nullptr;
	}

	// Do a "placment new" on the class to instantiate the object and run the constructor
	CommandBase* p_command = (CommandBase*) new (p_commandMemory) T;

	if (p_command != nullptr)
	{
		p_command->setCommandPool(&commandPool);
	}

	return p_command;
}


CommandGenerator& CommandGenerator::instance()
{
	return commandGeneratorSingleton;
}


CommandBase* CommandGenerator::allocateCommand(commandOpCode_t commandOpCode)
{
	CommandBase* p_command = nullptr;

	switch (commandOpCode)
	{
		case commandOpCodePing:
		{
			p_command = generateCommand<CommandPing>(m_debugCommandPool);
			break;
		}
		default:
		{
			LOG_ERROR(Logging::LogModuleIdCefInfrastructure, "Opcode={:d} not setup in CommandGenerator so couldn't generate a command={}", commandOpCode);
			break;
		}
	}

	return p_command;
}

void CommandGenerator::freeCommand(CommandBase* p_command)
{
	if (p_command == nullptr)
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Command Generator requested to free nullptr command!");
	}

	CommandPool* p_commandPool = p_command->getCommandPool();
	if (p_commandPool == nullptr)
	{
		// We are being asked to return a command to a pool, but we don't know which pool the command was allocated from!
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Command Generator requested to free a command=0x{:x) with no associated command pool!", p_command);
	}

	p_commandPool->freeCommandMemory(p_command);
}

