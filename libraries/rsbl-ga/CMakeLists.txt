# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

set(LIB_NAME rsbl-ga)

list(APPEND PUBLIC_HEADER_FILES
        include/rsbl-ga.h
)

list(APPEND PRIVATE_SOURCE_FILES
        src/rsbl-ga.cpp
        src/rsbl-ga-null.cpp
)

if (WIN32)
    list(APPEND PRIVATE_SOURCE_FILES
            src/dx12/rsbl-ga-dx12.cpp
    )
endif ()

# Vulkan backend (cross-platform when Vulkan SDK is available)
list(APPEND PRIVATE_SOURCE_FILES
        src/vulkan/rsbl-ga-vulkan.cpp
)

# Try to find Vulkan SDK
find_package(Vulkan QUIET)

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

# Enable Vulkan if SDK is found
if (Vulkan_FOUND)
    message(STATUS "Vulkan SDK found: ${Vulkan_INCLUDE_DIRS}")
    target_compile_definitions(${LIB_NAME} PRIVATE RSBL_VULKAN_ENABLED)
    target_include_directories(${LIB_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(${LIB_NAME} PRIVATE Vulkan::Vulkan)
else()
    message(STATUS "Vulkan SDK not found - Vulkan backend will not be available")
endif()

if (WIN32)
    # Override /Wall with /W3 for Windows implementation to reduce noise from Windows headers
    target_compile_options(${LIB_NAME} PRIVATE /W3)
endif ()