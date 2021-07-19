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
#ifndef __CEF_UTILITIES_H
#define __CEF_UTILITIES_H

/* Start of c/c++ guard */
#ifdef __cplusplus
extern "C" {
#endif


/**
 * Contains macros or static functions for use in the Common Embedded Framework.
 * The contents of this file are intended to include a "grab bag" of
 * simple utilities that are used commonly in the code.  If the utility is complex,
 * then a dedicated file/object should be used for that purpose.
 */

//! Gets the max of two numbers
#define MAX(a, b)  ((a) > (b) ? (a) : (b))

//! Gets the min of two numbers
#define MIN(a, b)  ((a) < (b) ? (a) : (b))

//! Milliseconds per second
#define MSECS_PER_SEC 1000

//! Gets the number of elements in an array
#define NUM_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))


/* End of c/c++ guard */
#ifdef __cplusplus
}
#endif

#endif  // end header guard
