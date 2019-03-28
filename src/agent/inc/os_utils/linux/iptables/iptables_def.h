// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IPTABLES_DEFS_H
#define IPTABLES_DEFS_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <libiptc/libiptc.h>
#include <stdbool.h>

typedef struct IptablesIterator* IptablesIteratorHandle;
typedef struct IptablesRulesIterator* IptablesRulesIteratorHandle;

extern const char IPTABLES_NEGATE[];
extern const char IPTABLES_EMPTY[];
extern const char IPTABLES_NEGATE_EXPRESSION_START[];
extern const char IPTABLES_NEGATE_EXPRESSION_END[];

extern const char IPTABLES_IPRANGE_MATCH[];
extern const char IPTABLES_TCP_MATCH[];
extern const char IPTABLES_MULTIPORT_MATCH[];

extern const char IPTABLES_ACCEPT_VERDICT[];
extern const char IPTABLES_REJECT_VERDICT[];
extern const char IPTABLES_DROP_VERDICT[];

#define IP_BYTES_RAW(n)         \
(unsigned int)((n)>>24)&0xFF,   \
(unsigned int)((n)>>16)&0xFF,   \
(unsigned int)((n)>>8)&0xFF,    \
(unsigned int)((n)&0xFF)

#define IP_BYTES(n) IP_BYTES_RAW(ntohl(n))

typedef enum _IptablesResults {

    IPTABLES_OK,
    IPTABLES_NO_DATA,
    IPTABLES_ITERATOR_HAS_NEXT,
    IPTABLES_ITERATOR_NO_MORE_ITEMS,
    IPTABLES_EXCEPTION

} IptablesResults;

typedef struct _IptablesRulesIterator {
     
    struct xtc_handle* iptcHandle;
    const struct ipt_entry* currentEntry;
    bool started;
    const char* chain;

} IptablesRulesIterator;

typedef struct _IptablesIterator {

    struct xtc_handle* iptcHandle;
    bool started;
    const char* currentChain;

} IptablesIterator;

typedef enum _IptablesActionType {
    IPTABLES_ACTION_ALLOW,
    IPTABLES_ACTION_DENY,
    IPTABLES_ACTION_OTHER
} IptablesActionType;

#endif //IPTABLES_DEFS_H