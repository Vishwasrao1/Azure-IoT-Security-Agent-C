# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

include("../../cmake_config/utilityFunctions.cmake")

include_directories(../../agent/inc)

set(theseTestsName certificate_manager_ut)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../agent/src/certificate_manager.c
)

set(${theseTestsName}_h_files
    openssl_mock.h
    std_mocks.h
)

umockc_build_test_artifacts(${theseTestsName} ON)