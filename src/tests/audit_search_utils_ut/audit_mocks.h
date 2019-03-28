// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <auparse.h>

MOCKABLE_FUNCTION(, const char*, auparse_find_field, auparse_state_t*, au, const char*, field);
MOCKABLE_FUNCTION(, int, auparse_get_field_int, auparse_state_t*, au);
MOCKABLE_FUNCTION(, const char*, auparse_get_field_str, auparse_state_t*, au);
MOCKABLE_FUNCTION(, const char*, auparse_interpret_field, auparse_state_t*, au);
