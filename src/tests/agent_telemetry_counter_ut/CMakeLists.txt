# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

include("../../cmake_config/utilityFunctions.cmake")
include_directories(../../agent/inc)
add_definitions(-DDISABLE_LOGS)

set(theseTestsName agent_telemetry_counter_ut)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../agent/src/agent_telemetry_counters.c
)

set(${theseTestsName}_h_files
    ../../azure-iot-sdk-c/c-utility/inc/azure_c_shared_utility/lock.h
)

umockc_build_test_artifacts(${theseTestsName} ON)   