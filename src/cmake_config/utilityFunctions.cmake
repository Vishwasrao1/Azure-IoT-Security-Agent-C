#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

function(umockc_build_test_artifacts whatIsBuilding use_gballoc)
    #the first argument is what is building
    #the second argument is whether the tests should be build with gballoc #defines or not
    #the following arguments are a list of libraries to link with

    if(${use_gballoc})
        add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
    endif()

    #setting includes
    set(sharedutil_include_directories ${sharedutil_include_directories} ${UMOCK_C_INC_FOLDER} ${TESTRUNNERSWITCHER_INC_FOLDER} ${CTEST_INC_FOLDER} ${SHARED_UTIL_INC_FOLDER} ${SHARED_UTIL_SRC_FOLDER})
    include_directories(${sharedutil_include_directories})

    #setting output type
    if (("${whatIsBuilding}" MATCHES ".*int.*") AND ${run_int_tests})
        umockc_linux_tests_add_exe(${whatIsBuilding} TRUE ${ARGN})
    elseif (("${whatIsBuilding}" MATCHES ".*ut.*") AND ${run_unittests})
        umockc_linux_tests_add_exe(${whatIsBuilding} FALSE ${ARGN})
    endif()
endfunction()

function(umockc_linux_tests_add_exe whatIsBuilding isIntegrationTest)
    add_executable(${whatIsBuilding}_exe
        ${${whatIsBuilding}_test_files} 
        ${${whatIsBuilding}_h_files} 
        ${${whatIsBuilding}_c_files}
        ${CMAKE_CURRENT_LIST_DIR}/main.c
    )

    if (${isIntegrationTest})
        target_link_libraries(${whatIsBuilding}_exe
                            iothub_client_amqp_transport
                            uamqp
                            iothub_client
                            parson
                            aziotsharedutil
                            uuid
                            pthread
                            ssl
                            crypto
                            m
        )
    endif()

    target_compile_definitions(${whatIsBuilding}_exe PUBLIC -DUSE_CTEST)
    target_include_directories(${whatIsBuilding}_exe PUBLIC ${sharedutil_include_directories})
    target_link_libraries(${whatIsBuilding}_exe aziotsharedutil umock_c ctest testrunnerswitcher m ${ARGN})
    add_test(NAME ${whatIsBuilding} COMMAND ${whatIsBuilding}_exe)
    if(${isIntegrationTest})
        set_tests_properties(${whatIsBuilding} PROPERTIES COST 1.0)
    endif()

    if(${run_valgrind})
        find_program(VALGRIND_FOUND NAMES valgrind)
        if(${VALGRIND_FOUND} STREQUAL VALGRIND_FOUND-NOTFOUND)
            message(WARNING "run_valgrind was TRUE, but valgrind was not found - there will be no tests run under valgrind")
        else()
            add_test(NAME ${whatIsBuilding}_valgrind COMMAND valgrind                 --gen-suppressions=all --num-callers=100 --error-exitcode=1 --leak-check=full --track-origins=yes $<TARGET_FILE:${whatIsBuilding}_exe>)
            if(${isIntegrationTest})
                set_tests_properties(${whatIsBuilding}_valgrind PROPERTIES COST 1.0)
            endif()
        endif()
    endif()

endfunction()
