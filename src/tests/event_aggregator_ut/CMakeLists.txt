# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

include("../../cmake_config/utilityFunctions.cmake")
include_directories(../../agent/inc)
include_directories(../../azure-iot-sdk-c)
include_directories(../../azure-iot-sdk-c/deps/parson)
include_directories(../../azure-iot-sdk-c/c-utility/inc)

add_definitions(-DDISABLE_LOGS)

set(theseTestsName event_aggregator_ut)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../agent/src/collectors/event_aggregator.c
    ../../agent/src/internal/time_utils.c
    ../../agent/src/utils.c
    ../../agent/src/collectors/linux/generic_event.c
    ../../agent/src/json/json_array_writer.c
    ../../agent/src/json/json_object_writer.c
    ../../azure-iot-sdk-c/deps/parson/parson.c
    ../../agent/src/twin_configuration_consts.c
    ../../agent/src/message_schema_consts.c
)

umockc_build_test_artifacts(${theseTestsName} ON)