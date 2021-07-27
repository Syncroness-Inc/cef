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


/**
 * Implementation of ShimBase functions
 * Note:  Pure virtual functions can cause the compiler/linker to pull in
 * an excess amount of code.  So, pure virtual functions should be implemented to
 * fail at run time instead (not ideal, but the least bad option).
 */

#include "ShimBase.hpp"
#include "Logging.hpp"

#include "ShimSTM.hpp"
//Instance of STM shim
static ShimSTM shimInstance;

ShimBase& ShimBase::getInstance()
{
	return shimInstance;
}

void ShimBase::rxCallback(void)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class ShimBase::rxCallback() called, supposed to be implemented in derived class");
}

void ShimBase::errorCallback(void)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class ShimBase::errorCallback() called, supposed to be implemented in derived class");
}

void ShimBase::startInterruptSend(void*sendBuffer, int bufferSize)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class ShimBase::startInterruptSend() called, supposed to be implemented in derived class");
}

void ShimBase::startInterruptReceive(void* receiveByte, SerialPortDriverHwImpl* callbackClass, bool (SerialPortDriverHwImpl::* callback)(void))
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class ShimBase::startInterruptReceive() called, supposed to be implemented in derived class");
}

void ShimBase::forceStopReceive(void)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class ShimBase::forceStopReveive() called, supposed to be implemented in derived class");
}

void ShimBase::startErrorCallback(SerialPortDriverHwImpl* errorCallbackClass, void (SerialPortDriverHwImpl::* errorCallback)(errorCode_t error))
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class ShimBase::startErrorCallback() called, supposed to be implemented in derived class");
}
