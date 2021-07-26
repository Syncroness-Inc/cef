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
#include "Cef/HwShim/ShimBase.hpp"
#include "Cef/Source/EmbeddedSw/DebugPort/FramingSignatureVerify.hpp"
#include "Cef/Source/EmbeddedSw/Logging/Logging.hpp"

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


void SerialPortDriverHwImpl::stopReceive()
{
	ShimBase::getInstance().forceStopReceive();
}
void SerialPortDriverHwImpl::receivedByteDriverHwCallback()
{
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
	//TODO we will have to stop receiving next byte once packet is done in Transport Layer
}
void SerialPortDriverHwImpl::receiveNextByte()
{
	if(mp_receiveBuffer != nullptr && m_currentBufferOffset <= m_receiveBufferSize)
	{
		ShimBase::getInstance().startInteruptReceive((mp_receiveBuffer + m_currentBufferOffset), this, &SerialPortDriverHwImpl::receivedByteDriverHwCallback);
	}

}

void SerialPortDriverHwImpl::setErrorCallback(void)
{
	ShimBase::getInstance().startErrorCallback(this, &SerialPortDriverHwImpl::errorCallback);
}

debugPortErrorCode_t SerialPortDriverHwImpl::errorCallback(debugPortErrorCode_t error)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Implement in CmdDebugPort");
}

