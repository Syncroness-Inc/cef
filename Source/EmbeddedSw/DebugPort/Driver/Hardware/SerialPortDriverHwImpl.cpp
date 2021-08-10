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


#include "SerialPortDriverHwImpl.hpp"
#include "ShimBase.hpp"
#include "FramingSignatureVerify.hpp"
#include "Logging.hpp"

void SerialPortDriverHwImpl::sendData(void* sendBuffer, int packetSize)
{
	ShimBase::getInstance().startInterruptSend(sendBuffer, packetSize);
}

uint32_t SerialPortDriverHwImpl::getCurrentBytesReceived(void)
{
	return m_currentBufferOffset;
}

bool SerialPortDriverHwImpl::getSendInProgress(void)
{
	return ShimBase::getInstance().getSendInProgress();
}

bool SerialPortDriverHwImpl::startReceive(void* receiveBuffer, uint32_t receiveSize)
{
	m_receiveBufferSize = receiveSize;
	mp_receiveBuffer = receiveBuffer;
	m_currentBufferOffset = 0;
	if((mp_receiveBuffer != nullptr) && (m_currentBufferOffset < m_receiveBufferSize))
	{
		return armReceiveNextByte();
	}
	else
	{
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Start debug receive as null buffer or overrun max buffer size.",
		        0, 0, 0);
		return false;
	}
}

void SerialPortDriverHwImpl::editReceiveSize(uint32_t newReceiveSize)
{
    /**
     * It is the responsibility of the calling routine to ensure that
     * that mp_receiveBuffer + newReceiveSize does not overflow as the
     * the SerialPortDriverHwImpl does not have the knowledge to make
     * this decision.
     */
    m_receiveBufferSize = newReceiveSize;

	if(m_currentBufferOffset >= newReceiveSize)
	{
		stopReceive();
	}
}

void SerialPortDriverHwImpl::stopReceive()
{
	ShimBase::getInstance().forceStopReceive();
}

bool SerialPortDriverHwImpl::receivedByteDriverHwCallback()
{
	/**
	 * Keep looking for the framing signature and resetting where add to
	 * buffer until the beginning of the buffer contains a complete framing
	 * signature.
	 * After a framing signature is found, continue to add data to the buffer until
	 * run out of buffer space
	 */

	if(m_currentBufferOffset < numElementsInDebugPacketFramingSignature)
	{
		/**Check framing signature can return increment if the byte matches the framing signature
		 * Or it can return "0" if the framing signature is not correct and the buffer offset needs to be 
		 * set back to starting point.*/
		m_currentBufferOffset = FramingSignatureVerify::checkFramingSignatureByte(mp_receiveBuffer, m_currentBufferOffset);
	}
	else // If past framing signature, increment offset for the next buffer receive point
	{
		m_currentBufferOffset++;
	}

	//Check to see if receive is finished/ buffer is full
	if(m_currentBufferOffset >= m_receiveBufferSize)
	{
		/**Router/TransportLayer job to know when a complete packet has been received.
		 * Stop receiving data till startReceive is invoked again.
		 *
		 * It is the responsibility of the transport layer to make sure the buffer
		 * is big enough to receive a complete packet.  The transport layer should start
		 * out with a buffer big enough for the largest expected packet size.
		 * */
		return false;
	}

	//Receive has not finished/still room left in the buffer
	//Set up receive next byte
	return armReceiveNextByte();
}

bool SerialPortDriverHwImpl::armReceiveNextByte()
{
	/**Error should be handled by the higher level routine that calls receiveNextByte.
	 * We do a final check to make sure we are not over-writing memory here.
	 */
	if(mp_receiveBuffer != nullptr && m_currentBufferOffset < m_receiveBufferSize)
	{
	    uint8_t* p_receiveMemoryAddress = (uint8_t*) mp_receiveBuffer + m_currentBufferOffset;
		ShimBase::getInstance().startInterruptReceive(p_receiveMemoryAddress,
		        this, &SerialPortDriverHwImpl::receivedByteDriverHwCallback);
		return true;
	}
	return false;
}

void SerialPortDriverHwImpl::setErrorCallback(void)
{
	ShimBase::getInstance().startErrorCallback(this, &SerialPortDriverHwImpl::errorCallback);
}

void SerialPortDriverHwImpl::errorCallback(errorCode_t error)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Implement in CmdDebugPort",
	        0, 0, 0);
}

