// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#include <linux/netfilter/xt_multiport.h>

#include "os_utils/linux/iptables/iptables_multiport.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(iptables_multiport_ut)

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

TEST_FUNCTION(IptablesMultiport_GetPorts_V1_ExpectSuccess)
{
    char testBuffer[50] = "";
    char data[sizeof(struct xt_entry_match) + sizeof(struct xt_multiport_v1)];
    memset(data, 0, sizeof(data));

    struct xt_entry_match* match = (struct xt_entry_match*)data;
    match->u.match_size = sizeof(data);

    struct xt_multiport_v1* multiport = (struct xt_multiport_v1*)(data + sizeof(struct xt_entry_match));
    multiport->flags = XT_MULTIPORT_SOURCE;
    multiport->count = 4;
    multiport->ports[0] = 4;
    multiport->ports[1] = 6;
    multiport->ports[2] = 8;
    multiport->ports[3] = 12;
    multiport->pflags[1] = 1;

    memset(testBuffer, 0, sizeof(testBuffer));
    IptablesResults result = IptablesMultiport_GetPorts(match, true, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "4,6-8,12", testBuffer);

    multiport->invert = 1;
    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesMultiport_GetPorts(match, true, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "!(4,6-8,12)", testBuffer);

    multiport->flags = XT_MULTIPORT_DESTINATION;
    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesMultiport_GetPorts(match, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "!(4,6-8,12)", testBuffer);
}

TEST_FUNCTION(IptablesMultiport_GetPorts_V1_NoData_ExpectSuccess)
{
    char testBuffer[50] = "";
    char data[sizeof(struct xt_entry_match) + sizeof(struct xt_multiport_v1)];
    memset(data, 0, sizeof(data));

    struct xt_entry_match* match = (struct xt_entry_match*)data;
    match->u.match_size = sizeof(data);

    struct xt_multiport_v1* multiport = (struct xt_multiport_v1*)(data + sizeof(struct xt_entry_match));
    multiport->flags = XT_MULTIPORT_SOURCE;

    memset(testBuffer, 0, sizeof(testBuffer));
    IptablesResults result = IptablesMultiport_GetPorts(match, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);
}

TEST_FUNCTION(IptablesMultiport_GetPorts_ExpectSuccess)
{
    char testBuffer[50] = "";
    char data[sizeof(struct xt_entry_match) + sizeof(struct xt_multiport)];
    memset(data, 0, sizeof(data));

    struct xt_entry_match* match = (struct xt_entry_match*)data;
    match->u.match_size = sizeof(data);

    struct xt_multiport* multiport = (struct xt_multiport*)(data + sizeof(struct xt_entry_match));
    multiport->flags = XT_MULTIPORT_SOURCE;
    multiport->count = 3;
    multiport->ports[0] = 1;
    multiport->ports[1] = 2;
    multiport->ports[2] = 3;

    memset(testBuffer, 0, sizeof(testBuffer));
    IptablesResults result = IptablesMultiport_GetPorts(match, true, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "1,2,3", testBuffer);

    multiport->flags = XT_MULTIPORT_DESTINATION;
    memset(testBuffer, 0, sizeof(testBuffer));
    result = IptablesMultiport_GetPorts(match, false, testBuffer, sizeof(testBuffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, "1,2,3", testBuffer);
}

END_TEST_SUITE(iptables_multiport_ut)
