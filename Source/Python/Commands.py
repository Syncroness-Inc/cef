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


import sys
from os.path import dirname, abspath
import ctypes
from abc import ABC, abstractmethod

sys.path.append(dirname(dirname(abspath(__file__))))
from Shared import cefContract


class Command(ABC):
    """
    Partially abstract base class for CEF commands. All commands have a header and body.
    The body can be either a request (originating from the Python Utility side) or a response
    to that request originating from the embedded target. Both use the same struct for header.
    """
    def __init__(self):
        self.header = cefContract.cefCommandHeader()
        self.request = None
        self.expectedResponse = None

    @abstractmethod
    def buildCommand(self):
        """
        Populate the command header and request body according to the CEF contract
        """
        pass

    @abstractmethod
    def validateResponseBody(self):
        """
        Command-specific field value checking
        """
        pass

    @abstractmethod
    def expectedResponseType(self):
        """
        Command-specific struct to validate against upon response
        """
        pass

    def validateResponseHeader(self, responseHeader: cefContract.cefCommandHeader):
        """
        Common header field value checking. The response sequence number must match what was sent out, 
        the command must have executed on-target without error, and the response OpCode must match what was sent.
        @param responseHeader: the command header obeying the structure defined in cefContract
        @return boolean: False if any check fails, else True
        """
        if responseHeader.m_commandSequenceNumber != self.header.m_commandSequenceNumber:
            print("Response Sequence Number mis-match - received: {}, sent: {}".format(responseHeader.m_commandSequenceNumber, self.header.m_commandSequenceNumber))
            return False
        if responseHeader.m_commandErrorCode != cefContract.errorCode.errorCode_OK.value:
            print("Response Error Code: {}".format(responseHeader.m_commandErrorCode))
            return False
        if responseHeader.m_commandOpCode != self.header.m_commandOpCode:
            print("Response OpCode mis-match - received: {}, sent: {}".format(responseHeader.m_commandOpCode, self.header.m_commandOpCode))
            return False
        return True

    def __len__(self):
        return len(bytes(self.request))

    def payload(self):
        return bytes(self.request)

    def setSequenceNumber(self, sequenceNumber):
        self.header.m_commandSequenceNumber = sequenceNumber

    def expectedResponseLength(self):
        return len(bytes(self.expectedResponse))


class CommandPing(Command):
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
        if receivedResponse.m_uint8Value != self.expectedResponse.m_uint8Value or \
            receivedResponse.m_uint16Value != self.expectedResponse.m_uint16Value or \
            receivedResponse.m_uint32Value != self.expectedResponse.m_uint32Value or \
            receivedResponse.m_uint64Value != self.expectedResponse.m_uint64Value or \
            receivedResponse.m_testValue != self.expectedResponse.m_testValue:
            return False
        else:
            return True

    def expectedResponseType(self):
        """
        The matching Ping response for a Ping request
        """
        return type(self.expectedResponse).__new__(cefContract.cefCommandPingResponse)
