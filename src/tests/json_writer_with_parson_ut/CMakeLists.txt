# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

include("../../cmake_config/utilityFunctions.cmake")

include_directories(../../agent/inc)
include_directories(../../azure-iot-sdk-c/deps/parson)
add_definitions(-DDISABLE_LOGS)

set(theseTestsName json_writer_with_parson_ut)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../agent/src/json/json_object_writer.c
    ../../agent/src/json/json_array_writer.c
    ../../azure-iot-sdk-c/deps/parson/parson.c
)

set(${theseTestsName}_h_files
    ../../azure-iot-sdk-c/deps/parson/parson.h
)

umockc_build_test_artifacts(${theseTestsName} ON)