// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AUDIT_SEARCH_H
#define AUDIT_SEARCH_H

#include <auparse.h>
#include <stdbool.h>
#include <stdint.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "os_utils/linux/audit/audit_search_utils.h"

/**
 * 
 * This is a generic iterator for searcing items in the audit logs using ausearch.
 * 
 * A sample for this api:
 * 
 * void search() {
 *      AuditSearch search;
 *      if (AuditSearch_Init(&search, "type", "EXECVE", "/var/tmp/checkpointFile") != AUDIT_SEARCH_OK) {
 *          printf("Error!\n");
 *          return;
 *      }
 *   
 *      AuditSearchResultValues hasNextResult = AuditSearch_GetNext(&search);
 *      
 *      while (hasNextResult == AUDIT_SEARCH_HAS_MORE_DATA) {
 *   
 *          int value;
 *          AuditSearch_ReadInt(&search, "uid", &value);
 *          printf("   uid: %d\n", value);
 *          AuditSearch_ReadInt(&search, "pid", &value);
 *          printf("   pid: %d\n", value);
 *          AuditSearch_ReadInt(&search, "ppid", &value);
 *          printf("   ppid: %d\n", value);
 *          const char* strValue;
 *          AuditSearch_ReadString(&search, "cwd", &strValue);
 *          printf("   cwd: %s\n", strValue);
 *          AuditSearch_InterpretString(&search, "proctitle", &strValue);
 *          printf("   proctitle: %s\n", strValue);
 *          AuditSearch_InterpretString(&search, "exe", &strValue);
 *          printf("   exe: %s\n", strValue);
 *          printf("-------------------------\n");
 *          
 *          hasNextResult = AuditSearch_GetNext(&search);
 *      }
 *   
 *      if (hasNextResult != AUDIT_SEARCH_NO_MORE_DATA) {
 *          printf("Error!\n");
 *      }
 *   
 *      if (AuditSearch_SetCheckpoint(&search) != AUDIT_SEARCH_OK) {
 *          printf("Error!\n");
 *      }
 *      AuditSearch_Deinit(&search);
 * }
 * 
 */

/**
 * @brief Initiates a new instance of audit search
 * 
 * @param   auditSearch     The audit instance we want to initiate.
 * @param   searchCriteria  The search criteria to aply.
 * @param   messageType     The type of the message we want to seacrh.
 * @param   checkpointFile  The path to a files which contains the last checkpoint of the search.
 * 
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearch_Init, AuditSearch*, auditSearch, AuditSearchCriteria, searchCriteria, const char*, messageType, const char*, checkpointFile);

/**
 * @brief Initiates a new instance of audit search with multiple search types.
 * 
 * @param   auditSearch         The audit instance we want to initiate.
 * @param   searchCriteria      The search criteria to aply.
 * #param   messageTypes        The array of types of the messages we want to search.
 * @param   messageTypesCount   Number of elements in the messageTypes array.
 * @param   checkpointFile      The path to a files which contains the last checkpoint of the search.
 * 
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearch_InitMultipleSearchCriteria, AuditSearch*, auditSearch, AuditSearchCriteria, searchCriteria, const char**, messageTypes, uint32_t, messageTypesCount, const char*, checkpointFile);

/**
 * @brief Deinitiate the given audit search instance.
 * 
 * @param   auditSearch     The audit instance we want to deinitiate.
 */
MOCKABLE_FUNCTION(, void, AuditSearch_Deinit, AuditSearch*, auditSearch);

/**
 * @brief Tries to progess the iterator to the next item.
 * 
 * @param   auditSearch     The search instance.
 * 
 * @return AUDIT_SEARCH_HAS_MORE_DATA in case there is anoteher item, AUDIT_SEARCH_NO_MORE_DATA in case the search has finished or appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearch_GetNext, AuditSearch*, auditSearch);

/**
 * @brief Reads the time of the current event.
 * 
 * @param auditSearch       The search instance.
 * @param timeInSeconds     Out param. The time (in seconds) of the event.
 * 
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearch_GetEventTime, AuditSearch*, auditSearch, uint32_t*, timeInSeconds);

/**
 * @brief Reads the first occurrence of the given field from the current search record as integer.
 * 
 * @param auditSearch   The search instance.
 * @param fieldName     The name of the field to read.
 * @param output        Out param. The value of the field.
 * 
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearch_ReadInt, AuditSearch*, auditSearch, const char*, fieldName, int*, output);

/**
 * @brief Reads the first occurrence of the given field from the current search record as string.
 * 
 * @param auditSearch   The search instance.
 * @param fieldName     The name of the field to read.
 * @param output        Out param. The value of the field.
 * 
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearch_ReadString, AuditSearch*, auditSearch, const char*, fieldName, const char**, output);

/**
 * @brief Reads the first occurrence of the given field from the current search record as audit interpert string.
 * 
 * @param auditSearch   The search instance.
 * @param fieldName     The name of the field to read.
 * @param output        Out param. The value of the field.
 * 
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearch_InterpretString, AuditSearch*, auditSearch, const char*, fieldName, const char**, output);

/**
 * @brief Sets the checkoint to the search time.
 * 
 * @param   auditSearch     The search instance.
 *
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearch_SetCheckpoint, AuditSearch*, auditSearch);

/**
 * @brief log all re records of the current event.
 * 
 * @param   auditSearch     The search instance.
 *
 * @return AUDIT_SEARCH_OK on success or an appropriate error. 
 */
MOCKABLE_FUNCTION(, AuditSearchResultValues, AuditSearch_LogEventText, AuditSearch*, auditSearch);

#endif //AUDIT_SEARCH_H
