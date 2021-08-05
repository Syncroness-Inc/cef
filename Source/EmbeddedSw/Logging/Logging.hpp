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
#ifndef __LOGGING_H
#define __LOGGING_H

/* Start of c/c++ guard */
#ifdef __cplusplus
extern "C" {
#endif


/**
 * Contains the API for logging.
 *
 * Each Log has a unique moduleID to better control logging fidelity when debugging.
 * The initial release of the code will not support filtering by logging module id.
 *
 * Variadic functions are not used because:
 *  1. The MicroChip compiler (at least some versions) don't support Variadic functions
 *  2. Variadic functions in some compilers can result in code bloat
 *  3. Variadic functions are processor architecture dependent.  For example, if the variables to be
 *     stored are 64 bits, and if the first user variable ("var1"), and var2 is a uint32_t, then when the
 *     unpacking of the variables are done with va_arg(ap, uint64_t) all is well on a 64 bit machine.  But,
 *     on a 32 bit machine, both var1 and var2 are packed into 64 bits and we get the wrong value.
 *
 * As such, the approach of enforcing always passing 3 variables in all log statements is enforced by the
 * method prototype.
 *
 *
 * Caution:  On some 32 bit compilers, when a void* is type cast to a uint64_t, the upper bit is extended
 * rather than zero filling.  0xF1234567 when typecast to a uint64_t, results in 0xffffffffF1234567
 * instead of 0x00000000F1234567 as one might expect.
 */


#include "cefMappings.hpp"
#include "cefContract.hpp"


class Logging
{
    public:
        //! Constructor
        Logging() :
            m_loggingInProgress(false)
            { }

        typedef enum logModuleId
        {
            LogModuleIdCefInfrastructure,
			LogModuleIdCefDebugCommands,

        } logModuleId_t;


        /**
         *  Obtain a reference to the Logging Singleton.
         *
         *  @return a reference to the Logging Singleton
         */
        static Logging& instance();


        /**
         * Adds a logging message to the logging queue
         *      See note at top of this file as to why a variadic function is not being used
         *
         * @param logType       what type of log is this (debug, info...)
         * @param logModuleId   what module generated this log
         * @param message       pointer to a string that describes the console message for this log
         * @param fileName      The name of the file that generated this log
         * @param lineNum       The line number in the file this log came from
         * @param var1          1st user variable in log statement
         * @param var2          2nd user variable in log statement
         * @param var3          3rd user variable in log statement
         */
        void logMessage(logType_t logType, logModuleId_t logModuleId, const char* message, const char* fileName, uint32_t lineNum,
                        uint64_t var1, uint64_t var2, uint64_t var3);

        /**
         * After a log fatal message has been posted, this routine is responsible for
         * additional fatal log processing
         */
        void postHandlingOfFatalError();

    private:
        //! Flag that is true when logging is in progress (used to detect recursive logging call)
        bool m_loggingInProgress;

        //! rolling line Sequence Number
        static uint16_t m_logSequenceNumber;
        STATIC_ASSERT(sizeof(m_logSequenceNumber) == sizeof(cefLog_t::m_logSequenceNumber),\
                        LOG_CONTRACT_SEQUENCE_NUMBER_SIZE_DOES_NOT_MATCH_CONTRACT);
};


/**
 Guidelines for choosing logging macros

 LOG_DEBUG is intended to allow us to be as verbose as we want in log statements and will
      be compiled out for non-debug builds.  Debug builds could potentially only
      be available in simulation environments because if we use a lot of debug statements (which
      is the intention), it could cause code bloat and slow down performance.

 LOG_INFO is intended to be included in all builds, so use sparingly.  This is for high-level
      tracing (e.g., one line per command handler that lets us know we made it to that command handler).

 LOG_WARNING This is a level below log error, but used to indicate that something highly
	suspicious occurred and should be examined

 LOG_ERROR is intended to only be called when we are actually about to return a error code to
      GSW indicating that something really bad happened and we couldn't process a command
      or perform some task.

  LOG_FATAL is intended to be called when a catastrophic event has occurred and if execution of
      the code is allowed to continue, unexpected results could occur. As such the the minimal
      failure analysis is printed out and the system and execution is halted (likely a reset occurs).
*/

//! We only want the filename, and not the complete path name so it doesn't take so many characters to transmit
#define __JUST_FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef DEBUG_BUILD
#define LOG_DEBUG(logModuleId, msg, var1, var2, var3) \
    Logging::instance().logMessage(logTypeDebug, logModuleId, msg, __JUST_FILENAME__, __LINE__, var1, var2, var3);
#else
    // Compile out debug log statements for non-debug builds
    #define LOG_DEBUG(msg, ...)
#endif


#define LOG_INFO(logModuleId, msg, var1, var2, var3) \
    Logging::instance().logMessage(logTypeInfo, logModuleId, msg, __JUST_FILENAME__, __LINE__, var1, var2, var3);

#define LOG_ERROR(logModuleId, msg, var1, var2, var3) \
    Logging::instance().logMessage(logTypeError, logModuleId, msg, __JUST_FILENAME__, __LINE__, var1, var2, var3);

#define LOG_WARNING(logModuleId, msg, var1, var2, var3) \
    Logging::instance().logMessage(logTypeWarning, logModuleId, msg, __JUST_FILENAME__, __LINE__, var1, var2, var3);

#define LOG_FATAL(logModuleId, msg, var1, var2, var3) \
    /* First log the fact that something went badly */ \
    Logging::instance().logMessage(logTypeFatal, logModuleId, msg, __JUST_FILENAME__, __LINE__, var1, var2, var3); \
    /* do post handling of logging the fatal error (no return from this function) */ \
    Logging::instance().postHandlingOfFatalError();


/* End of c/c++ guard */
#ifdef __cplusplus
}
#endif

#endif  // end header guard
