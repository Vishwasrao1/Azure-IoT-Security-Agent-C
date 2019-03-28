// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "authentication_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "azure_c_shared_utility/sastoken.h"
#include "azure_c_shared_utility/httpheaders.h"
#include "azure_c_shared_utility/httpapiex.h"
#include "azure_c_shared_utility/httpapiexsas.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "json/json_object_reader.h"
#include "azure_c_shared_utility/platform.h"
#include "utils.h"
#include "certificate_manager.h"
#include "os_utils/file_utils.h"

/**
 * @brief   extract the connection string from the server's json response.
 * 
 * @param   responseJson            the file containning the shared access key that is connected to the device.
 * @param   connectionString        out param. the returned conneciton string.
 * @param   connectionStringSize    out param. the returned conneciton string size.
 * 
 * @return true on success, false otherwise.
 */
static bool AuthenticationManager_ExtractConnectionString(char* responseJson, char* connectionString, uint32_t* connectionStringSize);

/**
 * @brief   tries to get the security module conection string from the server using a Sas Token.
 * 
 * @param   connectionString        out param. the returned conneciton string.
 * @param   connectionStringSize    out param. the returned conneciton string size.
 * 
 * @return true on success, false otherwise.
 */
static bool AuthenticationManager_GetConnectionStringUsingSasToken(char* connectionString, uint32_t* connectionStringSize);

/**
 * @brief   tries to get the security module conection string from the server using a self signed certificate.
 * 
 * @param   connectionString        out param. the returned conneciton string.
 * @param   connectionStringSize    out param. the returned conneciton string size.
 * 
 * @return true on success, false otherwise.
 */
static bool AuthenticationManager_GetConnectionStringUsingCertificate(char* connectionString, uint32_t* connectionStringSize);

/**
 * @brief   initializes authentication manager with the relative path to the device security module
 * 
 * @param   hostName    the hub name to connect
 * @param   deviceId    the device to connect to
 * 
 * @return true on success, false otherwise.
 */
static bool AuthenticationManager_InitSharedProperties(char* hostName, char* deviceId);

#define MAX_BUFF 500
#define KEY_SIZE 300

static const char* SECURITY_MODULE_API = "/devices/%s/modules/azureiotsecurity?api-version=2018-06-30";
static const char* PRIMARY_KEY = "authentication.symmetricKey.primaryKey";
static const char* GENERATED_CONNECTION_STRING = "HostName=%s;DeviceId=%s;ModuleId=azureiotsecurity;SharedAccessKey=%s";

typedef struct _authenticationManager {
    char relativeUrl[MAX_BUFF];
    char hostName[MAX_BUFF];
    char deviceId[MAX_BUFF];

    HTTPAPIEX_SAS_HANDLE sasHandle;
    char* certificate;
    char* certificatePrivateKey;

} AuthenticationManager;

AuthenticationManager authenticationManager;

bool AuthenticationManager_Init() {
    memset(authenticationManager.deviceId, 0, MAX_BUFF * sizeof(char));
    memset(authenticationManager.relativeUrl, 0, MAX_BUFF * sizeof(char));
    memset(authenticationManager.hostName, 0, MAX_BUFF * sizeof(char));

    authenticationManager.sasHandle = NULL;
    authenticationManager.certificate = NULL;
    authenticationManager.certificatePrivateKey = NULL;
    return (platform_init() == 0);
}

void AuthenticationManager_Deinit() {
    if (authenticationManager.sasHandle != NULL) {
        HTTPAPIEX_SAS_Destroy(authenticationManager.sasHandle);
        authenticationManager.sasHandle = NULL;
    }

    if (authenticationManager.certificate != NULL) {
        free(authenticationManager.certificate);
        authenticationManager.certificate = NULL;
    }

    if (authenticationManager.certificatePrivateKey != NULL) {
        free(authenticationManager.certificatePrivateKey);
        authenticationManager.certificatePrivateKey = NULL;
    }

    platform_deinit();
}

bool AuthenticationManager_InitFromSharedAccessKey(const char* filePath, char* hostName, char* deviceId) {
    char sharedAccessKey[KEY_SIZE] = "";
    if (FileUtils_ReadFile(filePath, &sharedAccessKey, KEY_SIZE, true) != FILE_UTILS_OK) {
        return false;
    }

    // omitting new line if exist
    char *newLinePosition = strchr(sharedAccessKey, '\n');
    if (newLinePosition != NULL) {
        *newLinePosition = '\0';
    }

    if (!AuthenticationManager_InitSharedProperties(hostName, deviceId)) {
        return false;
    }

    authenticationManager.sasHandle = HTTPAPIEX_SAS_Create_From_String(sharedAccessKey, authenticationManager.hostName, NULL);

    return authenticationManager.sasHandle != NULL;
}

bool AuthenticationManager_InitFromCertificate(const char* filePath, char* hostName, char* deviceId) {
    bool result = true;

    if (!CertificateManager_LoadFromFile(filePath, &authenticationManager.certificate, &authenticationManager.certificatePrivateKey)) {
        result = false;
        goto cleanup;
    }

    if (!AuthenticationManager_InitSharedProperties(hostName, deviceId)) {
        result = false;
        goto cleanup;
    }

cleanup:
    if (!result) {
        AuthenticationManager_Deinit();
    }

    return result;
}

static bool AuthenticationManager_ExtractConnectionString(char* responseJson, char* connectionString, uint32_t* connectionStringSize) {
    JsonObjectReaderHandle jsonReader = NULL;
    bool result = true;

    JsonReaderResult readerResult = JsonObjectReader_InitFromString(&jsonReader, responseJson);
    if (readerResult != JSON_READER_OK) {
        result = false;
        goto cleanup;
    }

    char* sharedAccessKey = NULL;
    readerResult = JsonObjectReader_ReadString(jsonReader, PRIMARY_KEY, &sharedAccessKey);
    if (readerResult != JSON_READER_OK) {
        result = false;
        goto cleanup;
    }

    if (!AuthenticationManager_GenerateConnectionStringFromSharedAccessKey(sharedAccessKey, connectionString, connectionStringSize, authenticationManager.hostName, authenticationManager.deviceId)) {
        result = false;
        goto cleanup;
    }

cleanup:
    if (jsonReader != NULL) {
        JsonObjectReader_Deinit(jsonReader);
    }

    return result;
}

static bool AuthenticationManager_GetConnectionStringUsingCertificate(char* connectionString, uint32_t* connectionStringSize) {
    bool result = true;
    bool httpApiInitialized = false;
    HTTP_HANDLE httpHandle = NULL;
    BUFFER_HANDLE responseContent = NULL;
    
    HTTP_HEADERS_HANDLE requestHttpHeaders = HTTPHeaders_Alloc();
    if (requestHttpHeaders == NULL) {
        result = false;
        goto cleanup;
    }

    HTTPAPI_RESULT httpResult = HTTPAPI_Init();
    if (httpResult != HTTPAPI_OK) {
        result = false;
        goto cleanup;
    }
    httpApiInitialized = true;

    unsigned int statusCode;
    httpHandle = HTTPAPI_CreateConnection(authenticationManager.hostName);
    if (httpHandle == NULL) {
        result = false;
        goto cleanup;
    }

    responseContent = BUFFER_new();
    if (responseContent == NULL) {
        result = false;
        goto cleanup;
    }

    if (HTTPAPI_SetOption(httpHandle, SU_OPTION_X509_CERT, authenticationManager.certificate) != HTTPAPI_OK) {
        result = false;
        goto cleanup;
    }

    if (HTTPAPI_SetOption(httpHandle, SU_OPTION_X509_PRIVATE_KEY, authenticationManager.certificatePrivateKey) != HTTPAPI_OK) {
        result = false;
        goto cleanup;
    }

    httpResult = HTTPAPI_ExecuteRequest( httpHandle, 
                                                        HTTPAPI_REQUEST_GET, 
                                                        authenticationManager.relativeUrl, 
                                                        requestHttpHeaders, 
                                                        NULL, 
                                                        0, 
                                                        &statusCode, 
                                                        NULL, 
                                                        responseContent);

    if (httpResult != HTTPAPI_OK || statusCode != 200) {
        result = false;
        goto cleanup;
    }

    char* resp_content;
    resp_content = (char*)BUFFER_u_char(responseContent);
    if (!AuthenticationManager_ExtractConnectionString(resp_content, connectionString, connectionStringSize)) {
        result = false;
        goto cleanup;
    }

cleanup:
    if (requestHttpHeaders != NULL) {
        HTTPHeaders_Free(requestHttpHeaders);
    }

    if (httpHandle != NULL) {
        HTTPAPI_CloseConnection(httpHandle);
    }

    if (httpApiInitialized) {
        HTTPAPI_Deinit();
    }

    if (responseContent != NULL) {
        BUFFER_delete(responseContent);
    }

    return result;
}


static bool AuthenticationManager_GetConnectionStringUsingSasToken(char* connectionString, uint32_t* connectionStringSize) {
    bool result = true;
    HTTPAPIEX_HANDLE httpExApiHandle = NULL;
    BUFFER_HANDLE responseContent = NULL;
    
    HTTP_HEADERS_HANDLE requestHttpHeaders = HTTPHeaders_Alloc();
    if (requestHttpHeaders == NULL) {
        result = false;
        goto cleanup;
    }

    if (HTTPHeaders_AddHeaderNameValuePair(requestHttpHeaders, "Authorization", "") != HTTP_HEADERS_OK) {
        result = false;
        goto cleanup;
    }

    unsigned int statusCode;
    httpExApiHandle = HTTPAPIEX_Create(authenticationManager.hostName);
    if (httpExApiHandle == NULL) {
        result = false;
        goto cleanup;
    }

    responseContent = BUFFER_new();
    if (responseContent == NULL) {
        result = false;
        goto cleanup;
    }

    HTTPAPIEX_RESULT httpResult = HTTPAPIEX_SAS_ExecuteRequest( authenticationManager.sasHandle, 
                                                                httpExApiHandle, 
                                                                HTTPAPI_REQUEST_GET, 
                                                                authenticationManager.relativeUrl, 
                                                                requestHttpHeaders, 
                                                                NULL, 
                                                                &statusCode, 
                                                                NULL, 
                                                                responseContent);

    if (httpResult != HTTPAPIEX_OK || statusCode != 200) {
        result = false;
        goto cleanup;
    }

    char* resp_content;
    resp_content = (char*)BUFFER_u_char(responseContent);
    if (!AuthenticationManager_ExtractConnectionString(resp_content, connectionString, connectionStringSize)) {
        result = false;
        goto cleanup;
    }

cleanup:

    if (requestHttpHeaders != NULL) {
        HTTPHeaders_Free(requestHttpHeaders);
    }

    if (httpExApiHandle != NULL) {
        HTTPAPIEX_Destroy(httpExApiHandle);
    }

    if (responseContent != NULL) {
        BUFFER_delete(responseContent);
    }

    return result;
}


bool AuthenticationManager_GetConnectionString(char* connectionString, uint32_t* connectionStringSize) {
    if (authenticationManager.sasHandle != NULL) {
        return AuthenticationManager_GetConnectionStringUsingSasToken(connectionString, connectionStringSize);
    } else if (authenticationManager.certificate != NULL) {
        return AuthenticationManager_GetConnectionStringUsingCertificate(connectionString, connectionStringSize); 
    }
    return false;
}

bool AuthenticationManager_GenerateConnectionStringFromSharedAccessKey(char* sharedAccessKey, char* connectionString, uint32_t* connectionStringSize, char* hostName, char* deviceId) {
    return Utils_ConcatenateToString(&connectionString, connectionStringSize, GENERATED_CONNECTION_STRING, hostName, deviceId, sharedAccessKey);
}

bool AuthenticationManager_InitSharedProperties(char* hostName, char* deviceId) {
    if (!Utils_CopyString(hostName, strlen(hostName)+1, authenticationManager.hostName, MAX_BUFF)) {
        return false;
    }

    if (!Utils_CopyString(deviceId, strlen(deviceId)+1, authenticationManager.deviceId, MAX_BUFF)) {
        return false;
    }

    uint32_t relativeUrl_size = MAX_BUFF;
    char* relativeUrl = authenticationManager.relativeUrl;
    if (!Utils_ConcatenateToString(&relativeUrl, &relativeUrl_size, SECURITY_MODULE_API, deviceId)) {
        return false;
    }

    return true;
}