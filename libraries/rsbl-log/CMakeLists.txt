# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

list(APPEND PUBLIC_HEADER_FILES
        include/rsbl-log.h
)

list(APPEND PRIVATE_SOURCE_FILES
        rsbl-log.cpp
)

add_library(rsbl-log STATIC
        ${PUBLIC_HEADER_FILES}
        ${PRIVATE_SOURCE_FILES}
)

target_include_directories(rsbl-log
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link quill as a public dependency
target_link_libraries(rsbl-log
        PUBLIC
        quill::quill
)

# Tests
rsbl_add_tests(
        SOURCES
        rsbl-log.test.cpp
        LIBRARIES rsbl-log
)
