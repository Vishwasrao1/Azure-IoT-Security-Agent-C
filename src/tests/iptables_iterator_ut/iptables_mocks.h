// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "os_utils/linux/iptables/iptables_def.h"

MOCKABLE_FUNCTION(, struct xtc_handle*, iptc_init, const char*, tablename);
MOCKABLE_FUNCTION(, void, iptc_free, struct xtc_handle*, handle);
MOCKABLE_FUNCTION(, const char*, iptc_first_chain, struct xtc_handle*, handle);
MOCKABLE_FUNCTION(, const char*, iptc_next_chain, struct xtc_handle*, handle);
MOCKABLE_FUNCTION(, int, iptc_builtin, const char*, chain, struct xtc_handle* const, handle);
MOCKABLE_FUNCTION(, const char *, iptc_get_policy, const char *, chain, struct xt_counters *, counter, struct xtc_handle *, handle);