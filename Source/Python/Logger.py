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

    CEF_LOG_FILENAME = "cefLog.log"

    def __init__(self):
        # logging.basicConfig(filename=self.CEF_LOG_FILENAME, encoding='utf-8', level=logging.DEBUG)
        logging.basicConfig(filename=self.CEF_LOG_FILENAME, format='%(levelname)s, %(message)s', level=logging.DEBUG)
        self.sequenceNumber = 0
        self.fieldPattern = "{:X}"

    def validateResponseHeader(self, responseHeader: cefContract.cefCommandHeader):
        # confirm message size
        expectedSize = ctypes.sizeof(cefContract.cefLog)
        if responseHeader.m_commandNumBytes != expectedSize:
            print("Log Response invalid number of bytes - receivd: {}, expected: {}".format(responseHeader.m_commandNumBytes, expectedSize))
            return False
        # log messages should only use the OK error code
        if responseHeader.m_commandErrorCode != cefContract.errorCode.errorCode_OK.value:
            print("Log Response Error Code: {}".format(responseHeader.m_commandErrorCode))
            return False

    def processLogMessage(self, log: cefContract.cefLog):
        #TODO sequence number incrementing and validation

        # check the log string for expected number of variables
        logString = bytes.decode(log.m_logString)
        numLeftBraces = logString.count("{")
        numRightBraces = logString.count("}")
        
        # check for missing variable braces and insert variables into the string
        malformed = numLeftBraces != numRightBraces
        #TODO should this be smarter and auto-detect the number of variable fields in the structure?
        logVars = [log.m_logVariable1, log.m_logVariable2, log.m_logVariable3] # for easier iterating
        if not malformed:
            for n in range(numLeftBraces):
                logString = logString.replace(self.fieldPattern, f'{logVars[n]:X}', 1)

        #TODO timestamp conversion? i.e. uint64 to a decimal

        # prepare the full log entry
        rawData = str(bytes(log))
        fileAndLine = bytes.decode(log.m_fileName) + ":" + str(log.m_fileLineNumber)
        #TODO do we want headers, either at the top of the file or as field names in the log entry?
        logEntry = ", ".join([str(log.m_logSequenceNumber),\
                              str(log.m_timeStamp),\
                              logString,\
                              fileAndLine,\
                              str(log.m_moduleId),\
                              rawData])

        # write to file
        if log.m_logType == cefContract.logType.logTypeDebug.value:
            logging.debug(logEntry)
        elif log.m_logType == cefContract.logType.logTypeInfo.value:
            logging.info(logEntry)
        elif log.m_logType == cefContract.logType.logTypeWarning.value:
            logging.warning(logEntry)
        elif log.m_logType == cefContract.logType.logTypeError.value:
            logging.error(logEntry)
        elif log.m_logType == cefContract.logType.logTypeFatal.value:
            logging.fatal(logEntry)


if __name__ == '__main__':
    message = cefContract.cefLog()
    message.m_logVariable1 = 100
    message.m_logVariable2 = 200
    message.m_logVariable3 = 300
    message.m_timeStamp = 12345
    message.m_logString = b"Test String var1 = 0x{:X} var2 = 0x{:X}"
    message.m_fileName = b"mysourcefile.cpp"
    message.m_fileLineNumber = 400
    message.m_logSequenceNumber = 0
    message.m_moduleId = 1 # debug commands
    message.m_logType = 4 # info

    logger = Logger()
    logger.processLogMessage(message)
