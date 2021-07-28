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
#ifndef __CEF_COMMAND_BASE_H
#define __CEF_COMMAND_BASE_H


/**
 * Base class for all command handlers
 */


#include "cefMappings.hpp"
#include "cefContract.hpp"

class CommandPool;  // forward declaration to avoid include dependency chain reaction


class CommandBase
{
    public:
        //! Constructor.
        CommandBase(commandOpCode_t commandOpCode) :
            m_commandOpCode(commandOpCode),
            m_commandState(commandStateCommandEntry),
			m_commandErrorCode(errorCode_OK),
			mp_parentCommand(nullptr),
            mp_commandPool(nullptr)
			{
    			m_commandSequenceNumber = m_rollingCommandSequenceNumber++;
			}

        typedef uint16_t commandSequenceNumber_t;

        /**
         * Execute method.  Must be implemented by the derived class.
         *                  In general, command should yield after doing work
         *                  that takes some significant amount of work, and then
         *                  'wait' until this method gets called again, saving
         *                  it's state in member variables such that it can pick
         *                  up where it left off.  This is so we can return to the main
         *                  loop again to process other commands.
         *                  Reasons to yield could be:
         *                      * The command has sent a child command and is waiting for a response.
         *                      * The command is polling for something to happen.  For example, there
         *                        are operations that we poll for in the hardware that could take
         *                        a significant amount of time.
         *
         * @param p_childCommand   	Pointer to child command if this execution is a child command response
         *
         * @return true if the command has finished its work, false if it still has more work to do.
         */
        virtual bool execute(CommandBase* p_childCommand);


        /**
         * Imports data from a CEF Command into this object.
         *
         * @param	p_cefCommand				 pointer to CEF command to import
         *
         * @return errorCode_OK if successfully imported the data; error code otherwise
         */
        virtual errorCode_t importFromCefCommand(void* p_cefCommand);

        /**
         * Imports common data from a CEF Command Header structure into the base class member variables.
         *
         * @param	p_cefCommandHeader			 pointer to CEF command header
         * @param   numBytesInCefRequestCommand  number of bytes in a CEF Request command ("sizeof" command)
         *
         * @return errorCode_OK if successfully imported the data; error code otherwise
         */
        errorCode_t importFromCefCommandBase(cefCommandHeader_t* p_cefCommandHeader, uint32_t numBytesInCefRequestCommand);


        /**
         * Exports data from this object to a CEF Command.
         *
         * @param	p_cefCommand				 pointer to CEF command to import
         *
         * @return errorCode_OK if successfully exported the data; error code otherwise
         */
        virtual errorCode_t exportToCefCommand(void* p_cefCommand);

        /**
         * Exports common data from the base class member variables to a CEF Command Header structure.
         *
         * @param	p_cefCommandHeader			  pointer to CEF command header structure
         * @param   numBytesInCefResponseCommand  number of bytes in a CEF Response command ("sizeof" command)
         *
         * @return errorCode_OK if successfully exported the data; error code otherwise
         */
        errorCode_t exportToCefCommandBase(cefCommandHeader_t* p_cefCommandHeader, uint32_t numBytesInCefResponseCommand);


        /**
         * Gets this command's opcode.
         *
         * @return command's opcode
         */
        commandOpCode_t getCommandOpCode(){return m_commandOpCode;}

        /**
         * Gets this command's error code (only should be called after execute() returns true
         *
         * @return	commands error code
         */
        errorCode_t getCommandErrorCode(){return m_commandErrorCode;}

        /**
         * Sets the command pool pointer associated with this command
         *
         * @return the command pool pointer associated with this command
         */
        void setCommandPool(CommandPool* p_commandPool)
        {
        	mp_commandPool = p_commandPool;
        }

        /**
         * Gets the command pool pointer associated with this command
         *
         * @return command pool associated with this command
         */
        CommandPool* getCommandPool()
        {
        	return mp_commandPool;
        }

        /**
         * Sets the parent command associated with this command
         * 
         * @param pointer to the parent command 
         */
        void setParentCommand(CommandBase* p_parentCommand)
        {
            mp_parentCommand = p_parentCommand;
        }

        /**
         * Get the parent command associated with this command
         * 
         * @return pointer to the parent command 
         */
        CommandBase* getParentCommand()
        {
            return mp_parentCommand;
        }

        /**
         * Common routine that can be used to validate p_childCommand of execute(void* p_childCommand).
         * If the command is not expecting a child response, and one occurs, then this
         * is a non-recoverable error as the parent isn't expecting a child child response, so the child response
         * belongs to another command, or there is a memory leak or failure to return a pointer such that the child command
         * is re-using memory it is not supposed to use (or a variety of other reasons).
         * In short, the system is unstable and needs to be stopped or something could go very wrong!
         *
         * Likewise, if the wrong child response comes back, then this is a similar situation
         * as an unexpected response.
         *
         * This routine prints appropriate debug information, then triggers a "log fatal".
         *
         * If multiple child commands could be returned to the parent command, then this method should
         * be overridden.
         *
         * @param p_childCommand	pointer to the child command received from execute(void* p_childCommand)
         * @param p_expectedChildCommand  Set to nullptr if not expecting a child command;
         * 							      pointer to expected child command otherwise
         */
        void validateChildResponse(CommandBase* p_childCommand, CommandBase* p_expectedChildCommand);


    protected:
        //! Command states
        enum
        {
            //----- These states should be implemented by all command handlers -----

            //! This should be the first state of any command handler
        	//! The command should command should validate request parameters, and perform additional command initialization
            commandStateCommandEntry,

            //! This is the last state of any command.  At the very least, the command should set the return value for
			//! the execute function to true in this state (indicated there is no more work to be done/command is completed).
			//! Common "command clean up" is typically done in this state as well.
            commandStateCommandComplete,

            //----- Command-specific states start here -----
            commandStateFirstDerivedState,

            //! Use to indicate an invalid state.
            commandStateInvalid = 0xFFFF
        };

        // Command state must fit in 16 bits
        STATIC_ASSERT((commandStateInvalid <= UINT16_MAX), command_state_must_fit_in_16_bits);
        typedef uint16_t commandState_t;

        //! This command's type
        commandOpCode_t m_commandOpCode;

        //! Current command state
        commandState_t m_commandState;

        // Rolling Sequence number of the command (to aid debug)
        commandSequenceNumber_t m_commandSequenceNumber;

        // For internally generated commands, a new sequence number is generated for each new command
        static commandSequenceNumber_t m_rollingCommandSequenceNumber;

        //! Current command status
        errorCode_t m_commandErrorCode;

        //! pointer to parent command (NULL if no parent command)
        CommandBase* mp_parentCommand;

        //! pointer to CommandPool object.  This variable is needed if the command was allocated by
        //! the CommandGenerator in order to release the command back to the correct CommandPool
        CommandPool* mp_commandPool;
};


#endif  // end header guard
