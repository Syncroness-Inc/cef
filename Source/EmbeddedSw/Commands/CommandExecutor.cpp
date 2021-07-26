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
 * A statement machine design pattern similar to how commands are implemented is used for the command executor.
 * A higher level routine controls hows many commands that the CommandExecutor is allowed to execute
 * on each invocation of executeCommands().  A command that is being executed may be a child command.  And when 
 * a child command finishes execution, it needs to call a parent command.  Calling the parent command counts towards the 
 * numCommandsAllowedToExecute limit.  Hence, the design needs to accommodate picking up with the execution of 
 * a parent command from a child command finishing.
 * 
 * The design also needs to account for no commands in the command execute list (this would be highly unusual, but
 * it might occur during startup as commands are being added to the list list)
 */

#include "CommandExecutor.hpp"
#include "Logging.hpp"
#include "CommandGenerator.hpp"


//******************************************** Total Number of Commands in the System *******************************//
/**
 * The total number of commands in the system needs to be known at compile time to size queues properly.
 *
 * We could consider adding a linked list pointer point to the CommandBase
 * to avoid the maintenance issue of the algorithm below, but the initial
 * implementation choose to keep it simple to start with.
 * The magic number of extra commands comes from
 *      1 Singleton, CommandExternalCommandProxy
 *      1 Singleton, CommandDebugPort
 *      2 Extra space for margin
 *      plus all the commands generated by CommandGenerator
 */
//const uint32_t TOTAL_NUMBER_OF_ALLOWED_ACTIVE_COMMANDS_IN_SYSTEM = (4 + debugCommandPoolMaxClassSizeInBytes);

static constexpr uint32_t totalNumberOfActiveCommandsInTheSystem =
		CommandGenerator::getTotalNumberOfCommandGeneratorCommands() + /* commands generated by CommandGenerator */
		1 + /* Singleton, CommandExternalCommandProxy */
		1 + /* Singleton, CommandDebugPort */
		2;  /* Extra space for margin*/


//! Singleton instantiation of CommandExecutor
static CommandExecutor commandExecutorSingleton(totalNumberOfActiveCommandsInTheSystem);


CommandExecutor::CommandExecutor(uint32_t maxNumberOfCommandsInCommandExecutor) :
		m_commandState(commandStateGetNextCommand),
		mp_commandToExecute(nullptr),
		mp_childCommand(nullptr),
		m_executeCommandsQueue(maxNumberOfCommandsInCommandExecutor)
{ }


CommandExecutor& CommandExecutor::instance()
{
	return commandExecutorSingleton;
}

uint32_t CommandExecutor::executeCommands(uint32_t numCommandsAllowedToExecute)
{
    uint32_t numCommandsExecuted = 0;

    // True if, for whatever reason, all done with the work want to do during 
    // this invocation of executeCommands.
    bool allDone = false;

    while ((numCommandsExecuted <  numCommandsAllowedToExecute) &&
           (allDone == false))
    {
       switch (m_commandState)
        {
            case commandStateGetNextCommand:
            {
            	void* p_temp = nullptr;
                bool gotCommand = m_executeCommandsQueue.get(p_temp);
                mp_commandToExecute = (CommandBase*)p_temp;

                if (gotCommand == false)
                {
                    // exit and come back into the same state again when hopefully there will be some commands!
                    allDone = true;
                    break;
                }
             
                mp_childCommand = nullptr;
				m_commandState = commandStateExecuteCommand;
                break;
            }
            case commandStateExecuteCommand:
            {

                bool commandDone = mp_commandToExecute->execute(mp_childCommand);
                ++numCommandsExecuted;

                if (commandDone == false)
                {
                    /**
                     * The command is finished executing for now, but still has more work to do.
                     * Re-add it to the queue to wait its turn to execute again.
                     */
                    bool successfullyAdded = m_executeCommandsQueue.put(mp_commandToExecute);
                    if (successfullyAdded == false)
                    {
                        // We just removed this command from the queue, so we should be able to add it back in!
                        // Plus, the queue should be sized big enough to accommodate all possible instantiated commands.
                        LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Unable to add command to command executor queue");
                        allDone = true;
                        break;                        
                    }

                    // Now go get the next command to execute
                    m_commandState = commandStateGetNextCommand;
                    break;
                }

                /** 
                 * The command has finished all the work is was supposed to do.  
                 * If it is a child command then the parent is responsible
                 * for releasing the command.  The parent command must be the next command to execute.
                 */
                mp_childCommand = mp_commandToExecute->getParentCommand();
                if (mp_childCommand != nullptr)
                {
                    // setup so next command to execute is the parent command
                    mp_commandToExecute = mp_commandToExecute->getParentCommand();
                    m_commandState = commandStateExecuteCommand;
                    break;
                }

                /** 
                 * The command is all done with all the work it was supposed to do (e.g. finished executing)
                 * It is not a child command, so we need to release/free the command.
                 */
                CommandGenerator::instance().freeCommand(mp_commandToExecute);
				m_commandState = commandStateGetNextCommand;
                break;
            }
            default:
            {
                // If we get here, we've lost our mind.
                LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Unhandled command state %d", m_commandState);
                allDone = true;
                break;
            }
        }  // switch
    }

    return numCommandsExecuted;
}

void CommandExecutor::addCommandToQueue(CommandBase* p_command)
{
	/**
	 * The Command Executor is supposed to be designed to handle all possible
	 * instantiated commands in the system.  Hence, this method should always succeed
	 * or else there is a programming error.  Look at the calculation of totalNumberOfActiveCommandsInTheSystem
	 * for hints to as why
	 */
    bool successfullyAdded = m_executeCommandsQueue.put(p_command);
    if (successfullyAdded == false)
    {
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Unable to add command to command executor queue");
    }
}
