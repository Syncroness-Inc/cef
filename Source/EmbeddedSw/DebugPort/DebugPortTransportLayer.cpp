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

//*********************8Delete moc router test code**************************************************
void* DebugPortTransportLayer::getSendBuffer()
{
	m_sendBufferAvailable = false;
	if(m_copy)
	{
		//load response
		m_myResponse.pingResponse.m_header.m_commandErrorCode = m_myRequest.pingResponse.m_header.m_commandErrorCode;
		m_myResponse.pingResponse.m_header.m_commandNumBytes = sizeof(cefCommandPingResponse_t);
		m_myResponse.pingResponse.m_header.m_commandOpCode = m_myRequest.pingResponse.m_header.m_commandOpCode;
		m_myResponse.pingResponse.m_header.m_commandRequestResponseSequenceNumberPython = m_myRequest.pingResponse.m_header.m_commandRequestResponseSequenceNumberPython;
		m_myResponse.pingResponse.m_header.m_padding1 = m_myRequest.pingResponse.m_header.m_padding1;

		m_myResponse.pingResponse.m_padding1 = m_myRequest.pingResponse.m_padding1;
		m_myResponse.pingResponse.m_padding2 = m_myRequest.pingResponse.m_padding2;
		m_myResponse.pingResponse.m_testValue = m_myRequest.pingResponse.m_testValue;
		m_myResponse.pingResponse.m_uint16Value = m_myRequest.pingResponse.m_uint16Value;
		m_myResponse.pingResponse.m_uint32Value = m_myRequest.pingResponse.m_uint32Value;
		m_myResponse.pingResponse.m_uint64Value = m_myRequest.pingResponse.m_uint64Value;
		m_myResponse.pingResponse.m_uint8Value = m_myRequest.pingResponse.m_uint8Value;
		//set response
		mp_myResponse = &m_myResponse;
		m_copy = false;
	}
	else
	{
		mp_myResponse = nullptr;
	}
	return mp_myResponse;
}
void DebugPortTransportLayer::returnSendBuffer(void*)
{
	m_sendBufferAvailable = true;
}
void* DebugPortTransportLayer::getReceiveBuffer()
{
	m_receiveBufferAvailable = false;
	return mp_myRequest;
}
void DebugPortTransportLayer::returnReceiveBuffer(void*)
{
	m_receiveBufferAvailable = true;
}
//*****************************************************************************************************************************

uint32_t DebugPortTransportLayer::calculateChecksum(void* myStruct, uint structSize)
{	
	uint32_t myChecksum = 0;
	unsigned char* p = (unsigned char *)myStruct;
	for (uint i=0; i<structSize; i++)
	{
		myChecksum += p[i];
	}
	return myChecksum;
}

void DebugPortTransportLayer::generatePacketHeader() //transmit = cefResponse
{
	//GENERATE HEADER
	//Framing Signature
	cefCompleteResponse_t* myXmitBuffer = (cefCompleteResponse_t*)mp_myResponse;
	myXmitBuffer->headerResponse.m_framingSignature = DEBUG_PACKET_UINT32_FRAMING_SIGNATURE;
	//Payload Checksum need to change sizeof to correct response
	myXmitBuffer->headerResponse.m_packetPayloadChecksum = calculateChecksum((void*)(&myXmitBuffer->pingResponse), sizeof(myXmitBuffer->pingResponse));
	//Get Size TODO myRequestType is not generic this will probable be fixed when generating
	myXmitBuffer->headerResponse.m_payloadSize = sizeof(myXmitBuffer->pingResponse);
	//Packet type TODO this could be loggin 
	myXmitBuffer->headerResponse.m_packetType = debugPacketType_commandResponse; 
	//0 for checksum
	myXmitBuffer->headerResponse.m_reserve = 0;
	//0 to calcultate checksum
	myXmitBuffer->headerResponse.m_packetHeaderChecksum = 0;
	//Calculate checksum
	myXmitBuffer->headerResponse.m_packetHeaderChecksum = calculateChecksum((void*)(&myXmitBuffer->headerResponse), sizeof(myXmitBuffer->headerResponse));
}

uint16_t DebugPortTransportLayer::receivePacketHeader() //receive = request
{
	cefCompleteRequest_t* myReceiveBuffer = (cefCompleteRequest_t*)mp_receiveBuffer;
	//Check to see if we have received enough bites for a full packet header
	uint header = sizeof(myReceiveBuffer->headerResponse);
	if(m_myDebugPortDriver.getCurrentBytesReceived() >= header)
	{
		//Check to see if Checksum header matches
		uint32_t headerCheck = calculateChecksum(&myReceiveBuffer->headerResponse, (sizeof(myReceiveBuffer->headerResponse)-sizeof(myReceiveBuffer->headerResponse.m_packetHeaderChecksum)));
		uint16_t packetHeaderChecksum = myReceiveBuffer->headerResponse.m_packetHeaderChecksum;
		if(headerCheck != packetHeaderChecksum)
		{
			//Checksum header does not match
			//TODO stop reset 
			LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "DebugTransportLayer Header Checksum does not match.");
			return debugPortRecvBadChecksom;
		}
		//Get/Set packet size (header + packet)
		m_receivePacketLength = myReceiveBuffer->headerResponse.m_payloadSize + sizeof(myReceiveBuffer->headerResponse);
		m_myDebugPortDriver.editReceiveSize(m_receivePacketLength);
		return debugPortRecvWaitForCefPacket;
	}
	return debugPortRecvWaitForPacketHeader;
}

void DebugPortTransportLayer::transmitStateMachine(void) //transmit = cefResponse 
{
	switch (m_transmitState)
	{
	case debugPortXmitWaitingForBuffer:
		mp_xmitBuffer = getSendBuffer();
		if(mp_xmitBuffer == nullptr)
		{
			//If buffer is not ready leave state machine don't block
			break;
		}
		//When buffer is ready generate packet header 
		m_receivePacketLength = DEBUG_PORT_MAX_PACKET_SIZE_BYTES;
		m_transmitState = debugPortXmitGeneratePacketHeader;
		break;
	case debugPortXmitGeneratePacketHeader:
		generatePacketHeader();
		m_transmitState = debugPortXmitReadyToSend;
		break;
	case debugPortXmitReadyToSend:
		cefCompleteResponse_t* myResponseBuffer = (cefCompleteResponse_t*)mp_xmitBuffer;
		m_myDebugPortDriver.sendData((uint8_t*)mp_xmitBuffer, sizeof(&myResponseBuffer));
		m_transmitState = debugPortXmitSendingPacket;
		break;	
	case debugPortXmitSendingPacket:
		//check if sending is finished
		if(m_myDebugPortDriver.getSendInProgress())
		{
			//Sending not finished leave state machine
			//TODO make a timeout if it takes to long
			break;
		}
		//sending finished
		m_transmitState = debugPortXmitFinishedSendingPacket;
		break;
	case debugPortXmitFinishedSendingPacket:
		returnSendBuffer(mp_xmitBuffer);
		m_transmitState = debugPortXmitWaitingForBuffer;
		break;	
	default:
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "DebugTransportLayer Transmit State Machien in unknown state.");
		break;
	}
}

void DebugPortTransportLayer::receiveStateMachine(void)
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
		m_receiveState = receivePacketHeader();
		break;
	case debugPortRecvWaitForCefPacket:
		if(m_myDebugPortDriver.getCurrentBytesReceived() < m_receivePacketLength)
		{
			break;
		}
		m_receiveState = debugPortRecvFinishedRecv;
		break;
	case debugPortRecvFinishedRecv:
	{
		//ensure checksum matches
		cefCompleteRequest_t* myReceiveBuffer = (cefCompleteRequest_t*)mp_receiveBuffer;
		uint32_t headerCheck = calculateChecksum(&myReceiveBuffer->pingResponse, myReceiveBuffer->headerResponse.m_packetPayloadChecksum);
		if(headerCheck != myReceiveBuffer->headerResponse.m_packetPayloadChecksum)
		{
			m_receiveState= debugPortRecvBadChecksom;
			//TODO tell router data no good
			LOG_WARNING(Logging::LogModuleIdCefInfrastructure, "DebugTransportLayer debug packet checksum does not match.");
			break;
		}
		//DELETE THIS IS TEST CODE (this is to help moc the router)
		else
		{
			m_copy = true;
		}
		returnReceiveBuffer(mp_receiveBuffer);
		m_receiveState = debugPortRecvWaitForBuffer;
		break;		
	}
	case debugPortRecvBadChecksom:
		returnSendBuffer(mp_receiveBuffer);
		m_receiveState= debugPortRecvWaitForBuffer;
		break;
	default:
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "DebugTransportLayer Receive State Machien in unknown state.");
		break;
	}
}
