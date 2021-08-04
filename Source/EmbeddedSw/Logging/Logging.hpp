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
 */

#include "cefMappings.hpp"
#include "cefContract.hpp"


class Logging
{
    public:

        typedef enum logModuleId
        {
            LogModuleIdCefInfrastructure,
			LogModuleIdCefDebugCommands,

        } logModuleId_t;

        /**
         * Adds a logging message to the logging queue
         *      See note at top of this file as to why a variadic function is not being used
         *
         * @param logType       what type of log is this (debug, info...)
         * @param logModuleId   what module generated this log
         * @param message       pointer to a string that describes the console message for this log
         * @param fileName      The name of the file that generated this log
         * @param lineNum       The line number in the file this log came from
         * @param Var1, Var2, Var3  Up to 3 variables that go with this log
         */
        void logMessage(logType_t logType, logModuleId_t logModuleId, char* message, char* fileName, uint32_t lineNum);
        void logMessage(logType_t logType, logModuleId_t logModuleId, char* message, char* fileName, uint32_t lineNum,
                        uint64_t var1);
        void logMessage(logType_t logType, logModuleId_t logModuleId, char* message, char* fileName, uint32_t lineNum,
                        uint64_t var1, uint64_t var2);
        void logMessage(logType_t logType, logModuleId_t logModuleId, char* message, char* fileName, uint32_t lineNum,
                        uint64_t var1, uint64_t var2, uint64_t var3);
};


// Don't use the LOG_MESSAGE macro directly, use one of the macros below
#define LOG_MESSAGE(logType, logModuleId, msg, __FILE__,__LINE__) \
            Logging::instance().logMessage(logType, logModuleId, msg, fileName, LineNumber);


#if 0
#define LOG_MESSAGE(logType, logModuleId, msg, __FILE__,__LINE__, Var1) \
            Logging::instance().logMessage(logType, logModuleId, msg, fileName, LineNumber, Var1);

#define LOG_MESSAGE(logType, logModuleId, msg, __FILE__,__LINE__, Var1, Var2) \
            Logging::instance().logMessage(logType, logModuleId, msg, fileName, LineNumber, Var1, Var2);
#define LOG_MESSAGE(logType, logModuleId, msg, __FILE__,__LINE__, Var1, Var2, Var3) \
            Logging::instance().logMessage(logType, logModuleId, msg, fileName, LineNumber, Var1, Var2, Var3);
#endif


//@todo implement LOG_MESSAGE in packet format...leaving spaceflight code to show what did in past
// for now, doing nothing...
#ifdef __SIMULATOR__
#define LOG_MESSAGE(level, logModuleId, msg, ...) \
	/* For now, printf, in future, route tracing through system messages to GSW to display */ \
	//printf("[%s] logModuleId %d %s:%d " msg "\n", level, logModuleId, __FILE__,__LINE__, ##__VA_ARGS__);

#else
#if 0
#define LOG_MESSAGE(level, logModuleId, msg, ...) \
	/* For now, printf, in future, route tracing through system messages to GSW to display */ \
    //xil_printf("[%s] logModuleId %d %s:%d " msg "\n", level, logModuleId, __FILE__,__LINE__, ##__VA_ARGS__);
#endif
#endif





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

#if 0
#ifdef DEBUG_BUILD
    LOG_DEBUG(logModuleId, msg)                     LOG_MESSAGE(logTypeDebug, logModuleId, msg)


    LOG_DEBUG(logModuleId, msg, Var1)               LOG_MESSAGE(logTypeDebug, logModuleId, Var1)
    LOG_DEBUG(logModuleId, msg, Var1, Var2)         LOG_MESSAGE(logTypeDebug, logModuleId, Var1, Var2)
    LOG_DEBUG(logModuleId, msg, Var1, Var2, Var3)   LOG_MESSAGE(logTypeDebug, logModuleId, Var1, Var2, Var3)
#else
    // Compile out debug log statements for non-debug builds
    #define LOG_DEBUG(msg, ...)
#endif



#define LOG_INFO(logModuleId, msg, ...) LOG_MESSAGE("INFO", logModuleId, msg, ##__VA_ARGS__)

#define LOG_ERROR(logModuleId, msg, ...) LOG_MESSAGE("ERROR", logModuleId, msg, ##__VA_ARGS__)

#define LOG_WARNING(logModuleId, msg, ...) LOG_MESSAGE("WARNING", logModuleId, msg, ##__VA_ARGS__)

#define LOG_FATAL(logModuleId, msg, ...) \
    /* First log the fact that something went badly */ \
    LOG_MESSAGE("ERROR", logModuleId, msg, ##__VA_ARGS__) \
    /* For now, assert.  On real hardware, we may do something else */ \
     while(1)
#if 0
	//!@todo Implement RUNTIME_ASSERT in cefMappings.h  ... will need to add continuation to line above
	RUNTIME_ASSERT(false);
#endif
#endif

#define LOG_INFO(logModuleId, msg, ...)

#define LOG_ERROR(logModuleId, msg, ...)

#define LOG_WARNING(logModuleId, msg, ...)

#define LOG_FATAL(logModuleId, msg, ...)




/* End of c/c++ guard */
#ifdef __cplusplus
}
#endif

#endif  // end header guard
