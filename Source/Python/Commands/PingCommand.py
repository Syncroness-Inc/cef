# ##################################################################
#\copyright COPYRIGHT AND PROPRIETARY RIGHTS NOTICES:
#
#Copyright (C) 2021, an unpublished work by Syncroness, Inc.
#All rights reserved.
#
#This material contains the valuable properties and trade secrets of
#Syncroness of Westminster, CO, United States of America
#embodying substantial creative efforts and confidential information,
#ideas and expressions, no part of which may be reproduced or
#transmitted in any form or by any means, electronic, mechanical, or
#otherwise, including photocopying and recording or in connection
#with any information storage or retrieval system, without the prior
#written permission of Syncroness.
################################################################## #


import ctypes

from .CommandBase import *


class CommandPing(CommandBase):
    """
    Basic command for debug and proof of connectivity between host and target. Expected values
    have prescribed sizes to test for alignement/boundary, and conspicuous bit patterns useful 
    for checking endianness compatibility.
    """

    OFFSET_VALUE = 0 # tests state-machine function on the target, which adds this value to the expected values
    TEST_VALUE = 0 # user-settable value, echoed back as part of a valid response

    def __init__(self):
        super().__init__()
        self.buildCommand()
        self.expectedResponseType = type(self.expectedResponse).__new__(cefContract.cefCommandPingResponse)

    def buildCommand(self):
        """
        Create the Ping request for transmission and the expected corresponding response according
        to cefContract.
        """
        # build the header
        self.header.m_commandSequenceNumber = 0 # this is populated at transmit-time
        self.header.m_commandErrorCode = cefContract.errorCode.errorCode_OK.value
        self.header.m_commandOpCode = cefContract.commandOpCode.commandOpCodePing.value
        self.header.m_commandNumBytes = ctypes.sizeof(cefContract.cefCommandPingRequest)

        # build the body
        self.request = cefContract.cefCommandPingRequest()
        self.request.m_header = self.header
        self.request.m_uint8Value = cefContract.CMD_PING_UINT8_REQUEST_EXPECTED_VALUE
        self.request.m_uint16Value = cefContract.CMD_PING_UINT16_REQUEST_EXPECTED_VALUE
        self.request.m_uint32Value = cefContract.CMD_PING_UINT32_REQUEST_EXPECTED_VALUE
        self.request.m_uint64Value = cefContract.CMD_PING_UINT64_REQUEST_EXPECTED_VALUE
        self.request.m_offsetToAddToResponse = self.OFFSET_VALUE
        self.request.m_testValue = self.TEST_VALUE

        # template for the expected response from the target
        self.expectedResponse = cefContract.cefCommandPingResponse()
        self.expectedResponse.m_header = self.header
        self.expectedResponse.m_uint8Value = cefContract.CMD_PING_UINT8_REQUEST_EXPECTED_VALUE + self.OFFSET_VALUE
        self.expectedResponse.m_uint16Value = cefContract.CMD_PING_UINT16_REQUEST_EXPECTED_VALUE + self.OFFSET_VALUE
        self.expectedResponse.m_uint32Value = cefContract.CMD_PING_UINT32_REQUEST_EXPECTED_VALUE + self.OFFSET_VALUE
        self.expectedResponse.m_uint64Value = cefContract.CMD_PING_UINT64_REQUEST_EXPECTED_VALUE + self.OFFSET_VALUE
        self.expectedResponse.m_testValue = self.TEST_VALUE

    def validateResponseBody(self, receivedResponse: cefContract.cefCommandPingResponse):
        """
        Ping-specific response field checking
        """
        self.receivedResponse = receivedResponse
        if receivedResponse.m_uint8Value != self.expectedResponse.m_uint8Value or \
            receivedResponse.m_uint16Value != self.expectedResponse.m_uint16Value or \
            receivedResponse.m_uint32Value != self.expectedResponse.m_uint32Value or \
            receivedResponse.m_uint64Value != self.expectedResponse.m_uint64Value :
            print("Invalid Ping response value(s)")
            return False
        else:
            return True
