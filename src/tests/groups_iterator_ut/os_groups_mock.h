// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <grp.h>
#include <stdlib.h>
#include <sys/types.h>

MOCKABLE_FUNCTION(, int, getgrouplist, const char*, user, gid_t, group, gid_t*, groups, int*, ngroups);
MOCKABLE_FUNCTION(, struct group*, getgrgid, gid_t, gid);
