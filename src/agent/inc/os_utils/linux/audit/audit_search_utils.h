// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AUDIT_SEARCH_UTILS_H
#define AUDIT_SEARCH_UTILS_H

#include <auparse.h>
#include <stdbool.h>
#include <stdint.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "os_utils/process_info_handler.h"

typedef struct _AuditSearch {

    auparse_state_t* audit;
    char* checkpointFile;
    time_t searchTime;
    bool firstSearch;
    ProcessInfo processInfo;

} AuditSearch;

typedef enum _AuditSearchResultValues {

    AUDIT_SEARCH_OK,
    AUDIT_SEARCH_HAS_MORE_DATA,
    AUDIT_SEARCH_NO_MORE_DATA,
    AUDIT_SEARCH_NO_DATA,
    AUDIT_SEARCH_FIELD_DOES_NOT_EXIST,
    AUDIT_SEARCH_RECORD_DOES_NOT_EXIST,
    AUDIT_SEARCH_EXCEPTION
    
} AuditSearchResultValues;

typedef enum _AuditSearchCriteria {
    AUDIT_SEARCH_CRITERIA_TYPE,
    AUDIT_SEARCH_CRITERIA_SYSCALL
} AuditSearchCriteria;

/**
 * @brief Reads the givn field from the current search record as integer.
 * 
 * @param auditSearch   The search instance.
 * @param fieldName     The name of the field to read.
 * @param output        Out param. The value of the field.
 * 
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearchUtils_ReadInt, AuditSearch*, auditSearch, const char*, fieldName, int*, output);

/**
 * @brief Reads the givn field from the current search record as string.
 * 
 * @param auditSearch   The search instance.
 * @param fieldName     The name of the field to read.
 * @param output        Out param. The value of the field.
 * 
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearchUtils_ReadString, AuditSearch*, auditSearch, const char*, fieldName, const char**, output);

/**
 * @brief Reads the givn field from the current search record as audit interpert string.
 * 
 * @param auditSearch   The search instance.
 * @param fieldName     The name of the field to read.
 * @param output        Out param. The value of the field.
 * 
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearchUtils_InterpretString, AuditSearch*, auditSearch, const char*, fieldName, const char**, output);

#endif //AUDIT_SEARCH_UTILS_H
