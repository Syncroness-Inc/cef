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

bool SerialPortDriverHwImpl::startReceive(void* receiveBuffer,  int receiveSize)
{
	m_receiveBufferSize = receiveSize;
	mp_receiveBuffer = receiveBuffer;
	m_currentBufferOffset = 0;
	if(mp_receiveBuffer != nullptr && m_currentBufferOffset < DEBUG_PORT_MAX_PACKET_SIZE_BYTES)
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

void SerialPortDriverHwImpl::editReceiveSize(int newReceiveSize)
{
	if(m_currentBufferOffset >= newReceiveSize)
	{
		stopReceive();
	}
	else if(newReceiveSize >= DEBUG_PORT_MAX_PACKET_SIZE_BYTES)
	{
		m_receiveBufferSize = DEBUG_PORT_MAX_PACKET_SIZE_BYTES;
	}
	else
	{
		m_receiveBufferSize = newReceiveSize;
	}

}

void SerialPortDriverHwImpl::stopReceive()
{
	ShimBase::getInstance().forceStopReceive();
}

bool SerialPortDriverHwImpl::receivedByteDriverHwCallback()
{
	//See if receive has finnished 
	if(m_currentBufferOffset >= (m_receiveBufferSize - 1) && m_currentBufferOffset < DEBUG_PORT_MAX_PACKET_SIZE_BYTES)
	{
		/**Router/TransportLayer job to know packet has been received
		 * Stop receiving data till startReceive is invoked again
		 * buffer offset is not expected to be greater then expected receive buffer but this will fail silently because 
		 * it memory is allocated for DEBUG_PORT_MAX_PACKET_SIZE_BYTES before overruning the buffer
		 * packet size is not known till packet header is unwrapped.  If this takes longer then receiving the 
		 * entire packet we could go above expected size*/
		return false;
	}
	else if(m_currentBufferOffset >= DEBUG_PORT_MAX_PACKET_SIZE_BYTES) //current buffer 0-527, MAX_BUFFER 528 (= will overrun)
	{
		/**Router/TransportLayer job to know packet has been received
		 * Stop receiving data till startReceive is invoked again
		 * Current buffer overran allocated buffer*/
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Received bytes exceeded expected and ALSO overrun buffer.",
		        0, 0, 0);
		return false;
	}

	//Receive has not finished
	//Check framing signature
	if(m_currentBufferOffset < sizeof(DEBUG_PACKET_UINT32_FRAMING_SIGNATURE))
	{
		/**Check framing signature can return incroment if the byte matches the framing signature
		 * Or it can return "0" if the framing signature is not correct and the buffer offset needs to be 
		 * set back to starting point.*/
		m_currentBufferOffset = FramingSignatureVerify::checkFramingSignatureByte(mp_receiveBuffer, m_currentBufferOffset);
	}
	else //If past framing signature increment offset
	{
		m_currentBufferOffset++;
	}
	//Set up receive next byte
	return armReceiveNextByte();
}

bool SerialPortDriverHwImpl::armReceiveNextByte()
{
	/**Error should be taken care of when receiveNextByte is called this is a last min check so
	 * we do not write over memory.
	 * All Fatal errors are handled in function that calls*/
	if(mp_receiveBuffer != nullptr && m_currentBufferOffset < m_receiveBufferSize)
	{
		ShimBase::getInstance().startInterruptReceive((mp_receiveBuffer + m_currentBufferOffset), this, &SerialPortDriverHwImpl::receivedByteDriverHwCallback);
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

