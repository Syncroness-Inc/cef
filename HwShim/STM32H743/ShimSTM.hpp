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

#include "ShimBase.hpp"
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
    * See base class for method documentation
    */
   void rxCallback() override;

   /**
    * See base class for method documentation
    */
   void errorCallback() override;

	/**
	 * See base class for method documentation
	 */
   void startInteruptSend(void*sendBuffer, int bufferSize) override;
   /**
    * See base class for method documentation
    */
   void startInterruptReceive(void* receiveByte, SerialPortDriverHwImpl* callbackClass, void (SerialPortDriverHwImpl::* callback)(void)) override;

   /**
    * See base class for method documentation
    */
   void startErrorCallback(SerialPortDriverHwImpl* errorCallbackClass, void (SerialPortDriverHwImpl::* errorCallback)(errorCode_t error)) override;

   /**
    * Forces the stop of receive interrupt
    */
   void forceStopReceive(void) override;

};


#endif  // end header guard
