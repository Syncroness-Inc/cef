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
#ifndef __CEF_BUFFER_H
#define __CEF_BUFFER_H

/**
 * CEF Buffer
 * 	Contains Buffer's starting address, number of bytes in the buffer, and number
 * 	of valid bytes in the buffer (i.e. number of bytes being used)
 *
 * 	Passing a CEF Buffer is preferable to passing just a memory pointer as it
 * 	allows checking for memory overflow (as the object has the size of the buffer).
 * 	Plus, the number of valid bytes in the buffer can optionally be used in the
 * 	event the buffer could have variable sized data added to it.
 */

#include "cefMappings.hpp"

class CefBuffer
{
public:
    /**
     * Constructor
     *
     * @param startAddress			Starting address for the buffer
     * @param maximumSizeInBytes 	Maximum number of bytes in the buffer
     * */
    CefBuffer(void *startAddress, uint32_t sizeInBytes) :
            m_startAddress(startAddress), m_maxBufferSizeInBytes(sizeInBytes), m_numValidBytes(0)
    {
    }

    /**
     * Gets the Buffer's start address
     *
     * @return Buffer's start address
     */
    void* getBufferStartAddress()
    {
        return m_startAddress;
    }

    /**
     * Gets the maximum number of bytes that can be stored in the buffer
     *
     * @return buffer size
     */
    uint32_t getMaxBufferSizeInBytes()
    {
        return m_maxBufferSizeInBytes;
    }

    /**
     * Sets the number of valid bytes in the buffer
     *
     * @param numValidBytes   Number of valid bytes stored in the buffer
     */
    errorCode_t setNumberOfValidBytes(uint32_t numValidBytes)
    {
        if (numValidBytes > m_maxBufferSizeInBytes)
        {
            return errorCode_BufferValidBytesExceedsBufferSize;
        }
        m_numValidBytes = numValidBytes;
        return errorCode_OK;
    }

    /**
     * Gets the number of valid bytes in the buffer
     *
     * @return number of valid bytes
     */
    uint32_t getNumberOfValidBytes()
    {
        return m_numValidBytes;
    }

private:
    //! Buffer starting address
    void *m_startAddress;

    //! Maximum number of bytes that can be stored in the buffer
    uint32_t m_maxBufferSizeInBytes;

    //! Number of valid bytes that are currently stored in the buffer
    uint32_t m_numValidBytes;
};

#endif  // end header guard

