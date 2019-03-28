// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "collectors/user_login_collector.h"

#include <stdlib.h>
#include <string.h>

#include "os_utils/linux/audit/audit_search.h"
#include "collectors/linux/generic_audit_event.h"
#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#include "logger.h"
#include "message_schema_consts.h"
#include "utils.h"

static const char* AUDIT_USER_LOGIN_TYPES[] = {"USER_LOGIN", "USER_AUTH"};
static uint32_t AUDIT_USER_LOGIN_TYPES_COUNT = sizeof(AUDIT_USER_LOGIN_TYPES) / sizeof(AUDIT_USER_LOGIN_TYPES[0]);
static const char AUDIT_USER_LOGIN_CHECKPOINT_FILE[] = "/var/tmp/userLoginCheckpoint";
static const char AUDIT_USER_LOGIN_EXECUTEABLE[] = "exe";
static const char AUDIT_USER_LOGIN_PROCESS_ID[] = "pid";
static const char AUDIT_USER_LOGIN_USER_ID[] = "id";
static const char AUDIT_USER_LOGIN_USER_NAME[] = "acct";
static const char AUDIT_USER_LOGIN_RESULT[] = "res";
static const char AUDIT_USER_LOGIN_RESULT_SUCCESS[] = "success";
static const char AUDIT_USER_LOGIN_RESULT_FAILED[] = "failed";
static const char AUDIT_USER_LOGIN_REMOTE_ADDRESS[] = "addr";
static const char AUDIT_USER_LOGIN_NOT_A_REAL_REMOTE_ADDRESS[] = "?";
static const char AUDIT_USER_LOGIN_OPERATION[] = "op";

/**
 * @brief Generates the payload for the login event.
 * 
 * @param   auditSearch               The search instacne.
 * @param   userLoginEventPayload     The event writer of the current login event.
 * 
 * @return EVENT_COLLECTOR_OK on success or the coressponind error on failure.
 */
EventCollectorResult UserLoginEvent_GeneratePayload(AuditSearch* auditSearch, JsonObjectWriterHandle userLoginEventPayload);

/**
 * @brief Generates a single login event and adds it to the queue.
 * 
 * @param   auditSearch     The search instacne.
 * @param   queue           The out queue of events.
 * 
 * @return EVENT_COLLECTOR_OK on success or the coressponind error on failure.
 */
EventCollectorResult UserLoginEvent_CreateSingleEvent(AuditSearch* auditSearch, SyncQueue* queue);

EventCollectorResult UserLoginCollector_GetEvents(SyncQueue* queue) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    bool auditSearchInitialize = false;
    AuditSearch auditSearch;
    uint32_t recordsWithError = 0;

    if (AuditSearch_InitMultipleSearchCriteria(&auditSearch, AUDIT_SEARCH_CRITERIA_TYPE, AUDIT_USER_LOGIN_TYPES, AUDIT_USER_LOGIN_TYPES_COUNT, AUDIT_USER_LOGIN_CHECKPOINT_FILE) != AUDIT_SEARCH_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    auditSearchInitialize = true;
 
    AuditSearchResultValues hasNextResult = AuditSearch_GetNext(&auditSearch);

    while (hasNextResult == AUDIT_SEARCH_HAS_MORE_DATA) {
        result = UserLoginEvent_CreateSingleEvent(&auditSearch, queue);
        if (result == EVENT_COLLECTOR_RECORD_HAS_ERRORS) {
            ++recordsWithError;
            result = EVENT_COLLECTOR_OK;
        } else if (result == EVENT_COLLECTOR_OUT_OF_MEM){
            result = EVENT_COLLECTOR_OK;
        } else if (result != EVENT_COLLECTOR_OK) {
            goto cleanup;
        }
        hasNextResult = AuditSearch_GetNext(&auditSearch);
    }

    if (hasNextResult != AUDIT_SEARCH_NO_MORE_DATA) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;    
    }
   
cleanup:

    if (auditSearchInitialize) {
        if (recordsWithError > 0) {
            Logger_Error("%d records had errors.", recordsWithError);
        }
        
        if (result != EVENT_COLLECTOR_OK) {
            Logger_Information("Setting up checkpoint even though The User Login run did not finish successfuly.");
        }
        
        if (AuditSearch_SetCheckpoint(&auditSearch) != AUDIT_SEARCH_OK) {
            result = EVENT_COLLECTOR_EXCEPTION;
        }

        AuditSearch_Deinit(&auditSearch);
    }

    return result;
}

EventCollectorResult UserLoginEvent_GeneratePayload(AuditSearch* auditSearch, JsonObjectWriterHandle userLoginEventPayload) {
    const char* auditStrValue = NULL;
    AuditSearchResultValues auditResult;
    EventCollectorResult result = EVENT_COLLECTOR_OK;

    result = GenericAuditEvent_HandleIntValue(userLoginEventPayload, auditSearch, AUDIT_USER_LOGIN_PROCESS_ID, USER_LOGIN_PROCESS_ID_KEY, false);
    if (result != EVENT_COLLECTOR_OK) {
        return result;
    }

    result = GenericAuditEvent_HandleIntValue(userLoginEventPayload, auditSearch, AUDIT_USER_LOGIN_USER_ID, USER_LOGIN_USER_ID_KEY, true);
    if (result != EVENT_COLLECTOR_OK) {
        return result;
    }

    result = GenericAuditEvent_HandleInterpretStringValue(userLoginEventPayload, auditSearch, AUDIT_USER_LOGIN_USER_NAME, USER_LOGIN_USERNAME_KEY, true);
    if (result != EVENT_COLLECTOR_OK) {
        return result;
    }

    result = GenericAuditEvent_HandleInterpretStringValue(userLoginEventPayload, auditSearch, AUDIT_USER_LOGIN_EXECUTEABLE, USER_LOGIN_EXECUTABLE_KEY, false);
    if (result != EVENT_COLLECTOR_OK) {
        return result;
    }

    auditResult = AuditSearch_ReadString(auditSearch, AUDIT_USER_LOGIN_REMOTE_ADDRESS, &auditStrValue);
    if (auditResult == AUDIT_SEARCH_OK) {
        if (!Utils_UnsafeAreStringsEqual(auditStrValue, AUDIT_USER_LOGIN_NOT_A_REAL_REMOTE_ADDRESS, false)) {
            if (JsonObjectWriter_WriteString(userLoginEventPayload, USER_LOGIN_REMOTE_ADDRESS_KEY, auditStrValue) != JSON_WRITER_OK) {
                return EVENT_COLLECTOR_EXCEPTION;
            }
        }
    } else if (auditResult != AUDIT_SEARCH_FIELD_DOES_NOT_EXIST) {
        return EVENT_COLLECTOR_RECORD_HAS_ERRORS;
    }

    auditResult = AuditSearch_ReadString(auditSearch, AUDIT_USER_LOGIN_RESULT, &auditStrValue);
    if (auditResult != AUDIT_SEARCH_OK) {
        return EVENT_COLLECTOR_RECORD_HAS_ERRORS;
    }
    
    if (Utils_UnsafeAreStringsEqual(auditStrValue, AUDIT_USER_LOGIN_RESULT_SUCCESS, false)) {
        if (JsonObjectWriter_WriteString(userLoginEventPayload, USER_LOGIN_RESULT_KEY, USER_LOGIN_RESULT_SUCCESS_VALUE) != JSON_WRITER_OK) {
            return EVENT_COLLECTOR_EXCEPTION;
        }
    } else if (Utils_UnsafeAreStringsEqual(auditStrValue, AUDIT_USER_LOGIN_RESULT_FAILED, false)) {
        if (JsonObjectWriter_WriteString(userLoginEventPayload, USER_LOGIN_RESULT_KEY, USER_LOGIN_RESULT_FAILED_VALUE) != JSON_WRITER_OK) {
            return EVENT_COLLECTOR_EXCEPTION;
        }
    } else {
        return EVENT_COLLECTOR_RECORD_HAS_ERRORS;
    }

    result = GenericAuditEvent_HandleStringValue(userLoginEventPayload, auditSearch, AUDIT_USER_LOGIN_OPERATION, USER_LOGIN_OPERATION_KEY, true);
    return result;
}

EventCollectorResult UserLoginEvent_CreateSingleEvent(AuditSearch* auditSearch, SyncQueue* queue) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;

    JsonObjectWriterHandle userLoginEvent = NULL;
    JsonObjectWriterHandle userLoginEventPayload = NULL;
    JsonArrayWriterHandle payloads = NULL;
    char* output = NULL;
    
    if (JsonObjectWriter_Init(&userLoginEvent) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    uint32_t eventTimeInSeconds = 0;
    if (AuditSearch_GetEventTime(auditSearch, &eventTimeInSeconds) != AUDIT_SEARCH_OK) {
        result = EVENT_COLLECTOR_RECORD_HAS_ERRORS;
        goto cleanup;
    }
    time_t eventTime = (time_t)eventTimeInSeconds;
    
    if (GenericEvent_AddMetadataWithTimes(userLoginEvent, EVENT_TRIGGERED_CATEGORY, USER_LOGIN_NAME, EVENT_TYPE_SECURITY_VALUE, USER_LOGIN_PAYLOAD_SCHEMA_VERSION, &eventTime) != EVENT_COLLECTOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_Init(&userLoginEventPayload) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = UserLoginEvent_GeneratePayload(auditSearch, userLoginEventPayload);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }
    
    if (JsonArrayWriter_Init(&payloads) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonArrayWriter_AddObject(payloads, userLoginEventPayload) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = GenericEvent_AddPayload(userLoginEvent, payloads); 
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }
    
    uint32_t outputSize = 0;
    if (JsonObjectWriter_Serialize(userLoginEvent, &output, &outputSize) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    
    QueueResultValues qResult = SyncQueue_PushBack(queue, output, outputSize);
    if (qResult == QUEUE_MAX_MEMORY_EXCEEDED) {
        result = EVENT_COLLECTOR_OUT_OF_MEM;
        goto cleanup;
    } else if (qResult != QUEUE_OK){
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (result != EVENT_COLLECTOR_OK) {
        if (output != NULL) {
            free(output);
        }
    }

    if (userLoginEventPayload != NULL) {
        JsonObjectWriter_Deinit(userLoginEventPayload);
    }

    if (userLoginEvent != NULL) {
        JsonObjectWriter_Deinit(userLoginEvent);
    }

    if (payloads != NULL) {
        JsonArrayWriter_Deinit(payloads);
    }

    return result;
}
