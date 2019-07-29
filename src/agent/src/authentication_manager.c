// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "authentication_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
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
#include "agent_errors.h"

typedef enum _AuthenticationMethod {
    SYMMETRIC_KEY,
    CERTIFICATE
} AuthenticationMethod;

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
static bool AuthenticationManager_GetConnectionStringFromHub(char* connectionString, uint32_t* connectionStringSize, AuthenticationMethod method);

/**
 * @brief   tries to get the hub host name from the server using a self signed certificate.
 * 
 * @return true on success, false otherwise.
 */
static bool AuthenticationManager_GetHostNameFromDPSService(AuthenticationMethod method);

/**
 * @brief   extract the hub host name from the server's json response.
 * 
 * @param   responseJson            the file containning the shared access key that is connected to the device.
 * 
 * @return true on success, false otherwise.
 */
static bool AuthenticationManager_UpdateFromDpsResponse(const char* responseJson);

/**
 * @brief   Deinitialize DPS specific data
 */
static void AuthenticationManager_DeinitDpsDetails();

/**
 * @brief   Send HTTP request with certificate based authentication.
 * 
 * @param   requestType             the type of the request (GET, POST, etc).
 * @param   hostName                the host name to send the request.
 * @param   relativeUrl             the relative url.
 * @param   content                 the request content, can be NULL.
 * @param   contentLength           the request content length, should be 0 if content == NULL.
 * @param   statusCode              out param, the response status code.
 * @param   responseContent         out param, the response buffer, should be initialized by the caller.
 * 
 * @return HTTPAPI_RESULT code.
 */
static bool AuthenticationManager_HttpRequestUsingCertificate(HTTPAPI_REQUEST_TYPE requestType, const char* hostName, const char* relativeUrl, const unsigned char* content, size_t contentLength, unsigned int* statusCode, BUFFER_HANDLE responseContent);

/**
 * @brief   Send HTTP request with sas-token based authentication.
 * 
 * @param   requestType             the type of the request (GET, POST, etc).
 * @param   hostName                the host name to send the request.
 * @param   relativeUrl             the relative url.
 * @param   sasScope                the scope of the SAS token.
 * @param   sasKeyName              the key name of the SAS token.
 * @param   content                 the request content, can be NULL.
 * @param   contentLength           the request content length, should be 0 if content == NULL.
 * @param   statusCode              out param, the response status code.
 * @param   responseContent         out param, the response buffer, should be initialized by the caller.
 * 
 * @return HTTPAPIEX_RESULT code.
 */
static bool AuthenticationManager_HttpRequestUsingSasToken( HTTPAPI_REQUEST_TYPE requestType, const char* hostName, const char* relativeUrl, const char* sasScope, const char* sasKeyName,
                                                                        const unsigned char* content, size_t contentLength, unsigned int* statusCode, BUFFER_HANDLE responseContent);

#define MAX_BUFF 500
#define KEY_SIZE 300

bool AuthenticationManager_EnsureHttpSuccessStatusCode(uint32_t statusCode) {
    if (statusCode == 200) // 200 OK
        return true;
      
    if (statusCode == 401)
        AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_UNAUTHORIZED, "Validate authentication configuration");
    else if (statusCode == 404)
        AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_NOT_FOUND, "Validate authentication configuration");
    else 
        AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_OTHER, "Unexpected server response %u", statusCode);

    return false;
}

// IoTHub strings
static const char* SECURITY_MODULE_API = "/devices/%s/modules/azureiotsecurity?api-version=2018-06-30";
static const char* PRIMARY_KEY = "authentication.symmetricKey.primaryKey";
static const char* GENERATED_CONNECTION_STRING = "HostName=%s;DeviceId=%s;ModuleId=azureiotsecurity;SharedAccessKey=%s";

// DPS strings
static const char* GLOBAL_DPS_HOSTNAME = "global.azure-devices-provisioning.net";
static const char* REGISTRATION_API = "/%s/registrations/%s?api-version=2019-03-31";
static const char* SAS_TOKEN_SCOPE = "%s/registrations/%s";
static const char* REGISTRATION_BODY = "{\"registrationId\" : \"%s\"}";
static const char* RESPONSE_ASSIGNED_HUB = "assignedHub";
static const char* RESPONSE_DEVICE_ID = "deviceId";
static const char* RESPONSE_LAST_UPDATED = "lastUpdatedDateTimeUtc";
static const char* RESPONSE_STATUS = "status";
static const char* RESPONSE_STATUS_ASSIGNED = "assigned";
static const char* CONTENT_TYPE_HEADER = "Content-Type";
static const char* CONTENT_TYPE_JSON = "application/json";

typedef struct _authenticationManager {
    char relativeUrl[MAX_BUFF];
    char hostName[MAX_BUFF];
    char deviceId[MAX_BUFF];

    char* sharedAccessKey;
    char* certificate;
    char* certificatePrivateKey;

    // DPS properties
    char* dpsRelativeUrl;
    char* sasTokenScope;
    char* dpsRequestContent;
    char idScope[MAX_BUFF];
    char registrationId[MAX_BUFF];
    char lastDPSUpdateTime[MAX_BUFF];

} AuthenticationManager;

static AuthenticationManager authenticationManager;

bool AuthenticationManager_Init() {
    memset(authenticationManager.deviceId, 0, MAX_BUFF * sizeof(char));
    memset(authenticationManager.relativeUrl, 0, MAX_BUFF * sizeof(char));
    memset(authenticationManager.hostName, 0, MAX_BUFF * sizeof(char));

    authenticationManager.dpsRelativeUrl = NULL;
    authenticationManager.sasTokenScope = NULL;
    authenticationManager.dpsRequestContent = NULL;
    memset(authenticationManager.idScope, 0, MAX_BUFF * sizeof(char));
    memset(authenticationManager.registrationId, 0, MAX_BUFF * sizeof(char));
    memset(authenticationManager.lastDPSUpdateTime, 0, MAX_BUFF * sizeof(char));

    authenticationManager.sharedAccessKey = NULL;
    authenticationManager.certificate = NULL;
    authenticationManager.certificatePrivateKey = NULL;

    return true;
}

void AuthenticationManager_Deinit() {
    if (authenticationManager.certificate != NULL) {
        free(authenticationManager.certificate);
        authenticationManager.certificate = NULL;
    }

    if (authenticationManager.certificatePrivateKey != NULL) {
        free(authenticationManager.certificatePrivateKey);
        authenticationManager.certificatePrivateKey = NULL;
    }

    if (authenticationManager.sharedAccessKey != NULL) {
        free(authenticationManager.sharedAccessKey);
        authenticationManager.sharedAccessKey = NULL;
    }

    AuthenticationManager_DeinitDpsDetails();
}

void AuthenticationManager_DeinitDpsDetails() {
    if (authenticationManager.dpsRelativeUrl != NULL) {
        free(authenticationManager.dpsRelativeUrl);
        authenticationManager.dpsRelativeUrl = NULL;
    }

    if (authenticationManager.sasTokenScope != NULL) {
        free(authenticationManager.sasTokenScope);
        authenticationManager.sasTokenScope = NULL;
    }

    if (authenticationManager.dpsRequestContent != NULL) {
        free(authenticationManager.dpsRequestContent);
        authenticationManager.dpsRequestContent = NULL;
    }
}

bool AuthenticationManager_InitFromSharedAccessKey(const char* filePath, char* hostName, char* deviceId) {
    bool result = true;
    authenticationManager.sharedAccessKey = malloc(KEY_SIZE * sizeof(char));
    if (authenticationManager.sharedAccessKey == NULL) {
        result = false;
        goto cleanup;
    }

    memset(authenticationManager.sharedAccessKey, 0, KEY_SIZE * sizeof(char));

    FileResults fileResult = FileUtils_ReadFile(filePath, authenticationManager.sharedAccessKey, KEY_SIZE, true); 
    if (fileResult != FILE_UTILS_OK) {
        if (fileResult == FILE_UTILS_FILE_NOT_FOUND)
            AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_FILE_NOT_EXIST, "File not found in path: %s", filePath);
        else if (fileResult == FILE_UTILS_NOPERM)
            AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_FILE_PERMISSIONS, "Couldn't open file in path: %s, check permissions", filePath);
        else 
            AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_OTHER, "Unexpected error while opening file: %s", filePath);
        result = false;
        goto cleanup;
    }

    // omitting new line if exist
    char* newLinePosition = strchr(authenticationManager.sharedAccessKey, '\n');
    if (newLinePosition != NULL) {
        *newLinePosition = '\0';
    }

    if (!AuthenticationManager_InitSharedProperties(hostName, deviceId)) {
        result = false;
        goto cleanup;
    }

cleanup:
    if (!result && authenticationManager.sharedAccessKey != NULL) {
        free(authenticationManager.sharedAccessKey);
        authenticationManager.sharedAccessKey = NULL;
    }

    return result;
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

static bool AuthenticationManager_HttpRequestUsingCertificate(HTTPAPI_REQUEST_TYPE requestType, const char* hostName, const char* relativeUrl, const unsigned char* content, size_t contentLength, unsigned int* statusCode, BUFFER_HANDLE responseContent) {
    HTTPAPI_RESULT result = HTTPAPI_OK;
    bool httpApiInitialized = false;
    HTTP_HANDLE httpHandle = NULL;
    HTTP_HEADERS_HANDLE requestHttpHeaders = NULL;
    
    requestHttpHeaders = HTTPHeaders_Alloc();
    if (requestHttpHeaders == NULL) {
        result = HTTPAPI_ALLOC_FAILED;
        goto cleanup;
    }

    if (contentLength > 0 && content != NULL) {
        if (HTTPHeaders_AddHeaderNameValuePair(requestHttpHeaders, CONTENT_TYPE_HEADER, CONTENT_TYPE_JSON) != HTTP_HEADERS_OK) {
            result = HTTPAPI_HTTP_HEADERS_FAILED;
            goto cleanup;
        }
    }

    result = HTTPAPI_Init();
    if (result != HTTPAPI_OK) {
        goto cleanup;
    }
    httpApiInitialized = true;

    httpHandle = HTTPAPI_CreateConnection(hostName);
    if (httpHandle == NULL) {
        result = HTTPAPI_ERROR;
        goto cleanup;
    }

    result = HTTPAPI_SetOption(httpHandle, SU_OPTION_X509_CERT, authenticationManager.certificate);
    if (result != HTTPAPI_OK) {
        goto cleanup;
    }

    result = HTTPAPI_SetOption(httpHandle, SU_OPTION_X509_PRIVATE_KEY, authenticationManager.certificatePrivateKey);
    if (result != HTTPAPI_OK) {
        goto cleanup;
    }

    result = HTTPAPI_ExecuteRequest(httpHandle, 
                                    requestType, 
                                    relativeUrl, 
                                    requestHttpHeaders, 
                                    content, 
                                    contentLength, 
                                    statusCode, 
                                    NULL, 
                                    responseContent);

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

    return result == HTTPAPI_OK ? true : false;
}

static bool AuthenticationManager_HttpRequestUsingSasToken( HTTPAPI_REQUEST_TYPE requestType, const char* hostName, const char* relativeUrl, const char* sasScope, const char* sasKeyName,
                                                                        const unsigned char* content, size_t contentLength, unsigned int* statusCode, BUFFER_HANDLE responseContent) {
    HTTPAPIEX_RESULT result = HTTPAPIEX_OK;
    HTTPAPIEX_SAS_HANDLE sasHandle = NULL;
    HTTPAPIEX_HANDLE httpExApiHandle = NULL;
    BUFFER_HANDLE requestContent = NULL;
    
    HTTP_HEADERS_HANDLE requestHttpHeaders = HTTPHeaders_Alloc();
    if (requestHttpHeaders == NULL) {
        result = HTTPAPIEX_ERROR;
        goto cleanup;
    }

    if (HTTPHeaders_AddHeaderNameValuePair(requestHttpHeaders, "Authorization", "") != HTTP_HEADERS_OK) {
        result = HTTPAPIEX_ERROR;
        goto cleanup;
    }
    
    if (contentLength > 0 && content != NULL) {
        if (HTTPHeaders_AddHeaderNameValuePair(requestHttpHeaders, CONTENT_TYPE_HEADER, CONTENT_TYPE_JSON) != HTTP_HEADERS_OK) {
            Logger_Error("Failed setting Content-Type header");
            result = HTTPAPIEX_ERROR;
            goto cleanup;
        }

        requestContent = BUFFER_create(content, contentLength);
        if (requestContent == NULL) {
            Logger_Error("Failed creating content buffer");
            result = HTTPAPIEX_ERROR;
            goto cleanup;
        }
    }

    httpExApiHandle = HTTPAPIEX_Create(hostName);
    if (httpExApiHandle == NULL) {
        result = HTTPAPIEX_ERROR;
        goto cleanup;
    }

    sasHandle = HTTPAPIEX_SAS_Create_From_String(authenticationManager.sharedAccessKey, sasScope, sasKeyName);
    if (sasHandle == NULL) {
        Logger_Error("Could not create SAS handle");
        result = HTTPAPIEX_ERROR;
        goto cleanup;
    }

    result = HTTPAPIEX_SAS_ExecuteRequest(  sasHandle, 
                                            httpExApiHandle, 
                                            requestType, 
                                            relativeUrl, 
                                            requestHttpHeaders, 
                                            requestContent, 
                                            statusCode, 
                                            NULL, 
                                            responseContent);

cleanup:

    if (sasHandle != NULL) {
        HTTPAPIEX_SAS_Destroy(sasHandle);
    }

    if (requestHttpHeaders != NULL) {
        HTTPHeaders_Free(requestHttpHeaders);
    }

    if (httpExApiHandle != NULL) {
        HTTPAPIEX_Destroy(httpExApiHandle);
    }

    if (requestContent != NULL) {
        BUFFER_delete(requestContent);
    }

    return result == HTTPAPIEX_OK ? true : false;
}

static bool AuthenticationManager_GetConnectionStringFromHub(char* connectionString, uint32_t* connectionStringSize, AuthenticationMethod method) {
    bool result = true;
    BUFFER_HANDLE responseContent = NULL;
    unsigned int statusCode;
    
    responseContent = BUFFER_new();
    if (responseContent == NULL) {
        result = false;
        goto cleanup;
    }

    if (method == SYMMETRIC_KEY)
        result = AuthenticationManager_HttpRequestUsingSasToken(   HTTPAPI_REQUEST_GET,
                                                                        authenticationManager.hostName,
                                                                        authenticationManager.relativeUrl,
                                                                        authenticationManager.hostName,
                                                                        NULL,
                                                                        NULL,
                                                                        0,
                                                                        &statusCode,
                                                                        responseContent);
    else if (method == CERTIFICATE)
        result = AuthenticationManager_HttpRequestUsingCertificate(HTTPAPI_REQUEST_GET, 
                                                                        authenticationManager.hostName, 
                                                                        authenticationManager.relativeUrl, 
                                                                        NULL, 
                                                                        0, 
                                                                        &statusCode, 
                                                                        responseContent);
    else {
        Logger_Error("Unrecognized authentication method");
        result = false;
        goto cleanup;
    }

    if (!result) {
        Logger_Error("Failed to send Http GET request to IoTHub");
        result = false;
        goto cleanup;
    }

    char* resp_content;
    resp_content = (char*)BUFFER_u_char(responseContent);

    result = AuthenticationManager_EnsureHttpSuccessStatusCode(statusCode);
    if (!result) {
        Logger_Error("IoTHub response does not state success");
        Logger_Error("Hub reponse:\n%s", resp_content);
        goto cleanup;
    }

    if (!AuthenticationManager_ExtractConnectionString(resp_content, connectionString, connectionStringSize)) {
        result = false;
        goto cleanup;
    }

cleanup:

    if (responseContent != NULL) {
        BUFFER_delete(responseContent);
    }

    return result;
}

bool AuthenticationManager_GetConnectionString(char* connectionString, uint32_t* connectionStringSize) {
    if (authenticationManager.sharedAccessKey != NULL) {
        return AuthenticationManager_GetConnectionStringFromHub(connectionString, connectionStringSize, SYMMETRIC_KEY);
    } else if (authenticationManager.certificate != NULL) {
        return AuthenticationManager_GetConnectionStringFromHub(connectionString, connectionStringSize, CERTIFICATE);
    }
    return false;
}

static bool AuthenticationManager_GetHostNameFromDPSService(AuthenticationMethod method) {
    bool result = true;
    BUFFER_HANDLE responseContent = NULL;
    char* sasTokenScope = NULL;
    unsigned int statusCode;

    responseContent = BUFFER_new();
    if (responseContent == NULL) {
        result = false;
        goto cleanup;
    }

    size_t tokenScopeSize = strlen(SAS_TOKEN_SCOPE) + strlen(authenticationManager.registrationId) + strlen(authenticationManager.idScope) + 1;
    sasTokenScope = malloc(tokenScopeSize * sizeof(char));
    if (sasTokenScope == NULL) {
        Logger_Error("Failed allocating buffer for sasTokenScope");
        result = false;
        goto cleanup;
    }
    memset(sasTokenScope, 0, tokenScopeSize);
    snprintf(sasTokenScope, tokenScopeSize, SAS_TOKEN_SCOPE, authenticationManager.idScope, authenticationManager.registrationId);

    if (method == SYMMETRIC_KEY)
        result = AuthenticationManager_HttpRequestUsingSasToken(   HTTPAPI_REQUEST_POST,    
                                                                                    GLOBAL_DPS_HOSTNAME,
                                                                                    authenticationManager.dpsRelativeUrl,
                                                                                    sasTokenScope,
                                                                                    "",
                                                                                    (unsigned char*)authenticationManager.dpsRequestContent,
                                                                                    strlen(authenticationManager.dpsRequestContent)+1,
                                                                                    &statusCode,
                                                                                    responseContent);
    else if (method == CERTIFICATE)
        result = AuthenticationManager_HttpRequestUsingCertificate(  HTTPAPI_REQUEST_POST,
                                                                                    GLOBAL_DPS_HOSTNAME,
                                                                                    authenticationManager.dpsRelativeUrl,
                                                                                    (const unsigned char*)authenticationManager.dpsRequestContent,
                                                                                    strlen(authenticationManager.dpsRequestContent)+1,
                                                                                    &statusCode,
                                                                                    responseContent);
    else {
        Logger_Error("Unrecognized authentication method");
        result = false;
        goto cleanup;        
    }

    if (result != true) {
        AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_OTHER, "Failed to get registration data from DPS");
        goto cleanup;
    }

    char* resp_content;
    resp_content = (char*)BUFFER_u_char(responseContent);

    result = AuthenticationManager_EnsureHttpSuccessStatusCode(statusCode);
    if (result != true) {
        Logger_Error("DPS response does not state success");
        Logger_Error("DPS response : %s", resp_content);
        goto cleanup;
    }
    
    if (!AuthenticationManager_UpdateFromDpsResponse(resp_content)) {
        result = false;
        goto cleanup;
    }

cleanup:

    if (sasTokenScope != NULL) {
        free(sasTokenScope);
    }

    if (responseContent != NULL) {
        BUFFER_delete(responseContent);
    }

    return result;
}

bool AuthenticationManager_UpdateFromDpsResponse(const char* responseJson) {
    JsonObjectReaderHandle jsonReader = NULL;
    bool result = true;

    JsonReaderResult readerResult = JsonObjectReader_InitFromString(&jsonReader, responseJson);
    if (readerResult != JSON_READER_OK) {
        result = false;
        goto cleanup;
    }

    char* assignedHub = NULL;
    char* deviceId = NULL;
    char* lastUpdated = NULL;
    char* status = NULL;

    readerResult = JsonObjectReader_ReadString(jsonReader, RESPONSE_STATUS, &status);
    if (readerResult != JSON_READER_OK) {
        result = false;
        goto cleanup;
    }

    if (Utils_UnsafeAreStringsEqual(status, RESPONSE_STATUS_ASSIGNED, true)) {
        Logger_Debug("DPS status: %s", RESPONSE_STATUS_ASSIGNED);
    } else {
        AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_OTHER, "DPS status: %s", status);
        result = false;
        goto cleanup;
    }

    readerResult = JsonObjectReader_ReadString(jsonReader, RESPONSE_ASSIGNED_HUB, &assignedHub);
    if (readerResult != JSON_READER_OK) {
        result = false;
        goto cleanup;
    }

    if (!Utils_CopyString(assignedHub, strlen(assignedHub)+1, authenticationManager.hostName, MAX_BUFF)) {
        result = false;
        goto cleanup;
    }

    readerResult = JsonObjectReader_ReadString(jsonReader, RESPONSE_DEVICE_ID, &deviceId);
    if (readerResult != JSON_READER_OK) {
        result = false;
        goto cleanup;
    }

    if (!Utils_CopyString(deviceId, strlen(deviceId)+1, authenticationManager.deviceId, MAX_BUFF)) {
        result = false;
        goto cleanup;
    }

    readerResult = JsonObjectReader_ReadString(jsonReader, RESPONSE_LAST_UPDATED, &lastUpdated);
    if (readerResult != JSON_READER_OK) {
        result = false;
        goto cleanup;
    }

    // If the "last updated" field hasn't changed, there was no re-provisioning
    if (Utils_UnsafeAreStringsEqual(lastUpdated, authenticationManager.lastDPSUpdateTime, true)) {
        Logger_Information("Last DPS update time: %s", lastUpdated);
        result = false;
        goto cleanup;
    }

    if (!Utils_CopyString(lastUpdated, strlen(lastUpdated)+1, authenticationManager.lastDPSUpdateTime, MAX_BUFF)) {
        result = false;
        goto cleanup;
    }

    uint32_t relativeUrl_size = MAX_BUFF;
    char* relativeUrl = authenticationManager.relativeUrl;
    if (!Utils_ConcatenateToString(&relativeUrl, &relativeUrl_size, SECURITY_MODULE_API, deviceId)) {
        return false;
    }

cleanup:
    if (jsonReader != NULL) {
        JsonObjectReader_Deinit(jsonReader);
    }

    return result;
}

bool AuthenticationManager_GetHostNameFromDPS() {
    if (authenticationManager.sharedAccessKey != NULL) {
        return AuthenticationManager_GetHostNameFromDPSService(SYMMETRIC_KEY);
    } else if (authenticationManager.certificate != NULL) {
        return AuthenticationManager_GetHostNameFromDPSService(CERTIFICATE); 
    }

    return false;
}

bool AuthenticationManager_SetDPSDetails(const char* idScope, const char* registrationId) {
    bool result = true;

    if (!Utils_CopyString(idScope, strlen(idScope)+1, authenticationManager.idScope, MAX_BUFF)) {
        result = false;
        goto cleanup;
    }

    if (!Utils_CopyString(registrationId, strlen(registrationId)+1, authenticationManager.registrationId, MAX_BUFF)) {
        result = false;
        goto cleanup;
    }

    size_t sharedSize = strlen(registrationId) + strlen(idScope) + 1;
    size_t dpsRelativeUrlSize = (strlen(REGISTRATION_API) + sharedSize) * sizeof(char);
    size_t sasTokenScopeSize = (strlen(SAS_TOKEN_SCOPE) + sharedSize) * sizeof(char);
    size_t dpsRequestContentSize = (strlen(REGISTRATION_BODY) + sharedSize) * sizeof(char);

    authenticationManager.dpsRelativeUrl = malloc(dpsRelativeUrlSize);
    authenticationManager.sasTokenScope = malloc(sasTokenScopeSize);
    authenticationManager.dpsRequestContent = malloc(dpsRequestContentSize);

    if (authenticationManager.dpsRelativeUrl == NULL ||
            authenticationManager.sasTokenScope == NULL ||
            authenticationManager.dpsRequestContent == NULL) {
        result = false;
        goto cleanup;
    }

    memset(authenticationManager.dpsRelativeUrl, 0, dpsRelativeUrlSize);
    memset(authenticationManager.sasTokenScope, 0, sasTokenScopeSize);
    memset(authenticationManager.dpsRequestContent, 0, dpsRequestContentSize);

    uint32_t buffSize = dpsRelativeUrlSize;
    char* dpsRelativeUrl = authenticationManager.dpsRelativeUrl;
    if (!Utils_ConcatenateToString(&dpsRelativeUrl, &buffSize, REGISTRATION_API, authenticationManager.idScope, authenticationManager.registrationId)) {
        result = false;
        goto cleanup;
    }

    if (authenticationManager.sharedAccessKey != NULL) {
        buffSize = sasTokenScopeSize;
        char* sasTokenScope = authenticationManager.sasTokenScope;
        if (!Utils_ConcatenateToString(&sasTokenScope, &buffSize, REGISTRATION_API, authenticationManager.idScope, authenticationManager.registrationId)) {
            result = false;
            goto cleanup;
        }
    }

    buffSize = dpsRequestContentSize;
    char* dpsRequestContent = authenticationManager.dpsRequestContent;
    if (!Utils_ConcatenateToString(&dpsRequestContent, &buffSize, REGISTRATION_BODY, authenticationManager.registrationId)) {
        result = false;
        goto cleanup;
    }

cleanup:
    if (!result) {
        AuthenticationManager_DeinitDpsDetails();
    }
    
    return result;
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

