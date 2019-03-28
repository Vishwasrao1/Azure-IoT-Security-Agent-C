// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#include "os_utils/linux/iptables/iptables_utils.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(iptables_utils_ut)

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

TEST_FUNCTION(IptablesUtils_FormatIp_ExpectSuccess)
{
    char testBuffer[50] = "";

    IptablesResults result = IptablesUtils_FormatIp(43200, 64767, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "192.168.0.0/14", testBuffer);

    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesUtils_FormatIp(43200, 64767, true, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "!192.168.0.0/14", testBuffer);

    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesUtils_FormatIp(185273099, 84108746, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "11.11.11.11/202.101.3.5", testBuffer);

    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesUtils_FormatIp(43200, 0, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "192.168.0.0", testBuffer);

    result = IptablesUtils_FormatIp(43200, 64767, false, testBuffer, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_EXCEPTION, result);

    result = IptablesUtils_FormatIp(0, 0, false, testBuffer, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);
}

TEST_FUNCTION(IptablesUtils_FormatProtocol_ExpectSuccess)
{
    char testBuffer[50] = "";

    IptablesResults result = IptablesUtils_FormatProtocol(IPPROTO_TCP, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "tcp", testBuffer);

    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesUtils_FormatProtocol(IPPROTO_UDP, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "udp", testBuffer);

    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesUtils_FormatProtocol(IPPROTO_ICMP, true, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "!icmp", testBuffer);

    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesUtils_FormatProtocol(IPPROTO_IPV6, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_EXCEPTION, result);
}

TEST_FUNCTION(IptablesUtils_FormatRangedPorts_ExpectSuccess)
{
    char testBuffer[50] = "";

    IptablesResults result = IptablesUtils_FormatRangedPorts(0, 65535, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);

    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesUtils_FormatRangedPorts(34, 56, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "34-56", testBuffer);

    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesUtils_FormatRangedPorts(6, 10, true, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "!(6-10)", testBuffer);
}

TEST_FUNCTION(IptablesUtils_GetActionTypeEnumFromActionString_ExpectSuccess)
{
    IptablesActionType resultAction;
    IptablesResults result;
    result = IptablesUtils_GetActionTypeEnumFromActionString(IPTABLES_ACCEPT_VERDICT, &resultAction);
    ASSERT_ARE_EQUAL(int, IPTABLES_ACTION_ALLOW, resultAction);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    IptablesUtils_GetActionTypeEnumFromActionString(IPTABLES_REJECT_VERDICT, &resultAction);
    ASSERT_ARE_EQUAL(int, IPTABLES_ACTION_DENY, resultAction);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    IptablesUtils_GetActionTypeEnumFromActionString(IPTABLES_DROP_VERDICT, &resultAction);
    ASSERT_ARE_EQUAL(int, IPTABLES_ACTION_DENY, resultAction);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    IptablesUtils_GetActionTypeEnumFromActionString("Unknown Action", &resultAction);
    ASSERT_ARE_EQUAL(int, IPTABLES_ACTION_OTHER, resultAction);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
}

TEST_FUNCTION(IptablesUtils_GetActionTypeEnumFromActionString_ExpectFail)
{
    IptablesActionType resultAction;
    IptablesResults result;
    result = IptablesUtils_GetActionTypeEnumFromActionString(NULL, &resultAction);
    ASSERT_ARE_EQUAL(int, IPTABLES_EXCEPTION, result);

    result = IptablesUtils_GetActionTypeEnumFromActionString(IPTABLES_REJECT_VERDICT, NULL);
    ASSERT_ARE_EQUAL(int, IPTABLES_EXCEPTION, result);
}

END_TEST_SUITE(iptables_utils_ut)
