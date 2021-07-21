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
#include "Cef/HwShim/SerialPortDriverHwImpl.hpp"
/**
 * Base Class for UART Shim
 */

class ShimBase { 
protected:
	//! Constructor.
	ShimBase()
	{}

	void (SerialPortDriverHwImpl::* m_callback)(void);
	SerialPortDriverHwImpl* m_callbackClass;
public:
   /**
    * @return Returns the instance of Shim
    */
   static ShimBase& getInstance();

   /**
    * Start send interrupt driven data
    * Will send data in the write buffer one byte at a time
    */
   virtual void startInteruptSend(void*sendBuffer, int bufferSize);
   /**
    * Start receive interrupt driven data
    * Will receive one byte of data then callback function will be called and
    * receive will have to be called again to continue to receive data
    * @param write buffer + offset location to write to memory
    */
   virtual void startInteruptRecieve(void* recieveByte, SerialPortDriverHwImpl* callbackClass, void (SerialPortDriverHwImpl::* callback)(void));
   /**
    * Callback for each rx byte received
    * Will increment the read buffer offset and start the interrupt receive 
    * Read buffer will look for the framing signature and allocate buffer accordingly
    */
   virtual void rxCallback();
 
};

#endif  // end header guard
