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


#include "ShimSTM.hpp"
#include "main.h"

//Hal callback override will call shim::rxCallback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	extern UART_HandleTypeDef huart3;
	if(&huart3 == huart)
	{
		ShimBase::getInstance().rxCallback();
	}
}

//Hal callback override will call shim::rxCallback
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	extern UART_HandleTypeDef huart3;
	if(&huart3 == huart)
	{
		ShimBase::getInstance().txCallback();
	}
}

//Hal callback override uart error shim::errorCallback
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	// TODO - figure out if it is transmit and let transport layer know it failed/can try again
	extern UART_HandleTypeDef huart3;
	if(&huart3 == huart)
	{
		ShimBase::getInstance().errorCallback();
	}
}

void ShimSTM::rxCallback()
{
	if(mp_rxCallbackClass != nullptr && mp_rxCallback != nullptr)
	{
		(mp_rxCallbackClass->*mp_rxCallback)();
	}
}

void ShimSTM::txCallback()
{
	/** 
	 * Decided if you need to do anything here in the future
	 * As of now HAL_UART_GetState is polled by Transport 
	 * and action is decided from there
	 ** */
}

void ShimSTM::errorCallback()
{
	if(mp_errorCallbackClass != nullptr && mp_errorCallback != nullptr)
	{
		extern UART_HandleTypeDef huart3;
		errorCode_t cefError = errorCode_debugPortErrorCodeUnknown;
		uint32_t halErrorCode = HAL_UART_GetError(&huart3);
		switch (halErrorCode)
		{
		case HAL_UART_ERROR_NONE :
			cefError = errorCode_debugPortErrorCodeNone;
			break;
		case HAL_UART_ERROR_PE :
			cefError = errorCode_debugPortErrorCodeParity;
			break;
		case HAL_UART_ERROR_NE :
			cefError = errorCode_debugPortErrorCodeNoise;
			break;
		case HAL_UART_ERROR_FE :
			cefError = errorCode_debugPortErrorCodeFrame;
			break;
		case HAL_UART_ERROR_ORE :
			cefError = errorCode_debugPortErrorCodeOverrun;
			break;
		default:
			break;
		}
		(mp_errorCallbackClass->*mp_errorCallback)(cefError);
	}
}

bool ShimBase::getSendInProgress(void)
{
	//We can in the future make this more sophisticated to return more information then sending/not sending
	extern UART_HandleTypeDef huart3;
	if(HAL_UART_GetState(&huart3) == HAL_UART_STATE_BUSY_TX)
	{
		return true;
	}
	return false;
}

bool ShimSTM::startInterruptSend(void*sendBuffer, int bufferSize)
{
		extern UART_HandleTypeDef huart3;
		HAL_StatusTypeDef startSend = HAL_UART_Transmit_IT(&huart3, (uint8_t *)sendBuffer, bufferSize);
		if(startSend )
		//TODO
		return true;
}

void ShimSTM::startInterruptReceive(void* receiveByte, SerialPortDriverHwImpl* callbackClass, bool (SerialPortDriverHwImpl::* callback)(void))
{
	mp_rxCallbackClass = callbackClass;
	mp_rxCallback = callback;
	extern UART_HandleTypeDef huart3;
	HAL_UART_Receive_IT(&huart3, ((uint8_t *)receiveByte), sizeof(uint8_t));
}

void ShimSTM::startErrorCallback(SerialPortDriverHwImpl* errorCallbackClass, void (SerialPortDriverHwImpl::* errorCallback)(errorCode_t error))
{
	mp_errorCallbackClass = errorCallbackClass;
	mp_errorCallback = errorCallback;
}

void ShimSTM::forceStopReceive(void)
{
	extern UART_HandleTypeDef huart3;
	HAL_UART_AbortReceive_IT (&huart3);
}


