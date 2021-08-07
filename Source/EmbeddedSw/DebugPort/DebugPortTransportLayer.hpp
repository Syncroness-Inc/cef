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
#ifndef __DEBUG_PORT_TRANSPORT_LAYER_H
#define __DEBUG_PORT_TRANSPORT_LAYER_H
#include "cefContract.hpp"
#include "SerialPortDriverHwImpl.hpp"
#include "CefBuffer.hpp"

/**
 * DebugPortTransportLayer runs a state machine for the Transmit and Receive process
 *
 * Transport layer keeps track on where the in the send/receive process it is and also
 * is in charge of constructing the cefCommandDebugPortHeader_t on receive, and
 * validating cefCommandDebugPortHeader_t on transmit.
 *
 * On the first implementation, the receive data needs to be in contiguous memory.  As such
 * the receive data is copied into the the buffer passed from the Application layer.  Eventually
 * the code should be re-factored to work with two buffers...one for the payload header, and
 * one for the payload data (what comes from the applicaton layer/DebugPortRouter)
 */


/**
 * Only one DebugPortDriver should be instantiated.
 * Currently there is only one type of driver, but eventually there could be multiple
 * physical interfaces (serial port, ethernet, simulator...)
 *
 * For now, we just have the serial port
 */

typedef SerialPortDriverHwImpl MyDebugPortDriver;



class DebugPortTransportLayer {
public:
	//! Constructor.
	DebugPortTransportLayer():
        m_transmitState(stateXmitWaitingForBuffer),
        m_receiveState(stateXmitWaitingForBuffer),
        m_expectedNumBytesInReceivePacket(0),
        myReceiveCefBuffer(&myReceiveBuffer[0], NUM_ELEMENTS(myReceiveBuffer)),
        mp_commandReceiveCefBuffer(nullptr),
        m_receiveErrorStatus(errorCode_OK),
        mp_transmitPayload(nullptr),
        m_transmitDebugDataType(debugPacketType_invalid)
        { }

   /**
    * State machine to transmit a packet.
    */
   void transmitStateMachine(void);

   /**
    * State machine to receive packet.
    */
   void receiveStateMachine(void);


private:
   /**
    * Transmit States Machine - See transmitStateMachine()
    */
      enum
      {
         stateXmitWaitingForBuffer          = 0,
         stateXmitGeneratePacketHeader      = 1,
         stateXmitStartHeaderTransmit       = 2,
         stateXmitWaitForHeaderToTransmit   = 3,
         stateXmitStartPayloadTransmit      = 4,
         stateXmitWaitForPayloadToTransmit  = 5,
         stateXmitFinishedSendingPacket     = 6,
      };
      typedef uint16_t debugPortTransmitStates_t;

   /**
    * Receive States Machine - receiveStateMachine()
    */
      enum 
      {
         stateRecvWaitForBuffer             = 0,
         stateRecvWaitForPacketHeader       = 1,
         stateRecvWaitForCefPacket          = 2,
         stateRecvFinishedRecv              = 3,
         stateReceiveFinished               = 4,
      };
      typedef uint16_t debugPortReceiveStates_t;

   /**
    * Generates the cefCommandDebugPortHeader_t for the packet to transmit
    */
   void generatePacketHeader(void);

   /**
    * Waiting on packet header.  Checks to see if complete packet header has been received and checksum matches
    * 
    * @return debugPortReceiveStates_t - Returns the receive state
    */
   uint16_t receivePacketHeader(void);

   /**
    * Calculates the byte checksum of a byte array
    *
    * @param p_byteArray   void pointer to start of byte array to calculate checksum for
    * @param numBytes      number of bytes in the byte array
    * 
    * @return uint32_t  checksum
    */
   uint32_t calculateChecksum(void* p_byteArray, uint32_t numBytes);

   //! Transmit state machine state
   debugPortTransmitStates_t  m_transmitState;

   //! Receive state machine statee
   debugPortReceiveStates_t   m_receiveState;

   //! Instance of debug port driver
   MyDebugPortDriver m_myDebugPortDriver;

   //! Number of bytes currently expected in receive packet (debug port packet header & debug packet)
   uint32_t m_expectedNumBytesInReceivePacket;

   //! Local memory to receive data into
   uint8_t myReceiveBuffer[DEBUG_PORT_MAX_PACKET_SIZE_BYTES];

   //! CefBuffer object used internally that is setup during the constructor so
   //! it is guaranteed to be a non-null memory.  Contains memory for both header and payload.
   CefBuffer myReceiveCefBuffer;

   //! Pointer to the Command Receive CEF Buffer (nullptr when the router doesn't want to be
   //! trying to receive a command.)
   CefBuffer* mp_commandReceiveCefBuffer;

   //! Receive error status
   errorCode_t m_receiveErrorStatus;

   //! Buffer with the transmit payload (does not include Transport Header)
   CefBuffer* mp_transmitPayload;

   //! What type of packet we are currently transmitting
   debugPacketDataType_t m_transmitDebugDataType;

   //! Packet Header for Transmit (re-built for each transmit sequence)
   cefCommandDebugPortHeader_t m_transmitPacketHeader;
};

#endif  // end header guard
