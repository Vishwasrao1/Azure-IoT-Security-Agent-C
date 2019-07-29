// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/file_utils.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h> 
#include <unistd.h>

FileResults FileUtils_WriteToFile(const char* filename, const void* data, uint32_t dataSize) {
    FileResults result = FILE_UTILS_OK;
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        result = FILE_UTILS_ERROR;
        goto cleanup;
    }

    ssize_t bytesWritten = write(fd, data, dataSize);
    if (bytesWritten == -1) {
        result = FILE_UTILS_ERROR;
        goto cleanup;
    } else if (bytesWritten != dataSize) {
        result = FILE_UTILS_SIZE_MISMATCH;
        goto cleanup;
    }

cleanup:
    if (fd != -1) {
        close(fd);
    }   
    return result;
}

FileResults FileUtils_ReadFile(const char* filename, void* data, uint32_t readCount, bool maxCount) {
    FileResults result = FILE_UTILS_OK;
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        if (errno == ENOENT) {
            result = FILE_UTILS_FILE_NOT_FOUND;
        } else if (errno == EACCES) {
            result = FILE_UTILS_NOPERM;
        } else {
            result = FILE_UTILS_ERROR;
        }
        goto cleanup;
    }

    ssize_t bytesRead = read(fd, data, readCount);
    if (bytesRead == -1) {
        result = FILE_UTILS_ERROR;
        goto cleanup;
    } else if (maxCount) {
        result = (bytesRead <= readCount) ? FILE_UTILS_OK : FILE_UTILS_ERROR;
        goto cleanup;
    } else if (bytesRead != readCount) {
        result = FILE_UTILS_SIZE_MISMATCH;
        goto cleanup;
    }

cleanup:
    if (fd != -1) {
        close(fd);
    }   
    return result;

}

FileResults FileUtils_OpenFile(const char* filename, const char* mode, FILE** outFile) {
    FileResults result = FILE_UTILS_OK;
    *outFile = fopen(filename, mode);
    if (*outFile == NULL) {
        if (errno == ENOENT) {
            result = FILE_UTILS_FILE_NOT_FOUND;
        } else if (errno == EACCES) {
            result = FILE_UTILS_NOPERM;
        } else {
            result = FILE_UTILS_ERROR;
        }
    }

    return result;
}