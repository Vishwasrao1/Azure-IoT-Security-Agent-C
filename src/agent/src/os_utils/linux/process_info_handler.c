// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/process_info_handler.h"

#include <unistd.h>

bool ProcessInfoHandler_SwitchRealAndEffectiveUsers() {
    if (setreuid(geteuid(), getuid()) < 0) {
        return false;
    }
    return true;
}

bool ProcessInfoHandler_ChangeToRoot(ProcessInfo* processInfo) {
    
    processInfo->effectiveUid = geteuid();
    processInfo->wasSet = false;
    if (processInfo->effectiveUid == 0) {
        return true;
    }
    
    if (seteuid(0) < 0) {
        return false;
    }
    processInfo->wasSet = true;

    return true;
}

bool ProcessInfoHandler_Reset(ProcessInfo* processInfo) {
    if (!processInfo->wasSet) {
        return true;
    }
    if (seteuid(processInfo->effectiveUid) < 0) {
        return false;
    }

    return true;
}
