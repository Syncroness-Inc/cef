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
#ifndef __DEBUG_PORT_DRIVER_H
#define __DEBUG_PORT_DRIVER_H
#include "Cef/Source/Shared/cefContract.hpp"

/**
 * Base Class for DebugPortDriver
 * Send Data/Receive Data/Stop Receive
 */

class DebugPortDriver {
protected:
	//! Constructor.
	DebugPortDriver() {}

public:
   /**
    * Start Send Data
    * @param sendBuffer 
    * @param packetSize 
    */
   virtual void sendData(void* sendBuffer, int packetSize);
   /**
    * Start receiving data
    * @param receive buffer location
    * @param size of packet to receive
    */
   virtual void startReceive(void* receiveBuffer,  int receiveSize = DEBUG_PORT_MAX_PACKET_SIZE_BYTES);
   /**
    * Stops receiving data
    * */
   virtual void stopReceive(void);
   /**
    * Sets the callback to receive any errors
    */
   virtual void setErrorCallback(void);
   /**
    * Callback function for when error occurs for send/receive
    */
   virtual debugPortErrorCode_t errorCallback(void);
 
};

#endif  // end header guard
