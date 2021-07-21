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
#include "Cef/HwShim/DebugPortDriver.hpp"
/**
 * Base Class for DebugPortDriver
 */

class SerialPortDriverHwImpl : public DebugPortDriver {
protected:
	//! Constructor.
	SerialPortDriverHwImpl():DebugPortDriver()
	{}


public:

   void sendData(void* sendBuffer, int packetSize) override;
   /**
    * Start receiving data
    * @param recieve buffer location
    * @param size of packet to recieve
    */
   void startRecieve(void* recieveBuffer,  int recieveSize) override;
   /**
    * Stops recieving data
    * */
   void stopRecieve() override;

   void recievedByte();

   void recieveNextByte();

};

#endif  // end header guard
