# Copyright 2025 Robert Srinivasiah 
# Licensed under the MIT License, see the LICENSE file for more info

cmake_minimum_required(VERSION 3.28)

project(rsbl)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "32-bit builds not supported")
endif()

# Ignore MinSizeRel
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "" FORCE)

#if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
#    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
#endif()

list(APPEND CMAKE_MODULE_PATH
        ${PROJECT_SOURCE_DIR}/cmake)

# TODO: enable C++23 support?
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# TODO: Do I care about enabling/disabling exception handling?
# TODO: warnings as errors?
# TODO: compiler specific bits? Disable specific errors? Move vendor specific stuff to modules?
# * gcc/clang - check out USD per-platform settings: https://github.com/PixarAnimationStudios/OpenUSD/tree/dev/cmake/

if (MSVC)
    include(msvc-defaults)
endif()

# Haven't thought about this much before, but Claude suggested it
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(external/doctest)
add_subdirectory(external/quill)

add_subdirectory(libraries)
add_subdirectory(apps)
