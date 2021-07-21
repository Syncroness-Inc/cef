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
#ifndef __CEF_COMMAND_GENERATOR_H
#define __CEF_COMMAND_GENERATOR_H


/**
 * Command Generator is a "data store" to allocate memory for an object (based on opcode), and then
 * instantiate that object with a "placement new" on the allocated memory.  The Command Generator serves
 * as an arbitrator on how commands are allocated by appropriately applying the rules as to
 * how to manage the limited amount of command memory available to run commands.  This one singleton object is
 * intended to be the "one place" in the system that determines if a command should be allocated memory
 * from the amount of command memory allocated at compile time.
 *
 * As projects evolve, the command generator may need to become more sophisticated on arbitrating resources.
 * For example, a waiting list may be needed.  To start with, if no memory is available, then the requestor is responsible
 * for deciding what to do (poll until a command is ready, raise an error...)
 *
 * The initial implementation assumes a simplistic first come, first serve implementation.  Commands that always
 * must be run should be instantiated outside of the Command Generator at startup time.
 *
 */

#include "cefMappings.hpp"
#include "CommandBase.hpp"
#include "CommandPool.hpp"

class CommandGenerator
{
	public:
		//! Constructor
		CommandGenerator()
			{ }

		/**
		 *  Obtain a reference to the Command Generator.
		 *
		 *  @return a reference to the Command Generator
		 */
		static CommandGenerator& instance();

		/**
		 * Allocates a command based on the command's opCode
		 *
		 * @return	nullptr if could not allocate a command (likely because no memory currently available)
		 * 			Otherwise, the object corresponding to the opCode
		 */
		CommandBase* allocateCommand(commandOpCode_t commandOpCode);

		/**
		 * Free the command back to appropriate command pool
		 *
		 * @param p_command		the command to be "freed" (that is, returned to allocating command pool)
		 */
		void freeCommand(CommandBase* p_command);

	private:
		static CommandPool m_debugCommandPool;

};

#endif  // end header guard
