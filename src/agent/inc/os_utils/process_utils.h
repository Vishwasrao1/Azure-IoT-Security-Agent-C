// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

/**
 * @brief Executes the given command anr writes the output the the given buffer.
 * 
 * @param   command      The command to run.
 * @param   output       A output buffer to write the output.
 * @param   outputSize   In out param. when calling this fuction this should contain the size of the buffer.
 *                       On return this will contain the amount data written to the output buffer.
 * 
 * @return true on success, false othewise.
 */
MOCKABLE_FUNCTION(, bool, ProcessUtils_Execute, const char*, command, char*, output, uint32_t*, outputSize);

#endif //PROCESS_UTILS_H