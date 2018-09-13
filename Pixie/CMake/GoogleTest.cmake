#[[
 enable_google_test(<path_to_googletest>)

 Integrates Google Test framework that reside in <path_to_googletest> into the project.
 It does this by setting all necessary parameters, enabling testing, including and
 performing the linking to the library so that that any file in the project is ready
 to include any of the google test headers.
#]]
macro(enable_google_test GOOGLE_TEST_DIR)
    enable_testing()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        # Force this option to ON so that Google Tests will use /MD instead of /MT
        # /MD is now the default for Visual Studio, so it should be our default, too
        option(gtest_force_shared_crt "USE shared (DLL) run-time lib even when Google Test is built as static lib." ON)
        # set(THREAD_LIBRARY -pthread)
        set(THREAD_LIBRARY "")
    else()
        set(THREAD_LIBRARY pthread)
    endif()
    
    if(APPLE)
        add_definitions(-DGTEST_USE_OWN_TR1_TUPLE=1)
    endif()
    
    add_subdirectory(${GOOGLE_TEST_DIR}
                     ${CMAKE_BINARY_DIR}/googletest
                     EXCLUDE_FROM_ALL)
    
    # Turn off strict warning check for gtest as those warnings do not concern us.
    set_property(TARGET gtest APPEND_STRING PROPERTY COMPILE_FLAGS " -w")
    
    include_directories(SYSTEM
            ${GOOGLE_TEST_DIR}/googlemock/include
            ${GOOGLE_TEST_DIR}/googletest/include)
endmacro()



#[[
 add_google_test(<test_target> <user_library> <sources>...)

 Adds a Google Mock based test executable, <test>, built from <sources> and
 addds the test so that CTest will run it. Both the executable and test will be
 named <target>
#]]
function(add_google_test test_target user_library)
    # build the next to user library (for windows dll compatibility)
    get_target_property(TEST_WORKING_DIRECTORY ${user_library} RUNTIME_OUTPUT_DIRECTORY)
   
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
            set(TEST_WORKING_DIRECTORY "${TEST_WORKING_DIRECTORY}/Debug")
        elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
            set(TEST_WORKING_DIRECTORY "${TEST_WORKING_DIRECTORY}/Release")    
        endif()
    # else()
    #     set(TEST_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()

    # Add the test target as an executable and set it up its properties
    add_executable(${test_target} ${ARGN})
    target_link_libraries(${test_target} PRIVATE ${user_library} gtest gtest_main gmock_main ${THREAD_LIBRARY})
    set_target_properties(${test_target} 
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${TEST_WORKING_DIRECTORY}")

    add_test(NAME ${test_target}} COMMAND $<TARGET_FILE:${test_target}>)

    if(PIXIE_RUN_UNIT_TESTS)
        # Runs the tests automatically at build time
        add_custom_command(TARGET ${test_target}
                           POST_BUILD
                           COMMAND ${test_target}
                           WORKING_DIRECTORY ${TEST_WORKING_DIRECTORY}
                           COMMENT "Running ${target}" VERBATIM)
    endif()
endfunction()