// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>

#include "security_agent.h"

int main() {
    SecurityAgent agent;
    bool agentInitiated = false;

    if (!SecurityAgent_Init(&agent)) {
        goto cleanup;
    }
    agentInitiated = true;

    if (!SecurityAgent_Start(&agent)) {
        goto cleanup;
    }

    SecurityAgent_Wait(&agent);

cleanup:
    if (agentInitiated) {
        SecurityAgent_Deinit(&agent);
    }

    return 0;
}
