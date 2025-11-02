# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

set(LIB_NAME rsbl-platform)

list(APPEND PUBLIC_HEADER_FILES
        include/rsbl-file.h
)

if (WIN32)
    list(APPEND PRIVATE_SOURCE_FILES
            win32/rsbl-win-file.cpp
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
