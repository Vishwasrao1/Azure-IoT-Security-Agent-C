// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/groups_iterator.h"

#include <grp.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct _GroupsIterator {

    gid_t* groups;
    uint32_t groupsCount;
    uint32_t currentGroupIndex;
    struct group* currentGroup;

} GroupsIterator;

bool GroupsIterator_Init(GroupsIteratorHandle* iterator, struct passwd* user) {

    bool success = true;
    
    GroupsIterator* iteratorObj = malloc(sizeof(GroupsIterator));
    if (iteratorObj == NULL) {
        success = false;
        goto cleanup;
    }

    memset(iteratorObj, 0, sizeof(GroupsIterator));

    if (getgrouplist(user->pw_name, user->pw_gid, iteratorObj->groups, (int*)&iteratorObj->groupsCount) == -1) {
        iteratorObj->groups = malloc(iteratorObj->groupsCount * sizeof (gid_t));
        if (iteratorObj->groups == NULL) {
            success = false;
            goto cleanup;        
        }
    }

    getgrouplist(user->pw_name, user->pw_gid, iteratorObj->groups, (int*)&iteratorObj->groupsCount);
    
cleanup:
    *iterator = (GroupsIteratorHandle)iteratorObj;
    
    if (!success) {
        GroupsIterator_Deinit(*iterator);
    }

    return success;
}

void GroupsIterator_Deinit(GroupsIteratorHandle iterator) {
    GroupsIterator* iteratorObj = (GroupsIterator*)iterator;
    if (iteratorObj != NULL) {
        if (iteratorObj->groups != NULL) {
            free(iteratorObj->groups);
        }
        free(iteratorObj);
    }
}

bool GroupsIterator_HasNext(GroupsIteratorHandle iterator) {
    GroupsIterator* iteratorObj = (GroupsIterator*)iterator;
    return iteratorObj->currentGroupIndex < iteratorObj->groupsCount;
}

bool GroupsIterator_Next(GroupsIteratorHandle iterator) {
    GroupsIterator* iteratorObj = (GroupsIterator*)iterator;
    iteratorObj->currentGroup = getgrgid(iteratorObj->groups[iteratorObj->currentGroupIndex]);
    ++(iteratorObj->currentGroupIndex);
    if (iteratorObj->currentGroup == NULL) {
        return false;
    }
    return true;
}

void GroupsIterator_Reset(GroupsIteratorHandle iterator) {
    GroupsIterator* iteratorObj = (GroupsIterator*)iterator;
    iteratorObj->currentGroupIndex = 0;
    iteratorObj->currentGroup = NULL;
}

uint32_t GroupsIterator_GetGroupsCount( GroupsIteratorHandle iterator) {
    GroupsIterator* iteratorObj = (GroupsIterator*)iterator;
    return iteratorObj->groupsCount;
}

const char* GroupsIterator_GetName( GroupsIteratorHandle iterator) {
    GroupsIterator* iteratorObj = (GroupsIterator*)iterator;
    return iteratorObj->currentGroup->gr_name;
}

uint32_t GroupsIterator_GetId( GroupsIteratorHandle iterator) {
    GroupsIterator* iteratorObj = (GroupsIterator*)iterator;
    return iteratorObj->currentGroup->gr_gid;
}
