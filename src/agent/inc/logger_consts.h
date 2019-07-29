// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOGGER_CONSTS_H
#define LOGGER_CONSTS_H

// This enum should be kept aligned with the Severity array in os_utils/linux/system_logger.c
typedef enum _Severity {
    SEVERITY_DEBUG          = 0,
    SEVERITY_INFORMATION    = 1,
    SEVERITY_WARNING        = 2,
    SEVERITY_ERROR          = 3,
    SEVERITY_FATAL          = 4,

    SEVERITY_MAX            = 5
} Severity;

#endif // LOGGER_CONSTS_H