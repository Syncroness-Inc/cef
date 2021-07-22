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
#include <stdio.h>
#include <functional>
#include "Cef/Source/EmbeddedSw/DebugPort/Driver/Hardware/SerialPortDriverHwImpl.hpp"
/**
 * Base Class for UART Shim
 */

class ShimBase { 
protected:
	//! Constructor.
	ShimBase()
	{}

   /**
    * Callback class instance for receive callback
    */
   SerialPortDriverHwImpl* m_callbackClass; 
   /**
    * Callback function for receive callback
    */
	void (SerialPortDriverHwImpl::* m_callback)(void);

   /**
    * Receive callback.  This will send callback to SerialPortDriverHwImpl to decided if receive should continue
    */
   virtual void rxCallback();
public:
   /**
    * @return Returns the instance of Shim
    */
   static ShimBase& getInstance();

   /**
	 * Start send 
	 * @param sendBuffer send buffer
    * @param bufferSize number of bytes to be sent in the buffer
	 */
   virtual void startInteruptSend(void*sendBuffer, int bufferSize);
   /**
    * Start receive interrupt driven data
	 * Will receive one byte of data then callback function will be called and
	 * receive will have to be called again to continue to receive data
    * @param receiveByte - location to store the received data
    * @param callbackClass - class of callback function (the class that started the send)
    * @param callback - callback function once the data byte has been received 
    */
   virtual void startInteruptReceive(void* receiveByte, SerialPortDriverHwImpl* callbackClass, void (SerialPortDriverHwImpl::* callback)(void));

 
};

#endif  // end header guard
