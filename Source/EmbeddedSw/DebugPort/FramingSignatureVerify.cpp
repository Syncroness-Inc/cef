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
	if(byteOffset < numElementsInDebugPacketFramingSignature)
	{
		return debugPacketFramingSignature[byteOffset];
	}
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Checking for framing signature outside of expected signature frame.",
	        0, 0, 0);
    return 0;  //0 is not in the framing signature
}

uint8_t FramingSignatureVerify::checkFramingSignatureByte(void* receiveBuffer, uint8_t byteOffset)
{
		//Don't allow it to compare offset that does not contain the framing signature
		if(byteOffset >= numElementsInDebugPacketFramingSignature)
		{
			LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Checking for framing signature outside of expected signature frame. {:d}, {:d}",
			        byteOffset, 0, 0);
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
