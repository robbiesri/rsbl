# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

set(LIB_NAME rsbl-platform)

list(APPEND PUBLIC_HEADER_FILES
        include/rsbl-file.h
        include/rsbl-platform.h
        include/rsbl-thread.h
        include/rsbl-window.h
)

if (MSVC)
    list(APPEND PRIVATE_SOURCE_FILES
            win32/rsbl-win-file.cpp
            win32/rsbl-win-platform.cpp
            win32/rsbl-win-thread.cpp
            win32/rsbl-win-window.cpp
    )
endif ()

add_library(${LIB_NAME} STATIC
        ${PUBLIC_HEADER_FILES}
        ${PRIVATE_SOURCE_FILES}
)

target_include_directories(${LIB_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${LIB_NAME}
        PUBLIC
        rsbl-core
)

if (MSVC)
    # Override /Wall with /W3 for MSVC to reduce noise from Windows headers
    target_compile_options(${LIB_NAME} PRIVATE /W3)
endif ()

# Tests
rsbl_add_tests(
        SOURCES
        rsbl-thread.test.cpp
        LIBRARIES ${LIB_NAME}
)
