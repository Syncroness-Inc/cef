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
    def __init__(self):
        self.header = cefContract.cefCommandHeader()
        self.header.m_commandSequenceNumber = 0 # this is populated at transmit-time
        self.header.m_commandErrorCode = cefContract.errorCode.errorCode_OK.value
        self.request = None
        self.expectedResponse = None

    @abstractmethod
    def buildCommand(self):
        """
        Populate the command header and request body according to the CEF contract
        """
        pass

    @abstractmethod
    def validateResponse(self):
        pass

    @abstractmethod
    def expectedResponseType(self):
        pass

    @abstractmethod
    def validateHeader(self):
        pass

    def __len__(self):
        return len(bytes(self.request))

    def payload(self):
        return bytes(self.request)

    def setSequenceNumber(self, sequenceNumber):
        self.header.m_commandSequenceNumber = sequenceNumber

    def expectedResponseLength(self):
        return len(bytes(self.expectedResponse))


class CommandPing(Command):

    OFFSET_VALUE = 0
    TEST_VALUE = 0

    def __init__(self):
        super().__init__()
        self.buildCommand()

    def buildCommand(self):
        self.header.m_commandOpCode = cefContract.commandOpCode.commandOpCodePing.value
        self.header.m_commandNumBytes = ctypes.sizeof(cefContract.cefCommandPingRequest)

        self.request = cefContract.cefCommandPingRequest()
        self.request.m_header = self.header
        self.request.m_uint8Value = cefContract.CMD_PING_UINT8_REQUEST_EXPECTED_VALUE
        self.request.m_uint16Value = cefContract.CMD_PING_UINT16_REQUEST_EXPECTED_VALUE
        self.request.m_uint32Value = cefContract.CMD_PING_UINT32_REQUEST_EXPECTED_VALUE
        self.request.m_uint64Value = cefContract.CMD_PING_UINT64_REQUEST_EXPECTED_VALUE
        self.request.m_offsetToAddToResponse = self.OFFSET_VALUE
        self.request.m_testValue = self.TEST_VALUE

        self.expectedResponse = cefContract.cefCommandPingResponse()
        self.expectedResponse.m_header = self.header
        self.expectedResponse.m_uint8Value = cefContract.CMD_PING_UINT8_REQUEST_EXPECTED_VALUE + self.OFFSET_VALUE
        self.expectedResponse.m_uint16Value = cefContract.CMD_PING_UINT16_REQUEST_EXPECTED_VALUE + self.OFFSET_VALUE
        self.expectedResponse.m_uint32Value = cefContract.CMD_PING_UINT32_REQUEST_EXPECTED_VALUE + self.OFFSET_VALUE
        self.expectedResponse.m_uint64Value = cefContract.CMD_PING_UINT64_REQUEST_EXPECTED_VALUE + self.OFFSET_VALUE
        self.expectedResponse.m_testValue = self.TEST_VALUE

    def validateResponse(self, receivedResponse: cefContract.cefCommandPingResponse):
        if receivedResponse.m_uint8Value != self.expectedResponse.m_uint8Value or \
            receivedResponse.m_uint16Value != self.expectedResponse.m_uint16Value or \
            receivedResponse.m_uint32Value != self.expectedResponse.m_uint32Value or \
            receivedResponse.m_uint64Value != self.expectedResponse.m_uint64Value or \
            receivedResponse.m_testValue != self.expectedResponse.m_testValue:
            return False
        else:
            return True

    def validateHeader(self, responseHeader: cefContract.cefCommandHeader):
        if responseHeader.m_commandOpCode != self.header.m_commandOpCode or \
            responseHeader.m_commandSequenceNumber != self.header.m_commandSequenceNumber or \
            responseHeader.m_commandErrorCode != cefContract.errorCode.errorCode_OK.value:
            return False
        else:
            return True

    def expectedResponseType(self):
        return type(self.expectedResponse).__new__(cefContract.cefCommandPingResponse)