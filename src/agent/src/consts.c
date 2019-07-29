// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "consts.h"
#include "internal/time_utils_consts.h"

const char AGENT_NAME[] = "ASCIoTAgent";

const char AGENT_VERSION[] = "0.0.4";

const char DEFAULT_MESSAGE_SCHEMA_VERSION[] = "1.0";

uint32_t DEFAULT_MAX_LOCAL_CACHE_SIZE = 10 * 1024 * 1024; // 10MB

uint32_t DEFAULT_MAX_MESSAGE_SIZE = 200 * 1024;

uint32_t DEFAULT_LOW_PRIORITY_MESSAGE_FREQUENCY = 5 * MILLISECONDS_IN_AN_HOUR;

uint32_t DEFAULT_HIGH_PRIORITY_MESSAGE_FREQUENCY = 7 * MILLISECONDS_IN_A_MINUTE;

uint32_t DEFAULT_SNAPSHOT_FREQUENCY = 13 * MILLISECONDS_IN_AN_HOUR;

const uint32_t SCHEDULER_INTERVAL = 1 * 1000;

const uint32_t TWIN_UPDATE_SCHEDULER_INTERVAL = 10 * 1000;

const uint32_t MESSAGE_BILLING_MULTIPLE = 4 * 1024;

const char CONFIGURATION_FILE[] = "/LocalConfiguration.json";
