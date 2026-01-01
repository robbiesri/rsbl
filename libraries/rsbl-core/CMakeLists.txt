# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

list(APPEND PUBLIC_HEADER_FILES
        include/rsbl-assert.h
        include/rsbl-core.h
        include/rsbl-dynamic-array.h
        include/rsbl-function.h
        include/rsbl-int-types.h
        include/rsbl-math-types.h
        include/rsbl-ptr.h
        include/rsbl-result.h
)

list(APPEND PRIVATE_SOURCE_FILES
        rsbl-assert.cpp
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

# Tests
rsbl_add_tests(
        SOURCES
        rsbl-result.test.cpp
        rsbl-dynamic-array.test.cpp
        rsbl-math.test.cpp
        rsbl-ptr.test.cpp
        rsbl-function.test.cpp
        rsbl-ref.test.cpp
        LIBRARIES rsbl-core
)