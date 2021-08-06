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
 * 
 * WARNING - For simplicity this class assumes we receive one byte on each interupt and 
 * the hardware/HAL layer has some kind of a fifo that is able to handle 
 * doing this without dropping data.  If data starts getting dropped and we miss packets
 * this will have to be refactored.
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
   void sendData(void* sendBuffer, int packetSize);

   /**
    * See base class for method documentation
    */
   bool startReceive(void* receiveBuffer,  uint32_t receiveSize = DEBUG_PORT_MAX_PACKET_SIZE_BYTES);

   /**
    * See base class for method documentation
    */
   uint32_t getCurrentBytesReceived(void);

   /**
    * See base class for method documentation
    */
   bool getSendInProgress(void);

   /**
    * See base class for method documentation
    */
   void editReceiveSize(uint32_t newReceiveSize);
   
   /**
    * See base class for method documentation
    * */
   void stopReceive();

   /**
    * Callback function when a startReceive has been called and 
    * receive has been successfully armed this callback will be called
    * once one byte has been received.
    * To continue receiving it will check and preform the following 
    * 1) If the number of expected bytes have been received
    * 2) If there is additional room in the buffer to receive a byte
    * 3) Check for a framing signature if needed
    * 4) Incroment buffer offset as needed
    * 5) Call receive next byte 
    * 
    * @return returns true if receiving is not finished and was able to arm 
    * receive to retrieve next byte of data
    */
   bool receivedByteDriverHwCallback(void);

   /**
    * Sets the callback to receive any errors
    */
   void setErrorCallback(void);

   /**
    * Callback function for sending or receiving error
    * 
    * @param error - current error
    */
   void errorCallback(errorCode_t error);

private:
   /**
    * Sets on interupt driven receive 
    * Arms to receive 1 byte of data
    * Arm to receive that byte at the (m_receiveBuffer + m_currentBufferOffset)
    * 
    * @return - return true if buffers to allocate data are valid 
    */
   bool armReceiveNextByte();

   //! Number of bytes to receive
   uint32_t m_receiveBufferSize;

   //! Current offset of the receive buffer

   uint32_t m_currentBufferOffset;

   //! Pointer to receive buffer
   void* mp_receiveBuffer;

};

#endif  // end header guard
