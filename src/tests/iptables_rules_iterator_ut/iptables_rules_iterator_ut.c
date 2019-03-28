// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "iptables_mocks.h"
#include "os_utils/linux/iptables/iptables_ip_utils.h"
#include "os_utils/linux/iptables/iptables_port_utils.h"
#include "os_utils/linux/iptables/iptables_utils.h"
#undef ENABLE_MOCKS

#include "os_utils/linux/iptables/iptables_rules_iterator.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static IptablesActionType actionTypeMock;
IptablesResults IptablesUtils_GetActionTypeEnumFromActionStringMock(const char* action, IptablesActionType* actionType) {
    *actionType = actionTypeMock;
    return IPTABLES_OK;
}

BEGIN_TEST_SUITE(iptables_rules_iterator_ut)

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

    REGISTER_GLOBAL_MOCK_HOOK(IptablesUtils_GetActionTypeEnumFromActionString, IptablesUtils_GetActionTypeEnumFromActionStringMock);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(IptablesUtils_GetActionTypeEnumFromActionString, NULL);
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(IptablesRulesIterator_Init_Deinit_ExpectSuccess)
{
    IptablesIterator chainIterator = {0};
    chainIterator.iptcHandle = (struct xtc_handle*)0x1;
    chainIterator.currentChain = "chain";
    IptablesIteratorHandle chainIteratorhandle = (IptablesIteratorHandle) &chainIterator;

    IptablesRulesIteratorHandle iterator = NULL;
    IptablesResults result = IptablesRulesIterator_Init(chainIteratorhandle, &iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    IptablesRulesIterator* iteratorObj = (IptablesRulesIterator*)iterator;
    ASSERT_IS_NULL(iteratorObj->currentEntry);
    ASSERT_ARE_EQUAL(char_ptr, "chain", iteratorObj->chain);
    ASSERT_IS_FALSE(iteratorObj->started);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    IptablesRulesIterator_Deinit(iterator);
}

TEST_FUNCTION(IptablesRulesIterator_GetNext_ExpectSuccess)
{
    IptablesRulesIterator iteratorObj = {0};
    IptablesRulesIteratorHandle iterator = (IptablesRulesIteratorHandle)&iteratorObj;
    iteratorObj.chain = "chain";
    struct xtc_handle* mockedHandle = (struct xtc_handle*)0x1;
    iteratorObj.iptcHandle = mockedHandle;
    const struct ipt_entry dummyEntry;
    
    STRICT_EXPECTED_CALL(iptc_first_rule("chain", mockedHandle)).SetReturn(&dummyEntry);
    IptablesResults result = IptablesRulesIterator_GetNext(iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_ITERATOR_HAS_NEXT, result);

    STRICT_EXPECTED_CALL(iptc_next_rule(&dummyEntry, mockedHandle)).SetReturn(&dummyEntry);
    result = IptablesRulesIterator_GetNext(iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_ITERATOR_HAS_NEXT, result);

    STRICT_EXPECTED_CALL(iptc_next_rule(&dummyEntry, mockedHandle)).SetReturn(NULL);
    result = IptablesRulesIterator_GetNext(iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_ITERATOR_NO_MORE_ITEMS, result);
    
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IptablesRulesIterator_GetIp_ExpectSuccess)
{
    IptablesRulesIterator iteratorObj = {0};
    IptablesRulesIteratorHandle iterator = (IptablesRulesIteratorHandle)&iteratorObj;
    const struct ipt_entry mockedEntry;
    iteratorObj.currentEntry = &mockedEntry;

    STRICT_EXPECTED_CALL(IptablesIpUtils_GetIp(&mockedEntry, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_NO_DATA);
    IptablesResults result = IptablesRulesIterator_GetSrcIp(iterator, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);

    STRICT_EXPECTED_CALL(IptablesIpUtils_GetIp(&mockedEntry, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_OK);
    result = IptablesRulesIterator_GetDestIp(iterator, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IptablesRulesIterator_GetPorts_ExpectSuccess)
{
    IptablesRulesIterator iteratorObj = {0};
    IptablesRulesIteratorHandle iterator = (IptablesRulesIteratorHandle)&iteratorObj;
    const struct ipt_entry mockedEntry;
    iteratorObj.currentEntry = &mockedEntry;

    STRICT_EXPECTED_CALL(IptablesPortUtils_GetPort(&mockedEntry, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_OK);
    IptablesResults result = IptablesRulesIterator_GetSrcPort(iterator, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    STRICT_EXPECTED_CALL(IptablesPortUtils_GetPort(&mockedEntry, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_NO_DATA);
    result = IptablesRulesIterator_GetDestPort(iterator, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IptablesRulesIterator_GetProtocol_ExpectSuccess)
{
    IptablesRulesIterator iteratorObj = {0};
    IptablesRulesIteratorHandle iterator = (IptablesRulesIteratorHandle)&iteratorObj;
    struct ipt_entry mockedEntry;
    iteratorObj.currentEntry = &mockedEntry;

    mockedEntry.ip.proto = 5;
    mockedEntry.ip.invflags = IPT_INV_PROTO;

    STRICT_EXPECTED_CALL(IptablesUtils_FormatProtocol(5, true, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_OK);
    IptablesResults result = IptablesRulesIterator_GetProtocol(iterator, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    mockedEntry.ip.invflags = 0;

    STRICT_EXPECTED_CALL(IptablesUtils_FormatProtocol(5, false, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_OK);
    result = IptablesRulesIterator_GetProtocol(iterator, NULL, 0);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}


TEST_FUNCTION(IptablesRulesIterator_GetAction_ExpectSuccess)
{
    IptablesRulesIterator iteratorObj = {0};
    IptablesRulesIteratorHandle iterator = (IptablesRulesIteratorHandle)&iteratorObj;
    char buffer[25] = "";

    char targetAccpet[] = "ACCEPT";
    char targetDrop[] = "DROP";
    char targetReject[] = "REJECT";
    char targetReturn[] = "RETURN";
    char targetOther[] = "SOMETHING";
    char targetGoto[] = "JUMPING JUMPING";

    struct ipt_entry mockedEntry = {0};
    iteratorObj.currentEntry = &mockedEntry;
    IptablesActionType actionType;
    
    STRICT_EXPECTED_CALL(iptc_get_target(&mockedEntry, IGNORED_PTR_ARG)).SetReturn(targetAccpet);
    STRICT_EXPECTED_CALL(IptablesUtils_GetActionTypeEnumFromActionString(IGNORED_PTR_ARG, &actionType));
    actionTypeMock = IPTABLES_ACTION_ALLOW;
    IptablesResults result = IptablesRulesIterator_GetAction(iterator, &actionType, buffer, sizeof(buffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(int, IPTABLES_ACTION_ALLOW, actionType);
    ASSERT_ARE_EQUAL(int, 0, strlen(buffer));

    STRICT_EXPECTED_CALL(iptc_get_target(&mockedEntry, IGNORED_PTR_ARG)).SetReturn(targetDrop);
    STRICT_EXPECTED_CALL(IptablesUtils_GetActionTypeEnumFromActionString(IGNORED_PTR_ARG, &actionType));
    actionTypeMock = IPTABLES_ACTION_DENY;
    result = IptablesRulesIterator_GetAction(iterator, &actionType, buffer, sizeof(buffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(int, IPTABLES_ACTION_DENY, actionType);
    ASSERT_ARE_EQUAL(int, 0, strlen(buffer));

    STRICT_EXPECTED_CALL(iptc_get_target(&mockedEntry, IGNORED_PTR_ARG)).SetReturn(targetReject);
    STRICT_EXPECTED_CALL(IptablesUtils_GetActionTypeEnumFromActionString(IGNORED_PTR_ARG, &actionType));
    actionTypeMock = IPTABLES_ACTION_DENY;
    result = IptablesRulesIterator_GetAction(iterator, &actionType, buffer, sizeof(buffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(int, IPTABLES_ACTION_DENY, actionType);
    ASSERT_ARE_EQUAL(int, 0, strlen(buffer));

    memset(buffer, 0, sizeof(buffer));
    STRICT_EXPECTED_CALL(iptc_get_target(&mockedEntry, IGNORED_PTR_ARG)).SetReturn(targetReturn);
    STRICT_EXPECTED_CALL(IptablesUtils_GetActionTypeEnumFromActionString(IGNORED_PTR_ARG, &actionType));
    actionTypeMock = IPTABLES_ACTION_OTHER;
    result = IptablesRulesIterator_GetAction(iterator, &actionType, buffer, sizeof(buffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(int, IPTABLES_ACTION_OTHER, actionType);
    ASSERT_ARE_EQUAL(char_ptr, "RETURN", buffer);

    memset(buffer, 0, sizeof(buffer));
    STRICT_EXPECTED_CALL(iptc_get_target(&mockedEntry, IGNORED_PTR_ARG)).SetReturn(targetOther);
    STRICT_EXPECTED_CALL(IptablesUtils_GetActionTypeEnumFromActionString(IGNORED_PTR_ARG, &actionType));
    actionTypeMock = IPTABLES_ACTION_OTHER;
    result = IptablesRulesIterator_GetAction(iterator, &actionType, buffer, sizeof(buffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(int, IPTABLES_ACTION_OTHER, actionType);
    ASSERT_ARE_EQUAL(char_ptr, "SOMETHING", buffer);

    mockedEntry.ip.flags = IPT_F_GOTO;
    memset(buffer, 0, sizeof(buffer));
    STRICT_EXPECTED_CALL(iptc_get_target(&mockedEntry, IGNORED_PTR_ARG)).SetReturn(targetGoto);
    STRICT_EXPECTED_CALL(IptablesUtils_GetActionTypeEnumFromActionString(IGNORED_PTR_ARG, &actionType));
    actionTypeMock = IPTABLES_ACTION_OTHER;
    result = IptablesRulesIterator_GetAction(iterator, &actionType, buffer, sizeof(buffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(int, IPTABLES_ACTION_OTHER, actionType);
    ASSERT_ARE_EQUAL(char_ptr, "goto JUMPING JUMPING", buffer);

    STRICT_EXPECTED_CALL(iptc_get_target(&mockedEntry, IGNORED_PTR_ARG)).SetReturn(NULL);
    result = IptablesRulesIterator_GetAction(iterator, &actionType, buffer, sizeof(buffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);

    char noTargetName[] = "";
    STRICT_EXPECTED_CALL(iptc_get_target(&mockedEntry, IGNORED_PTR_ARG)).SetReturn(noTargetName);
    result = IptablesRulesIterator_GetAction(iterator, &actionType, buffer, sizeof(buffer));
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IptablesRulesIterator_GetChainName_ExpectSuccess)
{
    IptablesRulesIterator iteratorObj = {0};
    const char* chain_name = "CHAIN";
    iteratorObj.chain = chain_name;
    IptablesRulesIteratorHandle iterator = (IptablesRulesIteratorHandle)&iteratorObj;
    const char* actuaclResult = NULL;

    IptablesResults result = IptablesRulesIterator_GetChainName(iterator, &actuaclResult);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, chain_name, actuaclResult);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(iptables_rules_iterator_ut)
