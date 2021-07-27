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
#include "cefContract.hpp"

/**
 * Base Class for DebugPortDriver
 * Send Data/Receive Data/Stop Receive
 */

class DebugPortDriver {

public:
   /**
    * Start Send Debug Data transfer. Command response and logging are the two data that can be sent.
    * 
    * @param sendBuffer - Pointer to start of buffer to send
    * @param packetSize - Number of Bytes to send
    */
   virtual void sendData(void* sendBuffer, int packetSize);
   
   /**
    * Start receiving data from Python utilities
    * 
    * @param receive buffer location
    * @param size of packet to receive
    * 
    * @return returns true if all buffers/offsets are valid to be able to arm receive data
    */
   virtual bool startReceive(void* receiveBuffer,  int receiveSize = DEBUG_PORT_MAX_PACKET_SIZE_BYTES);

   /**
    * Stops receiving data. This will instantly stop the receive of data even in the middle of a packet.
    * */
   virtual void stopReceive(void);

   /**
    * Sets the callback to receive any errors
    */
   virtual void setErrorCallback(void);

   /**
    * Callback function for when error occurs for send/receive
    * 
    * @return error code of the send or receive error
    */
   virtual errorCode_t errorCallback(void);

protected:
	//! Constructor.
	DebugPortDriver() {}

};

#endif  // end header guard
