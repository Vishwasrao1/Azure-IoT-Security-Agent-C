// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "twin_configuration_consts.h"

/* ===== Twin configuration Schema =====*/
const char* AGENT_CONFIGURATION_KEY = "azureiot*com^securityAgentConfiguration^1*0*0";
const char* LOW_PRIORITY_MESSAGE_FREQUENCY_KEY = "lowPriorityMessageFrequency";
const char* HIGH_PRIORITY_MESSAGE_FREQUENCY_KEY = "highPriorityMessageFrequency";
const char* MAX_LOCAL_CACHE_SIZE_KEY = "maxLocalCacheSizeInBytes";
const char* MAX_MESSAGE_SIZE_KEY = "maxMessageSizeInBytes";
const char* SNAPSHOT_FREQUENCY_KEY = "snapshotFrequency";
const char* HUB_RESOURCE_ID_KEY = "hubResourceId";
const char* EVENT_PROPERTIES_KEY = "eventPriorities";

/* ===== Event priorities schema =====*/

const char* PROCESS_CREATE_PRIORITY_KEY = "processCreate";
const char* LISTENING_PORTS_PRIORITY_KEY = "listeningPorts";
const char* SYSTEM_INFORMATION_PRIORITY_KEY = "systemInformation";
const char* LOCAL_USERS_PRIORITY_KEY = "localUsers";
const char* LOGIN_PRIORITY_KEY = "login";
const char* CONNECTION_CREATE_PRIORITY_KEY = "connectionCreate";
const char* FIREWALL_CONFIGURATION_PRIORITY_KEY = "firewallConfiguration";
const char* BASELINE_PRIORITY_KEY = "osBaseline";
const char* DIAGNOSTIC_PRIORITY_KEY = "diagnostic";
const char* OPERATIONAL_EVENT_KEY = "operational";