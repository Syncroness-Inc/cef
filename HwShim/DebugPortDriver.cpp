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

#include "DebugPortDriver.hpp"
#include "cefContract.hpp"


uint8_t DebugPortDriver::getDefinedFramingSignatureByte(uint8_t byteOffset)
{
	uint32_t framingSignature = DEBUG_PACKET_UINT32_FRAMING_SIGNATURE;
    return *((uint8_t *)&framingSignature + (sizeof(framingSignature) - 1) - byteOffset);
}
uint8_t DebugPortDriver::checkFramingSignatureByte(uint8_t byteOffset)
{
		//Don't allow it to compare offset that does not contain the framing signature
		if(byteOffset >= sizeof(DEBUG_PACKET_UINT32_FRAMING_SIGNATURE))
		{
			return 0;
		}
		//UART last byte recieved
		uint8_t myIncoming = *((uint8_t *)m_recieveBuffer + byteOffset);
		//Framing Signature compare byte
		uint8_t myHeader = getDefinedFramingSignatureByte(byteOffset);
		if(myHeader == myIncoming)
		{
			uint8_t nextVal = byteOffset  + 1;
			return nextVal;
		}
    //If not the correct framing signature byte back to 0 offset in write buffer
	return 0;
}

void DebugPortDriver::sendData(void* sendBuffer, int packetSize)
{
	printf("ERROR");
}


void DebugPortDriver::startRecieve(void* recieveBuffer,  int recieveSize = DEBUG_PORT_MAX_PACKET_SIZE_BYTES)
{
	printf("ERROR");
}


void DebugPortDriver::stopRecieve()
{
	printf("ERROR");
}


int DebugPortDriver::getReadBufferOffset()
{
	return m_currentBufferOffset;
}
