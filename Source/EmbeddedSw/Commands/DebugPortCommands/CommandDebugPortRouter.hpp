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
#ifndef __CEF_COMMAND_DEBUG_PORT_ROUTER_H
#define __CEF_COMMAND_DEBUG_PORT_ROUTER_H


/**
 * Interface definition for Command Debug Port Router
 *
 * The Debug Port Router is responsible:
 * 1. Coordinating the use use of Log and Cef Command buffers
 * 2. Instantiating the DebugPortTransport Layer
 * 3. Providing appropropriate public interfaces to
 * 		a. Logging
 * 		b. Cef Command Proxy
 */

#include "CommandBase.hpp"
#include "BufferPoolBase.hpp"
#include "cefContract.hpp"
#include "CefBuffer.hpp"

class CommandDebugPortRouter : public CommandBase
{
	public:
		/**
		 * Constructor
		 *
		 * @param logBufferPoolId  		log buffer pool ID (for debug), from BufferPoolBase::BufferPoolIdxx
		 * @param numBytesPerLogEntry  	Number of bytes per log entry
		 * @param maxNumLogEntries  	Maximum number of log entries to keep in the system at one time
		 */
		CommandDebugPortRouter(uint32_t logBufferPoolId,
				               uint32_t numBytesPerLoggingEntry,
				               uint32_t maxNumLoggingEntries);

		/**
		 *  Obtain a reference to the Command Debug Port.
		 *
		 *  @return a reference to the Command Debug Port
		 */
		static CommandDebugPortRouter& instance();


		//! See base class for method description
		bool execute(CommandBase* p_childCommand);

		//! Note:  CommandDebugPortRouter is not a CEF command, so it does not have an import/export method implemented


		/**
		 * Checks out a log buffer that must be returned once the log data is filled in
		 *
		 * @return returns nullptr if no buffer is available; pointer to log structure otherwise
		 */
		//*cefLog_t checkoutLogBuffer();
		cefLog_t* checkoutLogBufferLogging();

        /**
         * Returns a cefLog_t log pointer that was previously checked out
         *      Note:  It is a fatal error to return memory that was not previously checked out from
         *      the Command Debug Port Router
         *      Note:  It is assumed that the buffer is returned with valid log data
         *
         *  @param pointer to buffer that was checked out with checkoutLogBuffer
         */
        void checkinLogBufferLogging(cefLog_t* p_cefLogBuffer);

		/**
		 * Checks out the CEF Command Buffer to receive a command.
		 * 		It is assumed that upon check in of the buffer, there is a valid CEF command in the buffer
		 *
		 * @return nullptr if the CEF command buffer is not available; pointer to CefBuffer otherwise
		 */
		CefBuffer* checkoutCefCommandReceiveBuffer();

		/**
		 * Returns CEF Command Buffer with a valid CEF command
		 * 		It is a fatal error to return anything other the the CEF command buffer
		 *
		 * @param p_cefBuffer 	pointer to CEF command buffer
		 */
		void checkinCefCommandReceiveBuffer(CefBuffer* p_cefBuffer);

		/**
		 *  Gets a buffer containing the CEF command to be processed.  It is assumed the buffer contains a cef command request.
		 * 	This routine is called by the Embedded Sw routine responsible for processing a CEF Command
		 *
		 * @return nullptr if the CEF command buffer is not available; pointer to CefBuffer otherwise
		 */
		CefBuffer* checkoutCefCommandProxyProcessingBuffer();

		/**
		 * Returns the CEF command buffer than now contains a CEF command response
		 *   This routine is typically called by the Embedded Sw routine responsible for processing a CEF Command
		 *
		 * @param p_cefBuffer 	pointer to CEF command buffer that was previously checked out
		 */
		void checkinCefCommandProxyProcessingBuffer(CefBuffer* p_cefCommand);

		/**
		 * Gets the CEF buffer to transmit next
		 *      Note:  CefBuffer.getNumberOfValidBytes() contains the number of bytes to transmit
		 *
		 * @param debugDataType  what type of data is being transmitted (returned as a reference)
		 *
		 * @return nullptr if the CEF command buffer is not available, pointer to CefBuffer otherwise
		 */
		CefBuffer* checkoutCefTransmitBuffer(debugPacketDataType_t& debugDataType);

		/**
		 * Returns the CEF command buffer after transmit has been completed.
		 *      The DebugPortRouter now owns this buffer and can do
		 * 	    with the buffer whatever it pleases.
		 *
		 * @param p_cefBuffer 	pointer to cefBuffer that was previously checked out for transmitting
		 */
		void checkinCefTransmitBuffer(CefBuffer* p_cefBuffer);


	private:

        //! Command states
        enum
        {
            commandStateExecuteTransportFunctions = commandStateFirstDerivedState,
        };

        //! CEF Command Buffer State.  The states are expected to progress sequentially.
        enum
		{
        	cefCommandBufferState_bufferAvailable,
			cefCommandBufferState_receivingCommand,
			cefCommandBufferState_commandReceived,
			cefCommandBufferState_proxyCommandOwnsBuffer,
			cefCommandBufferState_readyToTransmit,
			cefCommandBufferState_transmittingBuffer
		};


        /**
         * Checks out next cefLog_t buffer for transmitting logging information
         *
         * @return nullptr if there is no logging data to be transmitted, valid pointer otherwise
         */
        cefLog_t* checkoutLogTransmitBuffer();

        /**
         * Returns a cefLog_t pointer of log data that was previously checked out for transmitting
         *      Note:  It is a fatal error to return memory that was not previously checked out from
         *      the Command Debug Port Router
         *      It is assumed that the buffer can be re-used for logging when checked back in
         *
         *  @param pointer to cefLog_t that was checked out with checkoutLogTransmitBuffer
         */
        void checkinLogTransmitBuffer(cefLog_t* p_cefLog);


        /**
         * Gets the CEF command buffer to transmit next
         *      Note:  CefBuffer.getNumberOfValidBytes() contains the number of bytes to transmit
         *
         * @param debugDataType  what type of data is being transmitted
         *
         * @return nullptr if the CEF command buffer is not available, pointer to CefBuffer otherwise
         */
        CefBuffer* checkoutCefCommandTransmitBuffer();

        /**
         * Returns the CEF command buffer after transmit has been completed.
         *      The DebugPortRouter now owns this buffer and can do
         *      with the buffer whatever it pleases.
         *
         * @param p_cefBuffer   pointer to cefBuffer that was previously checked out for transmitting
         */
        void checkinCefCommandTransmitBuffer(CefBuffer* p_cefBuffer);


        //! Pool of cefLog_t to allocate for logging
        BufferPoolBase m_logPool;

        //! List of logs that have been filled out and ready to be sent.
        RingBufferOfVoidPointers m_logsToSend;

        //! There is only one CEF command that can be in existence at one time
        //! Reserve space for this command here
        uint8_t m_cefCommand[DEBUG_PORT_MAX_APPLICATION_PAYLOAD];

        //! The CefBuffer that describes the CEF command.
        //!     CefBuffer.getNumberOfValidBytes() describes how many valid bytes are in the buffer
        CefBuffer m_cefCommandBuffer;

        //! CefBuffer that describes the log being transmitted (re-initialized for each transmit log)
        CefBuffer m_cefLogBufferTransmit;

        //! This is used as a sanity check to make sure in correct state
        CefBuffer* mp_cefBufferTransmit;

        //! Number of valid bytes in the cef Command response (including the header)
        uint32_t m_numBytesInCefCommandResponse;

        //! cefCommandPacket memory's state
        uint32_t m_cefCommandBufferState;
};

#endif  // end header guard
