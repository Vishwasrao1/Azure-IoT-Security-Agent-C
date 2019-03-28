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

char* connectionString = NULL;
char* agentId = NULL;
uint32_t triggeredEventsInterval = 0;


#define CONNECTION_STRING_SIZE 500
#define KEY_SIZE 300

static const char LOCAL_CONFIG_CONFIGURATION[] = "Configuration";
static const char LOCAL_CONFIG_AGENT_ID[] = "AgentId";
static const char LOCAL_CONFIG_AGENT_ID_TRIGGERED_EVENTS_INTERVAL[] = "TriggerdEventsInterval";

static const char LOCAL_CONFIG_AUTHENTICATION[] = "Authentication";
static const char LOCAL_CONFIG_AUTHENTICATION_SAS_TOKEN[] = "SasToken";
static const char LOCAL_CONFIG_AUTHENTICATION_SELF_SIGNED_CERTIFICATE[] = "SelfSignedCertificate";
static const char LOCAL_CONFIG_AUTHENTICATION_DEVICE_ID[] = "DeviceId";
static const char LOCAL_CONFIG_AUTHENTICATION_HOST_NAME[] = "HostName";
static const char LOCAL_CONFIG_AUTHENTICATION_AUTHENTICATION_MATHOD[] = "AuthenticationMethod";
static const char LOCAL_CONFIG_AUTHENTICATION_IDENTITY[] = "Identity";
static const char LOCAL_CONFIG_AUTHENTICATION_IDENTITY_VALUE_SECURITY_MODULE[] = "SecurityModule";
static const char LOCAL_CONFIG_AUTHENTICATION_IDENTITY_VALUE_DEVICE[] = "Device";
static const char LOCAL_CONFIG_AUTHENTICATION_FILE_PATH[] = "FilePath";

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
static LocalConfigurationResultValues LocalConfiguration_InitConnectionStringFromDevice(char* deviceId, char* hostName, char* authenticationMethod, char* filePath);

/**
 * @brief   initializes the security module connection string from the local configuration's provided json.
 * 
 * @param   jsonReader      a handle to the json reader.
 * 
 * @return LOCAL_CONFIGURATION_OK on success, LOCAL_CONFIGURATION_EXCEPTION otherwise.
 */
static LocalConfigurationResultValues LocalConfiguration_InitConnectionString(JsonObjectReaderHandle jsonReader);

static LocalConfigurationResultValues LocalConfiguration_InitConnectionStringFromDevice(char* deviceId, char* hostName, char* authenticationMethod, char* filePath){
    LocalConfigurationResultValues result = LOCAL_CONFIGURATION_EXCEPTION;
    bool authenticationManagerInitialized = false;

    if (!AuthenticationManager_Init()) {
        goto cleanup;
    }
    authenticationManagerInitialized = true;

    uint32_t localConnectionStringSize = CONNECTION_STRING_SIZE;
    char localConnectionString[CONNECTION_STRING_SIZE] = "";

    if (Utils_UnsafeAreStringsEqual(authenticationMethod, LOCAL_CONFIG_AUTHENTICATION_SAS_TOKEN, true)) {
        if (!AuthenticationManager_InitFromSharedAccessKey(filePath, hostName, deviceId)) {
            goto cleanup;
        }

        if (!AuthenticationManager_GetConnectionString(localConnectionString, &localConnectionStringSize)) {
            goto cleanup;
        }

        if (Utils_CreateStringCopy(&connectionString, localConnectionString) == false) {
            goto cleanup;
        }

        result = LOCAL_CONFIGURATION_OK;
        goto cleanup;
    
    } else if (Utils_UnsafeAreStringsEqual(authenticationMethod, LOCAL_CONFIG_AUTHENTICATION_SELF_SIGNED_CERTIFICATE, true)) {
        if (!AuthenticationManager_InitFromCertificate(filePath, hostName, deviceId)) {
            goto cleanup;
        }

        if (!AuthenticationManager_GetConnectionString(localConnectionString, &localConnectionStringSize)) {
            goto cleanup;
        }

        if (Utils_CreateStringCopy(&connectionString, localConnectionString) == false) {
            goto cleanup;
        }

        result = LOCAL_CONFIGURATION_OK;
        goto cleanup;
    }

cleanup:
    if (authenticationManagerInitialized) {
        AuthenticationManager_Deinit();
    }

    return result;
}

static LocalConfigurationResultValues LocalConfiguration_InitConnectionString(JsonObjectReaderHandle jsonReader){
    JsonReaderResult parseResult = JsonObjectReader_StepIn(jsonReader, LOCAL_CONFIG_AUTHENTICATION);
    if (parseResult != JSON_READER_OK) {
        return LOCAL_CONFIGURATION_EXCEPTION;
    }

    char* deviceId = NULL;
    char* hostName = NULL;
    char* authenticationMethod = NULL;
    char* identity = NULL;
    char* filePath = NULL;
    char sharedAccessKey[KEY_SIZE] = "";

    parseResult |= JsonObjectReader_ReadString(jsonReader, LOCAL_CONFIG_AUTHENTICATION_DEVICE_ID, &deviceId);
    parseResult |= JsonObjectReader_ReadString(jsonReader, LOCAL_CONFIG_AUTHENTICATION_HOST_NAME, &hostName);
    parseResult |= JsonObjectReader_ReadString(jsonReader, LOCAL_CONFIG_AUTHENTICATION_AUTHENTICATION_MATHOD, &authenticationMethod);
    parseResult |= JsonObjectReader_ReadString(jsonReader, LOCAL_CONFIG_AUTHENTICATION_IDENTITY, &identity);
    parseResult |= JsonObjectReader_ReadString(jsonReader, LOCAL_CONFIG_AUTHENTICATION_FILE_PATH, &filePath);

    if (parseResult != JSON_READER_OK) {
        return LOCAL_CONFIGURATION_EXCEPTION;
    }

    if (Utils_UnsafeAreStringsEqual(identity, LOCAL_CONFIG_AUTHENTICATION_IDENTITY_VALUE_SECURITY_MODULE, true)) {

        if ((FileUtils_ReadFile(filePath, &sharedAccessKey, KEY_SIZE, true) != FILE_UTILS_OK)) {
            return LOCAL_CONFIGURATION_EXCEPTION;
        }

        // omitting new line if exist
        char *newLinePosition = strchr(sharedAccessKey, '\n');
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

    } else if (Utils_UnsafeAreStringsEqual(identity, LOCAL_CONFIG_AUTHENTICATION_IDENTITY_VALUE_DEVICE, true)) {

        return LocalConfiguration_InitConnectionStringFromDevice(deviceId, hostName, authenticationMethod, filePath);
    }

    return LOCAL_CONFIGURATION_EXCEPTION;
}

LocalConfigurationResultValues LocalConfiguration_Init(){
    char* configurationFile = NULL;
    JsonObjectReaderHandle jsonReader = NULL;
    LocalConfigurationResultValues returnVal = LOCAL_CONFIGURATION_OK;

    // fetch executable location
    configurationFile = GetExecutableDirectory();
    if (configurationFile == NULL) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        Logger_Error("failed fetching current directory");
        goto cleanup;
    }

    int pathLength = strlen(configurationFile);
    char* temp = (char*)realloc(configurationFile, pathLength + strlen(CONFIGURATION_FILE) + 1);
    if (temp == NULL) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        Logger_Error("failed in realloc, baisically we should throw");
        goto cleanup; 
    }
    configurationFile = temp;
    memcpy(configurationFile + pathLength, CONFIGURATION_FILE, strlen(CONFIGURATION_FILE) + 1);

    JsonReaderResult result = JsonObjectReader_InitFromFile(&jsonReader, configurationFile);
    if (result != JSON_READER_OK) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        Logger_Error("failed reading configuration file");
        goto cleanup;
    }

    result = JsonObjectReader_StepIn(jsonReader, LOCAL_CONFIG_CONFIGURATION);
    if (result != JSON_READER_OK) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        goto cleanup;
    }

    char* strValue = NULL;
    result = JsonObjectReader_ReadString(jsonReader, LOCAL_CONFIG_AGENT_ID, &strValue);
    if (result != JSON_READER_OK) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        Logger_Error("failed reading agent id from configuraiton file");
        goto cleanup;
    }

    if (Utils_CreateStringCopy(&agentId, strValue) == false) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        goto cleanup;
    }

    result = JsonObjectReader_ReadTimeInMilliseconds(jsonReader, LOCAL_CONFIG_AGENT_ID_TRIGGERED_EVENTS_INTERVAL, &triggeredEventsInterval);
    if (result != JSON_READER_OK) {
        Logger_Error("failed reading triggered event interval from configuraiton file");
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        goto cleanup;
    }

    if (LocalConfiguration_InitConnectionString(jsonReader) != LOCAL_CONFIGURATION_OK) {
        returnVal = LOCAL_CONFIGURATION_EXCEPTION;
        Logger_Error("failed fetching connection string");
        goto cleanup;
    }

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

void LocalConfiguration_Deinit() {
    if (connectionString != NULL) {
        free(connectionString);
        connectionString = NULL;
    }
    if (agentId != NULL) {
        free(agentId);
        agentId = NULL;
    }
}

char* LocalConfiguration_GetConnectionString() {
    return connectionString;
}

char* LocalConfiguration_GetAgentId() {
    return agentId;
}

uint32_t LocalConfiguration_GetTriggeredEventInterval() {
    return triggeredEventsInterval;
}
