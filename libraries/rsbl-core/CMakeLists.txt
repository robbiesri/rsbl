# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

list(APPEND PUBLIC_HEADER_FILES
        #        ${INCLUDE_DIR}/rsbl-assert.h
        include/rsbl-core.h
        include/rsbl-int-types.h
        include/rsbl-result.h
)

list(APPEND PRIVATE_SOURCE_FILES
        #        ${SRC_DIR}/rsbl-assert.cpp
        rsbl-result.cpp
)

add_library(rsbl-core STATIC
        ${PUBLIC_HEADER_FILES}
        ${PRIVATE_SOURCE_FILES}
)

target_include_directories(rsbl-core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# TODO: tests (starting with Result)