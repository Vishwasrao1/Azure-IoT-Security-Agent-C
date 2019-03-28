// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "audit_mocks.h"
#include "os_utils/linux/audit/audit_search_utils.h"
#undef ENABLE_MOCKS

#include "os_utils/linux/audit/audit_search_record.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static auparse_state_t* mockedAudit;

BEGIN_TEST_SUITE(audit_search_record_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

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
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
    mockedAudit = (auparse_state_t*)0x1;
}

TEST_FUNCTION(AuditSearchRecord_Goto_ExpectSuccess)
{
    AuditSearch search;
    search.audit = mockedAudit;

    STRICT_EXPECTED_CALL(auparse_get_num_records(mockedAudit)).SetReturn(2);
    STRICT_EXPECTED_CALL(auparse_goto_record_num(mockedAudit, 0)).SetReturn(1);
    STRICT_EXPECTED_CALL(auparse_get_type(mockedAudit)).SetReturn(10);
    STRICT_EXPECTED_CALL(auparse_goto_record_num(mockedAudit, 1)).SetReturn(1);
    STRICT_EXPECTED_CALL(auparse_get_type(mockedAudit)).SetReturn(11);

    AuditSearchResultValues result = AuditSearchRecord_Goto(&search, 11);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuditSearchRecord_Goto_RecordNotFound_ExpectSuccess)
{
    AuditSearch search;
    search.audit = mockedAudit;

    STRICT_EXPECTED_CALL(auparse_get_num_records(mockedAudit)).SetReturn(2);
    STRICT_EXPECTED_CALL(auparse_goto_record_num(mockedAudit, 0)).SetReturn(1);
    STRICT_EXPECTED_CALL(auparse_get_type(mockedAudit)).SetReturn(10);
    STRICT_EXPECTED_CALL(auparse_goto_record_num(mockedAudit, 1)).SetReturn(1);
    STRICT_EXPECTED_CALL(auparse_get_type(mockedAudit)).SetReturn(11);

    AuditSearchResultValues result = AuditSearchRecord_Goto(&search, 12);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_RECORD_DOES_NOT_EXIST, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuditSearchRecord_Goto_GetRecordNumFailed_ExpectFailure)
{
    AuditSearch search;
    search.audit = mockedAudit;

    STRICT_EXPECTED_CALL(auparse_get_num_records(mockedAudit)).SetReturn(0);

    AuditSearchResultValues result = AuditSearchRecord_Goto(&search, 12);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuditSearchRecord_Goto_GetTypeFailed_ExpectFailure)
{
    AuditSearch search;
    search.audit = mockedAudit;

    STRICT_EXPECTED_CALL(auparse_get_num_records(mockedAudit)).SetReturn(2);
    STRICT_EXPECTED_CALL(auparse_goto_record_num(mockedAudit, 0)).SetReturn(1);
    STRICT_EXPECTED_CALL(auparse_get_type(mockedAudit)).SetReturn(0);

    AuditSearchResultValues result = AuditSearchRecord_Goto(&search, 12);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuditSearchRecord_Goto_GotoRecordNumFailed_ExpectFailure)
{
    AuditSearch search;
    search.audit = mockedAudit;

    STRICT_EXPECTED_CALL(auparse_get_num_records(mockedAudit)).SetReturn(2);
    STRICT_EXPECTED_CALL(auparse_goto_record_num(mockedAudit, 0)).SetReturn(0);

    AuditSearchResultValues result = AuditSearchRecord_Goto(&search, 12);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}


TEST_FUNCTION(AuditSearchRecord_ReadInt_ExpectSuccess)
{
    AuditSearch search;
    const char* fieldName = "djdjd";
    int output = 0;

    STRICT_EXPECTED_CALL(AuditSearchUtils_ReadInt(&search, fieldName, &output)).SetReturn(AUDIT_SEARCH_OK);

    AuditSearchResultValues result = AuditSearchRecord_ReadInt(&search, fieldName, &output);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuditSearchRecord_ReadInt_Failed_ExpectFailure)
{
    AuditSearch search;
    const char* fieldName = "djdjd";
    int output = 0;

    STRICT_EXPECTED_CALL(AuditSearchUtils_ReadInt(&search, fieldName, &output)).SetReturn(AUDIT_SEARCH_EXCEPTION);

    AuditSearchResultValues result = AuditSearchRecord_ReadInt(&search, fieldName, &output);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuditSearchRecord_InterpretString_ExpectSuccess)
{
    AuditSearch search;
    const char* fieldName = "djdjd";
    const char* output = 0;

    STRICT_EXPECTED_CALL(AuditSearchUtils_InterpretString(&search, fieldName, &output)).SetReturn(AUDIT_SEARCH_OK);

    AuditSearchResultValues result = AuditSearchRecord_InterpretString(&search, fieldName, &output);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuditSearchRecord_InterpretString_Fail_ExpectFailue)
{
    AuditSearch search;
    const char* fieldName = "djdjd";
    const char* output = 0;

    STRICT_EXPECTED_CALL(AuditSearchUtils_InterpretString(&search, fieldName, &output)).SetReturn(AUDIT_SEARCH_FIELD_DOES_NOT_EXIST);

    AuditSearchResultValues result = AuditSearchRecord_InterpretString(&search, fieldName, &output);

    ASSERT_ARE_EQUAL(int, AUDIT_SEARCH_FIELD_DOES_NOT_EXIST, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(audit_search_record_ut)
