// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/linux/iptables/iptables_iterator.h"

#include <errno.h>
#include <stdlib.h>

#include "os_utils/linux/iptables/iptables_rules_iterator.h"
#include "os_utils/linux/iptables/iptables_utils.h"
#include "utils.h"

static const char* FILTERS_TABLE = "filter";

IptablesResults IptablesIterator_Init(IptablesIteratorHandle* iterator) {
    IptablesResults result = IPTABLES_OK;
    IptablesIterator* iteratorObj = malloc(sizeof(IptablesIterator));
    if (iteratorObj == NULL) {
        result = IPTABLES_EXCEPTION;
        goto cleanup ;
    }
    iteratorObj->iptcHandle = NULL;
    memset(iteratorObj, 0, sizeof(IptablesIterator));
    iteratorObj->iptcHandle = iptc_init(FILTERS_TABLE);
    if (iteratorObj->iptcHandle == NULL) {
        if (errno == ENOPROTOOPT) {
            result = IPTABLES_NO_DATA;
        } else {
            result = IPTABLES_EXCEPTION;
        }
        goto cleanup;
    }

cleanup:
    *iterator = (IptablesIteratorHandle)iteratorObj;

    if (result != IPTABLES_OK) {
        if (iteratorObj != NULL) {
            IptablesIterator_Deinit(*iterator);
        }
        *iterator = NULL;
    }
    return result;
}

void IptablesIterator_Deinit(IptablesIteratorHandle iterator) {
    IptablesIterator* iteratorObj = (IptablesIterator*)iterator;
    if (iteratorObj->iptcHandle != NULL) {
        iptc_free(iteratorObj->iptcHandle);
    }
    free(iteratorObj);
}

IptablesResults IptablesIterator_GetNext(IptablesIteratorHandle iterator) {
    IptablesIterator* iteratorObj = (IptablesIterator*)iterator;
    if (!iteratorObj->started) {
        iteratorObj->currentChain = iptc_first_chain(iteratorObj->iptcHandle);
        iteratorObj->started = true;
    } else {
        iteratorObj->currentChain = iptc_next_chain(iteratorObj->iptcHandle);
    }

    if (iteratorObj->currentChain == NULL) {
        return IPTABLES_ITERATOR_NO_MORE_ITEMS;
    }
    return IPTABLES_ITERATOR_HAS_NEXT;
}

IptablesResults IptablesIterator_GetRulesIterator(IptablesIteratorHandle iterator, IptablesRulesIteratorHandle* rulesIterator) {
    return IptablesRulesIterator_Init(iterator, rulesIterator);
}

IptablesResults IptablesIterator_GetChainName(IptablesIteratorHandle iterator, const char** chainName) {
    IptablesIterator* iteratorObj = (IptablesIterator*)iterator;
    *chainName = iteratorObj->currentChain;
    
    return IPTABLES_OK;
}

IptablesResults IptablesIterator_GetPolicyAction(IptablesIteratorHandle iterator, IptablesActionType* actionType){
    IptablesIterator* iteratorObj = (IptablesIterator*)iterator;
    if (!iptc_builtin(iteratorObj->currentChain, iteratorObj->iptcHandle)) {
        return IPTABLES_NO_DATA;
    }
    struct xt_counters counters;
    const char* policy = iptc_get_policy(iteratorObj->currentChain, &counters, iteratorObj->iptcHandle);
    if (policy == NULL) {
        return IPTABLES_EXCEPTION;
    }
    
    return IptablesUtils_GetActionTypeEnumFromActionString(policy, actionType);
}