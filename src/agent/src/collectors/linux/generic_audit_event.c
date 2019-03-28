// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "collectors/linux/generic_audit_event.h"

EventCollectorResult GenericAuditEvent_HandleIntValue(JsonObjectWriterHandle eventWriter, AuditSearch* auditSearch, const char* auditField, const char* jsonKey, bool isOptional) {
    int auditIntValue = 0;
    AuditSearchResultValues auditResult = AuditSearch_ReadInt(auditSearch, auditField, &auditIntValue);
    if (auditResult == AUDIT_SEARCH_OK) {
        if (JsonObjectWriter_WriteInt(eventWriter, jsonKey, auditIntValue) != JSON_WRITER_OK) {
            return EVENT_COLLECTOR_EXCEPTION;
        }
    } else if (isOptional && auditResult == AUDIT_SEARCH_FIELD_DOES_NOT_EXIST) {
        return EVENT_COLLECTOR_OK;
    } else {
        return EVENT_COLLECTOR_RECORD_HAS_ERRORS;
    }
    return EVENT_COLLECTOR_OK;
}

EventCollectorResult GenericAuditEvent_HandleStringValue(JsonObjectWriterHandle eventWriter, AuditSearch* auditSearch, const char* auditField, const char* jsonKey, bool isOptional) {
    const char* auditStrValue = NULL;
    AuditSearchResultValues auditResult = AuditSearch_ReadString(auditSearch, auditField, &auditStrValue);
    if (auditResult == AUDIT_SEARCH_OK) {
        if (JsonObjectWriter_WriteString(eventWriter, jsonKey, auditStrValue) != JSON_WRITER_OK) {
            return EVENT_COLLECTOR_EXCEPTION;
        }
    } else if (isOptional && auditResult == AUDIT_SEARCH_FIELD_DOES_NOT_EXIST) {
        return EVENT_COLLECTOR_OK;
    } else {
        return EVENT_COLLECTOR_RECORD_HAS_ERRORS;
    }
    return EVENT_COLLECTOR_OK;
}

EventCollectorResult GenericAuditEvent_HandleInterpretStringValue(JsonObjectWriterHandle eventWriter, AuditSearch* auditSearch, const char* auditField, const char* jsonKey, bool isOptional) {
    const char* auditStrValue = NULL;
    AuditSearchResultValues auditResult = AuditSearch_InterpretString(auditSearch, auditField, &auditStrValue);
    if (auditResult == AUDIT_SEARCH_OK) {
        if (JsonObjectWriter_WriteString(eventWriter, jsonKey, auditStrValue) != JSON_WRITER_OK) {
            return EVENT_COLLECTOR_EXCEPTION;
        }
    } else if (isOptional && auditResult == AUDIT_SEARCH_FIELD_DOES_NOT_EXIST) {
        return EVENT_COLLECTOR_OK;
    } else {
        return EVENT_COLLECTOR_RECORD_HAS_ERRORS;
    }
    return EVENT_COLLECTOR_OK;
}

