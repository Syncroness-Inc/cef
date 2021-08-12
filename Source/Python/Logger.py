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
import logging

sys.path.append(dirname(dirname(abspath(__file__))))
from Shared import cefContract


class Logger:
    """
    Object for handling extracted log messages. Logs have a header and body, the
    header is the same structure used by command requests and responses. The body
    is a collection of useful debug data, contextualized with a string.
    This class primarily parses and assembles a full log entry from the received fields
    and stores the entry to disk.
    """

    def __init__(self):
        self.sequenceNumber = 0
        self.fieldPattern = "{:X}"

    def validateResponseHeader(self, responseHeader: cefContract.cefCommandHeader):
        # confirm message size
        expectedSize = ctypes.sizeof(cefContract.cefLog)

        # log messages should only use the OK error code
        if responseHeader.m_commandErrorCode != cefContract.errorCode.errorCode_OK.value:
            print("Log Response Error Code: {}".format(responseHeader.m_commandErrorCode))
            return False

    def processLogMessage(self, log: cefContract.cefLog):
        # check the log string for expected number of variables
        logString = bytes.decode(log.m_logString)
        numLeftBraces = logString.count("{")
        numRightBraces = logString.count("}")

        # check for a malformed string (missing variable braces)
        malformed = numLeftBraces != numRightBraces

        # save off the variables for easier iterating
        logVars = [log.m_logVariable1, log.m_logVariable2, log.m_logVariable3]
        
        # combine message string with attached variables
        if not malformed:
            for n in range(numLeftBraces):
                logString = logString.replace(self.fieldPattern, f'{logVars[n]:X}', 1)
        print(logString)

        # INFO  SEQNUM  TIMESTAMP  STRING  FILE:LINE   ID  TYPE  RAW
        # write to the log file

if __name__ == '__main__':
    message = cefContract.cefLog()
    message.m_logVariable1 = 100
    message.m_logVariable2 = 200
    message.m_logVariable3 = 300
    message.m_timeStamp = 12345
    message.m_logString = b"Test String, var1 = 0x{:X}, var2 = 0x{:X}"
    message.m_fileName = b"Module.cpp"
    message.m_fileLineNumber = 400
    message.m_logSequenceNumber = 0
    message.m_moduleId = 1 # debug commands
    message.m_logType = 1 # info

    logger = Logger()
    logger.processLogMessage(message)
