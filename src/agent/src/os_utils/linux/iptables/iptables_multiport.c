// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/linux/iptables/iptables_multiport.h"

#include <linux/netfilter/xt_multiport.h>

#include "os_utils/linux/iptables/iptables_utils.h"
#include "utils.h"


#define MULTIPORT_RANGE_FLAG 1 
#define MULTIPORT_INVERT_FLAG 1 


/**
 * @brief Format the data in case that the match is of type multiport.
 * 
 * @param   match         The given match. Should be of type multiport.
 * @param   buffer        The buffer to serialize the ports to.
 * @param   bufferSize    The size of the given buffer, in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_EXCEPTION otherwise.
 */
IptablesResults IptablesMultiport_FormatMultiportMatch(struct xt_entry_match* match, char* buffer, uint32_t bufferSize);

/**
 * @brief Format the data in case that the match is of type multiport v1.
 * 
 * @param   match         The given match. Should be of type multiport.
 * @param   buffer        The buffer to serialize the ports to.
 * @param   bufferSize    The size of the given buffer, in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_EXCEPTION otherwise.
 */
IptablesResults IptablesMultiport_FormatMultiportV1Match(struct xt_entry_match* match, char* buffer, uint32_t bufferSize);

IptablesResults IptablesMultiport_GetPorts(struct xt_entry_match* match, bool isSrcPorts, char* buffer, uint32_t bufferSize) {
    
    struct xt_multiport* multiportInfo = (struct xt_multiport*)match->data;
    if (isSrcPorts && (multiportInfo->flags == XT_MULTIPORT_DESTINATION)) {
        return IPTABLES_NO_DATA;
    }
    if (!isSrcPorts && (multiportInfo->flags == XT_MULTIPORT_SOURCE)) {
        return IPTABLES_NO_DATA;
    }

    uint32_t dataSize = match->u.match_size - sizeof(struct xt_entry_match);
    if (dataSize == sizeof(struct xt_multiport)) {
        //FIXME: do we want to suport this?
        return IptablesMultiport_FormatMultiportMatch(match, buffer, bufferSize);
    }
    
    if (dataSize == sizeof(struct xt_multiport_v1)) {
        return IptablesMultiport_FormatMultiportV1Match(match, buffer, bufferSize);
    }
    return IPTABLES_OK;
}

IptablesResults IptablesMultiport_FormatMultiportMatch(struct xt_entry_match* match, char* buffer, uint32_t bufferSize) {
    struct xt_multiport* multiportInfo = (struct xt_multiport*)match->data;

    for (int j = 0; j < multiportInfo->count; j++) {
            if (j > 0) {
                if (!Utils_ConcatenateToString(&buffer, &bufferSize, ",")) {
                    return IPTABLES_EXCEPTION;
                }
            }
            if (!Utils_ConcatenateToString(&buffer, &bufferSize, "%d", multiportInfo->ports[j])) {
                return IPTABLES_EXCEPTION;
            }
    }
    return IPTABLES_OK; 
}

IptablesResults IptablesMultiport_FormatMultiportV1Match(struct xt_entry_match* match, char* buffer, uint32_t bufferSize) {
    struct xt_multiport_v1* multiportInfoV1 = (struct xt_multiport_v1*)match->data;
    bool invert = multiportInfoV1->invert & MULTIPORT_INVERT_FLAG;

    if (invert) {
        if (!Utils_ConcatenateToString(&buffer, &bufferSize, IPTABLES_NEGATE_EXPRESSION_START)) {
           return IPTABLES_EXCEPTION;
        }
    }

    for (int j = 0; j < multiportInfoV1->count; j++) {
        if (j > 0) {
            if (!Utils_ConcatenateToString(&buffer, &bufferSize, ",")) {
                return IPTABLES_EXCEPTION;
            }
        }
        if (multiportInfoV1->pflags[j] & MULTIPORT_RANGE_FLAG) {
            if (!Utils_ConcatenateToString(&buffer, &bufferSize, "%d-%d", multiportInfoV1->ports[j], multiportInfoV1->ports[j + 1])) {
                return IPTABLES_EXCEPTION;
            }
            ++j;
        } else {
            if (!Utils_ConcatenateToString(&buffer, &bufferSize, "%d", multiportInfoV1->ports[j])) {
                return IPTABLES_EXCEPTION;
            }
        }
    }
    if (invert) {
        if (!Utils_ConcatenateToString(&buffer, &bufferSize, IPTABLES_NEGATE_EXPRESSION_END)) {
            return IPTABLES_EXCEPTION;
        }
    }
    return IPTABLES_OK;
}
