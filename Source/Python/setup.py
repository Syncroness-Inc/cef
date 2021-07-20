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

""" 
A setuptools based module for CEF Python Utility
usage: pip3 install .
"""

from setuptools import setup, find_packages

setup(
    name='cef_python',
    version='0.0.1',
    description='CEF Python Test Utility',
    url='http://bitbucket.syncroness.com:7990/projects/CEF/repos/cef/browse',
    author='Syncroness, Inc',
    author_email='clight@syncroness.com',
    packages=find_packages(),
    install_requires=[
        'pyserial',
    ],
)