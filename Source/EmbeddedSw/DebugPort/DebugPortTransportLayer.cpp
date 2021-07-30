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


#include "DebugPortTransportLayer.hpp"
#include "Logging.hpp"
#include "ShimBase.hpp"
#include "cefContract.hpp"

//Delete test **************************************************
void* DebugPortTransportLayer::getSendBuffer()
{
	m_sendBufferAvailable = false;
	return mp_xmitBuffer;
}
void DebugPortTransportLayer::returnSendBuffer(void*)
{
	m_sendBufferAvailable = true;
}
void* DebugPortTransportLayer::getReceiveBuffer()
{
	m_receiveBufferAvailable = false;
	return mp_receiveBuffer;
}
void DebugPortTransportLayer::returnReceiveBuffer(void*)
{
	m_receiveBufferAvailable = true;
}
//*************************************************************
uint32_t DebugPortTransportLayer::calculateChecksum(void* myStruct, uint structSize)
{	
	//cast to command/header TODO
	uint32_t myChecksum = 0;
	unsigned char *p = (unsigned char *)&myStruct;
	for (int i=0; i<structSize; i++) 
	{
		myChecksum += p[i];
	}
	return myChecksum;
}

bool DebugPortTransportLayer::generatePacketHeader(void* header, void* request)
{
//Cast 
cefCommandDebugPortHeader_t* myHeaderType = (cefCommandDebugPortHeader_t*)header;
//TODO there are multiple responses need to decide
cefCommandPingResponse_t* myRequestType = (cefCommandPingResponse_t*)request;

//GENERATE HEADER
//Framing Signature
myHeaderType->m_framingSignature = DEBUG_PACKET_UINT32_FRAMING_SIGNATURE;
//Payload Checksum need to change sizeof to correct response
myHeaderType->m_packetPayloadChecksum = calculateChecksum(request, sizeof(&myRequestType));
//Get Size TODO myRequestType is not generic this will probable be fixed when generating
myHeaderType->m_payloadSize = sizeof(&myRequestType);
//Packet type TODO this could be loggin 
myHeaderType->m_packetType = debugPacketType_commandResponse; 
//0 for checksum
myHeaderType->m_reserve = 0;
//0 to calcultate checksum
myHeaderType->m_packetHeaderChecksum = 0;
//Calculate checksum
myHeaderType->m_packetHeaderChecksum = calculateChecksum(header, sizeof(cefCommandDebugPortHeader_t));
}

bool DebugPortTransportLayer::receivePacketHeader()
{
	//Check to see if we have received enough bites for a full packet header
	if(m_myDebugPortDriver.getCurrentBytesReceived() >= sizeof(cefCommandDebugPortHeader_t))
	{
		//Check to see if Checksum header matches
		uint32_t headerCheck = calculateChecksum(mp_receiveBuffer, sizeof(cefCommandDebugPortHeader_t));
		cefCommandDebugPortHeader_t* myHeaderType = (cefCommandDebugPortHeader_t*)mp_receiveBuffer;
		if(headerCheck != myHeaderType->m_packetHeaderChecksum)
		{
			//Checksum header does not match
			//TODO stop reset 
			return false;
		}
		//Get/Set packet size
		m_receivePacketLength = myHeaderType->m_payloadSize;
		m_myDebugPortDriver.editReceiveSize(m_receivePacketLength);
		return true;
	}
	return false;
}

void DebugPortTransportLayer::xmit(void)
{
	switch (m_transmitState)
	{
	case debugPortXmitWaitingForBuffer:
		mp_xmitBuffer = getSendBuffer();
		if(mp_xmitBuffer == nullptr)
		{
			//If buffer is not ready leave state machien dont block
			break;
		}
		//When buffer is ready generate packet header 
		m_transmitState = debugPortXmitGeneratePacketHeader;
	case debugPortXmitGeneratePacketHeader:
		if(generatePacketHeader(mp_xmitBuffer, mp_xmitBuffer))
		{
			m_transmitState = debugPortXmitReadyToSend;
		}
		break;
	case debugPortXmitReadyToSend:
		//delete
		uint8_t sendBuffer[64];
		int packetSize = 64;
		//
		m_myDebugPortDriver.sendData((uint8_t(*)[64]) mp_xmitBuffer, packetSize);
		m_transmitState = debugPortXmitSendingPacket;
		break;	
	case debugPortXmitSendingPacket:
		//check if sending is finished
		if(m_myDebugPortDriver.getSendInProgress())
		{
			//Sending not finished leave stat machien
			break;
		}
		//sending finished
		m_transmitState = debugPortXmitFinishedSendingPacket;
	case debugPortXmitFinishedSendingPacket:
		returnSendBuffer(mp_xmitBuffer);
		m_transmitState = debugPortXmitWaitingForBuffer;
		break;		
	default:
	LOG_WARNING(Logging::LogModuleIdCefInfrastructure, "DebugTransportLayer Transmit State Machien in unknown state.");
		break;
	}
}


void DebugPortTransportLayer::recv(void)
{
	switch (m_receiveState)
	{
	case debugPortRecvWaitForBuffer:
		mp_receiveBuffer = getReceiveBuffer();
		if(mp_receiveBuffer != nullptr)
		{
			m_myDebugPortDriver.startReceive(mp_receiveBuffer);
			m_receiveState = debugPortRecvWaitForPacketHeader;
		}
		break;
	case debugPortRecvWaitForPacketHeader:
		if(receivePacketHeader())
		{
			m_receiveState = debugPortRecvWaitForCefPacket;
		}
		break;
	case debugPortRecvWaitForCefPacket:
		if(m_myDebugPortDriver.getCurrentBytesReceived() < m_receivePacketLength)
		{
			break;
		}
		m_receiveState = debugPortRecvFinishedRecv;
	case debugPortRecvFinishedRecv:
		returnReceiveBuffer(mp_receiveBuffer);
		m_receiveState = debugPortRecvWaitForBuffer;
		break;		
	default:
		LOG_WARNING(Logging::LogModuleIdCefInfrastructure, "DebugTransportLayer Receive State Machien in unknown state.");
		break;
	}
}