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
#ifndef __APP_MAIN_H
#define __APP_MAIN_H


#include "cefMappings.hpp"
#include "cefContract.hpp"


class AppMain
{
	public:
		/**
		 * Constructor
		 */
		AppMain() :
		    m_firstSystemErrorCode(errorCode_OK)
			{ }

		/**
		 *  Obtain a reference to AppMain object.
		 *
		 *  @return a reference to AppMain object
		 */
		static AppMain& instance();

		/**
		 * Finishes initialization beyond what is required by the Board Support Package, and
		 * then enters an infinite while loop.  This method is intended to be called once the
		 * BSP auto-generated code has been completed.
		 *
		 * @return THERE IS NO RETURN FROM THIS FUNCTION
		 */
		void runAppMain_noReturn();

		/**
		 * Sets the first system error code that occurs in the system.  If all is well in the system,
		 * this value should be set to errorCode_OK.  Otherwise, it is the first error code that
		 * will cause the system to degrade in performance/capability.
		 *
		 * @param systemErrorCode   system error code
		 */
		void setSystemErrorCode(errorCode_t systemErrorCode)
		{
		    if (m_firstSystemErrorCode == errorCode_OK)
		    {
		        m_firstSystemErrorCode = systemErrorCode;
		    }
		}

		/**
		 * Returns the first system error code that was reported
		 *
		 * @return the first system error code that was reported
		 */
		errorCode_t GetSystemErrorCode()
		{
		    return m_firstSystemErrorCode;
		}

	private:
		/**
		 * Completes initialization that is necessary beyond what the Board Support Package
		 * auto generate code completes.
		 */
		void initialize();

		/**
		 * The "infinite while loop" for the application code.
		 * There is no return from this function
		 */
		void run();


		/**
		 * If something "terrible" happens with the system that causes the system to "shutdown"
		 * or continue in a reduced capacity state, the first error that causes this state
		 * is stored in this variable.  There will likely be a cascade of errors, but
		 * this error code can at least give a starting point what caused the issue.
		 */
		errorCode_t m_firstSystemErrorCode;

};

#endif  // end header guard
