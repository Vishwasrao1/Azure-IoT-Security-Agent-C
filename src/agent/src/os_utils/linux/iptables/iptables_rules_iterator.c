// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/linux/iptables/iptables_rules_iterator.h"

#include <linux/netfilter/xt_multiport.h>
#include <stdlib.h>
#include <stdio.h>

#include "os_utils/linux/iptables/iptables_ip_utils.h"
#include "os_utils/linux/iptables/iptables_port_utils.h"
#include "os_utils/linux/iptables/iptables_utils.h"

#include "utils.h" 

IptablesResults IptablesRulesIterator_Init(IptablesIteratorHandle chainIterator, IptablesRulesIteratorHandle* iterator) {
    IptablesRulesIterator* iteratorObj = malloc(sizeof(IptablesRulesIterator));
    if (iteratorObj == NULL) {
        return IPTABLES_EXCEPTION;
    }
    iteratorObj->iptcHandle = NULL;
    memset(iteratorObj, 0, sizeof(IptablesRulesIterator));
    IptablesIterator* chainIteratorObj = (IptablesIterator*)chainIterator;
    
    iteratorObj->iptcHandle = chainIteratorObj->iptcHandle;
    
    iteratorObj->started = false;
    iteratorObj->currentEntry = NULL;
    if (Utils_CreateStringCopy((char **) &iteratorObj->chain, chainIteratorObj->currentChain) == false) {
        free(iteratorObj);
        return IPTABLES_EXCEPTION;
    }
    *iterator = (IptablesRulesIteratorHandle)iteratorObj;

    return IPTABLES_OK;
}

void IptablesRulesIterator_Deinit(IptablesRulesIteratorHandle iterator) {
    IptablesRulesIterator* iteratorObj = (IptablesRulesIterator*)iterator;
    if (iteratorObj->chain != NULL) {
        free((void*)iteratorObj->chain);
    }
    free(iteratorObj);
}

IptablesResults IptablesRulesIterator_GetNext(IptablesRulesIteratorHandle iterator) {
    IptablesRulesIterator* iteratorObj = (IptablesRulesIterator*)iterator;

    if (!iteratorObj->started) {
        iteratorObj->currentEntry = iptc_first_rule(iteratorObj->chain, iteratorObj->iptcHandle);
        iteratorObj->started = true;
    } else {
        iteratorObj->currentEntry = iptc_next_rule(iteratorObj->currentEntry, iteratorObj->iptcHandle);
    }

    if (iteratorObj->currentEntry == NULL) {
        return IPTABLES_ITERATOR_NO_MORE_ITEMS;
    }
    return IPTABLES_ITERATOR_HAS_NEXT;
}

IptablesResults IptablesRulesIterator_GetSrcIp(IptablesRulesIteratorHandle iterator, char* buffer, uint32_t bufferSize) {
    IptablesRulesIterator* iteratorObj = (IptablesRulesIterator*)iterator;
    return IptablesIpUtils_GetIp(iteratorObj->currentEntry, true, buffer, bufferSize);
}

IptablesResults IptablesRulesIterator_GetDestIp(IptablesRulesIteratorHandle iterator, char* buffer, uint32_t bufferSize) {
    IptablesRulesIterator* iteratorObj = (IptablesRulesIterator*)iterator;
    return IptablesIpUtils_GetIp(iteratorObj->currentEntry, false, buffer, bufferSize);
}

IptablesResults IptablesRulesIterator_GetSrcPort(IptablesRulesIteratorHandle iterator, char* buffer, uint32_t bufferSize) {
    IptablesRulesIterator* iteratorObj = (IptablesRulesIterator*)iterator;
    return IptablesPortUtils_GetPort(iteratorObj->currentEntry, true, buffer, bufferSize);
}

IptablesResults IptablesRulesIterator_GetDestPort(IptablesRulesIteratorHandle iterator, char* buffer, uint32_t bufferSize) {
    IptablesRulesIterator* iteratorObj = (IptablesRulesIterator*)iterator;
    return IptablesPortUtils_GetPort(iteratorObj->currentEntry, false, buffer, bufferSize);
}

IptablesResults IptablesRulesIterator_GetProtocol(IptablesRulesIteratorHandle iterator, char* buffer, uint32_t bufferSize) {
    IptablesRulesIterator* iteratorObj = (IptablesRulesIterator*)iterator;
    if (!iteratorObj->currentEntry->ip.proto) {
        return IPTABLES_NO_DATA;
    }
    
    bool invert = iteratorObj->currentEntry->ip.invflags & IPT_INV_PROTO;
    return IptablesUtils_FormatProtocol(iteratorObj->currentEntry->ip.proto, invert, buffer, bufferSize);
}

IptablesResults IptablesRulesIterator_GetAction(IptablesRulesIteratorHandle iterator, IptablesActionType* actionType, char* buffer, uint32_t bufferSize) {
    IptablesRulesIterator* iteratorObj = (IptablesRulesIterator*)iterator;
    const char* targetName = iptc_get_target(iteratorObj->currentEntry, iteratorObj->iptcHandle);  

    if (targetName == NULL || (*targetName == '\0')) {
        return IPTABLES_NO_DATA;
    }
	
    if (IptablesUtils_GetActionTypeEnumFromActionString(targetName, actionType) != IPTABLES_OK) {
        return IPTABLES_EXCEPTION;
    }

    if (iteratorObj->currentEntry->ip.flags & IPT_F_GOTO) {
        if (!Utils_ConcatenateToString(&buffer, &bufferSize, "goto %s", targetName)) {
            return IPTABLES_EXCEPTION;
        }
    } else {
        if (*actionType == IPTABLES_ACTION_OTHER) {
            if (!Utils_ConcatenateToString(&buffer, &bufferSize, "%s", targetName)) {
                return IPTABLES_EXCEPTION;
            }
        } 
    }
    
    return IPTABLES_OK;
}

IptablesResults IptablesRulesIterator_GetChainName(IptablesRulesIteratorHandle iterator, const char** chainName) {
    IptablesRulesIterator* iteratorObj = (IptablesRulesIterator*)iterator;
    *chainName = iteratorObj->chain;
    
    return IPTABLES_OK;
}