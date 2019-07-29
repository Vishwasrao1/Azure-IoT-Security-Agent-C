# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

include("../../cmake_config/utilityFunctions.cmake")
include_directories(../../agent/inc)
include_directories(../../${AZUREIOT_INC_FOLDER})
include_directories(../../azure-iot-sdk-c/deps/parson)
include_directories(../../azure-iot-sdk-c/c-utility/inc)

add_definitions(-DDISABLE_LOGS)

set(theseTestsName agent_int)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
    collectors_mocks.c
    iothub_adapter_mock.c
    local_config_mock.c
    test_defs.h
)

set(${theseTestsName}_c_files
    ../../agent/src/agent_telemetry_counters.c
    ../../agent/src/agent_telemetry_provider.c
    ../../agent/src/consts.c
    ../../agent/src/internal/internal_memory_monitor.c
    ../../agent/src/internal/time_utils.c
    ../../agent/src/json/json_array_reader.c
    ../../agent/src/json/json_array_writer.c
    ../../agent/src/json/json_object_reader.c
    ../../agent/src/json/json_object_writer.c
    ../../agent/src/json/json_reader.c
    ../../agent/src/message_schema_consts.c
    ../../agent/src/message_serializer.c
    ../../agent/src/queue.c
    ../../agent/src/scheduler_thread.c
    ../../agent/src/security_agent.c
    ../../agent/src/synchronized_memory_monitor.c
    ../../agent/src/synchronized_queue.c
    ../../agent/src/tasks/event_monitor_task.c
    ../../agent/src/tasks/event_publisher_task.c
    ../../agent/src/tasks/update_twin_task.c
    ../../agent/src/twin_configuration_consts.c
    ../../agent/src/twin_configuration_event_collectors.c
    ../../agent/src/twin_configuration.c
    ../../agent/src/twin_configuration_utils.c
    ../../agent/src/utils.c
    ../../agent/src/os_utils/linux/correlation_manager.c
    ../../agent/src/os_utils/linux/file_utils.c
    ../../agent/src/os_utils/linux/os_utils.c
    ../../agent/src/os_utils/linux/system_logger.c
)

set(${theseTestsName}_h_files
    ../../agent/inc/agent_telemetry_counters.h
    ../../agent/inc/agent_telemetry_provider.h
    ../../agent/inc/consts.h
    ../../agent/inc/internal/internal_memory_monitor.h
    ../../agent/inc/internal/time_utils.h
    ../../agent/inc/iothub_adapter.h
    ../../agent/inc/json/json_array_reader.h
    ../../agent/inc/json/json_array_writer.h
    ../../agent/inc/json/json_defs.h
    ../../agent/inc/json/json_object_reader.h
    ../../agent/inc/json/json_object_writer.h
    ../../agent/inc/local_config.h
    ../../agent/inc/logger.h
    ../../agent/inc/memory_monitor.h
    ../../agent/inc/message_schema_consts.h
    ../../agent/inc/message_serializer.h
    ../../agent/inc/queue.h
    ../../agent/inc/scheduler_thread.h
    ../../agent/inc/security_agent.h
    ../../agent/inc/synchronized_queue.h
    ../../agent/inc/tasks/event_monitor_task.h
    ../../agent/inc/tasks/event_publisher_task.h
    ../../agent/inc/tasks/update_twin_task.h
    ../../agent/inc/twin_configuration_consts.h
    ../../agent/inc/twin_configuration_defs.h
    ../../agent/inc/twin_configuration_event_collectors.h
    ../../agent/inc/twin_configuration.h
    ../../agent/inc/twin_configuration_utils.h
    ../../agent/inc/utils.h

    ../../agent/inc/os_utils/correlation_manager.h
    ../../agent/inc/os_utils/file_utils.h
    ../../agent/inc/os_utils/os_utils.h
)

umockc_build_test_artifacts(${theseTestsName} ON)


add_custom_command(
        TARGET agent_int_exe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/tests/agent_int/LocalConfiguration.json
                ${CMAKE_BINARY_DIR}/tests/agent_int/LocalConfiguration.json)

