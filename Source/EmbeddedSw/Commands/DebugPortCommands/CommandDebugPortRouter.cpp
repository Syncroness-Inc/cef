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

#include <new>      // for placement new

#include "CommandDebugPortRouter.hpp"
#include "Logging.hpp"
#include "AppMain.hpp"

/**
 * Implementation of CommandDebugPortRouterRouter Methods
 *
 * Logging:
 * 		When logging has a new log, it requests memory via checkoutLogBufferLogging()
 * 			Logging then fills in cefLog_t with logging information
 * 		Then the log is "returned" via checkinLogBufferLogging(), and added to a queue to transmit
 * 		checkoutLogTransmitBuffer() returns a pointer of the next log to transmit
 * 		Once the transmit has been completed, the buffer is returned via checkinLogTransmitBuffer()
 *
 * CEF Proxy Command
 * 		There is one buffer that is used for CEF Proxy Commands.
 * 		The buffer transitions between various states as it progresses through CEF Command processing
 * 		The buffer is "checked out" by an API, which transfers ownership (i.e. rights to modify) to who checked out
 * 		    the buffer.  When the buffer is "checked in", it transitions to the next state.
 */

/**
 * Maximum number of logging cefLogPacketAscii_t that can exist in the system at one time.
 * Caution:  Logging packets can be memory hogs, to be careful how big this number is
 */
static const uint32_t maxNumLoggingPackets = 10;

//! Singleton instantiation of CommandDebugPortRouter
static CommandDebugPortRouter commandDebugPortRouterSingleton(BufferPoolBase::BufferPoolId_Logging, sizeof(cefLog_t),
            maxNumLoggingPackets);

CommandDebugPortRouter::CommandDebugPortRouter(uint32_t logBufferPoolId, uint32_t numBytesPerLogEntry, uint32_t maxNumLogEntries) :
        CommandBase(commandOpCodeDebugPortRouter),
        m_logPool(logBufferPoolId, numBytesPerLogEntry, maxNumLogEntries),
        m_logsToSend(maxNumLogEntries),
        m_cefCommandBuffer(&m_cefCommand, sizeof(m_cefCommand)),
        m_cefLogBufferTransmit(nullptr, 0),
        mp_cefBufferTransmit(nullptr),
        m_cefCommandBufferState(cefCommandBufferState_bufferAvailable),
        m_fatalErrorHandling(false),
        m_executeActive(false)
{
}

CommandDebugPortRouter& CommandDebugPortRouter::instance()
{
    return commandDebugPortRouterSingleton;
}

bool CommandDebugPortRouter::execute(CommandBase *p_childCommand)
{
    bool commandDone = false;
    bool shouldYield = false;

    m_executeActive = true;

    validateNullChildResponse(p_childCommand);

    while (shouldYield == false)
    {
        switch (m_commandState)
        {
        case commandStateCommandEntry:
        {
            m_commandState = commandStateExecuteTransportFunctions;
            break;
        }
        case commandStateExecuteTransportFunctions:
        {
            // Only execute the receive if not in fatal handling mode
            if (m_fatalErrorHandling == false)
            {
                //@todo add in receive transport call
            }

            //@todo add in transmit transport call

            // Finished with transport functions; exit, and try transport functions again next time
            shouldYield = true;
            break;
        }
        case commandStateCommandComplete:		// DebugPort should run forever
        default:
        {
            // If we get here, we've lost our mind.
            LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Unhandled command state {:d}",
                    m_commandState, 0, 0);
            shouldYield = true;
            commandDone = true;
            break;
        }
        }
    }

    m_executeActive = false;
    return commandDone;
}

cefLog_t* CommandDebugPortRouter::checkoutLogBufferLogging()
{
    cefLog_t *p_cefLog = (cefLog_t*) m_logPool.allocate(sizeof(cefLog_t));

    if (p_cefLog == nullptr)
    {
        // No log buffers available; they must all be backed up in the transmit queue
        // Eventually we want to thrown away some data and make room for an overflow message
        return nullptr;
    }

    return p_cefLog;
}

void CommandDebugPortRouter::checkinLogBufferLogging(cefLog_t *p_cefLog)
{
    if (p_cefLog == nullptr)
    {
        LOG_FATAL(Logging::LogModuleIdCefDebugCommands, "A nullptr log buffer was returned!", 0, 0, 0);
    }

    // The p_cefLog is assumed to have valid logging data, and now is ready to be transmitted, so add
    // it to the log to send fifo.
    if (m_logsToSend.put(p_cefLog) == false)
    {
        // Something is messed up in the system setup as there should be room to send all logs we have buffer space for
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "m_logsToSend not setup correctly in CommandDebugPortRouter",
                0, 0, 0);
    }
}

cefLog_t* CommandDebugPortRouter::checkoutLogTransmitBuffer()
{
    void *p_cefLog = nullptr;

    if (m_logsToSend.get(p_cefLog) == false)
    {
        // No logs to send
        return nullptr;
    }

    return (cefLog_t*) p_cefLog;
}

void CommandDebugPortRouter::checkinLogTransmitBuffer(cefLog_t *p_cefLog)
{
    // If memory is attempted to be returned to a pool that it was not allocated from, then free() with trace fatal.
    m_logPool.free(p_cefLog);
}

CefBuffer* CommandDebugPortRouter::checkoutCefCommandReceiveBuffer()
{
    if (m_cefCommandBufferState != cefCommandBufferState_bufferAvailable)
    {
        // If the buffer is not available...
        return nullptr;
    }

    // Mark the packet as checked out
    m_cefCommandBufferState = cefCommandBufferState_receivingCommand;
    return &m_cefCommandBuffer;
}

void CommandDebugPortRouter::checkinCefCommandReceiveBuffer(CefBuffer *p_cefBuffer, errorCode_t cefCommandFetchStatus)
{
    // Sanity Check:  Confirm the memory was checked out appropriately in the first place
    if (m_cefCommandBufferState != cefCommandBufferState_receivingCommand)
    {
        // Attempting to return a packet that was not checked out!
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Attempt to return memory to CommandDebugPortRouter in wrong state{:d}, Allowed {:d}",
                m_cefCommandBufferState, cefCommandBufferState_receivingCommand, 0);
    }

    // Sanity Check:  Confirm that returning the right memory
    if (p_cefBuffer != &m_cefCommandBuffer)
    {
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Attempt to return unknown memory to CommandDebugPortRouter 0x{:x}",
                (uint64_t)p_cefBuffer, 0, 0);
    }

    // Sanity Check that we didn't overflow the buffer
    if (m_cefCommandBuffer.getNumberOfValidBytes() > m_cefCommandBuffer.getMaxBufferSizeInBytes())
    {
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Buffer overflow! valid={%d}, max={%d}",
                m_cefCommandBuffer.getNumberOfValidBytes(), m_cefCommandBuffer.getMaxBufferSizeInBytes(), 0);
    }

    // If we didn't successfully fetch a command, then the only choice we have is to try again
    if (cefCommandFetchStatus != errorCode_OK)
    {
        AppMain::instance().setSystemErrorCode(cefCommandFetchStatus);
        LOG_ERROR(Logging::LogModuleIdCefInfrastructure, "Failed to fetch CEF command.  Status = {%d}",
                cefCommandFetchStatus, 0, 0);
        m_cefCommandBufferState = cefCommandBufferState_bufferAvailable;
    }
    else
    {
        // This is a valid CEF Command, so change the state of the CEF Command Buffer
        // By design, the buffer should only be returned with a CEF Receive Command
        m_cefCommandBufferState = cefCommandBufferState_commandReceived;
    }
}

CefBuffer* CommandDebugPortRouter::checkoutCefCommandProxyProcessingBuffer()
{
    if (m_cefCommandBufferState != cefCommandBufferState_commandReceived)
    {
        return nullptr;
    }

    // Mark that the cefCommand Buffer (does not include packet header) is checked out to proxy command
    m_cefCommandBufferState = cefCommandBufferState_proxyCommandOwnsBuffer;
    return &m_cefCommandBuffer;
}

void CommandDebugPortRouter::checkinCefCommandProxyProcessingBuffer(CefBuffer *p_cefBuffer)
{
    // Sanity Check State
    if (m_cefCommandBufferState != cefCommandBufferState_proxyCommandOwnsBuffer)
    {
        // Attempting to return a packet that was not checked out!
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure,
                "Attempt to return memory to CommandDebugPortRouter in wrong state{:d}, Allowed {:d}",
                m_cefCommandBufferState, cefCommandBufferState_proxyCommandOwnsBuffer, 0);
    }

    // Sanity Check Address
    if (p_cefBuffer != &m_cefCommandBuffer)
    {
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure,
                "Attempt to return unknown memory 0x{:x} via returnCefCommandBufferForProxyProcessing, expected 0x{:x}",
                (uint64_t)p_cefBuffer, (uint64_t)&m_cefCommandBuffer, 0);
    }

    // Sanity Check numBytes makes sense
    if (m_cefCommandBuffer.getNumberOfValidBytes() > m_cefCommandBuffer.getMaxBufferSizeInBytes())
    {
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Illegal payload size={:d}, max allowed={:d}",
                m_cefCommandBuffer.getNumberOfValidBytes(), m_cefCommandBuffer.getMaxBufferSizeInBytes(), 0);
    }

    m_cefCommandBufferState = cefCommandBufferState_readyToTransmit;
}

CefBuffer* CommandDebugPortRouter::checkoutCefCommandTransmitBuffer()
{
    if (m_cefCommandBufferState != cefCommandBufferState_readyToTransmit)
    {
        return nullptr;
    }

    // Note:  The number of bytes to transmit is contained in m_cefCommandBuffer.getNumberOfValidBytes()

    m_cefCommandBufferState = cefCommandBufferState_transmittingBuffer;

    return &m_cefCommandBuffer;
}

void CommandDebugPortRouter::checkinCefCommandTransmitBuffer(CefBuffer *p_cefBuffer)
{
    // Sanity Check State
    if (m_cefCommandBufferState != cefCommandBufferState_transmittingBuffer)
    {
        // Attempting to return a packet that was not checked out!
        LOG_FATAL(Logging::LogModuleIdCefDebugCommands,
                "Attempt to return memory to CommandDebugPortRouter in wrong state{:d}, Allowed {:d}",
                m_cefCommandBufferState, cefCommandBufferState_transmittingBuffer, 0);
    }

    // Sanity Check Address
    if (p_cefBuffer != &m_cefCommandBuffer)
    {
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure,
                "Attempt to return unknown memory to Cef Command Transmit Buffer.  actual 0x{:x}, expected 0x{:x}",
                (uint64_t)p_cefBuffer, (uint64_t)&m_cefCommandBuffer, 0);
    }

    // All done with the buffer; mark buffer as being available
    m_cefCommandBufferState = cefCommandBufferState_bufferAvailable;

    // Reset the valid bytes to aid debug as the next step is to receive another command
    m_cefCommandBuffer.setNumberOfValidBytes(0);
}

CefBuffer* CommandDebugPortRouter::checkoutCefTransmitBuffer(debugPacketDataType_t &debugDataType)
{
    // Is there already a command checked out?
    if (mp_cefBufferTransmit != nullptr)
    {
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Attempt to checkout CEF Buffer to transmit when one is already checked out!",
                0, 0, 0);
    }

    // Initialize the return values
    debugDataType = debugPacketType_invalid;
    mp_cefBufferTransmit = nullptr;   //!<  Yes, the above nullptr check confirms this, but for defensive coding setting up anyhow.

    // Is there a Command Response waiting to be sent?
    if (m_cefCommandBufferState == cefCommandBufferState_readyToTransmit)
    {
        mp_cefBufferTransmit = checkoutCefCommandTransmitBuffer();
        if (mp_cefBufferTransmit == nullptr)
        {
            LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Programming Error:  Unexpected nullptr for CefCommand Response!",
                    0, 0, 0);
        }
        debugDataType = debugPacketType_commandResponse;
    }
    // Is there a log waiting to be sent?
    else if (m_logsToSend.isEmpty() == false)
    {
        cefLog_t *p_cefLog = checkoutLogTransmitBuffer();
        if (p_cefLog == nullptr)
        {
            LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Programming Error:  Unexpected nullptr for CefCommand Response!", 0, 0, 0);
        }

        // Need to convert to CefBuffer
        mp_cefBufferTransmit = (CefBuffer*) new ((void*) &m_cefLogBufferTransmit) CefBuffer((void*) p_cefLog, sizeof(cefLog_t));
        if (mp_cefBufferTransmit == nullptr)
        {
            LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Programming Error:  Unexpected nullptr for Log Transmit!", 0, 0, 0);
        }

        // Setup the number of valid bytes in the log buffer
        mp_cefBufferTransmit->setNumberOfValidBytes(sizeof(cefLog_t));

        debugDataType = debugPacketType_loggingData;
    }

    return mp_cefBufferTransmit;
}

void CommandDebugPortRouter::checkinCefTransmitBuffer(CefBuffer *p_cefBuffer)
{
    if (p_cefBuffer == nullptr)
    {
        LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Programming Error:  Request to return nullptr for Log Transmit Buffer!",
                0, 0, 0);
    }

    // Is this the Cef Command Buffer?
    if (p_cefBuffer == &m_cefCommandBuffer)
    {
        checkinCefCommandTransmitBuffer(p_cefBuffer);
    }
    // Then this must be a log buffer
    else
    {
        cefLog_t *p_cefLog = (cefLog_t*) p_cefBuffer->getBufferStartAddress();
        checkinLogTransmitBuffer(p_cefLog);
    }
}

void CommandDebugPortRouter::discardOlderLogs()
{
    // What percentage of the logs should we discard (33 is 33%) to make room for more logs
    const uint32_t percentToDiscard = 33;

    uint32_t numLogsToDiscard = (m_logsToSend.getMaximumNumberOfElements() * percentToDiscard) / 100;

    uint32_t numCurrentLogs = m_logsToSend.getCurrentNumberOfEntries();

    numLogsToDiscard = MIN(numLogsToDiscard, numCurrentLogs);

    // Remove log from send list, and return it to allocate list
    for (uint32_t i = 0; i < numLogsToDiscard; ++i)
    {
        void* p_log;
        if (m_logsToSend.get(p_log) == false)
        {
            // This is unexpected as should be enough logs to fetch...but try to continue continue
            LOG_ERROR(Logging::LogModuleIdCefInfrastructure, "Failed to discard {:d} logs, only discarded {:d} logs",
                    numLogsToDiscard, i, 0);
            break;
        }

        // Return the log memory to be used for a "new" log
        m_logPool.free((cefLog_t*)p_log);
    }
}

void CommandDebugPortRouter::fatalErrorHandlingLoop()
{
    /**
     * This routine is intended to be called from an infinite while loop after a
     * fatal error has occurred.  It is intended to provide whatever minimal services are
     * required to transmit the failed logs out the debug port.  As there are may be
     * other objects that provide fatal error handling as well, this function is intended
     * to do some work, and exit out when done.
     */

    m_fatalErrorHandling = true;

    /** if m_executeActive is true, then the fatal error occurred while the Debug Router's execute()
     * was active. If we call execute() again then we are re-entering a function whose design
     * intent was to run to completion.  If we continue to call execute() we could destroy
     * the forensic evidence of what cause the error in the first place.
     */
    if (m_executeActive == false)
    {
        execute(nullptr);
    }
}
