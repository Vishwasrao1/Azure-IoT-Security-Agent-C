// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/users_iterator.h"

#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/types.h>

#include "message_schema_consts.h"

typedef struct _UsersIterator {

    struct passwd* currentUser;

} UsersIterator;

UserIteratorResults UsersIterator_Init(UsersIteratorHandle* iterator) {
    UsersIterator* iteratorObj = malloc(sizeof(UsersIterator));
    if (iteratorObj == NULL) {
        return USER_ITERATOR_EXCEPTION;
    }

    memset(iteratorObj, 0, sizeof(UsersIterator));
    setpwent();
    *iterator = (UsersIteratorHandle)iteratorObj;
    return USER_ITERATOR_OK;
}

void UsersIterator_Deinit(UsersIteratorHandle iterator) {
    endpwent();
    free(iterator);
}

UserIteratorResults UsersIterator_GetNext(UsersIteratorHandle iterator) {
    UsersIterator* iteratorObj = (UsersIterator*)iterator;

    errno = 0;
    iteratorObj->currentUser = getpwent();
    if (iteratorObj->currentUser == NULL) {
        if (errno != 0) {
            return USER_ITERATOR_EXCEPTION;
        }
        return USER_ITERATOR_STOP;
    }

    return USER_ITERATOR_HAS_NEXT;
}

const char* UsersIterator_GetUsername(UsersIteratorHandle iterator) {
    UsersIterator* iteratorObj = (UsersIterator*)iterator;
    return iteratorObj->currentUser->pw_name;
}

int32_t UsersIterator_GetUserId(UsersIteratorHandle iterator) {
    UsersIterator* iteratorObj = (UsersIterator*)iterator;
    return iteratorObj->currentUser->pw_uid;
}

UserIteratorResults UsersIterator_CreateGroupsIterator(UsersIteratorHandle iterator, GroupsIteratorHandle* groupsIterator) {
    UsersIterator* iteratorObj = (UsersIterator*)iterator;

    if (!GroupsIterator_Init(groupsIterator, iteratorObj->currentUser)) {
        return USER_ITERATOR_EXCEPTION;
    }

    return USER_ITERATOR_OK;
}
