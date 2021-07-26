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
	ShimSTM():ShimBase() {}

   /**
    * Receive callback.  This will send callback to SerialPortDriverHwImpl to decided if receive should continue
    */
   void rxCallback() override;
   /**
    * Error callback.  This will send callback to SerialPortDriverHwImpl inform Debug port
    */
   void errorCallback() override;

	/**
	 * Start send 
	 * @param sendBuffer send buffer
    * @param bufferSize number of bytes to be sent in the buffer
	 */
   void startInteruptSend(void*sendBuffer, int bufferSize) override;
   /**
    * Start receive interrupt driven data
	 * Will receive one byte of data then callback function will be called and
	 * receive will have to be called again to continue to receive data
    * @param receiveByte - location to store the received data
    * @param callbackClass - class of callback function (the class that started the send)
    * @param callback - callback function once the data byte has been received 
    */
   void startInteruptReceive(void* receiveByte, SerialPortDriverHwImpl* callbackClass, void (SerialPortDriverHwImpl::* callback)(void)) override;

   /**
    * Callback for error during send/receive
    * 
    * @param errorCallbackClass - class of callback function for error info
    * @param errorCallback - callback function for error info
    */
   void startErrorCallback(SerialPortDriverHwImpl* errorCallbackClass, debugPortErrorCode_t (SerialPortDriverHwImpl::* errorCallback)(debugPortErrorCode_t error)) override;

   /**
    * Forces the stop of receive interrupt
    */
   void forceStopReceive(void) override;

};


#endif  // end header guard
