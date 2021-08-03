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
from abc import ABC, abstractmethod

sys.path.append(dirname(dirname(abspath(__file__))))
from Shared import cefContract


class CommandBase(ABC):
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
        Populate the command header and request body according to the CEF contract. This should make
        use of any and all command-specific expected/test values for both requests and responses.
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
