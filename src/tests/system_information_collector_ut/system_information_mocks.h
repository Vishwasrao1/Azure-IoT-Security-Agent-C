// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <sys/sysinfo.h>
#include <sys/utsname.h>

MOCKABLE_FUNCTION(, int, sysinfo, struct sysinfo*, info);
MOCKABLE_FUNCTION(, int, uname, struct utsname*, buf);
