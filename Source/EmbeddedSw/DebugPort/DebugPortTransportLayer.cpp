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
#include "CommandDebugPortRouter.hpp"


uint32_t DebugPortTransportLayer::calculateChecksum(void* p_byteArray, uint32_t numBytes)
{	
	uint32_t myChecksum = 0;
	unsigned char* p = (unsigned char *)p_byteArray;
	for (uint32_t i=0; i<numBytes; i++)
	{
		myChecksum += p[i];
	}
	return myChecksum;
}

void DebugPortTransportLayer::generatePacketHeader()
{
    uint32_t numBytesInPayload = mp_transmitPayload->getNumberOfValidBytes();
    void* p_payload = mp_transmitPayload->getBufferStartAddress();

	// GENERATE HEADER
    m_transmitPacketHeader.m_framingSignature = DEBUG_PACKET_UINT32_FRAMING_SIGNATURE;

    m_transmitPacketHeader.m_packetPayloadChecksum = calculateChecksum(p_payload, numBytesInPayload);

    m_transmitPacketHeader.m_payloadSize = numBytesInPayload;

    m_transmitPacketHeader.m_packetType = m_transmitDebugDataType;

	//0 for checksum
    m_transmitPacketHeader.m_reserve = 0;

	//0 to calculate checksum
    m_transmitPacketHeader.m_packetHeaderChecksum = 0;

	//Calculate checksum
    m_transmitPacketHeader.m_packetHeaderChecksum = calculateChecksum(p_payload, numBytesInPayload);
}

uint16_t DebugPortTransportLayer::receivePacketHeader() //receive = request
{
    cefCommandDebugPortHeader_t* p_header = (cefCommandDebugPortHeader_t*)myReceiveCefBuffer.getBufferStartAddress();

	//Check to see if we have received enough bytes for a full packet header
	uint32_t headerSizeInBytes = sizeof(cefCommandDebugPortHeader_t);
	if(m_myDebugPortDriver.getCurrentBytesReceived() >= headerSizeInBytes)
	{
		//Check to see if Checksum header matches
		uint32_t headerCheck = calculateChecksum(p_header, (sizeof(cefCommandDebugPortHeader_t)-sizeof(cefCommandDebugPortHeader_t::m_packetHeaderChecksum)));
		uint16_t packetHeaderChecksum = p_header->m_packetHeaderChecksum;
		if(headerCheck != packetHeaderChecksum)
		{
			//Checksum header does not match
			LOG_WARNING(Logging::LogModuleIdCefInfrastructure, "DebugTransportLayer Header Checksum does not match. actual=0x{:x}, expected=0x{:x}",
			        headerCheck, packetHeaderChecksum, 0);
			m_receiveErrorStatus = errorCode_debugPortTransportPacketHeaderChecksumMismatch;
			return stateReceiveFinished;
		}
		//Get/Set packet size (header + packet)
		m_expectedNumBytesInReceivePacket = p_header->m_payloadSize + sizeof(cefCommandDebugPortHeader_t);
		m_myDebugPortDriver.editReceiveSize(m_expectedNumBytesInReceivePacket);
		return stateRecvWaitForCefPacket;
	}
	return stateRecvWaitForPacketHeader;
}

void DebugPortTransportLayer::transmitStateMachine(void) //transmit = cefResponse 
{
    /**
     * To make sure the transport layer doesn't consume more than it's fair share of the
     * processor resources, only one state is executed each time through the loop.  This
     * is probably way conservative, but it avoids having to know how much work is going
     * on in each state.
     */

	switch (m_transmitState)
	{
        case stateXmitWaitingForBuffer:
        {
            mp_transmitPayload = CommandDebugPortRouter::instance().checkoutCefTransmitBuffer(m_transmitDebugDataType);
            if(mp_transmitPayload == nullptr)
            {
                //If buffer is not ready leave state machine don't block
                break;
            }

            //When buffer is ready generate packet header
            m_transmitState = stateXmitGeneratePacketHeader;
            break;
        }

        case stateXmitGeneratePacketHeader:
        {
            generatePacketHeader();
            m_transmitState = stateXmitStartHeaderTransmit;
            break;
        }

        case stateXmitStartHeaderTransmit:
        {
            m_myDebugPortDriver.sendData(&m_transmitPacketHeader, sizeof(m_transmitPacketHeader));
            m_transmitState = stateXmitWaitForHeaderToTransmit;
            break;
        }

        case stateXmitWaitForHeaderToTransmit:
        {
            //check if sending is finished
            if(m_myDebugPortDriver.getSendInProgress())
            {
                //Sending not finished leave state machine
                //TODO make a timeout if it takes to long
                break;
            }
            //sending finished
            m_transmitState = stateXmitStartPayloadTransmit;
            break;
        }

        case stateXmitStartPayloadTransmit:
        {
            m_myDebugPortDriver.sendData(mp_transmitPayload->getBufferStartAddress(), mp_transmitPayload->getNumberOfValidBytes());
            m_transmitState = stateXmitWaitForPayloadToTransmit;
            break;
        }

        case stateXmitWaitForPayloadToTransmit:
        {
            //check if sending is finished
            if(m_myDebugPortDriver.getSendInProgress())
            {
                //Sending not finished leave state machine
                //TODO make a timeout if it takes to long
                break;
            }
            //sending finished
            m_transmitState = stateXmitFinishedSendingPacket;
            break;
        }

	case stateXmitFinishedSendingPacket:
        {
            CommandDebugPortRouter::instance().checkinCefTransmitBuffer(mp_transmitPayload);
            m_transmitState = stateXmitWaitingForBuffer;
            break;
        }
	default:
        {
            LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "DebugTransportLayer Transmit State Machien in unknown state.",
                    0, 0, 0);
            break;
        }
	}
}

void DebugPortTransportLayer::receiveStateMachine(void)
{
    /**
     * To make sure the transport layer doesn't consume more than it's fair share of the
     * processor resources, only one state is executed each time through the loop.  This
     * is probably way conservative, but it avoids having to know how much work is going
     * on in each state.
     */

	switch (m_receiveState)
	{
        case stateRecvWaitForBuffer:
        {
            // Wait until we have a buffer to receive data into
            mp_commandReceiveCefBuffer = CommandDebugPortRouter::instance().checkoutCefCommandReceiveBuffer();

            /**
             * The first implementation of the Debug Port driver does not support receiving into
             * two different buffers (one for the Transport layer header, and another for the Application data).
             * As such, we need a contiguous amount of memory created in the transport layer, we receive into that
             * buffer, and then we copy the data into the mp_commandReceiveCefBuffer.
             *
             * Eventually we want to be able to receive into two different buffers and avoid this copy, as
             * well as the intanglement of the transport layer needing to know how big of a buffer the application
             * layer needs.
             *
             * Data will be transferred into myReceiveCefBuffer, then the command data will be
             * moved to mp_commandReceiveCefBuffer once a valid packet is received.
             */

            if(mp_commandReceiveCefBuffer == nullptr)
            {
                // Exit out of here and try for a buffer next time
                break;
            }

            m_receiveErrorStatus = errorCode_OK;

            m_myDebugPortDriver.startReceive(myReceiveCefBuffer.getBufferStartAddress(),
                                             myReceiveCefBuffer.getMaxBufferSizeInBytes());
            m_receiveState = stateRecvWaitForPacketHeader;

            break;
        }

        case stateRecvWaitForPacketHeader:
        {
            m_receiveState = receivePacketHeader();
            break;
        }

        case stateRecvWaitForCefPacket:
        {
            // Wait until have all the bytes in the packet
            if(m_myDebugPortDriver.getCurrentBytesReceived() < m_expectedNumBytesInReceivePacket)
            {
                break;
            }
            m_receiveState = stateRecvFinishedRecv;
            break;
        }

        case stateRecvFinishedRecv:
        {
            //ensure the payload checksum matches what is expected
            cefCommandDebugPortHeader_t* p_Header = (cefCommandDebugPortHeader_t*)myReceiveCefBuffer.getBufferStartAddress();
            uint8_t* p_payload = (uint8_t*)p_Header + sizeof(cefCommandDebugPortHeader_t);

            uint32_t headerCheck = calculateChecksum(p_payload, p_Header->m_payloadSize);
            if(headerCheck != p_Header->m_packetPayloadChecksum)
            {
                LOG_WARNING(Logging::LogModuleIdCefInfrastructure, "DebugTransportLayer debug packet checksum does not match.  Actual=0x{:x), Expected=0x{:x}",
                        headerCheck, p_Header->m_packetPayloadChecksum, 0);
                m_receiveErrorStatus = errorCode_debugPortTransportPayloadChecksumMismatch;
                m_receiveState = stateReceiveFinished;
                break;
            }

            /* We have a valid command, copy it into the checked out buffer if there is room */
            uint32_t numBytesInPayload = p_Header->m_payloadSize;
            if (numBytesInPayload > mp_commandReceiveCefBuffer->getMaxBufferSizeInBytes())
            {
                // The checked out buffer is not big enough to accept the command!
                LOG_WARNING(Logging::LogModuleIdCefInfrastructure, "DebugTransportLayer checked out buffer to small for payload.  Actual={%d), Received={%d}",
                        numBytesInPayload, mp_commandReceiveCefBuffer->getMaxBufferSizeInBytes(), 0);
                m_receiveErrorStatus = errorCode_debugPortTransportBufferNotBigEnoughForPayload;
                m_receiveState = stateReceiveFinished;
                break;
            }
            //! Update how many valid bytes are in the buffer
            mp_commandReceiveCefBuffer->setNumberOfValidBytes(numBytesInPayload);

            memcpy(mp_commandReceiveCefBuffer->getBufferStartAddress(), p_payload, numBytesInPayload);

            m_receiveState = stateReceiveFinished;
            break;
        }

        case stateReceiveFinished:
        {
            // Finished as much as we could do (we could have ran into an error) so return/checkin the buffer
            CommandDebugPortRouter::instance().checkinCefCommandReceiveBuffer(mp_commandReceiveCefBuffer, m_receiveErrorStatus);

            m_receiveState= stateRecvWaitForBuffer;
            break;
        }

	default:
		LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "DebugTransportLayer Receive State Machine in unknown state.", 0, 0, 0);
		break;
	}
}
