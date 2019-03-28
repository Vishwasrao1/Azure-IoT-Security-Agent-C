#include "os_utils/os_utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "azure_c_shared_utility/crt_abstractions.h"
#include "utils.h"

static int DEFAULT_BUFFER = 500;

char* GetExecutableDirectory() {
    char fullpath[DEFAULT_BUFFER];
    char* returnPath;
    size_t linkSize = readlink("/proc/self/exe", fullpath, sizeof(fullpath)-1);
    if (linkSize == -1) {
        return NULL;
    }
    fullpath[linkSize] = '\0';
    Utils_TrimLastOccurance(fullpath, '/');
    mallocAndStrcpy_s((char**)&returnPath, fullpath);
    return returnPath;
}

int OsUtils_GetProcessId() {
    return getpid();
}

unsigned int OsUtils_GetThreadId() {
    return pthread_self();
}