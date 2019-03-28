# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

include("../../cmake_config/utilityFunctions.cmake")

include_directories(../../agent/inc)
include_directories(../../azure-iot-sdk-c/deps/parson)
add_definitions(-DDISABLE_LOGS)

set(theseTestsName json_object_reader_ut)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../agent/src/json/json_object_reader.c
    ../../agent/src/consts.c
    ../../agent/src/internal/time_utils.c
    ../../agent/src/os_utils/linux/os_utils.c
    ../../azure-iot-sdk-c/deps/parson/parson.c
)

set(${theseTestsName}_h_files
    ../../azure-iot-sdk-c/deps/parson/parson.h
    ../../agent/inc/internal/time_utils.h

)

umockc_build_test_artifacts(${theseTestsName} ON)

add_custom_command(
        TARGET json_object_reader_ut_exe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/tests/json_object_reader_ut/json_reader_test.json
                ${CMAKE_BINARY_DIR}/tests/json_object_reader_ut/json_reader_test.json)
