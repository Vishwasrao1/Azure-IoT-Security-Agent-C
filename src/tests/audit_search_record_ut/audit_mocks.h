// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <auparse.h>

MOCKABLE_FUNCTION(, unsigned int, auparse_get_num_records, auparse_state_t*, au);
MOCKABLE_FUNCTION(, int, auparse_goto_record_num, auparse_state_t*, au, unsigned int, num);
MOCKABLE_FUNCTION(, int, auparse_get_type, auparse_state_t*, au);
MOCKABLE_FUNCTION(, const char*, auparse_get_record_text, auparse_state_t*, au);
