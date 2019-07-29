# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

include("../../cmake_config/utilityFunctions.cmake")
include_directories(../../agent/inc)

set(theseTestsName authentication_manager_ut)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../agent/src/agent_errors.c
    ../../agent/src/authentication_manager.c
    ../../agent/src/consts.c
)

set(${theseTestsName}_h_files
    ../../agent/inc/agent_errors.h
)

umockc_build_test_artifacts(${theseTestsName} ON)
