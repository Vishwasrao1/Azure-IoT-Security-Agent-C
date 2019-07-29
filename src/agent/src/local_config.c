// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <stdlib.h>

#include "authentication_manager.h"
#include "json/json_object_reader.h"
#include "local_config.h"
#include "logger.h"
#include "os_utils/os_utils.h"
#include "os_utils/file_utils.h"
#include "utils.h"
#include "agent_errors.h"

static char* connectionString = NULL;
static char* agentId = NULL;
static bool authenticationManagerInitialized = false;
static bool useDps = false;
static uint32_t triggeredEventsInterval = 0;
static uint32_t connectionTimeout = 0;
static int32_t systemLoggerMinimumSeverity = 0;
static int32_t diagnosticEventMinimumSeverity = 0;
static char* remoteConfigurationObjectName = NULL;

#define CONNECTION_STRING_SIZE 500
#define KEY_SIZE 300

static const char LOCAL_CONFIG_CONFIGURATION[] = "Configuration";
static const char LOCAL_CONFIG_AGENT_ID[] = "AgentId";
static const char LOCAL_CONFIG_AGENT_ID_TRIGGERED_EVENTS_INTERVAL[] = "TriggerdEventsInterval";
static const char LOCAL_CONFIG_CONNECTION_TIMEOUT[] = "ConnectionTimeout";
static const char LOCAL_CONFIG_REMOTE_CONFIGURATION_OBJECT_NAME[] = "RemoteConfigurationObjectName";

static const char LOCAL_CONFIG_AUTHENTICATION[] = "Authentication";
static const char LOCAL_CONFIG_AUTHENTICATION_SAS_TOKEN[] = "SasToken";
static const char LOCAL_CONFIG_AUTHENTICATION_SELF_SIGNED_CERTIFICATE[] = "SelfSignedCertificate";
static const char LOCAL_CONFIG_AUTHENTICATION_DEVICE_ID[] = "DeviceId";
static const char LOCAL_CONFIG_AUTHENTICATION_HOST_NAME[] = "HostName";
static const char LOCAL_CONFIG_AUTHENTICATION_AUTHENTICATION_METHOD[] = "AuthenticationMethod";
static const char LOCAL_CONFIG_AUTHENTICATION_IDENTITY[] = "Identity";
static const char LOCAL_CONFIG_AUTHENTICATION_IDENTITY_VALUE_SECURITY_MODULE[] = "SecurityModule";
static const char LOCAL_CONFIG_AUTHENTICATION_IDENTITY_VALUE_DEVICE[] = "Device";
static const char LOCAL_CONFIG_AUTHENTICATION_IDENTITY_VALUE_DPS[] = "DPS";
static const char LOCAL_CONFIG_AUTHENTICATION_FILE_PATH[] = "FilePath";
static const char LOCAL_CONFIG_AUTHENTICATION_DPS[] = "DPS";
static const char LOCAL_CONFIG_AUTHENTICATION_DPS_IDSCOPE[] = "IDScope";
static const char LOCAL_CONFIG_AUTHENTICATION_DPS_REGISTRATION_ID[] = "RegistrationId";

static const char LOCAL_CONFIG_LOGGING[] = "Logging";
static const char LOCAL_CONFIG_LOGGING_SYSTEM_LOGGER_MINIMUM_SEVERITY[] = "SystemLoggerMinimumSeverity";
static const char LOCAL_CONFIG_LOGGING_DIAGNOSTIC_EVENT_MINIMUM_SEVERITY[] = "DiagnoticEventMinimumSeverity";

/**
 * @brief   initializes the security module connection string using device authentication: certificate or sas token.
 * 
 * @param   deviceId                the device id to authenticate with.
 * @param   hostName                the hub name.
 * @param   authenticationMethod    the authentication method.
 * @param   filePath                the path of the authentication key.
 * 
 * @return LOCAL_CONFIGURATION_OK on success, LOCAL_CONFIGURATION_EXCEPTION otherwise.
 */
static LocalConfigurationResultValues LocalConfiguration_InitConnectionStringFromDevice();

/**
 * @brief   initializes the security module connection string using module sas token authentication.
 * 
 * @param   filePath                the path of the authentication key.
 * @param   hostName                the hub name.
 * @param   deviceId                the device id to authenticate with.
 * 
 * @return LOCAL_CONFIGURATION_OK on success, LOCAL_CONFIGURATION_EXCEPTION otherwise.
 */
static LocalConfigurationResultValues LocalConfiguration_InitConnectionStringFromModule(char* filePath, char* hostName, char* deviceId);

/**
 * @brief   initializes the security module connection string from the local configuration's provided json.
 * 
 * @param   jsonReader      a handle to the json reader.
 * 
 * @return LOCAL_CONFIGURATION_OK on success, LOCAL_CONFIGURATION_EXCEPTION otherwise.
 */
static LocalConfigurationResultValues LocalConfiguration_InitConnectionString(JsonObjectReaderHandle jsonReader);

/**
 * @brief   initializes the authentication manager to connect to the device.
 * 
 * @param   authenticationMethod    the authentication method.
 * @param   filePath                the path of the authentication key.
 * @param   hostName                the hub name.
 * @param   deviceId                the device id to authenticate with.
 * 
 * @return LOCAL_CONFIGURATION_OK on success, LOCAL_CONFIGURATION_EXCEPTION otherwise.
 */
static LocalConfigurationResultValues LocalConfiguration_InitAuthenticationThroughDevice(char* authenticationMethod, char* filePath, char* hostName, char* deviceId);

/**
 * @brief   initializes the authentication manager to connect to the DPS.
 * 
 * @param   idScope                 the ID Scope as registered on the DPS.
 * @param   registrationId          the registration id of the device.
 * @param   authenticationMethod    the authentication method.
 * @param   filePath                the path of the authentication key.
 * 
 * @return LOCAL_CONFIGURATION_OK on success, LOCAL_CONFIGURATION_EXCEPTION otherwise.
 */
static LocalConfigurationResultValues LocalConfiguration_InitAuthenticationThroughDps(const char* idScope, const char* registrationId, const char* authenticationMethod, const char* filePath);

static LocalConfigurationResultValues LocalConfiguration_InitAuthenticationThroughDevice(char* authenticationMethod, char* filePath, char* hostName, char* deviceId) {
    if (!AuthenticationManager_Init()) {
        AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_OTHER, "Could not initialize the Authentication Manager");
        return LOCAL_CONFIGURATION_EXCEPTION;
    }
    authenticationManagerInitialized = true;

    if (Utils_UnsafeAreStringsEqual(authenticationMethod, LOCAL_CONFIG_AUTHENTICATION_SAS_TOKEN, true)) {
        if (!AuthenticationManager_InitFromSharedAccessKey(filePath, hostName, deviceId)) {
            return LOCAL_CONFIGURATION_EXCEPTION;
        }
    } else if (Utils_UnsafeAreStringsEqual(authenticationMethod, LOCAL_CONFIG_AUTHENTICATION_SELF_SIGNED_CERTIFICATE, true)) {
        if (!AuthenticationManager_InitFromCertificate(filePath, hostName, deviceId)) {
            return LOCAL_CONFIGURATION_EXCEPTION;
        }
    } else {
        AgentErrors_LogError(ERROR_LOCAL_CONFIGURATION, SUBCODE_CANT_PARSE_CONFIGURATION, "Unexpected value for key %s", LOCAL_CONFIG_AUTHENTICATION_AUTHENTICATION_METHOD);
        return LOCAL_CONFIGURATION_EXCEPTION;
    }
    
    return LOCAL_CONFIGURATION_OK;
}

static LocalConfigurationResultValues LocalConfiguration_InitAuthenticationThroughDps(const char* idScope, const char* registrationId, const char* authenticationMethod, const char* filePath) {
    if (!AuthenticationManager_Init()) {
        AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_OTHER, "Could not initialize the Authentication Manager");
        return LOCAL_CONFIGURATION_EXCEPTION;
    }
    authenticationManagerInitialized = true;

    AuthenticationManager_SetDPSDetails(idScope, registrationId);

    if (Utils_UnsafeAreStringsEqual(authenticationMethod, LOCAL_CONFIG_AUTHENTICATION_SAS_TOKEN, true)) {
        if (!AuthenticationManager_InitFromSharedAccessKey(filePath, "", "")) {
            return LOCAL_CONFIGURATION_EXCEPTION;
        }
    } else if (Utils_UnsafeAreStringsEqual(authenticationMethod, LOCAL_CONFIG_AUTHENTICATION_SELF_SIGNED_CERTIFICATE, true)) {
        if (!AuthenticationManager_InitFromCertificate(filePath, "", "")) {
            return LOCAL_CONFIGURATION_EXCEPTION;
        }
    } else {
        AgentErrors_LogError(ERROR_LOCAL_CONFIGURATION, SUBCODE_CANT_PARSE_CONFIGURATION, "Unexpected value for key %s", LOCAL_CONFIG_AUTHENTICATION_AUTHENTICATION_METHOD);
        return LOCAL_CONFIGURATION_EXCEPTION;
    }

    if (!AuthenticationManager_GetHostNameFromDPS()) {
        return LOCAL_CONFIGURATION_EXCEPTION;
    }

    return LOCAL_CONFIGURATION_OK;
}

static LocalConfigurationResultValues LocalConfiguration_InitConnectionStringFromDevice(){
    uint32_t localConnectionStringSize = CONNECTION_STRING_SIZE;
    char localConnectionString[CONNECTION_STRING_SIZE] = "";

    if (!AuthenticationManager_GetConnectionString(localConnectionString, &localConnectionStringSize)) {
        return LOCAL_CONFIGURATION_EXCEPTION;
    }

    if (Utils_CreateStringCopy(&connectionString, localConnectionString) == false) {
        return LOCAL_CONFIGURATION_EXCEPTION;
    }

    return LOCAL_CONFIGURATION_OK;
}

static LocalConfigurationResultValues LocalConfiguration_InitConnectionStringFromModule(char* filePath, char* hostName, char* deviceId) {
    char sharedAccessKey[KEY_SIZE] = "";
    FileResults fileResult = FileUtils_ReadFile(filePath, &sharedAccessKey, KEY_SIZE, true);
    if (fileResult != FILE_UTILS_OK) {
        if (fileResult == FILE_UTILS_FILE_NOT_FOUND)
            AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_FILE_NOT_EXIST, "File not found in path: %s", filePath);
        else if (fileResult == FILE_UTILS_NOPERM)
            AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_FILE_PERMISSIONS, "Couldn't open file in path: %s, check permissions", filePath);
        else 
            AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_OTHER, "Unexpected error while opening file: %s", filePath);
        return LOCAL_CONFIGURATION_EXCEPTION;
    }

    // omitting new line if exist
    char* newLinePosition = strchr(sharedAccessKey, '\n');
    if (newLinePosition != NULL) {
        *newLinePosition = '\0';
    }

    uint32_t localConnectionStringSize = CONNECTION_STRING_SIZE;
    char localConnectionString[CONNECTION_STRING_SIZE] = "";

    if (!AuthenticationManager_GenerateConnectionStringFromSharedAccessKey(sharedAccessKey, localConnectionString, &localConnectionStringSize, hostName, deviceId)) {
        return LOCAL_CONFIGURATION_EXCEPTION;
    }

    if (Utils_CreateStringCopy(&connectionString, localConnectionString) == false) {
        return LOCAL_CONFIGURATION_EXCEPTION;
    }

    return LOCAL_CONFIGURATION_OK;
}

static void LocalConfiguration_LogReadErrors(JsonReaderResult readerResult, const char* key) {
    if (readerResult == JSON_READER_KEY_MISSING)
        AgentErrors_LogError(ERROR_LOCAL_CONFIGURATION, SUBCODE_MISSING_CONFIGURATION, "Key: %s" ,key);
    else if (readerResult == JSON_READER_PARSE_ERROR)
        AgentErrors_LogError(ERROR_LOCAL_CONFIGURATION, SUBCODE_CANT_PARSE_CONFIGURATION, "Key: %s" ,key);
    else if (readerResult != JSON_READER_OK) 
        AgentErrors_LogError(ERROR_LOCAL_CONFIGURATION, SUBCODE_OTHER, "Unknown error");
}

static JsonReaderResult LocalConfiguration_ReadJsonStringConfig(JsonObjectReaderHandle jsonReader, const char* key, char** outVal) {
    JsonReaderResult result = JsonObjectReader_ReadString(jsonReader, key, outVal);
    if (result != JSON_READER_OK) {
        LocalConfiguration_LogReadErrors(result, key);
    } else if (strlen(*outVal) == 0) {
        AgentErrors_LogError(ERROR_LOCAL_CONFIGURATION, SUBCODE_MISSING_CONFIGURATION, "Configuration can not be empty, Key %s", key);
        result = JSON_READER_KEY_MISSING;
    }
      
    return result;
}

static JsonReaderResult LocalConfiguration_ReadJsonTimeConfig(JsonObjectReaderHandle jsonReader, const char* key, uint32_t* outVal) {
    JsonReaderResult result = JsonObjectReader_ReadTimeInMilliseconds(jsonReader, key, outVal);
    if (result != JSON_READER_OK)
        LocalConfiguration_LogReadErrors(result, key);

    return result;
}

static LocalConfigurationResultValues LocalConfiguration_InitConnectionString(JsonObjectReaderHandle jsonReader){
    JsonReaderResult parseResult = JsonObjectReader_StepIn(jsonReader, LOCAL_CONFIG_AUTHENTICATION);
    if (parseResult != JSON_READER_OK) {
        LocalConfiguration_LogReadErrors(parseResult, LOCAL_CONFIG_AUTHENTICATION);
        return LOCAL_CONFIGURATION_EXCEPTION;
    }

    char* deviceId = NULL;
    char* hostName = NULL;
    char* authenticationMethod = NULL;
    char* identity = NULL;
    char* filePath = NULL;
    char* idScope = NULL;
    char* registrationId = NULL;

    parseResult |= LocalConfiguration_ReadJsonStringConfig(jsonReader, LOCAL_CONFIG_AUTHENTICATION_AUTHENTICATION_METHOD, &authenticationMethod);
    parseResult |= LocalConfiguration_ReadJsonStringConfig(jsonReader, LOCAL_CONFIG_AUTHENTICATION_IDENTITY, &identity);
    parseResult |= LocalConfiguration_ReadJsonStringConfig(jsonReader, LOCAL_CONFIG_AUTHENTICATION_FILE_PATH, &filePath);

    if (Utils_UnsafeAreStringsEqual(identity, LOCAL_CONFIG_AUTHENTICATION_IDENTITY_VALUE_DPS, true)) {
        useDps = true;
        parseResult |= JsonObjectReader_StepIn(jsonReader, LOCAL_CONFIG_AUTHENTICATION_DPS);
        parseResult |= LocalConfiguration_ReadJsonStringConfig(jsonReader, LOCAL_CONFIG_AUTHENTICATION_DPS_IDSCOPE, &idScope);
        parseResult |= LocalConfiguration_ReadJsonStringConfig(jsonReader, LOCAL_CONFIG_AUTHENTICATION_DPS_REGISTRATION_ID, &registrationId);
        parseResult |= JsonObjectReader_StepOut(jsonReader);
    } else {
        parseResult |= LocalConfiguration_ReadJsonStringConfig(jsonReader, LOCAL_CONFIG_AUTHENTICATION_HOST_NAME, &hostName);
        parseResult |= LocalConfiguration_ReadJsonStringConfig(jsonReader, LOCAL_CONFIG_AUTHENTICATION_DEVICE_ID, &deviceId);
    }

    if (parseResult != JSON_READER_OK) {
        return LOCAL_CONFIGURATION_EXCEPTION;
    }

    if (useDps) {
        LocalConfigurationResultValues result = LocalConfiguration_InitAuthenticationThroughDps(idScope, registrationId, authenticationMethod, filePath);
        if (result != LOCAL_CONFIGURATION_OK) {
            return result;
        }
        return LocalConfiguration_InitConnectionStringFromDevice(); 
    } else if (Utils_UnsafeAreStringsEqual(identity, LOCAL_CONFIG_AUTHENTICATION_IDENTITY_VALUE_DEVICE, true)) {
        LocalConfigurationResultValues result = LocalConfiguration_InitAuthenticationThroughDevice(authenticationMethod, filePath, hostName, deviceId);
        if (result != LOCAL_CONFIGURATION_OK) {
            return result;
        }
        return LocalConfiguration_InitConnectionStringFromDevice();
    } else if (Utils_UnsafeAreStringsEqual(identity, LOCAL_CONFIG_AUTHENTICATION_IDENTITY_VALUE_SECURITY_MODULE, true)) {
        if (Utils_UnsafeAreStringsEqual(authenticationMethod, LOCAL_CONFIG_AUTHENTICATION_SAS_TOKEN, true)) {
            return LocalConfiguration_InitConnectionStringFromModule(filePath, hostName, deviceId);
        } else {
            AgentErrors_LogError(ERROR_LOCAL_CONFIGURATION, SUBCODE_OTHER, "Unsupported authentication methoud for SeucrityModule authentication");
            return LOCAL_CONFIGURATION_EXCEPTION;
        }
    }
    
    AgentErrors_LogError(ERROR_LOCAL_CONFIGURATION, SUBCODE_CANT_PARSE_CONFIGURATION, "Unexpected value for key %s", LOCAL_CONFIG_AUTHENTICATION_IDENTITY);

    return LOCAL_CONFIGURATION_EXCEPTION;
}

static void LocalConfiguration_InitLogger(JsonObjectReaderHandle jsonReader) {
    if (JsonObjectReader_StepIn(jsonReader, LOCAL_CONFIG_LOGGING) != JSON_READER_OK) {
        Logger_Information("Could not find logging info in local config, using default values");
        return;
    }

    if (JsonObjectReader_ReadInt(jsonReader, LOCAL_CONFIG_LOGGING_SYSTEM_LOGGER_MINIMUM_SEVERITY, &systemLoggerMinimumSeverity) != JSON_READER_OK) {
        Logger_Information("Failed reading system logger minimum severity from configuraiton file, using default value");
    }

    if (JsonObjectReader_ReadInt(jsonReader, LOCAL_CONFIG_LOGGING_DIAGNOSTIC_EVENT_MINIMUM_SEVERITY, &diagnosticEventMinimumSeverity) != JSON_READER_OK) {
        Logger_Error("Failed reading diagnostic event minimum severity from configuraiton file, using default value");
    }
}

LocalConfigurationResultValues LocalConfiguration_Init(){
    char* configurationFile = NULL;
    JsonObjectReaderHandle jsonReader = NULL;
    LocalConfigurationResultValues returnVal = LOCAL_CONFIGURATION_OK;

    // fetch executable location
    configurationFile = GetExecutableDirectory();
    if (configurationFile == NULL) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        AgentErrors_LogError(ERROR_LOCAL_CONFIGURATION, SUBCODE_OTHER, "failed fetching current directory");
        goto cleanup;
    }

    int pathLength = strlen(configurationFile);
    char* temp = (char*)realloc(configurationFile, pathLength + strlen(CONFIGURATION_FILE) + 1);
    if (temp == NULL) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        AgentErrors_LogError(ERROR_LOCAL_CONFIGURATION, SUBCODE_OTHER, "Could not allocate memory");
        goto cleanup; 
    }
    configurationFile = temp;
    memcpy(configurationFile + pathLength, CONFIGURATION_FILE, strlen(CONFIGURATION_FILE) + 1);

    JsonReaderResult result = JsonObjectReader_InitFromFile(&jsonReader, configurationFile);
    if (result != JSON_READER_OK) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        AgentErrors_LogError(ERROR_LOCAL_CONFIGURATION, SUBCODE_FILE_FORMAT, "Failed to parse configuration file");
        goto cleanup;
    }

    result = JsonObjectReader_StepIn(jsonReader, LOCAL_CONFIG_CONFIGURATION);
    if (result != JSON_READER_OK) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        goto cleanup;
    }

    char* strValue = NULL;
    result = LocalConfiguration_ReadJsonStringConfig(jsonReader, LOCAL_CONFIG_AGENT_ID, &strValue);
    if (result != JSON_READER_OK) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        goto cleanup;
    }

    if (Utils_CreateStringCopy(&agentId, strValue) == false) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        goto cleanup;
    }

    result = LocalConfiguration_ReadJsonTimeConfig(jsonReader, LOCAL_CONFIG_AGENT_ID_TRIGGERED_EVENTS_INTERVAL, &triggeredEventsInterval);
    if (result != JSON_READER_OK) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        goto cleanup;
    }

    result = LocalConfiguration_ReadJsonTimeConfig(jsonReader, LOCAL_CONFIG_CONNECTION_TIMEOUT, &connectionTimeout);
    if (result != JSON_READER_OK) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        goto cleanup;
    }

    strValue = NULL;
    result = LocalConfiguration_ReadJsonStringConfig(jsonReader, LOCAL_CONFIG_REMOTE_CONFIGURATION_OBJECT_NAME, &strValue);
    if (result != JSON_READER_OK) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        goto cleanup;
    }

    if (Utils_CreateStringCopy(&remoteConfigurationObjectName, strValue) == false) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        goto cleanup;
    }

    if (LocalConfiguration_InitConnectionString(jsonReader) != LOCAL_CONFIGURATION_OK) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        goto cleanup;
    }

    result = JsonObjectReader_StepOut(jsonReader);
    if (result != JSON_READER_OK) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        goto cleanup;
    }

    LocalConfiguration_InitLogger(jsonReader);

cleanup:
    if (jsonReader != NULL) {
        JsonObjectReader_Deinit(jsonReader);
    }
    if (configurationFile != NULL) {
        free(configurationFile);
    }
    if (returnVal != LOCAL_CONFIGURATION_OK) {
        LocalConfiguration_Deinit();
    }

    return returnVal;
}

bool LocalConfiguration_TryRenewConnectionString() {
    Logger_Information("Try renew connection string");
    if (!AuthenticationManager_GetHostNameFromDPS()) {
        Logger_Error("Failed renewing registration details from dps");
        return false;
    }
    if (LocalConfiguration_InitConnectionStringFromDevice() != LOCAL_CONFIGURATION_OK) {
        Logger_Error("Failed renewing connection string");
        return false;
    }
    return true;
}

void LocalConfiguration_Deinit() {
    if (authenticationManagerInitialized) {
        AuthenticationManager_Deinit();
        authenticationManagerInitialized = false;
    }
    if (connectionString != NULL) {
        free(connectionString);
        connectionString = NULL;
    }
    if (agentId != NULL) {
        free(agentId);
        agentId = NULL;
    }
    if (remoteConfigurationObjectName != NULL) {
        free(agentId);
        agentId = NULL;
    }
}

const char* LocalConfiguration_GetConnectionString() {
    return connectionString;
}

const char* LocalConfiguration_GetAgentId() {
    return agentId;
}

uint32_t LocalConfiguration_GetTriggeredEventInterval() {
    return triggeredEventsInterval;
}

bool LocalConfiguration_UseDps() {
    return useDps;
}

int32_t LocalConfiguration_GetSystemLoggerMinimumSeverity() {
    return systemLoggerMinimumSeverity;
}

int32_t LocalConfiguration_GetDiagnosticEventMinimumSeverity() {
    return diagnosticEventMinimumSeverity;
}

uint32_t LocalConfiguration_GetConnectionTimeout() {
    return connectionTimeout;
}

const char* LocalConfiguration_GetRemoteConfigurationObjectName() {
    return remoteConfigurationObjectName;
}