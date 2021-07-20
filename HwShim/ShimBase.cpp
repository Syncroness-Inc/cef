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

#include "ShimBase.h"
#include "Cef/HwShim/STM32H743/ShimSTM.h"
#include "cefContract.hpp"
#include <stdio.h>

//Instance of STM shim
static ShimSTM instance;

ShimBase& ShimBase::getInstance()
{
	return instance;
}

bool ShimBase::checkForUartHandle()
{
	if(m_myUartHandle == nullptr)
	{
		printf("ERROR NEED HANDLE\n");
		return false;
	}
	return true;
}
bool ShimBase::checkReadPointer()
{
	if(m_readPtr == nullptr)
	{
		printf("ERROR NEED Valid ptr\n");
		return false;
	}
	return true;
}
bool ShimBase::checkWritePointer()
{
	if(m_writePtr == nullptr)
	{
		printf("ERROR NEED Valid ptr\n");
		return false;
	}
	return true;
}

uint8_t ShimBase::getDefinedFramingSignatureByte(uint8_t byteOffset)
{
	uint32_t framingSignature = DEBUG_PACKET_UINT32_FRAMING_SIGNATURE;
    return *((uint8_t *)&framingSignature + (sizeof(framingSignature) - 1) - byteOffset);
}
uint8_t ShimBase::checkFramingSignatureByte(uint8_t byteOffset)
{
	if(checkReadPointer())
	{
		//Don't allow it to compare offset that does not contain the framing signature
		if(byteOffset >= sizeof(DEBUG_PACKET_UINT32_FRAMING_SIGNATURE))
		{
			return 0;
		}
		//UART last byte recieved
		uint8_t myIncoming = *((uint8_t *)m_readPtr + byteOffset);
		//Framing Signature compare byte
		uint8_t myHeader = getDefinedFramingSignatureByte(byteOffset);
		if(myHeader == myIncoming)
		{
			uint8_t nextVal = byteOffset  + 1;
			return nextVal;
		}
	}
    //If not the correct framing signature byte back to 0 offset in write buffer
	return 0;
}

void ShimBase::rxCallback()
{
	if(m_bufferOffset < sizeof(DEBUG_PACKET_UINT32_FRAMING_SIGNATURE)) //Framing Signature 4 bytes, with offset 0,1,2,3
	{
		m_bufferOffset = checkFramingSignatureByte(m_bufferOffset);
	}
	else
	{
		m_bufferOffset++;
	}

	startInteruptRecieve(m_bufferOffset);
	if(m_bufferOffset == 10) //TODO set to max
	{
		//TODO error
		startInteruptSend();
		m_bufferOffset = 0;
	}
}

void ShimBase::startInteruptSend()
{
	printf("ERROR");
}

void ShimBase::startInteruptRecieve(uint8_t location)
{
	printf("ERROR");
}

void ShimBase::setUartHandle(uartHandle_t* myUartHandle)
{
	m_myUartHandle = myUartHandle;
}

void ShimBase::setBufferWriteLocation(void* writeMemoryLocation)
{
	m_writePtr = writeMemoryLocation;
}
void ShimBase::setBufferReadLocation(void* readMemoryLocation)
{
	m_readPtr = readMemoryLocation;
}
