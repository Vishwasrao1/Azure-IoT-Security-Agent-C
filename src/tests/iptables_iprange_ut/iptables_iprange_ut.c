// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"


#include "os_utils/linux/iptables/iptables_iprange.h"

#include <linux/netfilter/xt_iprange.h>


static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(iptables_iprange_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    
    umocktypes_bool_register_types();
    umocktypes_charptr_register_types();
    
    REGISTER_UMOCK_ALIAS_TYPE(IptablesResults, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(IptablesIprange_TryGetRangedIp_ExpectSuccess)
{
    char testBuffer[50] = "";
    char entryBuffer[sizeof(struct ipt_entry) + sizeof(struct xt_entry_match) + sizeof(struct xt_iprange_mtinfo)];
    memset(entryBuffer, 0, sizeof(entryBuffer));
    struct ipt_entry* mockedEntry = (struct ipt_entry*)entryBuffer;
    mockedEntry->target_offset = sizeof(entryBuffer);

    struct xt_entry_match* match = (struct xt_entry_match*)(entryBuffer + sizeof(struct ipt_entry));
    match->u.match_size = sizeof(entryBuffer) - sizeof(struct ipt_entry);
    memcpy(match->u.user.name, "iprange", sizeof("ipragne"));

    struct xt_iprange_mtinfo* iprange = (struct xt_iprange_mtinfo*)(entryBuffer + sizeof(struct ipt_entry) + sizeof(struct xt_entry_match));
    iprange->flags = IPRANGE_SRC;
    iprange->src_min.in.s_addr = 67305985;
    iprange->src_max.in.s_addr = 134678021;

    memset(testBuffer, 0, sizeof(testBuffer));
    IptablesResults result = IptablesIprange_TryGetRangedIp(mockedEntry, true, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "1.2.3.4-5.6.7.8", testBuffer);

    iprange->flags = IPRANGE_SRC | IPRANGE_SRC_INV;
    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesIprange_TryGetRangedIp(mockedEntry, true, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "!(1.2.3.4-5.6.7.8)", testBuffer);

    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesIprange_TryGetRangedIp(mockedEntry, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);

    iprange->src_min.in.s_addr = 0;
    iprange->src_max.in.s_addr = 0;
    iprange->flags = IPRANGE_DST;
    iprange->dst_min.in.s_addr = 67305985;
    iprange->dst_max.in.s_addr = 134678021;
    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesIprange_TryGetRangedIp(mockedEntry, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "1.2.3.4-5.6.7.8", testBuffer);
}

END_TEST_SUITE(iptables_iprange_ut)
