// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "iptables_mocks.h"
#include "os_utils/linux/iptables/iptables_rules_iterator.h"
#undef ENABLE_MOCKS

#include <errno.h>
#include "os_utils/linux/iptables/iptables_iterator.h"
#include "os_utils/linux/iptables/iptables_utils.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}


BEGIN_TEST_SUITE(iptables_iterator_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    
    umocktypes_bool_register_types();
    umocktypes_charptr_register_types();
    
    REGISTER_UMOCK_ALIAS_TYPE(IptablesResults, int);
    REGISTER_UMOCK_ALIAS_TYPE(IptablesIteratorHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(struct xtc_handle* const,  void*);
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

TEST_FUNCTION(IptablesIterator_Init_ExpectSuccess)
{
    IptablesIteratorHandle iterator = NULL;

    struct xtc_handle* mockedHandle = (struct xtc_handle*)0x1;
    STRICT_EXPECTED_CALL(iptc_init("filter")).SetReturn(mockedHandle);

    IptablesResults result = IptablesIterator_Init(&iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    IptablesIterator* iteratorObj = (IptablesIterator*)iterator;
    ASSERT_IS_FALSE(iteratorObj->started);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    IptablesIterator_Deinit(iterator);
}

TEST_FUNCTION(IptablesIterator_Init_NotDefined_ExpectSuccess)
{
    IptablesIteratorHandle iterator = NULL;

    struct xtc_handle* mockedHandle = (struct xtc_handle*)0x1;
    STRICT_EXPECTED_CALL(iptc_init("filter")).SetReturn(NULL);
    errno = ENOPROTOOPT;

    IptablesResults result = IptablesIterator_Init(&iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IptablesIterator_InitFailed_ExpectFailure)
{
    IptablesIteratorHandle iterator = NULL;

    STRICT_EXPECTED_CALL(iptc_init("filter")).SetReturn(NULL);

    IptablesResults result = IptablesIterator_Init(&iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IptablesIterator_Deinit_ExpectSuccess)
{
    IptablesIteratorHandle iterator = NULL;

    struct xtc_handle* mockedHandle = (struct xtc_handle*)0x1;
    STRICT_EXPECTED_CALL(iptc_init("filter")).SetReturn(mockedHandle);

    IptablesResults result = IptablesIterator_Init(&iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);

    STRICT_EXPECTED_CALL(iptc_free(mockedHandle));
    IptablesIterator_Deinit(iterator);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IptablesIterator_GetNext_ExpectSuccess)
{
    IptablesIteratorHandle iterator = NULL;

    struct xtc_handle* mockedHandle = (struct xtc_handle*)0x1;
    STRICT_EXPECTED_CALL(iptc_init("filter")).SetReturn(mockedHandle);

    IptablesResults result = IptablesIterator_Init(&iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    IptablesIterator* iteratorObj = (IptablesIterator*)iterator;
    
    STRICT_EXPECTED_CALL(iptc_first_chain(mockedHandle)).SetReturn("chain1");
    result = IptablesIterator_GetNext(iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_ITERATOR_HAS_NEXT, result);
    ASSERT_ARE_EQUAL(char_ptr, "chain1", iteratorObj->currentChain);

    STRICT_EXPECTED_CALL(iptc_next_chain(mockedHandle)).SetReturn("chain2");
    result = IptablesIterator_GetNext(iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_ITERATOR_HAS_NEXT, result);
    ASSERT_ARE_EQUAL(char_ptr, "chain2", iteratorObj->currentChain);

    STRICT_EXPECTED_CALL(iptc_next_chain(mockedHandle)).SetReturn(NULL);
    result = IptablesIterator_GetNext(iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_ITERATOR_NO_MORE_ITEMS, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    IptablesIterator_Deinit(iterator);
}

TEST_FUNCTION(IptablesIterator_GetRulesIterator_ExpectSuccess)
{
    IptablesIteratorHandle iterator = NULL;

    struct xtc_handle* mockedHandle = (struct xtc_handle*)0x1;
    STRICT_EXPECTED_CALL(iptc_init("filter")).SetReturn(mockedHandle);

    IptablesResults result = IptablesIterator_Init(&iterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    
    IptablesRulesIteratorHandle rulesIterator = NULL;
    STRICT_EXPECTED_CALL(IptablesRulesIterator_Init(iterator, &rulesIterator)).SetReturn(IPTABLES_OK);
    result = IptablesIterator_GetRulesIterator(iterator, &rulesIterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    
    STRICT_EXPECTED_CALL(IptablesRulesIterator_Init(iterator, &rulesIterator)).SetReturn(IPTABLES_EXCEPTION);
    result = IptablesIterator_GetRulesIterator(iterator, &rulesIterator);
    ASSERT_ARE_EQUAL(int, IPTABLES_EXCEPTION, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    IptablesIterator_Deinit(iterator);
}

TEST_FUNCTION(IptablesRulesIterator_GetChainName_ExpectSuccess)
{
    IptablesIterator iteratorObj = {0};
    const char* chain_name = "CHAIN";
    iteratorObj.currentChain = chain_name;
    IptablesIteratorHandle iterator = (IptablesIteratorHandle)&iteratorObj;
    const char* actualResult;

    IptablesResults result = IptablesIterator_GetChainName(iterator, &actualResult);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, chain_name, actualResult);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IptablesIterator_GetPolicyAction_ExpectSuccess)
{
    const char* chain_name = "CHAIN";
    const char* ACCEPT_POLICY = "ACCEPT";
    const char* REJECT_POLICY = "REJECT";
    const char* DROP_POLICY = "DROP";
    
    IptablesActionType actionTypeResult;
    IptablesIterator iteratorObj = {0};
    iteratorObj.currentChain = chain_name;
    IptablesIteratorHandle iterator = (IptablesIteratorHandle)&iteratorObj;

    STRICT_EXPECTED_CALL(iptc_builtin(chain_name, iteratorObj.iptcHandle)).SetReturn(1);
    STRICT_EXPECTED_CALL(iptc_get_policy(chain_name, IGNORED_PTR_ARG, iteratorObj.iptcHandle)).SetReturn(ACCEPT_POLICY);
    IptablesResults result = IptablesIterator_GetPolicyAction(iterator, &actionTypeResult);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(int, actionTypeResult, IPTABLES_ACTION_ALLOW);

    STRICT_EXPECTED_CALL(iptc_builtin(chain_name, iteratorObj.iptcHandle)).SetReturn(1);
    STRICT_EXPECTED_CALL(iptc_get_policy(chain_name, IGNORED_PTR_ARG, iteratorObj.iptcHandle)).SetReturn(REJECT_POLICY);
    result = IptablesIterator_GetPolicyAction(iterator, &actionTypeResult);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(int, actionTypeResult, IPTABLES_ACTION_DENY);

    STRICT_EXPECTED_CALL(iptc_builtin(chain_name, iteratorObj.iptcHandle)).SetReturn(1);
    STRICT_EXPECTED_CALL(iptc_get_policy(chain_name, IGNORED_PTR_ARG, iteratorObj.iptcHandle)).SetReturn(DROP_POLICY);
    result = IptablesIterator_GetPolicyAction(iterator, &actionTypeResult);
    ASSERT_ARE_EQUAL(int, IPTABLES_OK, result);
    ASSERT_ARE_EQUAL(int, actionTypeResult, IPTABLES_ACTION_DENY);

    STRICT_EXPECTED_CALL(iptc_builtin(chain_name, iteratorObj.iptcHandle)).SetReturn(0);
    result = IptablesIterator_GetPolicyAction(iterator, &actionTypeResult);
    ASSERT_ARE_EQUAL(int, IPTABLES_NO_DATA, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(iptables_iterator_ut)
