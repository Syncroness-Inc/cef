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

#include "AppMain.hpp"
#include "Logging.hpp"
#include "CommandExecutor.hpp"


//! Singleton instantiation of AppMain
static AppMain appMainSingleton;

AppMain& AppMain::instance()
{
	return appMainSingleton;
}

// These include files are just needed for testing.
#include "main.h"
#include "SerialPortDriverHwImpl.hpp"
uint8_t receiveBuffer[528];
SerialPortDriverHwImpl debugPortDriver;
uint8_t testSend[23] = {'U','A','R','T',' ','S','t','a','r','t',' ','S','u','c','c','e','s','s','f','u','l','\r','\n'};
static void tonyTesting()
{
	debugPortDriver.startReceive(&receiveBuffer);

	extern UART_HandleTypeDef huart3;
	HAL_UART_Transmit_IT(&huart3, (uint8_t *)testSend, sizeof(testSend));

}


// These includes are needed just for the test code
#include "CommandBase.hpp"
#include "CommandPing.hpp"
#include "RingBufferOfVoidPointers.hpp"
#include "CommandGenerator.hpp"
static void johnTesting()
{
#if 1 // turn this on to check ring buffer manually (this code will generate warnings
    cefCommandPingRequest_t cefCommandPingRequest;
    cefCommandPingResponse_t cefCommandPingResponse;

    cefCommandPingRequest.m_header.m_commandErrorCode = errorCode_OK;
    cefCommandPingRequest.m_header.m_commandOpCode = commandOpCodePing;
    cefCommandPingRequest.m_header.m_commandNumBytes = sizeof(cefCommandPingRequest);
    cefCommandPingRequest.m_header.m_commandRequestResponseSequenceNumberPython = 777;

    // Call the constructor as if Proxy Command generated the code.
    CommandPing testPing;


    // These numbers should result in an error as not what was expected
    cefCommandPingRequest.m_offsetToAddToResponse = 1;
    cefCommandPingRequest.m_testValue = 2;
    cefCommandPingRequest.m_uint8Value = 0x30;
    cefCommandPingRequest.m_uint16Value = 0x3000;
    cefCommandPingRequest.m_uint32Value = 0x40000000;
    cefCommandPingRequest.m_uint64Value = 0x5000000000000000;

    testPing.importFromCefCommand(&cefCommandPingRequest);

    testPing.execute(nullptr);

    testPing.exportToCefCommand(&cefCommandPingResponse);


    bool result ;

    void* pTemp = (void*)0x1234;

    uint32_t numEntries;



    RingBufferOfVoidPointers rb(3);
    result = rb.isEmpty();
    result = rb.isFull();
    numEntries = rb.getCurrentNumberOfEntries();  // 0

    // this is a hack to remove warnings
    if (result == true)
    {
  	  ++numEntries;
    }

    result = rb.put(pTemp);
    result = rb.isEmpty();
    result = rb.put((void*)2);
    numEntries = rb.getCurrentNumberOfEntries();  //2
    result = rb.put((void*)3);
    numEntries = rb.getCurrentNumberOfEntries();  // 3
    result = rb.put((void*)4);
    numEntries = rb.getCurrentNumberOfEntries();  // 3
    result = rb.isEmpty();
    result = rb.isFull();
    void* vTemp;
    result = rb.get(vTemp);
    numEntries = rb.getCurrentNumberOfEntries();  // 2
    result = rb.isEmpty();
    result = rb.isFull();
    result = rb.get(vTemp);
    numEntries = rb.getCurrentNumberOfEntries();  // 1
    result = rb.get(vTemp);
    numEntries = rb.getCurrentNumberOfEntries();  // 0
    result = rb.isEmpty();
    result = rb.isEmpty();
    result = rb.get(vTemp);
    result = rb.put((void*)5);
    numEntries = rb.getCurrentNumberOfEntries();  // 1
    result = rb.get(vTemp);
    result = rb.put((void*)6);
    result = rb.put((void*)7);
    numEntries = rb.getCurrentNumberOfEntries();  // 2
    result = rb.get(vTemp);
    result = rb.get(vTemp);
    numEntries = rb.getCurrentNumberOfEntries();  // 0
    result = rb.get(vTemp);


    result = rb.put((void*)8);
    result = rb.put((void*)9);
    result = rb.put((void*)10);
    result = rb.removeItem((void*)10);  // remove last item
    numEntries = rb.getCurrentNumberOfEntries();  // 2
    result = rb.removeItem((void*)8);  // first item
    numEntries = rb.getCurrentNumberOfEntries();  // 1
    result = rb.removeItem((void*)9);  // last item
    numEntries = rb.getCurrentNumberOfEntries();  // 8

    // test for CommandGenerator
    CommandGenerator myCommandGenerator;
    CommandBase* p_cgCommand = myCommandGenerator.allocateCommand(commandOpCodePing);
    //p_cgCommand->execute(nullptr);  // make sure not nullptr before call; and didn't initialize, but if in ping command, good enough!

    CommandExecutor::instance().addCommandToQueue(p_cgCommand);  //(to be executed by main while loop)
    //CommandExecutor::instance().executeCommands(2);   // return value should be 1

#endif


}

void AppMain::initialize()
{
	//! Tony, call uart shim initialization from here
}

void AppMain::runAppMain_noReturn()
{
	//! Complete initialization beyond what BSP has completed
	initialize();

	//! @toDo Remove this when have alternate means of testing
	tonyTesting();

	//! @toDo Remove this when have alternate means of testing
	johnTesting();

	//! run the infinite while loop.  There is no return from this routine.
	run();
}

void AppMain::run()
{
	/**
	 * Number of commands CommandExecutor allowed to execute each time through the while loop.
	 * This number needs to be tuned depending on how tight the watch dog timer needs to be set,
	 * as well as other tasks that may need to run from the forever while loop.
	 */
	uint32_t const numCommandsAllowedToExecute = 2;

	while (1)
	{
		CommandExecutor::instance().executeCommands(numCommandsAllowedToExecute);

		// When watch dog timer is implemented, this should be the one place the watch dog is petted
	}

	// We never should reach this point of the code!
    LOG_FATAL(Logging::LogModuleIdCefInfrastructure, "Unexpectedly exited AppMain infinite while loop!");
}
