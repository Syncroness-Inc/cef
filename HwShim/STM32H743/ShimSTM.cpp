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



#include "ShimSTM.h"

//Hal callback override will call shim::rxCallback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	ShimBase::getInstance().rxCallback();
}

void ShimSTM::startInteruptSend()
{
	if(checkForUartHandle() && checkWritePointer())
	{
		HAL_UART_Transmit_IT(m_myUartHandle, (uint8_t *)m_readPtr, 12);
	}
}

void ShimSTM::startInteruptRecieve(uint8_t location)
{
	if(checkForUartHandle() && checkReadPointer())
	{
		HAL_UART_Receive_IT(m_myUartHandle, ((uint8_t *)m_readPtr + location), sizeof(uint8_t));
	}
}



