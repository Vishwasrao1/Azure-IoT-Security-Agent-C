// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/linux/iptables/iptables_port_utils.h"

#include "os_utils/linux/iptables/iptables_multiport.h"
#include "os_utils/linux/iptables/iptables_utils.h"
#include "utils.h"

/**
 * @brief Gets the ports in case there is a Tcp match.
 * 
 * @param   match         A tcp mach of the given rule.
 * @param   isSrcIp       A flag which indicates whther this function should return a src or dest ip. 
 * @param   buffer        A buffer to write the result to.
 * @param   bufferSize    The size of the buffer in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA in case the wanted data isn't defines for this rule or IPTABLES_EXCEPTION in case of error.
 */
IptablesResults IptablesPortUtils_GetPortsTcpMatch(struct xt_entry_match* match, bool isSrcPorts, char* buffer, uint32_t bufferSize);

IptablesResults IptablesPortUtils_GetPort(const struct ipt_entry* entry, bool isSrcPorts, char* buffer, uint32_t bufferSize) {

    struct xt_entry_match* currentMatch;
    IptablesResults result = IPTABLES_NO_DATA;
    for (int i = sizeof(struct ipt_entry); i < entry->target_offset; i+=  currentMatch->u.match_size) {
        result = IPTABLES_NO_DATA;
        currentMatch = (void *)entry + i;
        if (Utils_UnsafeAreStringsEqual(currentMatch->u.user.name, IPTABLES_TCP_MATCH, true)) {
            result = IptablesPortUtils_GetPortsTcpMatch(currentMatch, isSrcPorts, buffer, bufferSize);
        } else if (Utils_UnsafeAreStringsEqual(currentMatch->u.user.name, IPTABLES_MULTIPORT_MATCH, true)) {
            result = IptablesMultiport_GetPorts(currentMatch, isSrcPorts, buffer, bufferSize);
        }

        if (result == IPTABLES_OK || result != IPTABLES_NO_DATA) {
            return result;
        }
    }
    return IPTABLES_NO_DATA;
}

IptablesResults IptablesPortUtils_GetPortsTcpMatch(struct xt_entry_match* match, bool isSrcPorts, char* buffer, uint32_t bufferSize) {
    struct xt_tcp* tcpInfo = (struct xt_tcp*)match->data;
    uint32_t minPort = 0;
    uint32_t maxPort = 0;
    bool invert = false;

    if (isSrcPorts) {
        minPort = tcpInfo->spts[0];
        maxPort = tcpInfo->spts[1];
        invert = tcpInfo->invflags & XT_TCP_INV_SRCPT;
    } else {
        minPort = tcpInfo->dpts[0];
        maxPort = tcpInfo->dpts[1];
        invert = tcpInfo->invflags & XT_TCP_INV_DSTPT;
    }

    return IptablesUtils_FormatRangedPorts(minPort, maxPort, invert, buffer, bufferSize);
}
