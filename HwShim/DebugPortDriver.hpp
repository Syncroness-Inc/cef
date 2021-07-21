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
#ifndef __DEBUG_PORT_DRIVER_H
#define __DEBUG_PORT_DRIVER_H
#include <stdio.h>
#include "Cef/Source/Shared/cefContract.hpp"
/**
 * Base Class for DebugPortDriver
 */

class DebugPortDriver {
protected:
	//! Constructor.
	DebugPortDriver() :
	m_recieveBufferSize(0),
	m_currentBufferOffset(0),
	m_recieveBuffer(nullptr)
	{}
   /**
    * @return Returns one byte of framing signature based on offset value
    * for an example 0x00010203
    * Offset of 0 will return 0x00 and 1 will return 0x01
    */
   uint8_t getDefinedFramingSignatureByte(uint8_t byteOffset);
   /**
    * Checks one bye of the write buffers based on offset amount and compares it to the framing signature data
    * @return When the data is the same it will return the byteOffset + 1
    * When the data does not match the framing signature it will return 0
    * If byteOffset is greater than the framing signature size it will return 0
    */
   uint8_t checkFramingSignatureByte(uint8_t byteOffset);
   /**
    * Current offset of the recieve buffer
    */
   int m_recieveBufferSize;
   int m_currentBufferOffset;
   void* m_recieveBuffer;

public:
   virtual void sendData(void* sendBuffer, int packetSize);
   /**
    * Start receiving data
    * @param recieve buffer location
    * @param size of packet to recieve
    */
   virtual void startRecieve(void* recieveBuffer,  int recieveSize);
   /**
    * Stops recieving data
    * */ 
   virtual void stopRecieve(void);
   /**
    * Returns the offset buffer length
    */
   int getReadBufferOffset(void);

 
};

#endif  // end header guard
