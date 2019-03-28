// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/process_utils.h"

#include <stdio.h>
#include <sys/wait.h>

#include "logger.h"

bool ProcessUtils_Execute(const char* command, char* output, uint32_t* outputSize) {
    bool success = true;
    FILE* commandStream = NULL;
    commandStream = popen(command, "r");
    if (commandStream == NULL) {
        success = false;
        goto cleanup;
    }
    
    clearerr(commandStream);
    uint32_t originalSize = *outputSize;
    *outputSize = fread(output, 1, *outputSize, commandStream);
    if (ferror(commandStream) != 0) {
        success = false;
        goto cleanup;
    }

    // there is more data to read
    if (originalSize == *outputSize && feof(commandStream) == 0) {
        success = false;
        goto cleanup;
    }

cleanup:
    if (commandStream != NULL) {
        int closeResult = pclose(commandStream);
        if (closeResult != 0) {
            Logger_Error("Excution of [%s] failed with return value of %d.", command, WEXITSTATUS(closeResult));
            success = false;
        }
    }

    return success;
}