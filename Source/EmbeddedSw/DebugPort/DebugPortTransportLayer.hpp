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
#include "DebugPortDriver.hpp"

/**
 * 
 * 
 */

class DebugPortTransportLayer {
public:
   //test code delete ***************************
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

   //************************************************


	//! Constructor.
	DebugPortTransportLayer():
   m_transmitState(debugPortXmitWaitingForBuffer),
   m_receiveState(debugPortRecvWaitForBuffer),
   mp_xmitBuffer(nullptr),
   mp_receiveBuffer(nullptr),
   m_receivePacketLength(0),
   m_sendBufferAvailable(false),     //test buffer delete
   m_receiveBufferAvailable(true),   //test buffer delete
   m_copy(false),                   //test delete
   mp_myRequest(&m_myRequest),        //test delete
   mp_myResponse(nullptr)       //test delete
   {}

   void xmit(void);
   void recv(void);
private:
   DebugPortDriver m_myDebugPortDriver;
   void generatePacketHeader();
   bool receivePacketHeader(void);
   uint32_t calculateChecksum(void* myStruct,  uint structSize);
   void* mp_xmitBuffer;
   void* mp_receiveBuffer;
   uint8_t m_receivePacketLength;
   


/**
 * Transmit States
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
 * Receive States
 */
   enum
   {
      debugPortRecvWaitForBuffer             = 0,	
      debugPortRecvWaitForPacketHeader          ,
      debugPortRecvWaitForCefPacket             ,
      debugPortRecvFinishedRecv                 ,
   };
   typedef uint16_t debugPortReceiveStates_t;

   debugPortReceiveStates_t   m_receiveState;
   debugPortTransmitStates_t  m_transmitState;




};

#endif  // end header guard
