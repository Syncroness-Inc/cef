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
#ifndef __CEF_COMMAND_EXECUTOR_H
#define __CEF_COMMAND_EXECUTOR_H


#include "cefMappings.hpp"
#include "CommandBase.hpp"
#include "CommandPool.hpp"



class CommandExecutor
{
	public:
		/**
		 * Constructor
		 *
		 * @param maxNumberOfCommandsInCommandExecutor   maximum number of commands the executor can execute at one time
		 */
		CommandExecutor(uint32_t maxNumberOfCommandsInCommandExecutor);

		/**
		 *  Obtain a reference to the Command Executor.
		 *
		 *  @return a reference to the Command Executor
		 */
		static CommandExecutor& instance();


		/**
		 * Executes numCommandsAllowedToExecute from the command queue
		 *
		 * @param numCommandsAllowedToExecute
		 *
		 * @return number of commands actually executed
		 */
		uint32_t executeCommands(uint32_t numCommandsAllowedToExecute);

		/**
		 * Adds the command to the CommandExecutor Queue to be executed
		 * 		Note:  Once a command is added to the queue, it remains on
		 * 		the queue until the command runs to completion. Hence, there is no
		 * 		removeCommandFromQueue.
		 *
		 * @param p_command		command to add to the queue
		 */
		void addCommandToQueue(CommandBase* p_command);


	private:
        //! m_commandState states for the CommandExecutor
        enum
        {
        	commandStateGetNextCommand,
			commandStateExecuteCommand
        };


        // This is the last state of the command (modeled after m_commandState of CommandBase)
        uint32_t m_commandState;

        //! The command to be executed, or currently executing
        CommandBase* mp_commandToExecute;

        //! The child command that is currently associated with mp_commandToExecute
        CommandBase* mp_childCommand;

        //! FIFO Queue of commands that need to be executed
		RingBufferOfVoidPointers m_executeCommandsQueue;

};

#endif  // end header guard
