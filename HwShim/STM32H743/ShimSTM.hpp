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
#ifndef __SHIM_STM_H
#define __SHIM_STM_H

#include "Cef/HwShim/ShimBase.hpp"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"

/**
 * STM Shim class
 */
class ShimSTM : public ShimBase {
public:
	//! Constructor.
	ShimSTM():ShimBase(),
	m_myUartHandle(nullptr)
	{}


	/**
	 * Unique Uart handler
	 */
	UART_HandleTypeDef* m_myUartHandle;

	/**
	 * Start receive interrupt driven data
	 * Will receive one byte of data then callback function will be called and
	 * receive will have to be called again to continue to receive data
	 * @param write buffer + offset location to write to memory
	 */
   void startInteruptSend(void*sendBuffer, int bufferSize) override;
   /**
    * Set uart info for shim layer to use in uart commands
    * @param Uart handler pointer
    */
   void startInteruptRecieve(void* recieveByte, SerialPortDriverHwImpl* callbackClass, void (SerialPortDriverHwImpl::* callback)(void)) override;

   void rxCallback() override;
   /**
    * Set uart info for shim layer to use in uart commands
    * @param Uart handler pointer
    */
   void setUartHandle(UART_HandleTypeDef* myUartHandle);
   /**
    * @return returns if uart handler is nullptr
    */
   bool checkForUartHandle();
 
};


#endif  // end header guard
