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
 * Implementation of Logging methods
 */

#include "Logging.hpp"
#include "CommandDebugPortRouter.hpp"
#include "AppMain.hpp"

uint16_t Logging::m_logSequenceNumber = 0;

//! Singleton declaration of the Logging
static Logging  loggingSingleton;

Logging& Logging::instance()
{
    return loggingSingleton;
}

void Logging::logMessage(logType_t logType, logModuleId_t logModuleId, const char* message,
        const char* fileName, uint32_t lineNum, uint64_t var1, uint64_t var2, uint64_t var3)
{
    /**
     * We could end up recursively calling into logMessage if within the logging infrastructure
     * we trigger another log.  There is only a small amount of code, that is hopefully highly tested,
     * so hopefully this situation only occurs during initial system debug.  Still, we need a way
     * to protect ourselves from a stack overflow from many recursive calls into logging.
     *
     * If we get into a recursive situation, then we are in deep trouble, and currently don't
     * have a good way of notifying anyone of the situation.  So, we will just go into an
     * infinite while loop and can either debug the failure with a debugger, or wait until
     * the watchdog timer resets us.
     */
    if (m_loggingInProgress == true)
    {
        // See note above why we will just wait until a debugger is hooked up or
        // the watchdog timer expires
        AppMain::instance().setSystemErrorCode(errorCode_LoggingCalledRecursively);
        while (1);
    }

    // Mark that logging is in progress
    m_loggingInProgress = true;

    /** Until we have the real time clock working, pick an arbitrary timestamp to start with.
     *  Then increment the time stamp by a fixed amount each time so we can see that the time 
     * is changing with each log entry in python utilities.
     */
    static uint64_t fakeTimestamp = (29.720638091 * (float)(LOGGING_UINT64_NSEC_TO_SECONDS));
    //increment just a bit more than a 0.1 seconds so can always see that digit move
    static uint64_t fakeTimestampIncrement = (0.139124163 * (float)(LOGGING_UINT64_NSEC_TO_SECONDS));

    // Allocate a log
    cefLog_t* p_log = CommandDebugPortRouter::instance().checkoutLogBufferLogging();
    if (p_log == nullptr)
    {
        /**
         * If we can't allocate a log, then we are likely generating logs faster than the
         * logs can be transmitted.  So, we will throw away some of the "older" logs as the
         * "newer" logs are likely to tell us why we are transmitting so much data.  Plus,
         * This means that a log_fatal always has room to be added to the queue (and any
         * error messages that may have occurred before the fatal are reported).
         *
         * The python code is responsible for detecting a discontinuity and adding an appropriate message
         * to the user console output when logs are "dropped".  The discontinuity is detected by
         * looking at m_logSequenceNumber.
         */
        CommandDebugPortRouter::instance().discardOlderLogs();

        // Try again to get a log
        p_log = CommandDebugPortRouter::instance().checkoutLogBufferLogging();
        if (p_log == nullptr)
        {
            /**
             * We can either
             *      1. exit and try to get a log buffer next time.  This is unlikely to succeed
             *         as we "should" always be able to create log space.  Hence, we are making
             *         a decision to proceed in an error case (which could make debug hard.)
             *      2. Enter an infinite while loop so a debugger can be connected to see what
             *         is going on.
             *      3. Do option 1, but set the system error state to an error to aid debug.
             *         A debugger can then be used to look at the system error code (or possibly
             *         be reported in a debug command).
             *
             * Option 3 will be implemented, for now.
             */
            AppMain::instance().setSystemErrorCode(errorCode_UnableToCreateLoggingSpace);
            m_loggingInProgress = false;
            return;
        }
    }

    // Construct the header
    p_log->m_header.m_commandErrorCode = errorCode_OK;
    p_log->m_header.m_commandNumBytes = sizeof(cefLog_t);
    p_log->m_header.m_commandOpCode = commandOpCodeNone;
    p_log->m_header.m_commandRequestResponseSequenceNumberPython = 0;   // not used for logging
    p_log->m_header.m_padding1 = 0;

    // Fill in the log variables
    p_log->m_logType = logType;
    p_log->m_moduleId = logModuleId;
    p_log->m_logVariable1 = var1;
    p_log->m_logVariable2 = var2;
    p_log->m_logVariable3 = var3;
    p_log->m_logSequenceNumber = m_logSequenceNumber++;

    //@todo once have clocks working, add time stamp here
    p_log->m_timeStamp = fakeTimestamp;
    fakeTimestamp += fakeTimestampIncrement;

    p_log->m_fileLineNumber = lineNum;

    // Careful to not overflow the allocated space, and to add null character if necessary
    size_t maxBytesToCopy = sizeof(p_log->m_fileName);
    strncpy(p_log->m_fileName, fileName, maxBytesToCopy);
    p_log->m_fileName[maxBytesToCopy - 1] = '\0';  // strncpy doesn't add null if string is bigger than maxBytesToCopy

    maxBytesToCopy = sizeof(p_log->m_logString);
    strncpy(p_log->m_logString, message, maxBytesToCopy);
    p_log->m_logString[maxBytesToCopy - 1] = '\0';  // strncpy doesn't add null if string is bigger than maxBytesToCopy

    // Return/checkin the log
    CommandDebugPortRouter::instance().checkinLogBufferLogging(p_log);

    // Return the log in use mutex
    m_loggingInProgress = false;
}

void Logging::postHandlingOfFatalError()
{
    /**
     * We have added a message to the log queue to transmit, but if we go into an infinite while
     * loop we will not transmit the message.  Something bad has happened to cause the trace fatal
     * so it might be risky to try to send information out the debug port.  On the other hand, if
     * we don't, debug becomes very difficult.  So, this code will attempt to keep the DebugPort transmit
     * alive, while not exercising the rest of the code.  This "should" be relatively low risk as
     * it is a small portion of the code that is extensively used and tested.  There is the possibility
     * that it is the DebugPort Transmit code that caused the problem, so a "in use" flag needs to be
     * put in the entrance of the DebugPort Transmit code to detect this situation and just enter an
     * infinite while loop...it is the least worse choice for something that hopefully never happens.
     *
     * Future consideration:  In "release" builds, it "may" be better to just go into an infinite
     * while loop.  If persistent memory is available, then dumping the log buffer to persistent memory
     * is another alternative.
     */

    // Set the system error code to aid debug
    AppMain::instance().setSystemErrorCode(errorCode_TraceFatalEncountered);

    while (1)
    {
        // Loop forever trying to send all the log data out to help debug
        CommandDebugPortRouter::instance().fatalErrorHandlingLoop();
    }
}

