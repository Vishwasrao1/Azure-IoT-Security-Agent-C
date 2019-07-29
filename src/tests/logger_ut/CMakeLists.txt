# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

include("../../cmake_config/utilityFunctions.cmake")
include_directories(../../agent/inc)

set(theseTestsName logger_ut)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../agent/src/logger.c
)

remove_definitions(-DTEST_LOG)
remove_definitions(-DDISABLE_LOGS)

umockc_build_test_artifacts(${theseTestsName} ON)