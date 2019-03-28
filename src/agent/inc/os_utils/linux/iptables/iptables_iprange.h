// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IPTABLES_IPRANGE_H
#define IPTABLES_IPRANGE_H

#include <libiptc/libiptc.h>
#include <stdbool.h>
#include <stdint.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "os_utils/linux/iptables/iptables_def.h"

/**
 * @brief Search for iprange match in the entry and then search for src\dest ips (according to the given flag). 
 *        If such ips are found - writes the range to the buffer.
 * 
 * @param   entry         The enrty we want to check for ranged ip.
 * @param   isSrcIp       A flag which indicates whther this function should return a src or dest ip. 
 * @param   buffer        A buffer to write the result to.
 * @param   bufferSize    The size of the buffer in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA in case the entry does not contain a iprange atch or the wanted ip was not found.
 *         IPTABLES_EXCEPTION in case of error.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesIprange_TryGetRangedIp, const struct ipt_entry*, entry, bool, isSrcIp, char*, buffer, uint32_t, bufferSize);

#endif //IPTABLES_IPRANGE_H