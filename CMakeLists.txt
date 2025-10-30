# Copyright 2025 Robert Srinivasiah 
# Licensed under the MIT License, see the LICENSE file for more info

cmake_minimum_required(VERSION 3.28)

project(rsbl)

# TODO: check for 32-bit?
# TODO: do I care about CMAKE_CONFIGURATION_TYPES? Modern tooling makes it simpler to manage the configs in the IDE

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

add_subdirectory(external/doctest)
add_subdirectory(external/quill)

add_subdirectory(libraries)
add_subdirectory(apps)
