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
#include "DebugPortDriver.hpp"

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
    * See base class for method documentation
    */
   void sendData(void* sendBuffer, int packetSize) override;

   /**
    * See base class for method documentation
    */
   void startReceive(void* receiveBuffer,  int receiveSize = DEBUG_PORT_MAX_PACKET_SIZE_BYTES) override;

   /**
    * Changes the number of bytes receive is expecting for packet to be finished.  The number of bytes received will
    * not be known until the packet header is received and decoded.  At this point the expected receive may change from
    * max to new amount.  
    * Rules
    * - Receive size can not excded Max Bytes
    * - If receive size is less then or equal to m_currentBufferOffset receive will be stopped
    * 
    * @param newReceiveSize - new expected bytes to receive in packet
    */
   void editReceiveSize(int newReceiveSize);
   
   /**
    * See base class for method documentation
    * */
   void stopReceive() override;

   /**
    * Callback function for when a receive byte is received.
    * To continue receiving it will
    * 1)check for a framing signature 
    * 2)incroment buffer offset as needed
    * 3)call receive next byte 
    */
   void receivedByteDriverHwCallback(void);

   /**
    * Sets the callback to receive any errors
    */
   void setErrorCallback(void);

   /**
    * Callback function for sending or receiveing error
    * 
    * @param error - current error
    */
   void errorCallback(errorCode_t error);

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
