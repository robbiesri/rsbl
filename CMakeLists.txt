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
    include(msvc-defaults)
endif ()

# Haven't thought about this much before, but Claude suggested it
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(external/doctest)
add_subdirectory(external/quill)

# Disable warnings for quill (external dependency)
if (TARGET quill)
    if (MSVC)
        target_compile_options(quill INTERFACE /W0)
    else ()
        target_compile_options(quill INTERFACE -w)
    endif ()
endif ()

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
