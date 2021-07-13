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
/* Header guard */
#ifndef __CEF_MAPPINGS_H
#define __CEF_MAPPINGS_H

/* Start of c/c++ guard */
#ifdef __cplusplus
extern "C" {
#endif


/**
 * Contains information unique to the specific tool chain of the build target.
 * Different tool chains have different tool include files, so this is the one place
 * in the CEF that build specific files are included.  This technique avoids having each
 * file pull in specific include files (which may not be correct as other files may have already
 * pulled in the include file), as well as having the #ifdef __SIMULATOR__ #else #endif chain
 * in most files in the system.
 *
 * In addition having all the files included in one location has proved helpful in debugging
 * other projects on simulator vs. hardware issues, as well as debugging code bloat.
 *
 * Each hw specific project must provide a hwPlatformMappings.hpp file.  This file should be
 * located in the "shim" directory as this is where all project specific cef hw dependency
 * information is contained (rather than having to search for files all over the directory structure).
 */


#include "cefUtilities.hpp"   // common macros that are platform independent


#ifdef __SIMULATOR__
	#include <cassert>
	#include <chrono>
	#include <cstdint>
	#include <cstdio>
	#include <cstring>
	#include <ctime>
	#include <errno.h>
	#include <fcntl.h>
	#include <fstream>
	#include <stdarg.h>
	#include <termios.h>
	#include <unistd.h>

	using namespace std;

	// We can use a regular assert in simulator
	#define RUNTIME_ASSERT(COND) assert(COND)

	// We can use a normal static assert in the simulator
	#define STATIC_ASSERT(COND,MSG) static_assert((COND), #MSG);



#else  // #ifdef __SIMULATOR__
	// For whatever hardware platform we are building, include the appropriate hardware specific tool chain include files
	#include "hwPlatformMappings.hpp"

#endif	// #ifdef __SIMULATOR__


/* End of c/c++ guard */
#ifdef __cplusplus
}
#endif

#endif  // end header guard
