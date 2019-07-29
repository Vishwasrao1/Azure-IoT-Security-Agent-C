# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

include("../../cmake_config/utilityFunctions.cmake")

include_directories(../../agent/inc)
include_directories(../../${AZUREIOT_INC_FOLDER})
add_definitions(-DDISABLE_LOGS -DUSE_MQTT -UUSE_AMQP)

set(theseTestsName iothub_adapter_mqtt_ut)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../agent/src/agent_errors.c
    ../../agent/src/consts.c
    ../../agent/src/iothub_adapter.c
    ../../agent/src/message_schema_consts.c
    ../../agent/src/tasks/update_twin_task.c
)

umockc_build_test_artifacts(${theseTestsName} ON)