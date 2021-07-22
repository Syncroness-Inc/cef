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
#ifndef __SERIAL_PORT_DRIVER_HW_IMPL_H
#define __SERIAL_PORT_DRIVER_HW_IMPL_H
#include <stdio.h>
#include "Cef/Source/EmbeddedSw/DebugPort/Driver/DebugPortDriver.hpp"
/**
 * Serial Port Driver for Hardware.
 * It drives the non blocking serial receive and send for hardware impl
 */
class SerialPortDriverHwImpl : public DebugPortDriver {
public:
	//! Constructor.
	SerialPortDriverHwImpl():DebugPortDriver(),
   m_receiveBufferSize(0),
	m_currentBufferOffset(0),
	mp_receiveBuffer(nullptr)
	{}

   /**
    * Start Send Data
    * @param sendBuffer 
    * @param packetSize 
    */
   void sendData(void* sendBuffer, int packetSize) override;
   /**
    * Start receiving data
    * @param receive buffer location
    * @param size of packet to receive
    */
   void startReceive(void* receiveBuffer,  int receiveSize = DEBUG_PORT_MAX_PACKET_SIZE_BYTES) override;
   /**
    * Stops receiving data
    * */
   void stopReceive() override;

   /**
    * Callback function for when a receive byte is received.
    * To continue receiving it will
    * 1)check for a framing signature 
    * 2)incroment buffer offset as needed
    * 3)call receive next byte 
    */
   void receivedByteDriverHwCallback();

private:
   /**
    * Receive is one byte at at time.  After each byte is received receiveNextByte decide 
    * if it is finished receiving and if not it will send command to let the hardware know
    * it needs to receive another byte.   
    */
   void receiveNextByte();

   /**
    * Number of bytes to receive
    */
   int m_receiveBufferSize;
   /**
    * Current offset of the receive buffer
    */
   int m_currentBufferOffset;
   /**
    * Pointer to receive buffer
    */
   void* mp_receiveBuffer;

};

#endif  // end header guard
