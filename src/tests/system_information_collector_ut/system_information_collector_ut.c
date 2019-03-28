// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"
#include "umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "collectors/generic_event.h"
#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#include "synchronized_queue.h"

#include "system_information_mocks.h"
#undef ENABLE_MOCKS

#include "collectors/system_information_collector.h"
#include "message_schema_consts.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static const char MOCKED_OS_NAME[] = "Linux";
static const char MOCKED_OS_RELEASE[] = "4.4.0-31-generic";
static const char MOCKED_OS_VERSION[] = "#50-Ubuntu SMP Wed Jul 13 00:07:12 UTC 2016";
static const char MOCKED_OS_FULL_VERSION[] = "4.4.0-31-generic #50-Ubuntu SMP Wed Jul 13 00:07:12 UTC 2016";
static const char MOCKED_HOST_NAME[] = "my-machine";
static const char MOCKED_HARDWARE[] = "x86_64";

static const int MOCKED_TOTAL_RAM = 15 * 64 * 1024;
static const int MOCKED_TOTAL_RAM_IN_KB = 15;
static const int MOCKED_FREE_RAM = 11 * 64 * 1024;
static const int MOCKED_FREE_RAM_IN_KB = 11;
static const int MOCKED_MEM_UNIT = 64;

int Mocked_uname(struct utsname* buf) {
    memcpy(buf->sysname, MOCKED_OS_NAME, sizeof(MOCKED_OS_NAME));
    memcpy(buf->nodename, MOCKED_HOST_NAME, sizeof(MOCKED_HOST_NAME));
    memcpy(buf->release, MOCKED_OS_RELEASE, sizeof(MOCKED_OS_RELEASE));
    memcpy(buf->version, MOCKED_OS_VERSION, sizeof(MOCKED_OS_VERSION));
    memcpy(buf->machine, MOCKED_HARDWARE, sizeof(MOCKED_HARDWARE));

    return 0;
}

int Mocked_sysinfo(struct sysinfo* info) {
    info->totalram = MOCKED_TOTAL_RAM;
    info->freeram = MOCKED_FREE_RAM;
    info->mem_unit = MOCKED_MEM_UNIT;

    return 0;
}

JsonWriterResult Mocked_JsonObjectWriter_Init(JsonObjectWriterHandle* writer) {
    *writer = (JsonObjectWriterHandle)0x1;
    return JSON_WRITER_OK;
}

JsonWriterResult Mocked_JsonArrayWriter_Init(JsonArrayWriterHandle* writer) {
    *writer = (JsonArrayWriterHandle)0x2;
    return JSON_WRITER_OK;
}

BEGIN_TEST_SUITE(system_information_collector_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonArrayWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(EventCollectorResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(QueueResultValues, int);

    REGISTER_GLOBAL_MOCK_HOOK(uname, Mocked_uname);
    REGISTER_GLOBAL_MOCK_HOOK(sysinfo, Mocked_sysinfo);
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_Init, Mocked_JsonObjectWriter_Init);
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_Init, Mocked_JsonArrayWriter_Init);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_Init, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_Init, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(uname, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(sysinfo, NULL);
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(SystemInformationCollector_GetEvents_ExpectSuccess)
{
    SyncQueue mockedQueue;
    
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadata(IGNORED_PTR_ARG, EVENT_PERIODIC_CATEGORY, SYSTEM_INFORMATION_NAME, EVENT_TYPE_SECURITY_VALUE, SYSTEM_INFORMATION_PAYLOAD_SCHEMA_VERSION)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    
    // os info
    STRICT_EXPECTED_CALL(uname(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, SYSTEM_INFORMATION_OS_NAME_KEY, MOCKED_OS_NAME)).SetReturn(JSON_WRITER_OK);    
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, SYSTEM_INFORMATION_OS_VERSION_KEY, MOCKED_OS_FULL_VERSION)).SetReturn(JSON_WRITER_OK);    
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, SYSTEM_INFORMATION_OS_ARCHITECTURE_KEY, MOCKED_HARDWARE)).SetReturn(JSON_WRITER_OK);    
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, SYSTEM_INFORMATION_HOST_NAME_KEY, MOCKED_HOST_NAME)).SetReturn(JSON_WRITER_OK);    
    
    // mem info
    STRICT_EXPECTED_CALL(sysinfo(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, SYSTEM_INFORMATION_TOTAL_PHYSICAL_MEMORY_KEY, MOCKED_TOTAL_RAM_IN_KB)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, SYSTEM_INFORMATION_FREE_PHYSICAL_MEMORY_KEY, MOCKED_FREE_RAM_IN_KB)).SetReturn(JSON_WRITER_OK);
    
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(&mockedQueue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(QUEUE_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    EventCollectorResult result = SystemInformationCollector_GetEvents(&mockedQueue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SystemInformationCollector_GetEvents_ExpectFailure)
{
    umock_c_negative_tests_init();
    SyncQueue mockedQueue;
    
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadata(IGNORED_PTR_ARG, EVENT_PERIODIC_CATEGORY, SYSTEM_INFORMATION_NAME, EVENT_TYPE_SECURITY_VALUE, SYSTEM_INFORMATION_PAYLOAD_SCHEMA_VERSION)).SetFailReturn(!EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);

    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    
    // os info
    STRICT_EXPECTED_CALL(uname(IGNORED_PTR_ARG)).SetFailReturn(-1);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, SYSTEM_INFORMATION_OS_NAME_KEY, MOCKED_OS_NAME)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, SYSTEM_INFORMATION_OS_VERSION_KEY, MOCKED_OS_FULL_VERSION)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, SYSTEM_INFORMATION_OS_ARCHITECTURE_KEY, MOCKED_HARDWARE)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, SYSTEM_INFORMATION_HOST_NAME_KEY, MOCKED_HOST_NAME)).SetFailReturn(!JSON_WRITER_OK);
    
    // mem info
    STRICT_EXPECTED_CALL(sysinfo(IGNORED_PTR_ARG)).SetFailReturn(-1);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, SYSTEM_INFORMATION_TOTAL_PHYSICAL_MEMORY_KEY, MOCKED_TOTAL_RAM_IN_KB)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, SYSTEM_INFORMATION_FREE_PHYSICAL_MEMORY_KEY, MOCKED_FREE_RAM_IN_KB)).SetFailReturn(!JSON_WRITER_OK);
    
    
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    // dosen't have a fail returne
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(EVENT_COLLECTOR_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(&mockedQueue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(!QUEUE_OK);
    // dosen't have a fail returne
    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(IGNORED_PTR_ARG));
    // dosen't have a fail returne
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    umock_c_negative_tests_snapshot();

    for (int i = 0; i < umock_c_negative_tests_call_count(); i++) {
        if (i == 13 || i == 17 || i == 18) {
            // skip deinit since they don't have a fail return
            continue;
        }
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        EventCollectorResult result = SystemInformationCollector_GetEvents(&mockedQueue);
        ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_EXCEPTION, result);
    }

    umock_c_negative_tests_deinit();

}

END_TEST_SUITE(system_information_collector_ut)
