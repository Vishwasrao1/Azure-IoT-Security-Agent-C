// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef GROUPS_ITERATOR_H
#define GROUPS_ITERATOR_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <pwd.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct GroupsIterator* GroupsIteratorHandle;

/**
 * @brief Creates a new groups iterator for the given user
 * 
 * @param   iterator    The newly created iterator.
 * @param   user        The user whose groups we want to iter.
 * 
 * @return true on success, false otherisw.
 */
MOCKABLE_FUNCTION(, bool, GroupsIterator_Init, GroupsIteratorHandle*, iterator, struct passwd*, user);

/**
 * @brief Deiniitiate the given iterator.
 * 
 * @param   iterator    The iterator to deinitiate.
 */
MOCKABLE_FUNCTION(, void, GroupsIterator_Deinit, GroupsIteratorHandle, iterator);

/**
 * @brief Returns whether the iterator has not items.
 * 
 * @param   iterator    The iterator instance.
 * 
 * @return true if there are more items to iter, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, GroupsIterator_HasNext, GroupsIteratorHandle, iterator);

/**
 * @brief Retrives the next item of the iterator.
 * 
 * @param   iterator    The iterator instance.
 *
 * @return true if the next item retrives successfuly, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, GroupsIterator_Next, GroupsIteratorHandle, iterator);

/**
 * @brief Resets the iteratos to the first element.
 * 
 * @param   iterator    The iterator instance.
 */
MOCKABLE_FUNCTION(, void, GroupsIterator_Reset, GroupsIteratorHandle, iterator);

/**
 * @brief Returns the numbers of groups in this iteratos.
 * 
 * @param   iterator    The iterator instance.
 * 
 * @return The number of groups of this iterator.
 */
MOCKABLE_FUNCTION(, uint32_t, GroupsIterator_GetGroupsCount, GroupsIteratorHandle, iterator);

/**
 * @brief Gets the current group name.
 * 
 * @param   iterator    The iterator instance.
 * 
 * @return The current group name.
 */
MOCKABLE_FUNCTION(, const char*, GroupsIterator_GetName, GroupsIteratorHandle, iterator);

/**
 * @brief Gets the current group id.
 * 
 * @param   iterator    The iterator instance.
 * 
 * @return The current group id.
 */
MOCKABLE_FUNCTION(, uint32_t, GroupsIterator_GetId, GroupsIteratorHandle, iterator);

#endif //GROUPS_ITERATOR_H