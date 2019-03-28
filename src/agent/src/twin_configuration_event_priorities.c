// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "twin_configuration_event_priorities.h"

#include <stdbool.h>

#include "azure_c_shared_utility/lock.h"

#include "utils.h"

static const char* PRIORITY_HIGH = "high";
static const char* PRIORITY_LOW = "low";
static const char* PRIORITY_OFF = "off";

static const TwinConfiguartionEventPriority PROCESS_CREATE_DEFAULT_PRIORITY = EVENT_PRIORITY_LOW;
static const TwinConfiguartionEventPriority LISTENING_PORTS_DEFAULT_PRIORITY = EVENT_PRIORITY_HIGH;
static const TwinConfiguartionEventPriority SYSTEM_INFORMATION_DEFAULT_PRIORITY = EVENT_PRIORITY_LOW;
static const TwinConfiguartionEventPriority LOCAL_USERS_DEFAULT_PRIORITY = EVENT_PRIORITY_HIGH;
static const TwinConfiguartionEventPriority LOGIN_DEFAULT_PRIORITY = EVENT_PRIORITY_HIGH;
static const TwinConfiguartionEventPriority CONNECTION_CREATE_DEFAULT_PRIORITY = EVENT_PRIORITY_LOW;
static const TwinConfiguartionEventPriority FIREWALL_CONFIGURATION_DEFAULT_PRIORITY = EVENT_PRIORITY_LOW;
static const TwinConfiguartionEventPriority BASELINE_DEFAULT_PRIORITY = EVENT_PRIORITY_LOW;
static const TwinConfiguartionEventPriority DIAGNOSTIC_DEFAULT_PRIORITY = EVENT_PRIORITY_LOW;
static const TwinConfiguartionEventPriority OPERATIONAL_EVENT_DEFAULT_PRIORITY = EVENT_PRIORITY_OPERATIONAL;

typedef struct _TwinConfigurationEventPriorities {
    TwinConfiguartionEventPriority processCreatePriority;
    TwinConfiguartionEventPriority listeningPortsPriority;
    TwinConfiguartionEventPriority systemInformationPriority;
    TwinConfiguartionEventPriority localUsersPriority;
    TwinConfiguartionEventPriority loginPriority;
    TwinConfiguartionEventPriority connectionCreatePriority;
    TwinConfiguartionEventPriority firewallConfigurationPriority;
    TwinConfiguartionEventPriority baselinePriority;
    TwinConfiguartionEventPriority diagnostic;
    TwinConfiguartionEventPriority operational;

    LOCK_HANDLE lock;
} TwinConfigurationEventPriorities;

static TwinConfigurationEventPriorities eventPriorities;

/**
 * @brief Set the priorities of all the events.
 * 
 * @param   propertiesReader    The json reader of the preperties.
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
static TwinConfigurationResult TwinConfigurationEventPriorities_SetValues(JsonObjectReaderHandle propertiesReader);

/**
 * @brief Set a single event's priority.
 * 
 * @param   propertiesReader    The json reader of the preperties.
 * @param   key                 The event priority key in the json.
 * @param   priorityField       The field of the event priority.
 * @param   defaultValue        The dafult priority for this event.
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
static TwinConfigurationResult TwinConfigurationEventPriorities_SetSingleValues(JsonObjectReaderHandle propertiesReader, const char* key, TwinConfiguartionEventPriority* priorityField, TwinConfiguartionEventPriority defaultValue);

/**
 * @brief returns the enum type representing this value.
 * 
 * @param   str         The str representation of the priority.
 * @param   priority    Out param. The enum representing the given priority.
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
static TwinConfigurationResult TwinConfigurationEventPriorities_PriorityAsEnum(const char* str, TwinConfiguartionEventPriority* priority);

/**
 * @brief returns the string value reprsenting this enum
 * 
 * @param   priority            the given enum priority
 * @param   priorityAsString    Out param. The string representing the given priority.
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
static TwinConfigurationResult TwinConfigurationEventPriorities_PriorityEnumAsString(TwinConfiguartionEventPriority priority, char const **  priorityAsString);
/**
 * @brief Returns the priority of the wanted event type.
 *          This function should be called only after the config lock is locked.
 * 
 * @param   eventType   The wanted event type.
 * @param   priority     Out param. The wanted priority.
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
static TwinConfigurationResult TwinConfigurationEventPriorities_SafeGetPriority(TwinConfiguartionEventType eventType, TwinConfiguartionEventPriority* prioriy);

/**
 * @brief writes the current event priorities object
 * 
 * @param   prioritiesJson   json object writer handle
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
static TwinConfigurationResult TwinConfigurationEventPriorities_SafeGetPrioritiesJson(JsonObjectWriterHandle prioritiesJson);

TwinConfigurationResult TwinConfigurationEventPriorities_Init() {
    eventPriorities.lock = Lock_Init();
    if (eventPriorities.lock == NULL) {
        return TWIN_LOCK_EXCEPTION; 
    }

    eventPriorities.processCreatePriority = PROCESS_CREATE_DEFAULT_PRIORITY;
    eventPriorities.listeningPortsPriority = LISTENING_PORTS_DEFAULT_PRIORITY;
    eventPriorities.systemInformationPriority = SYSTEM_INFORMATION_DEFAULT_PRIORITY;
    eventPriorities.localUsersPriority = LOCAL_USERS_DEFAULT_PRIORITY;
    eventPriorities.loginPriority = LOGIN_DEFAULT_PRIORITY;
    eventPriorities.connectionCreatePriority = CONNECTION_CREATE_DEFAULT_PRIORITY;
    eventPriorities.firewallConfigurationPriority = FIREWALL_CONFIGURATION_DEFAULT_PRIORITY;
    eventPriorities.baselinePriority = BASELINE_DEFAULT_PRIORITY;
    eventPriorities.diagnostic = DIAGNOSTIC_DEFAULT_PRIORITY;
    eventPriorities.operational = OPERATIONAL_EVENT_DEFAULT_PRIORITY;

    return TWIN_OK;
}

void TwinConfigurationEventPriorities_Deinit() {
    if (eventPriorities.lock != NULL) {
        Lock_Deinit(eventPriorities.lock);
        eventPriorities.lock = NULL;
    }
}

TwinConfigurationResult TwinConfigurationEventPriorities_Update(JsonObjectReaderHandle propertiesReader) {
    TwinConfigurationResult returnValue = TWIN_OK;
    bool isLocked = false;

    if (Lock(eventPriorities.lock) != LOCK_OK) {
        returnValue = TWIN_LOCK_EXCEPTION;
        goto cleanup;
    }
    isLocked = true;

    returnValue = TwinConfigurationEventPriorities_SetValues(propertiesReader);
    
cleanup:
    if (isLocked) {
        if (Unlock(eventPriorities.lock)) {
            returnValue = TWIN_LOCK_EXCEPTION;
        }
    } 
    return returnValue;
}

TwinConfigurationResult TwinConfigurationEventPriorities_GetPriority(TwinConfiguartionEventType eventType, TwinConfiguartionEventPriority* prioriy) {
    TwinConfigurationResult result = TWIN_OK;
    bool isLocked = false;
    if (Lock(eventPriorities.lock) != LOCK_OK) {
        result = TWIN_LOCK_EXCEPTION;
        goto cleanup;
    }
    isLocked = true;

    result = TwinConfigurationEventPriorities_SafeGetPriority(eventType, prioriy);
    
cleanup:
    if (isLocked) {
        if (Unlock(eventPriorities.lock)) {
           return TWIN_LOCK_EXCEPTION;
        }
    }
    
    return result;
}

TwinConfigurationResult  TwinConfigurationEventPriorities_GetPrioritiesJson(JsonObjectWriterHandle prioritiesJson){
    TwinConfigurationResult result = TWIN_OK;
    bool isLocked = false;
    if (Lock(eventPriorities.lock) != LOCK_OK) {
        result = TWIN_LOCK_EXCEPTION;
        goto cleanup;
    }
    isLocked = true;
    result = TwinConfigurationEventPriorities_SafeGetPrioritiesJson(prioritiesJson);
    
cleanup:
    if (isLocked) {
        if (Unlock(eventPriorities.lock)) {
           return TWIN_LOCK_EXCEPTION;
        }
    }
    
    return result;
}

static TwinConfigurationResult TwinConfigurationEventPriorities_SetValues(JsonObjectReaderHandle propertiesReader) {
    TwinConfigurationResult result = TWIN_OK; 
    TwinConfigurationEventPriorities newPriorities = { 0 };

    result = TwinConfigurationEventPriorities_SetSingleValues(propertiesReader, PROCESS_CREATE_PRIORITY_KEY, &(newPriorities.processCreatePriority), PROCESS_CREATE_DEFAULT_PRIORITY);
    if (result != TWIN_OK) {
        goto cleanup;
    }

    result = TwinConfigurationEventPriorities_SetSingleValues(propertiesReader, LISTENING_PORTS_PRIORITY_KEY, &(newPriorities.listeningPortsPriority), LISTENING_PORTS_DEFAULT_PRIORITY);
    if (result != TWIN_OK) {
        goto cleanup;
    }

    result = TwinConfigurationEventPriorities_SetSingleValues(propertiesReader, SYSTEM_INFORMATION_PRIORITY_KEY, &(newPriorities.systemInformationPriority), SYSTEM_INFORMATION_DEFAULT_PRIORITY);
    if (result != TWIN_OK) {
        goto cleanup;
    }

    result = TwinConfigurationEventPriorities_SetSingleValues(propertiesReader, LOCAL_USERS_PRIORITY_KEY, &(newPriorities.localUsersPriority), LOCAL_USERS_DEFAULT_PRIORITY);
    if (result != TWIN_OK) {
        goto cleanup;
    }

    result = TwinConfigurationEventPriorities_SetSingleValues(propertiesReader, LOGIN_PRIORITY_KEY, &(newPriorities.loginPriority), LOGIN_DEFAULT_PRIORITY);
    if (result != TWIN_OK) {
        goto cleanup;
    }

    result = TwinConfigurationEventPriorities_SetSingleValues(propertiesReader, CONNECTION_CREATE_PRIORITY_KEY, &(newPriorities.connectionCreatePriority), CONNECTION_CREATE_DEFAULT_PRIORITY);
    if (result != TWIN_OK) {
        goto cleanup;
    }

    result = TwinConfigurationEventPriorities_SetSingleValues(propertiesReader, FIREWALL_CONFIGURATION_PRIORITY_KEY, &(newPriorities.firewallConfigurationPriority), FIREWALL_CONFIGURATION_DEFAULT_PRIORITY);
    if (result != TWIN_OK) {
        goto cleanup;
    }

    result = TwinConfigurationEventPriorities_SetSingleValues(propertiesReader, BASELINE_PRIORITY_KEY, &(newPriorities.baselinePriority), BASELINE_DEFAULT_PRIORITY);
    if (result != TWIN_OK) {
        goto cleanup;
    }

    result = TwinConfigurationEventPriorities_SetSingleValues(propertiesReader, DIAGNOSTIC_PRIORITY_KEY, &(newPriorities.diagnostic), DIAGNOSTIC_DEFAULT_PRIORITY);
    if (result != TWIN_OK) {
        goto cleanup;
    }

    result = TwinConfigurationEventPriorities_SetSingleValues(propertiesReader, OPERATIONAL_EVENT_KEY, &(newPriorities.operational), OPERATIONAL_EVENT_DEFAULT_PRIORITY);
    if (result != TWIN_OK) {
        goto cleanup;
    }
    //Operational events can be off/operational prio
    eventPriorities.operational = eventPriorities.operational == EVENT_PRIORITY_OFF ? EVENT_PRIORITY_OFF : EVENT_PRIORITY_OPERATIONAL;

cleanup:
    if (result == TWIN_OK){
        newPriorities.lock = eventPriorities.lock;
        memcpy(&eventPriorities, &newPriorities, sizeof(TwinConfigurationEventPriorities));
    }
    
    return result;
}

static TwinConfigurationResult TwinConfigurationEventPriorities_SetSingleValues(JsonObjectReaderHandle propertiesReader, const char* key, TwinConfiguartionEventPriority* priorityField, TwinConfiguartionEventPriority defaultValue) {
    char* strValue = NULL;

    JsonReaderResult readResult = JsonObjectReader_ReadString(propertiesReader, key, &strValue);
    if (readResult == JSON_READER_KEY_MISSING) {
        *priorityField = defaultValue;
        return TWIN_OK;
    } else if (readResult == JSON_READER_PARSE_ERROR) {
        return TWIN_PARSE_EXCEPTION;
    } else if (readResult != JSON_READER_OK) {
        return TWIN_EXCEPTION;
    }

    TwinConfiguartionEventPriority priority;
    TwinConfigurationResult result = TwinConfigurationEventPriorities_PriorityAsEnum(strValue, &priority);
    if (result != TWIN_OK) {
        return result;
    }
    *priorityField = priority;

    return TWIN_OK;
}

static TwinConfigurationResult TwinConfigurationEventPriorities_PriorityAsEnum(const char* str, TwinConfiguartionEventPriority* priority) {
    if (Utils_UnsafeAreStringsEqual(str, PRIORITY_HIGH, false)) {
        *priority = EVENT_PRIORITY_HIGH;
    } else if (Utils_UnsafeAreStringsEqual(str, PRIORITY_LOW, false )) {
        *priority = EVENT_PRIORITY_LOW;
    } else if (Utils_UnsafeAreStringsEqual(str, PRIORITY_OFF, false)) {
        *priority = EVENT_PRIORITY_OFF;
    } else {
        return TWIN_PARSE_EXCEPTION;
    }
    return TWIN_OK;
}

static TwinConfigurationResult TwinConfigurationEventPriorities_PriorityEnumAsString(TwinConfiguartionEventPriority priority, char const **  priorityAsString){
    *priorityAsString = NULL;
    if (priority == EVENT_PRIORITY_HIGH || priority == EVENT_PRIORITY_OPERATIONAL){
        *priorityAsString = PRIORITY_HIGH;
    } else if (priority == EVENT_PRIORITY_LOW){
        *priorityAsString = PRIORITY_LOW;
    } else if (priority == EVENT_PRIORITY_OFF){
        *priorityAsString = PRIORITY_OFF;
    } else {
        return TWIN_EXCEPTION;
    }
    return TWIN_OK;
}

static TwinConfigurationResult TwinConfigurationEventPriorities_SafeGetPriority(TwinConfiguartionEventType eventType, TwinConfiguartionEventPriority* prioriy) {
    switch (eventType) {
        case EVENT_TYPE_PROCESS_CREATE:
            *prioriy = eventPriorities.processCreatePriority;
            break;
        case EVENT_TYPE_LISTENING_PORTS:
            *prioriy = eventPriorities.listeningPortsPriority;
            break;
        case EVENT_TYPE_SYSTEM_INFORMATION:
            *prioriy = eventPriorities.systemInformationPriority;
            break;
        case EVENT_TYPE_LOCAL_USERS:
            *prioriy = eventPriorities.localUsersPriority;
            break;
        case EVENT_TYPE_USER_LOGIN:
            *prioriy = eventPriorities.loginPriority;
            break;
        case EVENT_TYPE_CONNECTION_CREATE:
            *prioriy = eventPriorities.connectionCreatePriority;
            break;
        case EVENT_TYPE_FIREWALL_CONFIGURATION:
            *prioriy = eventPriorities.firewallConfigurationPriority;
            break;
        case EVENT_TYPE_BASELINE:
            *prioriy = eventPriorities.baselinePriority;
            break;
        case EVENT_TYPE_DIAGNOSTIC:
            *prioriy = eventPriorities.diagnostic;
            break;
        case EVENT_TYPE_OPERATIONAL_EVENT:
            *prioriy = eventPriorities.operational;
            break;
        default:
            return TWIN_EXCEPTION;
    }
    return TWIN_OK;
}

static TwinConfigurationResult TwinConfigurationEventPriorities_SafeGetPrioritiesJson(JsonObjectWriterHandle prioritiesJson){
    TwinConfigurationResult result = TWIN_OK;

    const char* priorityAsString;
    TwinConfigurationEventPriorities_PriorityEnumAsString(eventPriorities.baselinePriority, &priorityAsString);
    if (JsonObjectWriter_WriteString(prioritiesJson, BASELINE_PRIORITY_KEY, priorityAsString) != JSON_WRITER_OK) {
        result = TWIN_EXCEPTION;
        goto cleanup;
    }

    TwinConfigurationEventPriorities_PriorityEnumAsString(eventPriorities.connectionCreatePriority, &priorityAsString);
    if (JsonObjectWriter_WriteString(prioritiesJson, CONNECTION_CREATE_PRIORITY_KEY, priorityAsString) != JSON_WRITER_OK) {
        result = TWIN_EXCEPTION;
        goto cleanup;
    }

    TwinConfigurationEventPriorities_PriorityEnumAsString(eventPriorities.diagnostic, &priorityAsString);
    if (JsonObjectWriter_WriteString(prioritiesJson, DIAGNOSTIC_PRIORITY_KEY, priorityAsString) != JSON_WRITER_OK) {
        result = TWIN_EXCEPTION;
        goto cleanup;
    }

    TwinConfigurationEventPriorities_PriorityEnumAsString(eventPriorities.firewallConfigurationPriority, &priorityAsString);
    if (JsonObjectWriter_WriteString(prioritiesJson, FIREWALL_CONFIGURATION_PRIORITY_KEY, priorityAsString) != JSON_WRITER_OK) {
        result = TWIN_EXCEPTION;
        goto cleanup;
    }

    TwinConfigurationEventPriorities_PriorityEnumAsString(eventPriorities.listeningPortsPriority, &priorityAsString);
    if (JsonObjectWriter_WriteString(prioritiesJson, LISTENING_PORTS_PRIORITY_KEY, priorityAsString) != JSON_WRITER_OK) {
        result = TWIN_EXCEPTION;
        goto cleanup;
    }

    TwinConfigurationEventPriorities_PriorityEnumAsString(eventPriorities.localUsersPriority, &priorityAsString);
    if (JsonObjectWriter_WriteString(prioritiesJson, LOCAL_USERS_PRIORITY_KEY, priorityAsString) != JSON_WRITER_OK) {
        result = TWIN_EXCEPTION;
        goto cleanup;
    }

    TwinConfigurationEventPriorities_PriorityEnumAsString(eventPriorities.loginPriority, &priorityAsString);
    if (JsonObjectWriter_WriteString(prioritiesJson, LOGIN_PRIORITY_KEY, priorityAsString) != JSON_WRITER_OK) {
        result = TWIN_EXCEPTION;
        goto cleanup;
    }

    TwinConfigurationEventPriorities_PriorityEnumAsString(eventPriorities.operational, &priorityAsString);
    if (JsonObjectWriter_WriteString(prioritiesJson, OPERATIONAL_EVENT_KEY, priorityAsString) != JSON_WRITER_OK) {
        result = TWIN_EXCEPTION;
        goto cleanup;
    }

    TwinConfigurationEventPriorities_PriorityEnumAsString(eventPriorities.processCreatePriority, &priorityAsString);
    if (JsonObjectWriter_WriteString(prioritiesJson, PROCESS_CREATE_PRIORITY_KEY, priorityAsString) != JSON_WRITER_OK) {
        result = TWIN_EXCEPTION;
        goto cleanup;
    }

    TwinConfigurationEventPriorities_PriorityEnumAsString(eventPriorities.systemInformationPriority, &priorityAsString);
    if (JsonObjectWriter_WriteString(prioritiesJson, SYSTEM_INFORMATION_PRIORITY_KEY, priorityAsString) != JSON_WRITER_OK) {
        result = TWIN_EXCEPTION;
        goto cleanup;
    }

cleanup:
    return result;
}