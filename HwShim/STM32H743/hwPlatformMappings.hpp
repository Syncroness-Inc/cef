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
#ifndef __HW_PLATFORM_MAPPING_H
#define __HW_PLATFORM_MAPPING_H

/* Start of c/c++ guard */
#ifdef __cplusplus
extern "C" {
#endif


/**
 * Contains information unique to the specific tool chain of the build target.
 * Different tool chains have different tool include files, so this is the one place
 * in the CEF that build specific files are included.  This technique avoids having each
 * file pull in specific include files (which creates maintenance/challenge when moving
 * between different hardware build chains), as well as having
 * the #ifdef __SIMULATOR__ #else #endif chain in most files in the system.
 *
 * In addition having all the files included in one location has proved helpful in debugging
 * other projects on simulator vs hardware issues, as well as debugging code bloat.
 *
 * This file is included in the cefMappings.hpp file, which is in turn included in most files.
 * Only tool chain files (that i.e. compiler supplied files) should be included here...No HAL or
 * application specific files should be included.
 */

#include <string.h>     // for memcpy() and strlen()
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>


//#define RUNTIME_ASSERT(COND)  TBD if this is needed, and need to be cautious on how much memory it uses

// Some tool chains require special handling of static_assert
#define STATIC_ASSERT(COND,MSG) static_assert((COND), #MSG);


// Each IDE may use a different flag to indicate a release or debug build
#ifdef DEBUG
  #define DEBUG_BUILD
#endif


/* End of c/c++ guard */
#ifdef __cplusplus
}
#endif

#endif  // end header guard
