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

//Hal callback override will call shim::rxCallback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	ShimBase::getInstance().rxCallback();
}

void ShimSTM::rxCallback()
{
	(m_callbackClass->*m_callback)();
}

void ShimSTM::startInteruptSend(void*sendBuffer, int bufferSize)
{
	if(checkForUartHandle())
	{
		HAL_UART_Transmit_IT(m_myUartHandle, (uint8_t *)sendBuffer, bufferSize);
	}
}

void ShimSTM::startInteruptRecieve(void* recieveByte, SerialPortDriverHwImpl* callbackClass, void (SerialPortDriverHwImpl::* callback)(void))
{
	if(checkForUartHandle())
	{
		m_callback = callback;
		HAL_UART_Receive_IT(m_myUartHandle, ((uint8_t *)recieveByte), sizeof(uint8_t));
	}
}

void ShimSTM::setUartHandle(UART_HandleTypeDef* myUartHandle)
{
	m_myUartHandle = myUartHandle;
}

bool ShimSTM::checkForUartHandle()
{
	if(m_myUartHandle == nullptr)
	{
		printf("ERROR NEED HANDLE\n");
		return false;
	}
	return true;
}



