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

#include "FramingSignatureVerify.hpp"
#include "cefContract.hpp"
#include "Logging.hpp"


uint8_t FramingSignatureVerify::getDefinedFramingSignatureByte(uint8_t byteOffset)
{
	uint32_t framingSignature = DEBUG_PACKET_UINT32_FRAMING_SIGNATURE;
	//WARNING - this returns expected Big-endianness.  Jira card in backlog to make this work
	//regardless of endianness. Will refactor is time permits or a project runs into a problem 
	if(byteOffset < sizeof(DEBUG_PACKET_UINT32_FRAMING_SIGNATURE))
	{
		/**CHRIS - JOHN - I think if we get ride of this byte swap for endinaness and Chris sends his data without doing any change of 
		 * endianess everything will end up working correct - That would be the first thing I would test at least*/
		return *((uint8_t *)&framingSignature + (sizeof(framingSignature) - 1) - byteOffset);
	}
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Checking for framing signature outside of expected signature frame.",
	        0, 0, 0);
    return 0;  //0 is not in the framing signature
}

uint8_t FramingSignatureVerify::checkFramingSignatureByte(void* receiveBuffer, uint8_t byteOffset)
{
		//Don't allow it to compare offset that does not contain the framing signature
		if(byteOffset >= sizeof(DEBUG_PACKET_UINT32_FRAMING_SIGNATURE))
		{
			LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Checking for framing signature outside of expected signature frame.",
			        0, 0, 0);
			return 0;
		}
		//UART last byte received
		uint8_t myIncoming = *((uint8_t *)receiveBuffer + byteOffset);
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
