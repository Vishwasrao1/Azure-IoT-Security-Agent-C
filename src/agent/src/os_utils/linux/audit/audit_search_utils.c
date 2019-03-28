// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/linux/audit/audit_search_utils.h"

#include <errno.h>

/**
 * @brief Search the given field in the current search record.
 * 
 * @param   auditSearch     The search instance.
 * @param   fieldName       The name of the field to search.
 * 
 * @return AUDIT_SEARCH_OK in case the field exists, AUDIT_SEARCH_FIELD_DOES_NOT_EXIST in case the feld does not exists and AUDIT_SEARCH_EXCEPTION otherwise.
 */
AuditSearchResultValues AuditSearchUtils_FindField(AuditSearch* auditSearch, const char* fieldName);

AuditSearchResultValues AuditSearchUtils_ReadInt(AuditSearch* auditSearch, const char* fieldName, int* output) {
    AuditSearchResultValues searchResult = AuditSearchUtils_FindField(auditSearch, fieldName); 
    if (searchResult != AUDIT_SEARCH_OK) {
        return searchResult;
    }

    *output = auparse_get_field_int(auditSearch->audit);
    if (*output == -1 && errno != 0) {
        return AUDIT_SEARCH_EXCEPTION;
    }
    
    return AUDIT_SEARCH_OK;
}

AuditSearchResultValues AuditSearchUtils_ReadString(AuditSearch* auditSearch, const char* fieldName, const char** output) {
    AuditSearchResultValues searchResult = AuditSearchUtils_FindField(auditSearch, fieldName); 
    if (searchResult != AUDIT_SEARCH_OK) {
        return searchResult;
    }

    *output = auparse_get_field_str(auditSearch->audit);
    if (*output == NULL) {
        return AUDIT_SEARCH_EXCEPTION;
    }
    
    return AUDIT_SEARCH_OK;
}

AuditSearchResultValues AuditSearchUtils_InterpretString(AuditSearch* auditSearch, const char* fieldName, const char** output) {
    AuditSearchResultValues searchResult = AuditSearchUtils_FindField(auditSearch, fieldName); 
    if (searchResult != AUDIT_SEARCH_OK) {
        return searchResult;
    }

    *output = auparse_interpret_field(auditSearch->audit);
    if (*output == NULL) {
        return AUDIT_SEARCH_EXCEPTION;
    }
    
    return AUDIT_SEARCH_OK;
}

AuditSearchResultValues AuditSearchUtils_FindField(AuditSearch* auditSearch, const char* fieldName) {
    errno = 0;
    if (auparse_find_field(auditSearch->audit, fieldName) == NULL) {
        if (errno != 0) {
            return AUDIT_SEARCH_EXCEPTION;
        }
        return AUDIT_SEARCH_FIELD_DOES_NOT_EXIST;
    }

    return AUDIT_SEARCH_OK;
}
