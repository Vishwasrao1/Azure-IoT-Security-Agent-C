// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/linux/iptables/iptables_utils.h"

#include <arpa/inet.h>

#include "os_utils/linux/iptables/iptables_def.h"
#include "utils.h"

static const char* IPTABLES_TCP_PROTOCOL = "tcp";
static const char* IPTABLES_UDP_PROTOCOL = "udp";
static const char* IPTABLES_ICMP_PROTOCOL = "icmp";

static const int MIN_PORT = 0;
static const int MAX_PORT = 65535;

IptablesResults IptablesUtils_FormatIp(uint32_t ip, uint32_t mask, bool invert, char* buffer, uint32_t bufferSize) {
    if (ip == 0 && mask == 0) {
        return IPTABLES_NO_DATA;
    }

    uint32_t tmpMask = 0xffffffff;
    bool foundMask = false;
    int i = 0;
    while (!foundMask && i < 32) {
        if (tmpMask == ntohl(mask)) {
            foundMask = true;
        } else {
            tmpMask = tmpMask << 1;
            ++i;
        }
    }

    const char* formatPrefix = invert ? IPTABLES_NEGATE : IPTABLES_EMPTY;

    if (foundMask) {
        if (!Utils_ConcatenateToString(&buffer, &bufferSize, "%s%u.%u.%u.%u/%d", formatPrefix, IP_BYTES(ip), (32 - i))) {
           return IPTABLES_EXCEPTION;
        }
    } else if (mask ==0) {
        if (!Utils_ConcatenateToString(&buffer, &bufferSize, "%s%u.%u.%u.%u", formatPrefix, IP_BYTES(ip), (32 - i))) {
           return IPTABLES_EXCEPTION;
        }
    } else {
        if (!Utils_ConcatenateToString(&buffer, &bufferSize, "%s%u.%u.%u.%u/%u.%u.%u.%u", formatPrefix, IP_BYTES(ip), IP_BYTES(mask))) {
            return IPTABLES_EXCEPTION;
        }
    }
    return IPTABLES_OK;
}

IptablesResults IptablesUtils_FormatProtocol(uint16_t protocol, bool invert, char* buffer, uint32_t bufferSize) {
    const char* protocolString = NULL;
    switch (protocol) {
        case IPPROTO_TCP:
            protocolString = IPTABLES_TCP_PROTOCOL;
            break;
        case IPPROTO_UDP:
            protocolString = IPTABLES_UDP_PROTOCOL;
            break;
        case IPPROTO_ICMP:
            protocolString = IPTABLES_ICMP_PROTOCOL;
            break;
        default:
            //FIXME: there are many more protocols, we need to see if we want to support them
            return IPTABLES_EXCEPTION;
    }
    
    const char* formatPrefix = invert ? IPTABLES_NEGATE : IPTABLES_EMPTY;
    
    if (!Utils_ConcatenateToString(&buffer, &bufferSize, "%s%s", formatPrefix, protocolString)) {
       return IPTABLES_EXCEPTION;
    }
    return IPTABLES_OK;
}


IptablesResults IptablesUtils_FormatRangedPorts(uint16_t lowPort, uint16_t highPort, bool invert, char* buffer, uint32_t bufferSize) {
    if (lowPort != MIN_PORT || highPort != MAX_PORT) {
        const char* formatPrefix = invert ? IPTABLES_NEGATE_EXPRESSION_START : IPTABLES_EMPTY;
        if (lowPort == highPort) {
            if (!Utils_ConcatenateToString(&buffer, &bufferSize, "%s%d ", formatPrefix, lowPort)) {
                return IPTABLES_EXCEPTION;
            }
        } else {
            const char* formatSuffix = invert ? IPTABLES_NEGATE_EXPRESSION_END : IPTABLES_EMPTY;
            if (!Utils_ConcatenateToString(&buffer, &bufferSize, "%s%d-%d%s", formatPrefix, lowPort, highPort, formatSuffix)) {
                return IPTABLES_EXCEPTION;
            }
        }
        return IPTABLES_OK;
    }
    return IPTABLES_NO_DATA;
}

IptablesResults IptablesUtils_GetActionTypeEnumFromActionString(const char* action, IptablesActionType* actionType) {
    if (action == NULL || actionType == NULL){
        return IPTABLES_EXCEPTION;
    }

    if (Utils_UnsafeAreStringsEqual(IPTABLES_ACCEPT_VERDICT, action, true)) {
        *actionType = IPTABLES_ACTION_ALLOW;
    } else if (Utils_UnsafeAreStringsEqual(IPTABLES_REJECT_VERDICT, action, true)
                || Utils_UnsafeAreStringsEqual(IPTABLES_DROP_VERDICT, action, true)) {
        *actionType = IPTABLES_ACTION_DENY;
    } else {
        *actionType = IPTABLES_ACTION_OTHER;
    } 

    return IPTABLES_OK;
}