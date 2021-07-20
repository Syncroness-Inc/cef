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
#ifndef __SHIM_BASE_H
#define __SHIM_BASE_H

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"

/**
 * Base Class for UART shim
 */

class ShimBase { 
protected:
	//! Constructor.
	ShimBase() :
	m_myUartHandle(nullptr),
	m_writePtr(nullptr),
	m_readPtr(nullptr),
	m_bufferOffset(0)
	{};
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
    * @return returns if uart handler is nullptr
    */
   bool checkForUartHandle();
   /**
    * @return If read pointer is nullptr
    */
   bool checkReadPointer();
   /**
    * @return If write pointer is nullptr
    */
   bool checkWritePointer();

   /**
    * Uart common handler type
    */
   typedef UART_HandleTypeDef uartHandle_t;
   /**
    * Unique Uart handler
	*/
   uartHandle_t* m_myUartHandle;
   /**
    * Pointer to the write buffer that CEF Main will provide 528 max bytes
    */
   void* m_writePtr;
   /**
    * Pointer to the read buffer that CEF Main will provide 528 max bytes
    */
   void* m_readPtr;
   /**
    * Offset location of read buffer
    */
   uint8_t m_bufferOffset;

public:
   /**
    * @return Returns the instance of Shim
    */
   static ShimBase& getInstance();

   /**
    * Start send interrupt driven data
    * Will send data in the write buffer one byte at a time
    */
   virtual void startInteruptSend();
   /**
    * Start receive interrupt driven data
    * Will receive one byte of data then callback function will be called and
    * receive will have to be called again to continue to receive data
    * @param write buffer + offset location to write to memory
    */
   virtual void startInteruptRecieve(uint8_t location);
   /**
    * Set uart info for shim layer to use in uart commands
    * @param Uart handler pointer
    */
   void setUartHandle(uartHandle_t* myUartHandle);
   /**
    * Set the write buffer pointer - memory will be allocated by CEF Main
    * @param write buffer location
    * */ 
   void setBufferWriteLocation(void* writeMemoryLocation); 
   /**
    * Sets the read buffer pointer - memory will be allocated by CEF Main
    * @param read buffer location
    * */ 
   void setBufferReadLocation(void* readMemoryLocation); 
   /**
    * Callback for each rx byte received
    * Will increment the read buffer offset and start the interrupt receive 
    * Read buffer will look for the framing signature and allocate buffer accordingly
    */
   void rxCallback(); 
 
};

#endif  // end header guard
