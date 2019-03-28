// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <pwd.h>

MOCKABLE_FUNCTION(, void, setpwent);
MOCKABLE_FUNCTION(, void, endpwent);
MOCKABLE_FUNCTION(, struct passwd*, getpwent);
