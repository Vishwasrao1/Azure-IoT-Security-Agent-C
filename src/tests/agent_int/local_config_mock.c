// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "local_config.h"

LocalConfigurationResultValues LocalConfiguration_Init() {
    return LOCAL_CONFIGURATION_OK;
}

void LocalConfiguration_Deinit() {
    return;
}

const char* LocalConfiguration_GetConnectionString() {
    return "ConnctionStringStub";
}

const char* LocalConfiguration_GetAgentId() {
    return "7aaeef0e-614f-4ff2-97d2-1442186f73fa";
}

uint32_t LocalConfiguration_GetTriggeredEventInterval() {
    return 1000;
}

const char* LocalConfiguration_GetRemoteConfigurationObjectName() {
    return "ms_iotn:urn_azureiot_Security_SecurityAgentConfiguration";
}