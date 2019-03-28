// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef USERS_ITERATOR_H
#define USERS_ITERATOR_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <stdint.h>

#include "os_utils/groups_iterator.h"

typedef enum _UserIteratorResults {

    USER_ITERATOR_OK,
    USER_ITERATOR_HAS_NEXT,
    USER_ITERATOR_STOP,
    USER_ITERATOR_EXCEPTION

} UserIteratorResults;

typedef struct UsersIterator* UsersIteratorHandle;

/**
 * @brief Initiates a new local users iterator.
 * 
 * @param   iterator    The newly created users iterator.
 * 
 * @return USER_ITERATOR_OK of success or USER_ITERATOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, UserIteratorResults, UsersIterator_Init, UsersIteratorHandle*, iterator);

/**
 * @bried Deiniitates the users iterator instance.
 * 
 * @param   iterator    The iterator instance.
 */
MOCKABLE_FUNCTION(, void, UsersIterator_Deinit, UsersIteratorHandle, iterator);

/**
 * @bried Try to get the next user of this iterator.
 * 
 * @param   iterator    The iterator instance.
 * 
 * @return USER_ITERATOR_HAS_NEXT in case there is another user, USER_ITERATOR_STOP in case there arn't any more users
 *          or an error value otherwise.
 */
MOCKABLE_FUNCTION(, UserIteratorResults, UsersIterator_GetNext, UsersIteratorHandle, iterator);

/**
 * @bried Gets the current username.
 * 
 * @param   iterator    The iterator instance.
 * 
 * @return The username of the current user.
 */
MOCKABLE_FUNCTION(, const char*, UsersIterator_GetUsername, UsersIteratorHandle, iterator);

/**
 * @bried Gets the current user id.
 * 
 * @param   iterator    The iterator instance.
 * 
 * @return The user id of the current user.
 */
MOCKABLE_FUNCTION(, int32_t, UsersIterator_GetUserId, UsersIteratorHandle, iterator);

/**
 * @bried Creates a new groups iterator for the current user.
 * 
 * @param   iterator            The iterator instance.
 * @param   groupsIterator      Out param. The groups iterator instance.
 * 
 * @return USER_ITERATOR_OK on success, USER_ITERATOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, UserIteratorResults, UsersIterator_CreateGroupsIterator, UsersIteratorHandle, iterator, GroupsIteratorHandle*, groupsIterator);

#endif //USERS_ITERATOR_H