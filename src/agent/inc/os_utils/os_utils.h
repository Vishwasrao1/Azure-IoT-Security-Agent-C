// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef OS_UTILS_H
#define OS_UTILS_H

#include "macro_utils.h"
#include "umock_c_prod.h"

/**
 * @brief   tries to fetch the executable directory of the running application
 * 
 * @return  the path of the running application, NULL otherwise.
 */
MOCKABLE_FUNCTION(, char*, GetExecutableDirectory);

/**
 * @brief   gets the running process id
 * 
 * @return  the running process id
 */
MOCKABLE_FUNCTION(, int, OsUtils_GetProcessId);

/**
 * @brief   gets the running thread id
 * 
 * @return  the running thread id
 */
MOCKABLE_FUNCTION(, unsigned int, OsUtils_GetThreadId);

#endif //OS_UTILS_H