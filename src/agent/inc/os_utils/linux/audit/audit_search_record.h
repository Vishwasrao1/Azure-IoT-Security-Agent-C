// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AUDIT_SEARCH_RECORD_H
#define AUDIT_SEARCH_RECORD_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "os_utils/linux/audit/audit_search_utils.h"

/**
 * @brief Sets the audit search to point to a record with the given type on its current event.
 * 
 * @param   auditSearch     The search instance.
 * @param   wantedType      The type of the wanted record.
 * 
 * @return AUDIT_SEARCH_OK on success, AUDIT_SEARCH_RECORD_DOES_NOT_EXIST if the event does not contain a record
 *         with the wanted type or AUDIT_SEARCH_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearchRecord_Goto, AuditSearch*, auditSearch, int, wantedType); 

/**
 * @brief Returns the maximum length (in bytes) of the string representation of the current record.
 * 
 * @param   auditSearch     The search instance.
 * @param   length          Out param. The maximum length of the string representation of the record.
 * 
 * @return AUDIT_SEARCH_OK on success, AUDIT_SEARCH_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearchRecord_MaxRecordLength, AuditSearch*, auditSearch, uint32_t*, length);

/**
 * @brief Reads the givn field from the current search record as integer.
 * 
 * @param auditSearch   The search instance.
 * @param fieldName     The name of the field to read.
 * @param output        Out param. The value of the field.
 * 
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearchRecord_ReadInt, AuditSearch*, auditSearch, const char*, fieldName, int*, output);

/**
 * @brief Reads the givn field from the current search record as audit interpert string.
 * 
 * @param auditSearch   The search instance.
 * @param fieldName     The name of the field to read.
 * @param output        Out param. The value of the field.
 * 
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearchRecord_InterpretString, AuditSearch*, auditSearch, const char*, fieldName, const char**, output);

#endif //AUDIT_SEARCH_RECORD_H

