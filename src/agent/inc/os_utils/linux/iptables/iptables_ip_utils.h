// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IPTABLES_IP_UTILS_H
#define IPTABLES_IP_UTILS_H

#include <libiptc/libiptc.h>
#include <stdbool.h>
#include <stdint.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "os_utils/linux/iptables/iptables_def.h"

/**
 * @brief A generic get ip which gets the ip of this rule (the type of the ip - src\dest is determined by the given flag).
 * 
 * @param   entry         The ip entry.
 * @param   isSrcIp       A flag which indicates whther this function should return a src or dest ip. 
 * @param   buffer        A buffer to write the result to.
 * @param   bufferSize    The size of the buffer in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA in case the wanted data isn't defines for this rule or IPTABLES_EXCEPTION in case of error.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesIpUtils_GetIp, const struct ipt_entry*, entry, bool, isSrcIp, char*, buffer, uint32_t, bufferSize);

#endif //IPTABLES_IP_UTILS_H