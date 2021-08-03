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
#ifndef __CEF_COMMAND_CEF_COMMAND_PROXY_H
#define __CEF_COMMAND_CEF_COMMAND_PROXY_H


/**
 * Interface definition for "proxy" for processing python generate CEF Commands
 */

#include "CommandBase.hpp"
#include "CefBuffer.hpp"


class CommandCefCommandProxy : public CommandBase
{
	public:
		/**
		 * Constructor
		 */
		CommandCefCommandProxy() :
			CommandBase(commandOpCodeCefCommandProxy),
			mp_cefCommandHeader(nullptr),
			mp_childCommand(nullptr)
			{ }

		/**
		 *  Obtain a reference to the Instance of this Singleton
		 *
		 *  @return a reference to the Instance of this Singleton
		 */
		static CommandCefCommandProxy& instance();


		//! See base class for method description
		bool execute(CommandBase* p_parentCommand);

		//! Note:  CommandCefCommandProxy is not a CEF command, so it does not have an import/export method implemented


	private:

        //! Command states
        enum
        {
            commandStateGetCefCommandRequest = commandStateFirstDerivedState,
			commandStateProcessCommand,
			commandStateImportCefCommandInfoToAllocatedCommand,
			commandStateScheduleCommand,
			commandStateWaitForChildResponse,
			commandStateSendAndReleaseResources,
			commandStateReportError,
        };

        //! Pointer to the CEF Buffer containing the command to be processed
        CefBuffer* mp_cefBuffer;

        //! The CEF command to be processed
        cefCommandHeader_t* mp_cefCommandHeader;

        //! The allocated internal command that is generated based on information in mp_cefCommand
        //! This is a child command so once it finished executing it returns to this object
        CommandBase* mp_childCommand;

};

#endif  // end header guard
