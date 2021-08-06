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

/**
 * DebugPortTransportLayer runs a state machine for the Transmit and Receive process
 * All buffer memory comes from the router layer and all send/receive is done in the Port Driver
 * Transport layer keeps track on where the in the send/receive process it is and also
 * is in charge of constructing and destructing the cefCommandDebugPortHeader_t
 */

class DebugPortTransportLayer {
public:
   //**************************test code delete ***************************
   typedef struct
   {
      cefCommandDebugPortHeader_t headerResponse;
      cefCommandPingResponse_t   pingResponse;
      
   } cefCompleteResponse_t;
   typedef struct
   {
      cefCommandDebugPortHeader_t headerResponse;
      cefCommandPingRequest_t   pingResponse;
      
   } cefCompleteRequest_t;
   bool m_sendBufferAvailable;
   bool m_receiveBufferAvailable;
   bool m_copy;
   void* getSendBuffer();
   void returnSendBuffer(void*);
   void* getReceiveBuffer();
   void returnReceiveBuffer(void*);
   void* mp_myRequest;
   void* mp_myResponse;
   cefCompleteResponse_t m_myResponse;
   cefCompleteRequest_t m_myRequest;
   //*************************************************************************


	//! Constructor.
	DebugPortTransportLayer():
   m_sendBufferAvailable(false),                         //test delete
   m_receiveBufferAvailable(true),                       //test delete
   m_copy(false),                                        //test delete
   mp_myRequest(&m_myRequest),                           //test delete
   mp_myResponse(nullptr),                               //test delete
   m_transmitState(debugPortXmitWaitingForBuffer),
   m_receiveState(debugPortRecvWaitForBuffer),
   mp_xmitBuffer(nullptr),
   mp_receiveBuffer(nullptr),
   m_receivePacketLength(0)
   {}

   /**
    * State machine for transmit a packet.  Will go throught debugPortTransmitStates_t. 
    * 
    * debugPortXmitWaitingForBuffer          - Waiting for memory buffer from Router to start Transmit
    * debugPortXmitGeneratePacketHeader      - Adds the debug Port Packet header onto the packet
    * debugPortXmitReadyToSend               - Packet header and packet is ready to be sent/starts the send
    * debugPortXmitSendingPacket             - Checks to see if transmit has finished
    * debugPortXmitFinishedSendingPacket     - Packet finished return memory buffer to router 
    */
   void transmitStateMachine(void);

   /**
    * State machine for Receive packet.  Will go throught debugPortReceiveStates_t
    * 
    * debugPortRecvWaitForBuffer          - Waiting for memory buffer from router to start receive
    * debugPortRecvWaitForPacketHeader    - Waiting for all bytes of debug port packet header
    *                                        - Checks packet header checksum
    * debugPortRecvWaitForCefPacket       - Waiting for all bytes of debug port packet
    * debugPortRecvFinishedRecv           - Checks packet checksum & returns the memory buffer
    * debugPortRecvBadChecksom            - Returns the memory buffer 
    */
   void receiveStateMachine(void);
private:
   /**
    * Transmit States Machine - See transmitStateMachine()
    */
      enum
      {
         debugPortXmitWaitingForBuffer          = 0,
         debugPortXmitGeneratePacketHeader         ,
         debugPortXmitReadyToSend                  ,
         debugPortXmitSendingPacket                ,
         debugPortXmitFinishedSendingPacket        ,
      };
      typedef uint16_t debugPortTransmitStates_t;

   /**
    * Receive States Machine - receiveStateMachine()
    */
      enum 
      {
         debugPortRecvWaitForBuffer             = 0,
         debugPortRecvWaitForPacketHeader          ,
         debugPortRecvWaitForCefPacket             ,
         debugPortRecvFinishedRecv                 ,
         debugPortRecvBadChecksom                  ,
      };
      typedef uint16_t debugPortReceiveStates_t;

   /**
    * Generates the cefCommandDebugPortHeader_t for the packet
    */
   void generatePacketHeader(void);

   /**
    * Waiting on packet header.  Checks to see if complete packet header has been received and checksum matches
    * 
    * @return debugPortReceiveStates_t - Returns the receive state
    *                                      -  debugPortRecvWaitForPacketHeader - all packet header bytes have not been received
    *                                      -  debugPortRecvBadChecksom - missmatch checksum
    *                                      -  debugPortRecvWaitForCefPacket - packet header received and packet header checksum matches 
    */
   uint16_t receivePacketHeader(void);

   /**
    * Calculates the checksom of a struct
    * 
    * @param myStruct - void pointer to start of struct to calculate checksun for
    * @param structSize - number of bytes of the struct (or that you want the checksum calculated for)
    * @return uint32_t - checksum
    */
   uint32_t calculateChecksum(void* myStruct,  uint structSize);

   //!State machine current transmit state 
   debugPortTransmitStates_t  m_transmitState;

   //!State machine current receive state 
   debugPortReceiveStates_t   m_receiveState;

   //! Instance of debug port driver
   SerialPortDriverHwImpl m_myDebugPortDriver;

   //!Pointer to transmit buffer (nullptr when router has not given mem buffer)
   void* mp_xmitBuffer;

   //!Pointer to receive buffer (nullptr when router has not given mem buffer)
   void* mp_receiveBuffer;

   //!Number of bytes of receive packet (debug port packet header & debug packet)
   uint8_t m_receivePacketLength;
};

#endif  // end header guard
