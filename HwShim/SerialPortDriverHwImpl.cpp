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
 * Implementation of ShimBase functions
 * Note:  Pure virtual functions can cause the compiler/linker to pull in
 * an excess amount of code.  So, pure virtual functions should be implemented to
 * fail at run time instead (not ideal, but the least bad option).
 */

#include "SerialPortDriverHwImpl.hpp"
#include "Cef/HwShim/ShimBase.hpp"

void SerialPortDriverHwImpl::sendData(void* sendBuffer, int packetSize)
{
	ShimBase::getInstance().startInteruptSend(sendBuffer, packetSize);
}


void SerialPortDriverHwImpl::startRecieve(void* recieveBuffer,  int recieveSize = DEBUG_PORT_MAX_PACKET_SIZE_BYTES)
{
	m_recieveBufferSize = recieveSize;
	m_recieveBuffer = recieveBuffer;
	m_currentBufferOffset = 0;
	recieveNextByte();
}


void SerialPortDriverHwImpl::stopRecieve()
{
	printf("ERROR");
}
void SerialPortDriverHwImpl::recievedByte()
{
	//TODO - header
	m_currentBufferOffset++;
	recieveNextByte();
}
void SerialPortDriverHwImpl::recieveNextByte()
{
	if(m_recieveBuffer != nullptr && m_currentBufferOffset <= m_recieveBufferSize)
	{
		ShimBase::getInstance().startInteruptRecieve((m_recieveBuffer + m_currentBufferOffset), this, &SerialPortDriverHwImpl::recievedByte);
	}

}

