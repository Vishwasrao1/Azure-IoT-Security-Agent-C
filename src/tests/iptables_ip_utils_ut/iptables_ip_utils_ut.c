// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "os_utils/linux/iptables/iptables_iprange.h"
#include "os_utils/linux/iptables/iptables_utils.h"
#undef ENABLE_MOCKS

#include "os_utils/linux/iptables/iptables_ip_utils.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(iptables_ip_utils_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    
    umocktypes_bool_register_types();
    umocktypes_charptr_register_types();
    
    REGISTER_UMOCK_ALIAS_TYPE(IptablesResults, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(uint16_t, unsigned short);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(IptablesIpUtils_GetIp_NoIp_ExpectSuccess)
{
    const struct ipt_entry mockedEntry = {0};

    STRICT_EXPECTED_CALL(IptablesIprange_TryGetRangedIp(&mockedEntry, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_NO_DATA);
    STRICT_EXPECTED_CALL(IptablesUtils_FormatIp(0, 0, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_NO_DATA);
    IptablesResults result = IptablesIpUtils_GetIp(&mockedEntry, true, NULL, 0);
    
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IptablesIpUtils_GetIp_RangedIp_ExpectSuccess)
{
    const struct ipt_entry mockedEntry = {0};

    STRICT_EXPECTED_CALL(IptablesIprange_TryGetRangedIp(&mockedEntry, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_OK);
    IptablesResults result = IptablesIpUtils_GetIp(&mockedEntry, true, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    STRICT_EXPECTED_CALL(IptablesIprange_TryGetRangedIp(&mockedEntry, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_OK);
    result = IptablesIpUtils_GetIp(&mockedEntry, false, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IptablesIpUtils_GetIp_ExpectSuccess)
{
    struct ipt_entry mockedEntry = {0};
 
    mockedEntry.ip.src.s_addr = 12345;
    mockedEntry.ip.smsk.s_addr = 0xffff;
    mockedEntry.ip.dst.s_addr = 98765;
    mockedEntry.ip.dmsk.s_addr = 0x1111;
    mockedEntry.ip.invflags = IPT_INV_DSTIP;

    STRICT_EXPECTED_CALL(IptablesIprange_TryGetRangedIp(&mockedEntry, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_NO_DATA);
    STRICT_EXPECTED_CALL(IptablesUtils_FormatIp(12345, 0xffff, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_OK);
    IptablesResults result = IptablesIpUtils_GetIp(&mockedEntry, true, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    STRICT_EXPECTED_CALL(IptablesIprange_TryGetRangedIp(&mockedEntry, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_NO_DATA);
    STRICT_EXPECTED_CALL(IptablesUtils_FormatIp(98765, 0x1111, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_OK);
    result = IptablesIpUtils_GetIp(&mockedEntry, false,  NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(iptables_ip_utils_ut)
