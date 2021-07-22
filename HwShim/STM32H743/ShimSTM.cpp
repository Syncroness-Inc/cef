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
#include "Inc/main.h"

//Hal callback override will call shim::rxCallback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	extern UART_HandleTypeDef huart3;
	if(&huart3 == huart)
	{
		ShimBase::getInstance().rxCallback();
	}
}

void ShimSTM::rxCallback()
{
	(m_callbackClass->*m_callback)();
}

void ShimSTM::startInteruptSend(void*sendBuffer, int bufferSize)
{
	extern UART_HandleTypeDef huart3;
	HAL_UART_Transmit_IT(&huart3, (uint8_t *)sendBuffer, bufferSize);
}

void ShimSTM::startInteruptReceive(void* recieveByte, SerialPortDriverHwImpl* callbackClass, void (SerialPortDriverHwImpl::* callback)(void))
{
	m_callbackClass = callbackClass;
	m_callback = callback;
	extern UART_HandleTypeDef huart3;
	HAL_UART_Receive_IT(&huart3, ((uint8_t *)recieveByte), sizeof(uint8_t));
}


