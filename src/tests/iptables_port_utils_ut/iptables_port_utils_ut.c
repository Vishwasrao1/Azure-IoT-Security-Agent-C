// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "os_utils/linux/iptables/iptables_multiport.h"
#include "os_utils/linux/iptables/iptables_utils.h"
#undef ENABLE_MOCKS

#include "os_utils/linux/iptables/iptables_port_utils.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(iptables_port_utils_ut)

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
    REGISTER_UMOCK_ALIAS_TYPE(uint16_t, unsigned short);
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

TEST_FUNCTION(IptablesPortUtils_GetPort_NoPorts_ExpectSuccess)
{
    struct ipt_entry mockedEntry;
    memset(&mockedEntry, 0, sizeof(mockedEntry));
    
    IptablesResults result = IptablesPortUtils_GetPort(&mockedEntry, true, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);

    result = IptablesPortUtils_GetPort(&mockedEntry, false, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IptablesPortUtils_GetPort_TcpMatchPorts_ExpectSuccess)
{
    char mockedEntryData[sizeof(struct ipt_entry) + sizeof(struct xt_entry_match) + sizeof(struct xt_tcp)];
    memset(mockedEntryData, 0, sizeof(mockedEntryData));
    struct ipt_entry* mockedEntry = (struct ipt_entry*)mockedEntryData;
    mockedEntry->target_offset = sizeof(mockedEntryData);

    struct xt_entry_match* match = (struct xt_entry_match*)(mockedEntryData + sizeof(struct ipt_entry));
    memcpy(match->u.user.name, "tcp", 3);
    match->u.match_size = sizeof(struct xt_entry_match) + sizeof(struct xt_tcp);

    struct xt_tcp* tcpInfo = (struct xt_tcp*)match->data;
    tcpInfo->spts[0] = 1;
    tcpInfo->spts[1] = 20;
    tcpInfo->dpts[0] = 3000;
    tcpInfo->dpts[1] = 3078;
    tcpInfo->invflags = XT_TCP_INV_SRCPT;

    STRICT_EXPECTED_CALL(IptablesUtils_FormatRangedPorts(1, 20, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_OK);
    IptablesResults result = IptablesPortUtils_GetPort(mockedEntry, true, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    STRICT_EXPECTED_CALL(IptablesUtils_FormatRangedPorts(3000, 3078, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_OK);
    result = IptablesPortUtils_GetPort(mockedEntry, false, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    STRICT_EXPECTED_CALL(IptablesUtils_FormatRangedPorts(3000, 3078, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_EXCEPTION);
    result = IptablesPortUtils_GetPort(mockedEntry, false, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_EXCEPTION, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IptablesPortUtils_GetPort_MultiportsMatchPorts_ExpectSuccess)
{
    char mockedEntryData[sizeof(struct ipt_entry) + sizeof(struct xt_entry_match)];
    memset(mockedEntryData, 0, sizeof(mockedEntryData));
    struct ipt_entry* mockedEntry = (struct ipt_entry*)mockedEntryData;
    mockedEntry->target_offset = sizeof(mockedEntryData);

    struct xt_entry_match* match = (struct xt_entry_match*)(mockedEntryData + sizeof(struct ipt_entry));
    memcpy(match->u.user.name, "multiport", 9);
    match->u.match_size = sizeof(struct xt_entry_match);

    STRICT_EXPECTED_CALL(IptablesMultiport_GetPorts(match, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_OK);
    IptablesResults result = IptablesPortUtils_GetPort(mockedEntry, true, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    STRICT_EXPECTED_CALL(IptablesMultiport_GetPorts(match, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_NO_DATA);
    result = IptablesPortUtils_GetPort(mockedEntry, true, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);

    STRICT_EXPECTED_CALL(IptablesMultiport_GetPorts(match, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_OK);
    result = IptablesPortUtils_GetPort(mockedEntry, false, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(iptables_port_utils_ut)
