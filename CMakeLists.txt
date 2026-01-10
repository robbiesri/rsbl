# Copyright 2025 Robert Srinivasiah 
# Licensed under the MIT License, see the LICENSE file for more info

cmake_minimum_required(VERSION 3.28)

project(rsbl)

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "32-bit builds not supported")
endif ()

# Ignore MinSizeRel
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "" FORCE)

#if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
#    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
#endif()

list(APPEND CMAKE_MODULE_PATH
        ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if (MSVC)
    message(STATUS "MSVC compiler detected. Version: ${MSVC_VERSION}")
    message(STATUS "C++ Compiler path: ${CMAKE_CXX_COMPILER}")
    message(STATUS "C Compiler path: ${CMAKE_C_COMPILER}")
    message(STATUS "Linker path: ${CMAKE_LINKER}")
    include(msvc-defaults)

    # Display Windows SDK version
    if (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        message(STATUS "Windows SDK Version: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
    endif ()
endif ()

# Haven't thought about this much before, but Claude suggested it
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(external/doctest)
add_subdirectory(external/quill)

# Treat quill headers as system headers to suppress warnings
if (TARGET quill)
    get_target_property(QUILL_INCLUDE_DIRS quill INTERFACE_INCLUDE_DIRECTORIES)
    set_target_properties(quill PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${QUILL_INCLUDE_DIRS}"
    )
endif()

# TODO : control
enable_testing()

# Include test helpers
include(test-helpers)

add_subdirectory(libraries)

# app specific dependencies
add_subdirectory(external/fastgltf)

# Disable warnings for fastgltf (external dependency)
if (TARGET fastgltf)
    if (MSVC)
        target_compile_options(fastgltf PRIVATE /W0)
    else ()
        target_compile_options(fastgltf PRIVATE -w)
    endif ()
endif ()

add_subdirectory(apps)
