// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <sys/types.h>


#include "macro_utils.h"
#include "umock_c_prod.h"

MOCKABLE_FUNCTION(, uid_t, getuid);
MOCKABLE_FUNCTION(, uid_t, geteuid);
MOCKABLE_FUNCTION(, int, setreuid, uid_t, ruid, uid_t, euid);
MOCKABLE_FUNCTION(, int, seteuid, uid_t, euid);

