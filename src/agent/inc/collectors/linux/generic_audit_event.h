// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef GENERIC_AUDIT_EVENT_H
#define GENERIC_AUDIT_EVENT_H

#include <stdbool.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "collectors/generic_event.h"
#include "json/json_object_writer.h"
#include "os_utils/linux/audit/audit_search.h"

/**
 * @brief Reads the string field from the audit search and writes it to the json writer.
 * 
 * @param   eventWriter             A handle to the writer of the event object.
 * @param   auditSearch             The audit search instance.
 * @param   auditField              The name of the field to read from the audo search.
 * @param   jsonKey                 The value of the json key to write to the writer.
 * @param   isOptional              A flag which indicates whether this fiels is optional.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, GenericAuditEvent_HandleStringValue, JsonObjectWriterHandle, eventWriter, AuditSearch*, auditSearch, const char*, auditField, const char*, jsonKey, bool, isOptional);

/**
 * @brief Reads the interpret string field from the audit search and writes it to the json writer.
 * 
 * @param   eventWriter             A handle to the writer of the event object.
 * @param   auditSearch             The audit search instance.
 * @param   auditField              The name of the field to read from the audo search.
 * @param   jsonKey                 The value of the json key to write to the writer.
 * @param   isOptional              A flag which indicates whether this fiels is optional.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, GenericAuditEvent_HandleInterpretStringValue, JsonObjectWriterHandle, eventWriter, AuditSearch*, auditSearch, const char*, auditField, const char*, jsonKey, bool, isOptional);


/**
 * @brief Reads the interger field from the audit search and writes it to the json writer.
 * 
 * @param   eventWriter             A handle to the writer of the event object.
 * @param   auditSearch             The audit search instance.
 * @param   auditField              The name of the field to read from the audo search.
 * @param   jsonKey                 The value of the json key to write to the writer.
 * @param   isOptional              A flag which indicates whether this fiels is optional.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, GenericAuditEvent_HandleIntValue, JsonObjectWriterHandle, eventWriter, AuditSearch*, auditSearch, const char*, auditField, const char*, jsonKey, bool, isOptional);

#endif //GENERIC_AUDIT_EVENT_H