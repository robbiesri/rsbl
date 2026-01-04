# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

set(LIB_NAME rsbl-ga)

list(APPEND PUBLIC_HEADER_FILES
        include/rsbl-ga.h
)

list(APPEND PRIVATE_SOURCE_FILES
        rsbl-ga.cpp
        rsbl-ga-null.cpp
)

if (MSVC)
    list(APPEND PRIVATE_SOURCE_FILES
            rsbl-ga-dx12.cpp
    )
    # DX12 libraries will be linked below
endif ()

# Try to find Vulkan SDK
find_package(Vulkan QUIET)

# Add Vulkan backend only if SDK is found
if (Vulkan_FOUND)
    list(APPEND PRIVATE_SOURCE_FILES
            rsbl-ga-vulkan.cpp
    )
else()
    # Add stub implementation when Vulkan is not available
    list(APPEND PRIVATE_SOURCE_FILES
            rsbl-ga-vulkan-stub.cpp
    )
endif()

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
    target_include_directories(${LIB_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(${LIB_NAME} PRIVATE Vulkan::Vulkan)
else()
    message(STATUS "Vulkan SDK not found - Vulkan backend will not be available")
endif()

if (MSVC)
    # DX12 support
    message(STATUS "DX12 backend enabled for MSVC")
    target_link_libraries(${LIB_NAME} PRIVATE d3d12.lib dxgi.lib)

    # Override /Wall with /W3 for MSVC to reduce noise from Windows headers
    target_compile_options(${LIB_NAME} PRIVATE /W3)
endif ()