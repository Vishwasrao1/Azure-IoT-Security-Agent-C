# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

include("../../cmake_config/utilityFunctions.cmake")
include_directories(../../agent/inc)
include_directories(../../azure-iot-sdk-c/deps/parson)
add_definitions(-DDISABLE_LOGS)

set(theseTestsName twin_configuration_utils_ut)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../agent/src/json/json_array_writer.c
    ../../agent/src/consts.c
    ../../agent/src/internal/time_utils.c
    ../../agent/src/json/json_array_reader.c
    ../../agent/src/json/json_object_reader.c
    ../../agent/src/json/json_object_writer.c
    ../../agent/src/json/json_reader.c
    ../../agent/src/os_utils/linux/os_utils.c
    ../../agent/src/twin_configuration_utils.c
    ../../agent/src/utils.c
    ../../azure-iot-sdk-c/deps/parson/parson.c

)

umockc_build_test_artifacts(${theseTestsName} ON)