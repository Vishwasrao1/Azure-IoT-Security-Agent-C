// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/linux/audit/audit_search_record.h"

#include <stdbool.h>

AuditSearchResultValues AuditSearchRecord_Goto(AuditSearch* auditSearch, int wantedType) {
    uint32_t numOfRecords = auparse_get_num_records(auditSearch->audit);
    if (numOfRecords == 0) {
        return AUDIT_SEARCH_EXCEPTION;
    }

    uint32_t index = 0;
    bool found = false;
    while (!found && index < numOfRecords) {
        if (auparse_goto_record_num(auditSearch->audit, index) == 0) {
            return AUDIT_SEARCH_EXCEPTION;
        }

        int currentType = auparse_get_type(auditSearch->audit);
        if (currentType == 0) {
            return AUDIT_SEARCH_EXCEPTION;
        }
        if (currentType == wantedType) {
            return AUDIT_SEARCH_OK;
        }
        ++index;
    }

    return AUDIT_SEARCH_RECORD_DOES_NOT_EXIST;
}

AuditSearchResultValues AuditSearchRecord_MaxRecordLength(AuditSearch* auditSearch, uint32_t* length) {
    const char* recordAsString = auparse_get_record_text(auditSearch->audit);
    if (recordAsString == NULL) {
        return AUDIT_SEARCH_EXCEPTION;
    }
    *length = strlen(recordAsString);
    return AUDIT_SEARCH_OK;
}

AuditSearchResultValues AuditSearchRecord_ReadInt(AuditSearch* auditSearch, const char* fieldName, int* output) {
    return AuditSearchUtils_ReadInt(auditSearch, fieldName, output);
}

AuditSearchResultValues AuditSearchRecord_InterpretString(AuditSearch* auditSearch, const char* fieldName, const char** output) {
    return AuditSearchUtils_InterpretString(auditSearch, fieldName, output);
}

