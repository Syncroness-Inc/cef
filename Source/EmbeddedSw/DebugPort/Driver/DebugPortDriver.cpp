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

#include "DebugPortDriver.hpp"
#include "Logging.hpp"


bool DebugPortDriver::sendData(void* sendBuffer, uint32_t packetSize)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class DebugPortDriver::sendData() called, supposed to be implemented in derived class",
	        0, 0, 0);
	return false;
}

bool DebugPortDriver::getSendInProgress(void)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class DebugPortDriver::getSendBusy() called, supposed to be implemented in derived class",
	        0, 0, 0);
	return false;
}

bool DebugPortDriver::startReceive(void* receiveBuffer, uint32_t receiveSize)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class DebugPortDriver::startReceive() called, supposed to be implemented in derived class",
	        0, 0, 0);
	return false;  // needed to avoid compiler warning
}

uint32_t DebugPortDriver::getCurrentBytesReceived(void)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class DebugPortDriver::getCurrentBytesReceived() called, supposed to be implemented in derived class",
	        0, 0, 0);
	return 0;
}

void DebugPortDriver::editReceiveSize(uint32_t newReceiveSize)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class DebugPortDriver::editReceiveSize() called, supposed to be implemented in derived class",
	        0, 0, 0);
}

void DebugPortDriver::stopReceive()
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class DebugPortDriver::stopReceive() called, supposed to be implemented in derived class",
	        0, 0, 0);
}

void DebugPortDriver::setErrorCallback(void)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class DebugPortDriver::setErrorCallback() called, supposed to be implemented in derived class",
	        0, 0, 0);
}

errorCode_t DebugPortDriver::errorCallback(void)
{
	LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Base class DebugPortDriver::errorCallback() called, supposed to be implemented in derived class",
	        0, 0, 0);
	return errorCode_LogFatalReturn;
}

