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
#include "SerialPortDriverHwImpl.hpp"
/**
 * Base Class for UART Shim
 */

class ShimBase { 

public:
   /**
    * Singelton Instance of Shim
    * 
    * @return Returns the instance of Shim
    */
   static ShimBase& getInstance();

   /**
    * Receive finished callback.  
    * This will send callback to SerialPortDriverHwImpl to decided if receive should continue
    */
   virtual void rxCallback();

   /**
    * Transmit finished callback.  
    * This will send callback to SerialPortDriverHwImpl to indicate send is finished.
    */
   virtual void txCallback();

   /**
    * @brief Returns if send if in progress via hardware data to inform transport layer if this is currently happening
    * 
    * @return true A send in currently in progress
    * @return false No send is going on
    */
   virtual bool getSendInProgress(void);

   /**
    * Error callback.  This will send callback to SerialPortDriverHwImpl inform Debug port
    */
   virtual void errorCallback();

   /**
	 * Start send - will send number of bytes of buffer size starting at sendBuffer location 
    * 
	 * @param sendBuffer send buffer
    * @param bufferSize number of bytes to be sent in the buffer
    * 
    * @return returns true if it was able to start a send routine (dependint on m_startInProgress)
	 */
   virtual bool startInterruptSend(void* sendBuffer, int bufferSize);

   /**
    * Start receive interrupt driven data
	 * Will receive one byte of data then callback function will be called and
	 * receive will have to be called again to continue to receive data
    * 
    * @param receiveByte - location to store the received data
    * @param callbackClass - class of callback function (the class that started the receive)
    * @param callback - callback function once the data byte has been received 
    */
   virtual void startInterruptReceive(void* receiveByte, SerialPortDriverHwImpl* callbackClass, bool (SerialPortDriverHwImpl::* callback)(void));

   /**
    * Callback for error during send/receive
    * 
    * @param errorCallbackClass - class of callback function for error info
    * @param errorCallback - callback function for error info
    */
   virtual void startErrorCallback(SerialPortDriverHwImpl* errorCallbackClass, void (SerialPortDriverHwImpl::* errorCallback)(errorCode_t error));

   /**
    * Forces the stop of receive interrupt
    */
   virtual void forceStopReceive(void);

protected:
	//! Constructor.
	ShimBase():
   mp_rxCallbackClass(nullptr),
   mp_rxCallback(nullptr),
   mp_errorCallbackClass(nullptr),
   mp_errorCallback(nullptr)
 	{}


   //! Callback class instance for receive callback
   SerialPortDriverHwImpl* mp_rxCallbackClass; 
   //! Callback function for receive callback
	bool (SerialPortDriverHwImpl::* mp_rxCallback)(void);
   //! Callback class instance for receive error callback
   SerialPortDriverHwImpl* mp_errorCallbackClass; 
   /**
    * Callback function for receive error callback
    * 
    * @param - debug buffer error code for send or receive error
    */
	void (SerialPortDriverHwImpl::* mp_errorCallback)(errorCode_t);

};

#endif  // end header guard
