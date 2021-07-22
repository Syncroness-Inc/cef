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
#ifndef __FRAMING_SIGNATURE_VERIFY_H
#define __FRAMING_SIGNATURE_VERIFY_H
#include <stdio.h>

/**
 * Helper to check framing signature
 */

class FramingSignatureVerify {
private:
	//! Constructor.
	FramingSignatureVerify() {}
 public:
   /**
    * @return Returns one byte of framing signature based on offset value
    *  - for an example 0x00010203
    *  - Offset of 0 will return 0x00 and 1 will return 0x01
    * @param byte to get in framing signature
    */
   static uint8_t getDefinedFramingSignatureByte(uint8_t byteOffset);
   /**
    * Checks one bye of the write buffers based on offset amount and compares it to the framing signature data
    * @return When the data is the same it will return the byteOffset + 1
    *  - When the data does not match the framing signature it will return 0
    *  - If byteOffset is greater than the framing signature size it will return 0
    * @param receiveBuffer pointer to the buffer to check
    * @param byte to check in the framing signature 
    */
   static uint8_t checkFramingSignatureByte(void* receiveBuffer, uint8_t byteOffset);
   
 
};

#endif  // end header guard
