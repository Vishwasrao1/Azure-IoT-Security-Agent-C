# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

include("../../cmake_config/utilityFunctions.cmake")
include_directories(../../agent/inc)
add_definitions(-DDISABLE_LOGS)

set(theseTestsName twin_configuration_event_collectors_ut)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../agent/src/twin_configuration_consts.c
    ../../agent/src/twin_configuration_event_collectors.c
    ../../agent/src/utils.c
)

set(${theseTestsName}_h_files
    ../../agent/inc/json/json_object_writer.h
    ../../agent/inc/twin_configuration_consts.h
)

umockc_build_test_artifacts(${theseTestsName} ON)