// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TWIN_CONFIGURATION_DEFS_H
#define TWIN_CONFIGURATION_DEFS_H

/**
 * All the result types of the TwinConfiguration functions
 */
typedef enum _TwinConfigurationResult {
    TWIN_OK,
    TWIN_LOCK_EXCEPTION,
    TWIN_PARSE_EXCEPTION,
    TWIN_MEMORY_EXCEPTION,
    TWIN_EXCEPTION
} TwinConfigurationResult;

/**
 * reprsent the status of a twin configuration
 */
typedef enum _TwinConfigurationStatus {
    CONFIGURATION_OK,
    CONFIGURATION_TYPE_MISMATCH,
} TwinConfigurationStatus;

/**
 * reprsent the status of a twin configuration bundle
 */
typedef struct _TwinConfigurationBundleStatus {
    TwinConfigurationStatus maxLocalCacheSize;
    TwinConfigurationStatus maxMessageSize;
    TwinConfigurationStatus lowPriorityMessageFrequency;
    TwinConfigurationStatus highPriorityMessageFrequency;
    TwinConfigurationStatus snapshotFrequency;
    TwinConfigurationStatus eventPriorities;
 } TwinConfigurationBundleStatus;

#endif //TWIN_CONFIGURATION_DEFS_H
