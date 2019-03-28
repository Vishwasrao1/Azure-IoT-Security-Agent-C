# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

cmake_minimum_required(VERSION 2.8.11)

project(azure_iot_security_agent)

include("cmake_config/agentRules.cmake")

if (POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

#
#making a global variable to know if we are on linux, windows, or macosx.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
    # on Linux, enable valgrind
    # these commands (MEMORYCHECK...) need to apear BEFORE include(CTest) or they will not have any effect
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --error-exitcode=1")
endif()

option(run_valgrind "set run_valgrind to ON if tests are to be run under valgrind/helgrind/drd. Default is OFF" OFF)
option(run_unittests "set run_unittests to ON to run unittests (default is OFF)" OFF)
option(run_int_tests "set run_int_tests to ON to run integration (default is OFF)" OFF)
option(disable_logs "set disable_logs to ON to disable all logs (default is OFF)" OFF)

# Make warning as error
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall")
set(original_cmake_flags ${CMAKE_C_FLAGS})

# build ths sdk
# do not add or build any tests of the dependencies
set(use_amqp ON CACHE BOOL "enable amqp")
set(use_http ON CACHE BOOL "enable http")
set(use_mqtt OFF CACHE BOOL "disable mqtt")
set(skip_samples ON CACHE BOOL "disable samples")
set(build_service_client OFF CACHE BOOL "disable service client")
set(build_provisioning_service_client OFF CACHE BOOL "disable provisioning service client")
set(build_python OFF CACHE BOOL "disable python build")
set(CMAKE_C_FLAGS " -Wno-unused-variable")

set(original_run_unittests ${run_unittests})
set(run_unittests OFF)

add_subdirectory(azure-iot-sdk-c)

set(run_unittests ${original_run_unittests})
 #finish building the sdk

set(AZUREIOT_INC_FOLDER "azure-iot-sdk-c/iothub_client/inc")
set(CMAKE_C_FLAGS " -Wno-unused-variable")

add_subdirectory(umock-c)

# undo set(CMAKE_C_FLAGS " -Wno-unused-variable")
set(CMAKE_C_FLAGS ${original_cmake_flags})

if (${disable_logs})
    add_definitions(-DDISABLE_LOGS)
endif()

add_subdirectory(agent)

enable_testing()

if ( (${run_unittests})
      OR ( ${run_int_tests})
   )
    if (NOT ${disable_logs})
        add_definitions(-DTEST_LOG)
    endif()
    
    set(CMAKE_C_FLAGS " -Wno-unused-variable")
    add_subdirectory(tests)
    remove_definitions(-DTEST_LOG)
endif()

remove_definitions(-DDISABLE_LOGS)

# Set CMAKE_INSTALL_LIBDIR if not defined
include(GNUInstallDirs)
