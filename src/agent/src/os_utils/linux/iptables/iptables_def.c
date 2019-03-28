// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/linux/iptables/iptables_def.h"

const char IPTABLES_NEGATE[] = "!";
const char IPTABLES_EMPTY[] = "";
const char IPTABLES_NEGATE_EXPRESSION_START[] = "!(";
const char IPTABLES_NEGATE_EXPRESSION_END[] = ")";

const char IPTABLES_IPRANGE_MATCH[] = "iprange";
const char IPTABLES_TCP_MATCH[] = "tcp";
const char IPTABLES_MULTIPORT_MATCH[] = "multiport";

const char IPTABLES_ACCEPT_VERDICT[] = "ACCEPT";
const char IPTABLES_REJECT_VERDICT[] = "REJECT";
const char IPTABLES_DROP_VERDICT[] = "DROP";