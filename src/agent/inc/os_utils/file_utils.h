// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum _FileResults {

    FILE_UTILS_OK,
    FILE_UTILS_FILE_NOT_FOUND,
    FILE_UTILS_SIZE_MISMATCH,
    FILE_UTILS_ERROR,
    FILE_UTILS_NOPERM

} FileResults;

/**
 * @brief Truncates the file and write the no data to the file.
 * 
 * @param   filename    The path of the file to write the data to.
 * @param   data        The data to write to the file.
 * @param   dataSize    The size of the data to write.
 * 
 * @return FILE_UTILS_OK in case of success or the appropriate error in case of failure.
 */
MOCKABLE_FUNCTION(, FileResults, FileUtils_WriteToFile, const char*, filename, const void*, data, uint32_t, dataSize);

/**
 * @brief Read the data from the file and assign it to the data param.
 * 
 * @param   filename    The path of the file to read from.
 * @param   data        Out param. Will contain the data that was read from the file.
 * @param   readCount   Amount of bytes to read.
 * @param   maxCount    indicates whther the amount is maximal or accurate.
 * 
 * @return FILE_UTILS_OK in case of success or the appropriate error in case of failure.
 */
MOCKABLE_FUNCTION(, FileResults, FileUtils_ReadFile, const char*, filename, void*, data, uint32_t, readCount, bool, maxCount);

/**
 * @brief Opens a file 
 * 
 * @param   filename    The path of the file to open
 * @param   mode        Flags for opening the file (i.e "r", "rw", "rb" etc...)
 * @param   outFile     Out param. the opened file handler
 * 
 * @return FILE_UTILS_OK in case of success or the appropriate error in case of failure.
 */
MOCKABLE_FUNCTION(, FileResults, FileUtils_OpenFile, const char*, filename, const char*, mode, FILE**, outFile);

#endif //FILE_UTILS_H