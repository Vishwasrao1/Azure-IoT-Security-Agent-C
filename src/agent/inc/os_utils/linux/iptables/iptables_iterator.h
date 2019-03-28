// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IPTABLES_ITERATOR_H
#define IPTABLES_ITERATOR_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "os_utils/linux/iptables/iptables_def.h"

/**
 * @brief Initiates a new iptables iterator
 * 
 * @param   iterator    The instance to initiate.
 * 
 * @return IPTABLES_OK on success, IPTABLES_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesIterator_Init, IptablesIteratorHandle*, iterator);

/**
 * @brief Deinitates the given iterator instance.
 * 
 * @param   iterator    The instance do deinitiate.
 */
MOCKABLE_FUNCTION(, void, IptablesIterator_Deinit, IptablesIteratorHandle, iterator);

/**
 * @brief Retrive the next item (the next chain), if exsits, of the iterator.
 * 
 * @param   iterator    The iterator instance.
 * 
 * @return IPTABLES_ITERATOR_HAS_NEXT if the there is an item, IPTABLES_ITERATOR_NO_MORE_ITEMS if we reached the end of the iterator.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesIterator_GetNext, IptablesIteratorHandle, iterator);

/**
 * @brief Generates a new rules iterator of the current chain of this iterator.
 * 
 * @param   iterator        The iterator instance.
 * @param   rulesIterator   Out param. The newly created rules iterator.
 * 
 * @return IPTABLES_OK on success, IPTABLES_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesIterator_GetRulesIterator, IptablesIteratorHandle, iterator, IptablesRulesIteratorHandle*, rulesIterator);

/**
 * @brief Gets the action of the chain's policy.
 * 
 * @param   iterator      The iterator instance.
 * @param   actionType    Out param. The type of the action.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA in case the wanted data isn't defines for this rule or IPTABLES_EXCEPTION in case of error.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesIterator_GetPolicyAction, IptablesIteratorHandle, iterator, IptablesActionType*, actionType);

/**
 * @brief Gets the current chain name
 * 
 * @param   iterator      The iterator instance.
 * @param   chainName     A pointer to the chain name of the given chain
 *                        The value is valid as long as the iterator does not change state.
 *  
 * @return IPTABLES_OK on success, IPTABLES_EXCEPTION in case of error.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesIterator_GetChainName, IptablesIteratorHandle, iterator, const char**, chainName);
#endif //IPTABLES_ITERATOR_H