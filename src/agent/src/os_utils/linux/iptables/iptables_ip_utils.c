// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/linux/iptables/iptables_ip_utils.h"

#include "os_utils/linux/iptables/iptables_iprange.h"
#include "os_utils/linux/iptables/iptables_utils.h"

/**
 * @brief Tries to extrac the ip from the current entry.
 * 
 * @param   ipt_entry     The current ip entry.
 * @param   isSrcIp       A flag which indicates whther this function should return a src or dest ip. 
 * @param   buffer        A buffer to write the result to.
 * @param   bufferSize    The size of the buffer in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA in case the wanted data isn't defines for this rule or IPTABLES_EXCEPTION in case of error.
 */
IptablesResults IptablesIpUtils_GetRegularIp(const struct ipt_entry*, bool isSrcIp, char* buffer, uint32_t bufferSize);

IptablesResults IptablesIpUtils_GetIp(const struct ipt_entry* entry, bool isSrcIp, char* buffer, uint32_t bufferSize) {
    
    IptablesResults result = IptablesIprange_TryGetRangedIp(entry, isSrcIp, buffer, bufferSize);
    if (result == IPTABLES_OK || result != IPTABLES_NO_DATA) {
        return result;
    }

    return IptablesIpUtils_GetRegularIp(entry, isSrcIp, buffer, bufferSize);
}

IptablesResults IptablesIpUtils_GetRegularIp(const struct ipt_entry* entry, bool isSrcIp, char* buffer, uint32_t bufferSize) {
    uint32_t ip = 0;
    uint32_t mask = 0;
    bool invert = false;

    if (isSrcIp) {
        ip = entry->ip.src.s_addr;
        mask = entry->ip.smsk.s_addr;
        invert = entry->ip.invflags & IPT_INV_SRCIP;
    } else {
        ip = entry->ip.dst.s_addr;
        mask = entry->ip.dmsk.s_addr;
        invert = entry->ip.invflags & IPT_INV_DSTIP;
    }
    
    return IptablesUtils_FormatIp(ip, mask, invert, buffer, bufferSize);
}

