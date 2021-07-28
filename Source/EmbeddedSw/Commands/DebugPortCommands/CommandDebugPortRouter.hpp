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
 * 1. Coordinating the use use of Logging and Cef Command buffers
 * 2. Instantiating the DebugPortTransport Layer
 * 3. Providing appropropriate public interfaces to
 * 		a. Logging
 * 		b. Cef Command Proxy
 */

#include "CommandBase.hpp"
#include "BufferPoolBase.hpp"
#include "cefContract.hpp"

class CommandDebugPortRouter : public CommandBase
{
	public:
		/**
		 * Constructor
		 *
		 * @param logBufferPoolId  log buffer pool ID (for debug), from BufferPoolBase::BufferPoolIdxx
		 * @param numBytesPerLoggingEntry  Number of bytes per logging entry
		 * @param maxNumLoggingEntries  Maximum number of log entries to keep in the system at one time
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
		 * Checks out a logging buffer that must be returned once the logging data is filled in
		 * 		Note that this is a cefLogging_t, NOT a cefLoggingPacket_t
		 *
		 * @return returns nullptr if no buffer is available; pointer to logging structure otherwise
		 */
		//*cefLogging_t checkoutLogBuffer();
		cefLogging_t* checkoutLogBuffer();

		/**
		 * Returns a cefLogging_t logging pointer that was previously checked out
		 * 		Note:  It is a fatal error to return memory that was not previously checked out from
		 * 		the Command Debug Port Router
		 * 		Note:  It is assumed that the buffer is returned with valid log data
		 *
		 * 	@param pointer to buffer that was checked out with checkoutLogBuffer
		 */
		void returnLogBuffer(cefLogging_t* p_cefLogBuffer);

		/**
		 * Checks out a logging packet for transmitting.  It is assumed that the buffer contains
		 * valid cefLogging_t data
		 *
		 * @return nullptr if no logging packet is available, pointer to logging packet otherwise
		 */
		cefLoggingPacket_t* checkoutLogPacket();

		/**
		 * Returns a cefLoggingPacket_t pointer that was previously checked out
		 * 		Note:  It is a fatal error to return memory that was not previously checked out from
		 * 		the Command Debug Port Router
		 * 		It is assumed that the Log Packet is no longer needed when checked back in
		 *
		 * 	@param pointer to buffer that was checked out with checkoutLogBuffer
		 */
		void returnLogPacket(cefLoggingPacket_t* p_cefLogBuffer);


		/**
		 * Checks out the CEF Command Packet Buffer to receive a command.
		 * 		It is assumed that upon check in of the buffer, there is a valid CEF command in the buffer
		 *
		 * @return nullptr if the CEF packet buffer is not available; pointer to cefCommandPacketMaximum_t struct otherwise
		 */
		cefCommandPacketMaximum_t*  checkoutCefReceiveCommandPacket();

		/**
		 * Returns CEF Command Packet Buffer with a valid CEF Command
		 * 		It is a fatal error to return anything other the the CEF command buffer
		 *
		 * @param p_cefCommandPacket 	pointer to CEF command packet buffer
		 */
		void returnCefReceiveCommandPacket(cefCommandPacketMaximum_t*);

		/**
		 * Gets a buffer containing the CEF command to be processed.  It is assumed the buffer contains a cef command request.
		 * 	The buffer is described by cefCommandMaximum_t
		 * 	This routine is typically called by the Embedded Sw routine responsible for processing a CEF Command
		 *
		 * @return nullptr if the CEF command buffer is not available; pointer to cefCommandMaximum_t struct otherwise
		 */
		cefCommandMaximum_t* checkoutCefCommandBuffer();

		/**
		 * Returns the CEF command buffer than now contains a CEF command response
		 *   This routine is typically called by the Embedded Sw routine responsible for processing a CEF Command
		 *
		 * @param p_cefCommandPacket 	pointer to CEF command buffer
		 * @param numBytesInCommandResponse	 Number of valid bytes in the command response (including the header)
		 */
		void returnCefCommandBuffer(cefCommandMaximum_t* p_cefCommandResponse, uint32_t numBytesInCommandResponse);

		/**
		 * Gets the CEF command packet buffer that has a CEF Command Response payload to Transmit
		 *
		 * @param numBytesInPayload
		 *
		 * @return nullptr if the CEF command buffer is not available, pointer to cefCommandMaximum_t struct otherwise
		 * 				the cefCommandMaximum_t has a valid payload to transfer
		 */
		cefCommandPacketMaximum_t* checkoutCefTransmitCommandPacket(uint32_t& numBytesInPayload);

		/**
		 * Returns the CEF command packet buffer.  The DebugPortRouter now owns this buffer and can do
		 * 		with the buffer whatever it pleases.
		 *
		 * @param p_cefCommandPacket	pointer to cefCommandPacketMaximum_t to be returned
		 */
		void returnCefTransmitCommandPacket(cefCommandPacketMaximum_t* p_cefCommandPacket);


	private:

        //! Command states
        enum
        {
            commandStateExecuteTransportFunctions = commandStateFirstDerivedState,
        };

        //! CEF Command Packet State.  The states are expected to progress sequentially.
        enum
		{
        	cefCommandPacketState_bufferAvailable,
			cefCommandPacketState_receivingPacket,
			cefCommandPacketState_packetReceived,
			cefCommandPacketState_proxyCommandOwnsPacket,
			cefCommandPacketState_readyToTransmit,
			cefCommandPacketState_transmittingPacket
		};


        //! Pool of cefLoggingPacketAscii_t to allocate for logging
        BufferPoolBase m_loggingPool;

        //! List of logs that have been filled out and ready to be sent.
        RingBufferOfVoidPointers m_logsToSend;

        //! There is only one CEF command that can be in existence at one time
        cefCommandPacketMaximum_t m_cefCommandPacket;

        //! Number of valid bytes in the cef Command response (including the header)
        uint32_t m_numBytesInCefCommandResponse;

        //! cefCommandPacket memory's state
        uint32_t m_cefCommandPacketState;
};

#endif  // end header guard
