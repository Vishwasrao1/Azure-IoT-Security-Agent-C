// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "os_utils/linux/iptables/iptables_def.h"

MOCKABLE_FUNCTION(, const struct ipt_entry*, iptc_first_rule, const char*, chain, struct xtc_handle*, handle);
MOCKABLE_FUNCTION(, const struct ipt_entry*, iptc_next_rule, const struct ipt_entry*, prev, struct xtc_handle*, handle);
MOCKABLE_FUNCTION(, const char*, iptc_get_target, const struct ipt_entry*, entry, struct xtc_handle*, handle);
