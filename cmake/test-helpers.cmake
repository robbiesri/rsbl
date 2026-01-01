# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

# Function to simplify creating tests
# Usage:
#   rsbl_add_test(
#       NAME test-name
#       SOURCES source1.cpp source2.cpp ...
#       LIBRARIES lib1 lib2 ...
#   )
function(rsbl_add_test)
    set(options "")
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES LIBRARIES)

    cmake_parse_arguments(TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Validate required arguments
    if(NOT TEST_NAME)
        message(FATAL_ERROR "rsbl_add_test: NAME argument is required")
    endif()

    if(NOT TEST_SOURCES)
        message(FATAL_ERROR "rsbl_add_test: SOURCES argument is required")
    endif()

    # Create the test executable
    add_executable(${TEST_NAME} ${TEST_SOURCES})

    # Link libraries (always include doctest)
    target_link_libraries(${TEST_NAME}
        PRIVATE
        doctest::doctest
        ${TEST_LIBRARIES}
    )

    # Register with CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# Function to create multiple single-file tests from a list
# Automatically derives test names from source file names
# Usage:
#   rsbl_add_tests(
#       SOURCES test1.test.cpp test2.test.cpp ...
#       LIBRARIES lib1 lib2 ...
#   )
function(rsbl_add_tests)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SOURCES LIBRARIES)

    cmake_parse_arguments(TESTS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Validate required arguments
    if(NOT TESTS_SOURCES)
        message(FATAL_ERROR "rsbl_add_tests: SOURCES argument is required")
    endif()

    # Iterate over each source file
    foreach(SOURCE_FILE ${TESTS_SOURCES})
        # Get filename with extension
        get_filename_component(FILE_NAME ${SOURCE_FILE} NAME)

        # Remove .cpp extension
        string(REGEX REPLACE "\\.cpp$" "" TEST_BASE_NAME ${FILE_NAME})

        # Create test name by replacing dots with dashes (e.g., rsbl-result.test -> rsbl-result-test)
        string(REPLACE "." "-" TEST_NAME ${TEST_BASE_NAME})

        # Create the test using the existing helper
        rsbl_add_test(
            NAME ${TEST_NAME}
            SOURCES ${SOURCE_FILE}
            LIBRARIES ${TESTS_LIBRARIES}
        )
    endforeach()
endfunction()
