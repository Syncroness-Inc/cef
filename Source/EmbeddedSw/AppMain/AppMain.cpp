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
#include "CommandDebugPortRouter.hpp"
#include "CommandCefCommandProxy.hpp"


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

// Need to allocate these variables out of the scope of the stack

CommandPing testPing;

static void johnTesting()
{
#if 0 // turn this on to check ping command
    cefCommandPingRequest_t cefCommandPingRequest;
    cefCommandPingResponse_t cefCommandPingResponse;

    cefCommandPingRequest.m_header.m_commandErrorCode = errorCode_OK;
    cefCommandPingRequest.m_header.m_commandOpCode = commandOpCodePing;
    cefCommandPingRequest.m_header.m_commandNumBytes = sizeof(cefCommandPingRequest);
    cefCommandPingRequest.m_header.m_commandRequestResponseSequenceNumberPython = 777;

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
#endif


#if 0  // turn this on to test RingBufferOfVoidPointers
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
#endif  // ring buffer test

#if 0  // we don't want to do this anymore, as a later command will need to allocate a ping command
    // test for CommandGenerator
    //CommandGenerator myCommandGenerator;
    bool allocatableCommand = false;
    CommandBase* p_cgCommand = CommandGenerator::instance().allocateCommand(commandOpCodePing, allocatableCommand);
    //p_cgCommand->execute(nullptr);  // make sure not nullptr before call; and didn't initialize, but if in ping command, good enough!

    CommandExecutor::instance().addCommandToQueue(p_cgCommand);  //(to be executed by main while loop)
    //CommandExecutor::instance().executeCommands(2);   // return value should be 1
#endif


#if 0 // turn this on to check router...may screw up normal operation...so test manually, disable, then reboot...
    //*************** Check out Logging in CommandDebugPortRouter
    // Tests below assumes code has been hacked to only have 2 Log Buffers.
    cefLog_t* p_logTemp = (cefLog_t*)0x1;
    cefLog_t* p_logTemp2 = (cefLog_t*) 0x3;
    cefLog_t* p_logTemp3 = (cefLog_t*) 0x4;
    cefLogPacket_t* p_logPacket1 = (cefLogPacket_t*)0x5;
    p_logTemp3 = p_logTemp2;

    p_logPacket1 = CommandDebugPortRouter::instance().checkoutLogPacket();  // nullptr
    p_logTemp = CommandDebugPortRouter::instance().checkoutLogBuffer();	// valid pointer
    p_logTemp2= CommandDebugPortRouter::instance().checkoutLogBuffer();	// valid pointer
    p_logTemp3= CommandDebugPortRouter::instance().checkoutLogBuffer();	// should be a nullptr
    CommandDebugPortRouter::instance().returnLogBuffer(p_logTemp2); // should work
    p_logPacket1 = CommandDebugPortRouter::instance().checkoutLogPacket();  // should work
    CommandDebugPortRouter::instance().returnLogPacket(p_logPacket1);   // should work
    CommandDebugPortRouter::instance().returnLogBuffer(p_logTemp); // should work
    p_logPacket1 = CommandDebugPortRouter::instance().checkoutLogPacket();  // should work, 16 bytes higher address than p_logTemp because has header
    CommandDebugPortRouter::instance().returnLogPacket(p_logPacket1);   // should work, inspect member variables to make sure numEntries what expect on two lists



    //*************** Check out cefCommandBuffer in CommandDebugPortRouter
    cefCommandPacketMaximum_t* p_cefPacket = (cefCommandPacketMaximum_t*)0x1;
    cefCommandMaximum_t* p_cefCommand = (cefCommandMaximum_t*)0x2;
    uint32_t numBytesInCommandResponse = 7;

    //CommandDebugPortRouter::instance().returnCefReceiveCommandPacket(p_cefPacket); // log fatal

    // confirm get nullptr back on all these functions  (or log fatal)
    p_cefCommand = CommandDebugPortRouter::instance().checkoutCefCommandBuffer();
    //CommandDebugPortRouter::instance().returnCefCommandBuffer(p_cefCommand, 8);  // logfatal

    p_cefPacket = CommandDebugPortRouter::instance().checkoutCefTransmitCommandPacket(numBytesInCommandResponse);  // nullptr
    //CommandDebugPortRouter::instance().returnCefTransmitCommandPacket(p_cefPacket);   //logfatal

    // confirm normal flow works
    // go to the point where can show get the request buffer for a 2nd time

    p_cefPacket = CommandDebugPortRouter::instance().checkoutCefReceiveCommandPacket();
    //p_cefPacket = CommandDebugPortRouter::instance().checkoutCefReceiveCommandPacket();  // nullptr
    CommandDebugPortRouter::instance().returnCefReceiveCommandPacket(p_cefPacket);

    p_cefCommand = CommandDebugPortRouter::instance().checkoutCefCommandBuffer();
    CommandDebugPortRouter::instance().returnCefCommandBuffer(p_cefCommand, numBytesInCommandResponse);

    p_cefPacket = CommandDebugPortRouter::instance().checkoutCefTransmitCommandPacket(numBytesInCommandResponse);
    CommandDebugPortRouter::instance().returnCefTransmitCommandPacket(p_cefPacket);

    // this lines screws up the next test p_cefPacket = CommandDebugPortRouter::instance().checkoutCefReceiveCommandPacket();
#endif // router


#if 1
    /*********** Check out CommandCefCommandProxy *************/
    // Drive the CEF Command packet through its state machine
    cefCommandPacketMaximum_t* p_cefPacket2 = CommandDebugPortRouter::instance().checkoutCefReceiveCommandPacket();

    // Initialize it so we run the ping command (don't worry about internals of command...it will just return an error code)
	cefCommandPingRequest_t* p_cefPing = (cefCommandPingRequest_t*)&(p_cefPacket2->m_cefCommandPayload);
	p_cefPing->m_header.m_commandErrorCode = 0;
	p_cefPing->m_header.m_commandNumBytes = sizeof(cefCommandPingRequest_t);
	p_cefPing->m_header.m_commandOpCode = commandOpCodePing;
	p_cefPing->m_header.m_commandRequestResponseSequenceNumberPython = 777;
	p_cefPing->m_header.m_padding1 = 333;

    CommandDebugPortRouter::instance().returnCefReceiveCommandPacket(p_cefPacket2);   // return it so it drives things to the right state
    // when we start up the code, CommandCefCommandProxy should find a CEF ping command to work on
    // and it should return a response to be transmitted to the CommandDebugPortRouter (look at singleton to see buffer state)
#endif

}

void AppMain::initialize()
{
	//! Tony, call uart shim initialization from here

	CommandExecutor::instance().addCommandToQueue(&CommandDebugPortRouter::instance());
	CommandExecutor::instance().addCommandToQueue(&CommandCefCommandProxy::instance()); 
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
