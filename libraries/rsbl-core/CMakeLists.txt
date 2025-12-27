# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

list(APPEND PUBLIC_HEADER_FILES
        include/rsbl-assert.h
        include/rsbl-core.h
        include/rsbl-dynamic-array.h
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
rsbl_add_test(
        NAME rsbl-result-test
        SOURCES rsbl-result.test.cpp
        LIBRARIES rsbl-core
)

rsbl_add_test(
        NAME rsbl-dynamic-array-test
        SOURCES rsbl-dynamic-array.test.cpp
        LIBRARIES rsbl-core
)

rsbl_add_test(
        NAME rsbl-math-test
        SOURCES rsbl-math.test.cpp
        LIBRARIES rsbl-core
)

rsbl_add_test(
        NAME rsbl-ptr-test
        SOURCES rsbl-ptr.test.cpp
        LIBRARIES rsbl-core
)