// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root fo

#include "os_utils/linux/iptables/iptables_iprange.h"

#include <linux/netfilter/xt_iprange.h>

#include "os_utils/linux/iptables/iptables_utils.h"
#include "utils.h"

/**
 * @brief Formats the given range of ips (with option to invert the value) and writes it to the given buffer.
 * 
 * @param   minIp        The min ip value.
 * @param   maxIp        The max ip value.
 * @param   invert       A flge which indicates whether the ip should be inverted (adding !).
 * @param   buffer       The buffer to write the ip to.
 * @param   bufferSize   The size of the outout buffer.
 * 
 * @return IPTABLES_OK in case of success, IPTABLES_EXCEPTOIN otherwise.
 */
IptablesResults IptablesUtils_FormatRangedIp(uint32_t minIp, uint32_t maxIp, bool invert, char* buffer, uint32_t bufferSize);

IptablesResults IptablesIprange_TryGetRangedIp(const struct ipt_entry* entry, bool isSrcIp, char* buffer, uint32_t bufferSize) {
    struct xt_entry_match* currentMatch;		
    for (int i = sizeof(struct ipt_entry); i < entry->target_offset; i+=  currentMatch->u.match_size) {
        currentMatch = (void *)entry + i;
        if (Utils_UnsafeAreStringsEqual(currentMatch->u.user.name, IPTABLES_IPRANGE_MATCH, true)) {
            const struct xt_iprange_mtinfo* info = (const void *)currentMatch->data;

            if (isSrcIp && info->flags & IPRANGE_SRC) {
                bool invert  = info->flags & IPRANGE_SRC_INV;
                return IptablesUtils_FormatRangedIp(info->src_min.in.s_addr, info->src_max.in.s_addr, invert, buffer, bufferSize);
            }
            
            if (!isSrcIp && info->flags & IPRANGE_DST) {
                bool invert  = info->flags & IPRANGE_DST_INV;
                return IptablesUtils_FormatRangedIp(info->dst_min.in.s_addr, info->dst_max.in.s_addr, invert, buffer, bufferSize);
            }
        }
    }   
    return IPTABLES_NO_DATA;
}

IptablesResults IptablesUtils_FormatRangedIp(uint32_t minIp, uint32_t maxIp, bool invert, char* buffer, uint32_t bufferSize) {

    const char* formatPrefix = invert ? IPTABLES_NEGATE_EXPRESSION_START : IPTABLES_EMPTY;
    const char* formatSuffix = invert ? IPTABLES_NEGATE_EXPRESSION_END : IPTABLES_EMPTY;

    if (!Utils_ConcatenateToString(&buffer, &bufferSize, "%s%u.%u.%u.%u-%u.%u.%u.%u%s", formatPrefix, IP_BYTES(minIp), IP_BYTES(maxIp), formatSuffix)) {
        return IPTABLES_EXCEPTION;
    }
    return IPTABLES_OK;
}
