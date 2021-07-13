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

class CommandBase
{
    public:
		// ID of initiator/creator of the command
		typedef uint8_t commandInitiatorId_t;

		// Rolling Sequence number of the command
		typedef uint16_t commandSequenceNumber_t;


        //! Constructor.
        CommandBase(commandOpCode_t commandOpCode,
        			commandInitiatorId_t commandInitiatorId = commandInitiatorIdDefault,
					commandSequenceNumber_t commandSequenceNumber = 0) :
            m_commandOpCode(commandOpCode),
            m_commandState(commandStateCommandEntry),
			m_commandErrorCode(errorCode_OK),
			mp_parentCommand(nullptr)
			{
        		commandInitiatorInit(commandInitiatorId, commandSequenceNumber);
			}

        /**
         * To aid in debug, a rolling sequence number is assigned for each command.
         * To further aid debug the an ID can be assigned who initiated a command.
         */
        enum
        {
            //! Initiated internal to the embedded software (this is the default, generic ID)
        	//! Illegal command initiator IDs default to commandInitiatorIdDefault rather than report an error.
            commandInitiatorIdDefault,

			//! Commands initiated by Python Utilities (which have own unique sequence number)
			commandInitiatiorIdPythonUtilities,

			//! Illegal command initiator IDs rather than report an error possibly during initialization of system
			commandInitiatorIdOutOfRangeInitiatorId,
        };


        /**
         * Constructor helper function to setup sequence number and commandInitiator Id
         *
         * @param commandInitiatorId	Initiator ID.  If illegal ID detected, then defaults to commandInitiatorIdDefault
         * @param commandSequenceNumber Depending on the InitiatorID, this parameter may be ignored.
         */
        void commandInitiatorInit(commandInitiatorId_t commandInitiatorId, commandSequenceNumber_t commandSequenceNumber);


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
        virtual bool execute(void* p_childCommand);


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

        // ID of initiator/creator of the command
        commandInitiatorId_t m_commandInitiatorId;

        // Rolling Sequence number of the command
        commandSequenceNumber_t m_commandSequenceNumber;

        // For internally generated commands, a new sequence number is generated for each new command
        static commandSequenceNumber_t m_rollingCommandSequenceNumber;

        //! Current command status
        errorCode_t m_commandErrorCode;

        //! pointer to parent command (NULL if no parent command)
        void* mp_parentCommand;
};



#endif  // end header guard
