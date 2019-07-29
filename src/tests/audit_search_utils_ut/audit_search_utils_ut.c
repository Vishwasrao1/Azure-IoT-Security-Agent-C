// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"
#include "umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "audit_mocks.h"
#include "os_utils/process_info_handler.h"
#undef ENABLE_MOCKS

#include "os_utils/linux/audit/audit_search_utils.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static auparse_state_t* mockedAudit;

BEGIN_TEST_SUITE(audit_search_utils_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_bool_register_types();
    umocktypes_charptr_register_types();
    REGISTER_UMOCK_ALIAS_TYPE(AuditSearchResultValues, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
    mockedAudit = (auparse_state_t*)0x1;
}

TEST_FUNCTION(AuditSearchUtils_ReadInt_ExpectSuccess)
{
    AuditSearch search;
    search.audit = mockedAudit;

    const char* fieldName = "djdjd";
    const char* auparseFindFieldReturnValue = "dummy";
    int expectedValue = 7;
    STRICT_EXPECTED_CALL(auparse_find_field(mockedAudit, fieldName)).SetReturn(auparseFindFieldReturnValue);
    STRICT_EXPECTED_CALL(auparse_get_field_int(mockedAudit)).SetReturn(expectedValue);

    int output = 0;
    AuditSearchResultValues result = AuditSearchUtils_ReadInt(&search, fieldName, &output);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_OK, result);
    ASSERT_ARE_EQUAL(int, expectedValue, output);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuditSearchUtils_ReadInt_AuditFunctionFailed_ExpectFailure)
{
    AuditSearch search;
    search.audit = mockedAudit;

    const char* fieldName = "djdjd";
    const char* auparseFindFieldReturnValue = "dummy";
    int expectedValue = 7;
    STRICT_EXPECTED_CALL(auparse_find_field(mockedAudit, fieldName)).SetReturn(NULL);

    int output = 0;
    AuditSearchResultValues result = AuditSearchUtils_ReadInt(&search, fieldName, &output);
    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_FIELD_DOES_NOT_EXIST, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuditSearchUtils_ReadString_ExpectSuccess)
{
    AuditSearch search;
    search.audit = mockedAudit;

    const char* fieldName = "djdjd";
    const char* auparseFindFieldReturnValue = "dummy";
    const char* expectedValue = "this is a value";
    STRICT_EXPECTED_CALL(auparse_find_field(mockedAudit, fieldName)).SetReturn(auparseFindFieldReturnValue);
    STRICT_EXPECTED_CALL(auparse_get_field_str(mockedAudit)).SetReturn(expectedValue);

    const char* output = 0;
    AuditSearchResultValues result = AuditSearchUtils_ReadString(&search, fieldName, &output);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, expectedValue, output);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuditSearchUtils_ReadString_FieldDoesNotExist_ExpectSuccess)
{
    AuditSearch search;
    search.audit = mockedAudit;

    const char* fieldName = "djdjd";
    const char* auparseFindFieldReturnValue = "dummy";
    const char* expectedValue = "this is a value";
    STRICT_EXPECTED_CALL(auparse_find_field(mockedAudit, fieldName)).SetReturn(NULL);

    const char* output = 0;
    AuditSearchResultValues result = AuditSearchUtils_ReadString(&search, fieldName, &output);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_FIELD_DOES_NOT_EXIST, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuditSearchUtils_InterpretString_ExpectSuccess)
{
    AuditSearch search;
    search.audit = mockedAudit;
    
    const char* fieldName = "djdjd";
    const char* auparseFindFieldReturnValue = "dummy";
    const char* expectedValue = "this is a value";
    STRICT_EXPECTED_CALL(auparse_find_field(mockedAudit, fieldName)).SetReturn(auparseFindFieldReturnValue);
    STRICT_EXPECTED_CALL(auparse_interpret_field(mockedAudit)).SetReturn(expectedValue);

    const char* output = NULL;
    AuditSearchResultValues result = AuditSearchUtils_InterpretString(&search, fieldName, &output);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, expectedValue, output);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuditSearchUtils_InterpretString_Fail_ExpectFailue)
{
    AuditSearch search;
    search.audit = mockedAudit;

    const char* fieldName = "djdjd";
    const char* auparseFindFieldReturnValue = "dummy";
    const char* expectedValue = "this is a value";

    umock_c_negative_tests_init();

    STRICT_EXPECTED_CALL(auparse_find_field(mockedAudit, fieldName)).SetReturn(auparseFindFieldReturnValue).SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(auparse_interpret_field(mockedAudit)).SetReturn(expectedValue).SetFailReturn(NULL);
    
    umock_c_negative_tests_snapshot();

    for (int i = 0; i < umock_c_negative_tests_call_count(); i++) {
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        const char* output = 0;
        AuditSearchResultValues result = AuditSearchUtils_InterpretString(&search, fieldName, &output);
        ASSERT_ARE_NOT_EQUAL(int, AUDIT_SEARCH_OK, result);
    }
}

END_TEST_SUITE(audit_search_utils_ut)
