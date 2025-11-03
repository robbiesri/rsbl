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
