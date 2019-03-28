// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IPTABLES_RULES_ITERATOR_H
#define IPTABLES_RULES_ITERATOR_H

#include <stdbool.h>
#include <stdint.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "os_utils/linux/iptables/iptables_def.h"

/**
 * @brief Initiates a new iptables rules iterator
 * 
 * @param   chainIterator   The chain interator to intiiate from.
 * @param   iterator        The instance to initiate.
 * 
 * @return IPTABLES_OK on success, IPTABLES_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesRulesIterator_Init, IptablesIteratorHandle, chainIterator, IptablesRulesIteratorHandle*, iterator);

/**
 * @brief Deinitates the given iterator instance.
 * 
 * @param   iterator    The instance do deinitiate.
 */
MOCKABLE_FUNCTION(, void, IptablesRulesIterator_Deinit, IptablesRulesIteratorHandle, iterator);

/**
 * @brief Retrive the next item (the next rule), if exsits, of the iterator.
 * 
 * @param   iterator    The iterator instance.
 * 
 * @return IPTABLES_ITERATOR_HAS_NEXT if the there is an item, IPTABLES_ITERATOR_NO_MORE_ITEMS if we reached the end of the iterator.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesRulesIterator_GetNext, IptablesRulesIteratorHandle, iterator);

/**
 * @brief Gets the src ip of the given rule.
 * 
 * @param   iterator      The iterator instance.
 * @param   buffer        A buffer to write the result to.
 * @param   bufferSize    The size of the buffer in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA in case the wanted data isn't defines for this rule or IPTABLES_EXCEPTION in case of error.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesRulesIterator_GetSrcIp, IptablesRulesIteratorHandle, iterator, char*, buffer, uint32_t, bufferSize);

/**
 * @brief Gets the dest ip of the given rule.
 * 
 * @param   iterator      The iterator instance.
 * @param   buffer        A buffer to write the result to.
 * @param   bufferSize    The size of the buffer in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA in case the wanted data isn't defines for this rule or IPTABLES_EXCEPTION in case of error.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesRulesIterator_GetDestIp, IptablesRulesIteratorHandle, iterator, char*, buffer, uint32_t, bufferSize);

/**
 * @brief Gets the src port of the given rule.
 * 
 * @param   iterator      The iterator instance.
 * @param   buffer        A buffer to write the result to.
 * @param   bufferSize    The size of the buffer in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA in case the wanted data isn't defines for this rule or IPTABLES_EXCEPTION in case of error.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesRulesIterator_GetSrcPort, IptablesRulesIteratorHandle, iterator, char*, buffer, uint32_t, bufferSize);

/**
 * @brief Gets the dest port of the given rule.
 * 
 * @param   iterator      The iterator instance.
 * @param   buffer        A buffer to write the result to.
 * @param   bufferSize    The size of the buffer in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA in case the wanted data isn't defines for this rule or IPTABLES_EXCEPTION in case of error.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesRulesIterator_GetDestPort, IptablesRulesIteratorHandle, iterator, char*, buffer, uint32_t, bufferSize);

/**
 * @brief Gets the protocol of the given rule.
 * 
 * @param   iterator      The iterator instance.
 * @param   buffer        A buffer to write the result to.
 * @param   bufferSize    The size of the buffer in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA in case the wanted data isn't defines for this rule or IPTABLES_EXCEPTION in case of error.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesRulesIterator_GetProtocol, IptablesRulesIteratorHandle, iterator, char*, buffer, uint32_t, bufferSize);

/**
 * @brief Gets the action of the given rule.
 * 
 * @param   iterator      The iterator instance.
 * @param   actionType    Out param. The type of the action.
 * @param   buffer        A buffer to write the result to. A result is written only in case that action type is IPTABLES_ACTION_OTHER.
 * @param   bufferSize    The size of the buffer in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA in case the wanted data isn't defines for this rule or IPTABLES_EXCEPTION in case of error.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesRulesIterator_GetAction, IptablesRulesIteratorHandle, iterator, IptablesActionType*, actionType, char*, buffer, uint32_t, bufferSize);

/**
 * @brief Gets the chain name that contians the given rule
 * 
 * @param   iterator      The iterator instance.
 * @param   chainName     A pointer to the chain name of the given rule
 *                        The value is valid as long as the iterator does not change state.
 * 
 * @return IPTABLES_OK on success, IPTABLES_EXCEPTION in case of error.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesRulesIterator_GetChainName, IptablesRulesIteratorHandle, iterator, const char**, chainName);

#endif //IPTABLES_RULES_ITERATOR_H