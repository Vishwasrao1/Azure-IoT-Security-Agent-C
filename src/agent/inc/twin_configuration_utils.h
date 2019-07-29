// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TWIN_CONFIGURATION_UTILS_H
#define TWIN_CONFIGURATION_UTILS_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "twin_configuration_defs.h"
#include "json/json_object_reader.h"
#include "json/json_object_writer.h"

/**
 * @brief Reads PnP formatted time configuration from twin json reader in PnP 
 * 
 * @param reader        Twin json reader
 * @param key           The key to read
 * @param value         Out param. the value of the configuration
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationUtils_GetConfigurationTimeValueFromJson, JsonObjectReaderHandle, reader, const char*, key, uint32_t*, value);

/**
 * @brief Reads PnP formatted uint configuration from twin json reader in PnP 
 * 
 * @param reader        Twin json reader
 * @param key           The key to read
 * @param value         Out param. the value of the configuration
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationUtils_GetConfigurationUintValueFromJson, JsonObjectReaderHandle, reader, const char*, key, uint32_t*, value);

/**
 * @brief Reads PnP formatted string configuration from twin json reader in PnP 
 * 
 * @param reader        Twin json reader
 * @param key           The key to read
 * @param value         Out param. the value of the configuration
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationUtils_GetConfigurationStringValueFromJson, JsonObjectReaderHandle, reader, const char*, key, char**, value);

/**
 * @brief Reads PnP formatted bool configuration from twin json reader in PnP 
 * 
 * @param reader        Twin json reader
 * @param key           The key to read
 * @param value         Out param. the value of the configuration
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationUtils_GetConfigurationBoolValueFromJson, JsonObjectReaderHandle, reader, const char*, key, bool*, value);

/**
 * @brief writes PnP formatted time configuration to json writer
 * 
 * @param writer        Twin json writer
 * @param key           The key to write
 * @param value         the value of the configuration to write
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationUtils_WriteUintConfigurationToJson, JsonObjectWriterHandle, writer, const char*, key, uint32_t, value);

/**
 * @brief writes PnP formatted string configuration to json writer
 * 
 * @param writer        Twin json writer
 * @param key           The key to write
 * @param value         the value of the configuration to write
 * 
 * @return TWIN_OK on success or an error code upon failure
 */                  
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationUtils_WriteStringConfigurationToJson, JsonObjectWriterHandle, writer, const char*, key, const char*, value);

/**
 * @brief writes PnP formatted bool configuration to json writer
 * 
 * @param writer        Twin json writer
 * @param key           The key to write
 * @param value         the value of the configuration to write
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationUtils_WriteBoolConfigurationToJson, JsonObjectWriterHandle, writer, const char*, key, bool, value);

#endif //TWIN_CONFIGURATION_UTILS_H