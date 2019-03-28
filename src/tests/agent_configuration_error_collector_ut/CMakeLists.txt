# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

include("../../cmake_config/utilityFunctions.cmake")
include_directories(../../agent/inc)
add_definitions(-DDISABLE_LOGS)

set(theseTestsName agent_configuration_error_collector_ut)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../agent/src/collectors/agent_configuration_error_collector.c
    ../../agent/src/message_schema_consts.c
    ../../agent/src/twin_configuration_consts.c
    ../../agent/src/utils.c
    ../../agent/src/consts.c
)

set(${theseTestsName}_h_files
    ../../agent/inc/collectors/agent_configuration_error_collector.h
    ../../agent/inc/twin_configuration_consts.h
    ../../agent/inc/twin_configuration_defs.h
    ../../agent/inc/utils.h
    ../../agent/inc/consts.h

)
umockc_build_test_artifacts(${theseTestsName} ON)   