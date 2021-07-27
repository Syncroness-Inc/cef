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
	ShimBase::getInstance().startInteruptSend(sendBuffer, packetSize);
}

void SerialPortDriverHwImpl::startReceive(void* receiveBuffer,  int receiveSize)
{
	m_receiveBufferSize = receiveSize;
	mp_receiveBuffer = receiveBuffer;
	m_currentBufferOffset = 0;
	receiveNextByte();
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

void SerialPortDriverHwImpl::receivedByteDriverHwCallback()
{
	//See if receive has finnished 
	if(m_currentBufferOffset == m_receiveBufferSize)
	{
		//Router/TransportLayer job to know packet has been received
		//Stop receiving data till startReceive is invoked again
		return;
	}
	else if(m_currentBufferOffset > m_receiveBufferSize && m_currentBufferOffset < DEBUG_PORT_MAX_PACKET_SIZE_BYTES)
	{
		//Router/TransportLayer job to know packet has been received
		//Stop receiving data till startReceive is invoked again
		//Current Buffer Offset should not be greater then the expected buffer size Log a problem
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Received bytes exceeded expected did not overrun buffer.");
		return;
	}
	else if(m_currentBufferOffset >= DEBUG_PORT_MAX_PACKET_SIZE_BYTES) //current buffer 0-527, MAX_BUFFER 528 (= will overrun)
	{
		//Router/TransportLayer job to know packet has been received
		//Stop receiving data till startReceive is invoked again
		//Current Buffer Offset should not be greater then the expected buffer size Log a problem
		//Current buffer overran allocated buffer
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Received bytes exceeded expected and ALSO overrun buffer.");
		return;
	}

	//Receive has not finished
	//Check framing signature
	if(m_currentBufferOffset < sizeof(DEBUG_PACKET_UINT32_FRAMING_SIGNATURE))
	{
		m_currentBufferOffset = FramingSignatureVerify::checkFramingSignatureByte(mp_receiveBuffer, m_currentBufferOffset);
	}
	else //If past framing signature increment offset
	{
		m_currentBufferOffset++;
	}
	//Set up receive next byte
	receiveNextByte();
}

void SerialPortDriverHwImpl::receiveNextByte()
{
	if(mp_receiveBuffer != nullptr && m_currentBufferOffset <= m_receiveBufferSize)
	{
		ShimBase::getInstance().startInterruptReceive((mp_receiveBuffer + m_currentBufferOffset), this, &SerialPortDriverHwImpl::receivedByteDriverHwCallback);
	}

}

void SerialPortDriverHwImpl::setErrorCallback(void)
{
	ShimBase::getInstance().startErrorCallback(this, &SerialPortDriverHwImpl::errorCallback);
}

void SerialPortDriverHwImpl::errorCallback(errorCode_t error)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Implement in CmdDebugPort");
}

