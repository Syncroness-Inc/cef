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
        # default append to file, log all levels
        logging.basicConfig(filename=self.CEF_LOG_FILENAME, format='%(levelname)s, %(message)s', level=logging.DEBUG)
        
        self.printVarsInHex = True
        self.sequenceNumber = 0
        self.droppedLogs = 0
        self.fieldPattern = "{:X}" # this should match what is supplied for a string in the embedded source

    def printBreak(self):
        """
        Add a discontinuity line to the log
        """
        f = open(self.CEF_LOG_FILENAME, 'a')
        f.write('---------------------------------------------------------------------------------------------\n')
        f.close()

    def validateResponseHeader(self, responseHeader: cefContract.cefCommandHeader):
        """
        Header field value checking. Verify that the received size matches the expected structure
        and confirm the error code field was not erroneously populated. Log sequence numbers are 
        packaged in the actual body and validated during log entry processing instead of here.
        @param responseHeader: the log header obeying the structure defined in cefContract
        @return boolean: False if either check fails, else True
        """
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
        """
        Prepare the log message content before writing to file.
        1. Check the received Log Sequence Number against the local running count
        2. Check the Log String for proper formatting and if so, populate it with the enclosed log variables
            (else, simply print the received string as-is)
        3. Concatenate all the received data for the log into a writable entry
        4. Write the entry to the log file with appropriate level
        5. Increment the local sequence number for the next log
        @param log: received log message obeying the structure in cefContract
        """

        # 1. check log sequence number
        if log.m_logSequenceNumber != self.sequenceNumber:
            self.droppedLogs += 1
            self.printBreak() # print a discontinuity to the log fto visualize a sequence number gap
            print("Log Sequence Number error - received: {}, current: {}".format(log.m_logSequenceNumber, self.sequenceNumber))

        # 2. check the log string for expected number of variables
        logString = bytes.decode(log.m_logString)
        numLeftBraces = logString.count("{")
        numRightBraces = logString.count("}")
        malformed = numLeftBraces != numRightBraces # check for a complete string
        
        # 2. insert variables into the string 
        logVars = [log.m_logVariable1, log.m_logVariable2, log.m_logVariable3] # for easier iterating
        if not malformed:
            for n in range(numLeftBraces):
                if self.printVarsInHex:
                    logString = logString.replace(self.fieldPattern, f'{logVars[n]:X}', 1)
                else:
                    logString = logString.replace(self.fieldPattern, str(logVars[n]), 1)

        # 3. prepare the full log entry
        rawData = str(bytes(log)) # for extra debug, the full byte-dump of the log at the end of the entry
        fileAndLine = bytes.decode(log.m_fileName) + ":" + str(log.m_fileLineNumber)
        logEntry = ", ".join([str(log.m_logSequenceNumber),\
                              str(log.m_timeStamp),\
                              logString,\
                              fileAndLine,\
                              str(log.m_moduleId),\
                              rawData])

        # 4. write to file
        if log.m_logType == cefContract.logType.logTypeDebug.value:
            logging.debug(logEntry)
        elif log.m_logType == cefContract.logType.logTypeInfo.value:
            logging.info(logEntry)
        elif log.m_logType == cefContract.logType.logTypeWarning.value:
            logging.warning(logEntry)
        elif log.m_logType == cefContract.logType.logTypeError.value:
            logging.error(logEntry)
        elif log.m_logType == cefContract.logType.logTypeFatal.value:
            logging.fatal(logEntry) # 'CRITICAL' in python

        # 5. update sequence number
        self.sequenceNumber += 1


if __name__ == '__main__':
    message = cefContract.cefLog()
    message.m_logVariable1 = 100
    message.m_logVariable2 = 200
    message.m_logVariable3 = 300
    message.m_timeStamp = 12345
    message.m_logString = b"Test String var1 = 0x{:X} var2 = 0x{:X}"
    # message.m_logString = b"Test String"
    message.m_fileName = b"mysourcefile.cpp"
    message.m_fileLineNumber = 400
    message.m_logSequenceNumber = 0
    message.m_moduleId = 1 # debug commands
    message.m_logType = 1 # info

    logger = Logger()
    # logger.printBreak()
    logger.processLogMessage(message)
