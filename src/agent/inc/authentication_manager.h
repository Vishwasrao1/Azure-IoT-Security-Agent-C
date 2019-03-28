// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AUTHENTICATION_MANAGER_H
#define AUTHENTICATION_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "macro_utils.h"
#include "umock_c_prod.h"

/**
 * @brief   initializes the authentication manager using using a shared access key.
 * 
 * @param   filePath            the file containning the shared access key that is connected to the device.
 * @param   hostname            the host name of the hub.
 * @param   deviceId            the device id.
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, AuthenticationManager_InitFromSharedAccessKey, const char*, filePath, char*, hostname, char*, deviceId);

/**
 * @brief   initializes the authentication manager using using a self signed certificate.
 * 
 * @param   filePath            the file containning both the certificate and the private key that is connected to the device.
 * @param   hostname            the host name of the hub.
 * @param   deviceId            the device id.
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, AuthenticationManager_InitFromCertificate, const char*, filePath, char*, hostname, char*, deviceId);

/**
 * @brief   retreives the connection string of the the device's security module.
 * 
 * @param   connectionString        out param. the returned conneciton string.
 * @param   connectionStringSize    out param. the returned conneciton string size.
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, AuthenticationManager_GetConnectionString, char*, connectionString, uint32_t*, connectionStringSize);

/**
 * @brief   initializes the authentication manager.
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, AuthenticationManager_Init);

/**
 * @brief   deinitializes the authentication manager.
 */
MOCKABLE_FUNCTION(, void, AuthenticationManager_Deinit);

/**
 * @brief   generates a connection string according.
 * 
 * @param   sharedAccessKey         a shared access key.
 * @param   connectionString        out param. the returned conneciton string.
 * @param   connectionStringSize    out param. the returned conneciton string size.
 * @param   hostName                the device id.
 * @param   deviceId                the hub name.
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, AuthenticationManager_GenerateConnectionStringFromSharedAccessKey, char*, sharedAccessKey, char*, connectionString, uint32_t*, connectionStringSize, char*, hostName, char*, deviceId);

#endif //AUTHENTICATION_MANAGER_H