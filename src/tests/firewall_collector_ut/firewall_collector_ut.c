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
#include "os_utils/linux/iptables/iptables_iterator.h"
#include "os_utils/linux/iptables/iptables_rules_iterator.h"
#include "os_utils/process_info_handler.h"
#include "synchronized_queue.h"
#undef ENABLE_MOCKS

#include "collectors/firewall_collector.h"
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

JsonWriterResult Mocked_JsonObjectWriter_Init(JsonObjectWriterHandle* writer) {
    *writer = (JsonObjectWriterHandle)0x1;
    return JSON_WRITER_OK;
}

JsonWriterResult Mocked_JsonArrayWriter_Init(JsonArrayWriterHandle* writer) {
    *writer = (JsonArrayWriterHandle)0x2;
    return JSON_WRITER_OK;
}

IptablesResults Mocked_IptablesIterator_Init(IptablesIteratorHandle* iterator) {
    *iterator = (IptablesIteratorHandle)0x3;
    return IPTABLES_OK;
}

IptablesResults Mocked_IptablesIterator_GetRulesIterator(IptablesIteratorHandle iterator, IptablesRulesIteratorHandle* rulesIterator) {
    *rulesIterator = (IptablesRulesIteratorHandle)0x4;
    return IPTABLES_OK;
}

static const char CHAIN_NAME[] = "INPUT";
IptablesResults Mocked_IptablesIterator_GetChainName(IptablesIteratorHandle iterator, const char** chainName) {
    *chainName = CHAIN_NAME;
    return IPTABLES_OK;
}

IptablesResults Mocked_IptablesIterator_GetPolicyAction(IptablesIteratorHandle iterator, IptablesActionType* actionType) {
    *actionType = IPTABLES_ACTION_ALLOW;
    return IPTABLES_OK;
}


IptablesResults Mocked_IptablesRulesIterator_GetChainName(IptablesRulesIteratorHandle iterator, const char** chainName) {
    *chainName = CHAIN_NAME;
    return IPTABLES_OK;
}

static const char SOURCE_IP[] = "1.2.3.4";
IptablesResults Mocked_IptablesRulesIterator_GetIp(IptablesRulesIteratorHandle iterator, char* buffer, uint32_t bufferSize) {
    memcpy(buffer, SOURCE_IP, strlen(SOURCE_IP));
    return IPTABLES_OK;
}

static const char DEST_PORTS[] = "5-7";
IptablesResults Mocked_IptablesRulesIterator_GetPort(IptablesRulesIteratorHandle iterator, char* buffer, uint32_t bufferSize) {
    memcpy(buffer, DEST_PORTS, strlen(DEST_PORTS));
    return IPTABLES_OK;
}

IptablesResults Mocked_IptablesRulesIterator_GetAction(IptablesRulesIteratorHandle iterator, IptablesActionType* actionType, char* buffer, uint32_t bufferSize) {
    *actionType = IPTABLES_ACTION_ALLOW;
}

BEGIN_TEST_SUITE(firewall_collector_ut)

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
    REGISTER_UMOCK_ALIAS_TYPE(int32_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonArrayWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(EventCollectorResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(QueueResultValues, int);
    REGISTER_UMOCK_ALIAS_TYPE(IptablesResults, int);
    REGISTER_UMOCK_ALIAS_TYPE(IptablesIteratorHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IptablesRulesIteratorHandle, void*);

    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_Init, Mocked_JsonObjectWriter_Init);
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_Init, Mocked_JsonArrayWriter_Init);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesIterator_Init, Mocked_IptablesIterator_Init);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesIterator_GetChainName, Mocked_IptablesIterator_GetChainName);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesIterator_GetPolicyAction, Mocked_IptablesIterator_GetPolicyAction);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesRulesIterator_GetChainName, Mocked_IptablesRulesIterator_GetChainName);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesIterator_GetRulesIterator, Mocked_IptablesIterator_GetRulesIterator);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesRulesIterator_GetSrcIp, Mocked_IptablesRulesIterator_GetIp);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesRulesIterator_GetDestPort, Mocked_IptablesRulesIterator_GetPort);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(IptablesRulesIterator_GetDestPort, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesRulesIterator_GetSrcIp, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesIterator_GetRulesIterator, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesIterator_GetChainName, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesIterator_GetPolicyAction, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesRulesIterator_GetChainName, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesRulesIterator_Init, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(IptablesIterator_Init, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_Init, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_Init, NULL);
    
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(FirewallCollector_GetEvents_ExpectSuccess)
{
    SyncQueue mockedQueue;
        
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadata(IGNORED_PTR_ARG, EVENT_PERIODIC_CATEGORY, FIREWALL_RULES_NAME, EVENT_TYPE_SECURITY_VALUE, FIREWALL_RULES_PAYLOAD_SCHEMA_VERSION)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG));
    
    // iterate chains
    STRICT_EXPECTED_CALL(ProcessInfoHandler_ChangeToRoot(IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(IptablesIterator_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IptablesIterator_GetNext(IGNORED_PTR_ARG)).SetReturn(IPTABLES_ITERATOR_HAS_NEXT);

    // iterate rules
    STRICT_EXPECTED_CALL(IptablesIterator_GetRulesIterator(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetNext(IGNORED_PTR_ARG)).SetReturn(IPTABLES_ITERATOR_HAS_NEXT);

    // write rule
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteBool(IGNORED_PTR_ARG, FIREWALL_RULES_ENABLED_KEY, true)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, FIREWALL_RULES_PRIORITY_KEY, 0)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetChainName(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_CHAIN_NAME_KEY, CHAIN_NAME)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_DIRECTION_KEY, "In")).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetSrcIp(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_SRC_ADDRESS_KEY, SOURCE_IP)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetSrcPort(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_NO_DATA);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetDestIp(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_NO_DATA);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetDestPort(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_DEST_PORT_KEY, DEST_PORTS)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetProtocol(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(IPTABLES_NO_DATA);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetAction(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_ACTION_KEY, "Allow")).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetNext(IGNORED_PTR_ARG)).SetReturn(IPTABLES_ITERATOR_NO_MORE_ITEMS);

    //Write policy rule
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteBool(IGNORED_PTR_ARG, FIREWALL_RULES_ENABLED_KEY, true)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, FIREWALL_RULES_PRIORITY_KEY, 1)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(IptablesIterator_GetChainName(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_CHAIN_NAME_KEY, CHAIN_NAME)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_DIRECTION_KEY, "In")).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(IptablesIterator_GetPolicyAction(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_ACTION_KEY, "Allow")).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(IptablesRulesIterator_Deinit(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(IptablesIterator_GetNext(IGNORED_PTR_ARG)).SetReturn(IPTABLES_ITERATOR_NO_MORE_ITEMS);
    STRICT_EXPECTED_CALL(IptablesIterator_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ProcessInfoHandler_Reset(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(&mockedQueue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(QUEUE_OK);

    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    EventCollectorResult result = FirewallCollector_GetEvents(&mockedQueue);
    //ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(FirewallCollector_GetEvents_NoIptables_ExpectSuccess)
{
    SyncQueue mockedQueue;
        
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadata(IGNORED_PTR_ARG, EVENT_PERIODIC_CATEGORY, FIREWALL_RULES_NAME, EVENT_TYPE_SECURITY_VALUE, FIREWALL_RULES_PAYLOAD_SCHEMA_VERSION)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG));
    
    // iterate chains
    STRICT_EXPECTED_CALL(ProcessInfoHandler_ChangeToRoot(IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(IptablesIterator_Init(IGNORED_PTR_ARG)).SetReturn(IPTABLES_NO_DATA);
    STRICT_EXPECTED_CALL(ProcessInfoHandler_Reset(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(&mockedQueue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(QUEUE_OK);

    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    EventCollectorResult result = FirewallCollector_GetEvents(&mockedQueue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}


TEST_FUNCTION(FirewallCollector_GetEvents_ExpectFailure)
{
    SyncQueue mockedQueue;
    umock_c_negative_tests_init();

    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadata(IGNORED_PTR_ARG, EVENT_PERIODIC_CATEGORY, FIREWALL_RULES_NAME, EVENT_TYPE_SECURITY_VALUE, FIREWALL_RULES_PAYLOAD_SCHEMA_VERSION)).SetFailReturn(!EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    
    // iterate chains
    STRICT_EXPECTED_CALL(ProcessInfoHandler_ChangeToRoot(IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(IptablesIterator_Init(IGNORED_PTR_ARG)).SetFailReturn(IPTABLES_EXCEPTION);
    // no fail return
    STRICT_EXPECTED_CALL(IptablesIterator_GetNext(IGNORED_PTR_ARG)).SetReturn(IPTABLES_ITERATOR_HAS_NEXT);

    // iterate rules
    STRICT_EXPECTED_CALL(IptablesIterator_GetRulesIterator(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!IPTABLES_OK);
    // no fail return
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetNext(IGNORED_PTR_ARG)).SetReturn(IPTABLES_ITERATOR_HAS_NEXT);

    // write rule
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteBool(IGNORED_PTR_ARG, FIREWALL_RULES_ENABLED_KEY, true)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetSrcIp(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(IPTABLES_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_SRC_ADDRESS_KEY, SOURCE_IP)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetSrcPort(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(IPTABLES_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_SRC_PORT_KEY, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetDestIp(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(IPTABLES_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_DEST_ADDRESS_KEY, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetDestPort(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(IPTABLES_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_DEST_PORT_KEY, DEST_PORTS)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetProtocol(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(IPTABLES_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_PROTOCOL_KEY, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetAction(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(IPTABLES_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, FIREWALL_RULES_ACTION_KEY, "Allow")).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    // no fail return
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));
    // no fail return
    STRICT_EXPECTED_CALL(IptablesRulesIterator_GetNext(IGNORED_PTR_ARG)).SetReturn(IPTABLES_ITERATOR_NO_MORE_ITEMS);
    // no fail return
    STRICT_EXPECTED_CALL(IptablesRulesIterator_Deinit(IGNORED_PTR_ARG));
    // no fail return
    STRICT_EXPECTED_CALL(ProcessInfoHandler_Reset(IGNORED_PTR_ARG));

    // no fail return
    STRICT_EXPECTED_CALL(IptablesIterator_GetNext(IGNORED_PTR_ARG)).SetReturn(IPTABLES_ITERATOR_NO_MORE_ITEMS);
    // no fail return
    STRICT_EXPECTED_CALL(IptablesIterator_Deinit(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(EVENT_COLLECTOR_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(&mockedQueue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(!QUEUE_OK);

    // no fail return
    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(IGNORED_PTR_ARG));
    // no fail return
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    umock_c_negative_tests_snapshot();

    for (int i = 0; i < umock_c_negative_tests_call_count(); i++) {
        if (i == 3 || i == 5 || i == 7 || i == 23 || i == 24 || i == 25 || i == 26 || i == 27 || i == 28 || i == 32 || i == 33) {
            // no fail return
            continue;
        }
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        EventCollectorResult result = FirewallCollector_GetEvents(&mockedQueue);
        ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_EXCEPTION, result);
    }

    umock_c_negative_tests_deinit();

}

END_TEST_SUITE(firewall_collector_ut)
