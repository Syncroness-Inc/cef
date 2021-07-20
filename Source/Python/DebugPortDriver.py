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


from abc import ABC, abstractmethod

class DebugPortDriver(ABC):
    """ 
    Abstract class for DebugPort communication interfaces. This serves
    as the generic template for any physical interface implementation
    (UART, ethernet, etc.). The nature of the data (single bytes, packets, etc)
    is left to the child classes to implement.
    """

    def __init__(self):
        pass

    @abstractmethod
    def open(self):
        """
        Start/enable the interface
        """
        pass

    @abstractmethod
    def close(self):
        """
        Close/disable the interface
        """
        pass

    @abstractmethod
    def send(self):
        """
        Transmit/write data out
        """
        pass

    @abstractmethod
    def receive(self):
        """
        Read data in
        """
        pass