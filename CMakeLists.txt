# Copyright 2025 Robert Srinivasiah 
# Licensed under the MIT License, see the LICENSE file for more info

cmake_minimum_required(VERSION 3.28)

project(rsbl)

# TODO: check for 32-bit?
# TODO: support CMAKE_MODULE_PATH
# TODO: enable C++23 support?
# TODO: do I care about CMAKE_CONFIGURATION_TYPES? Modern tooling makes it simpler to manage the configs in the IDE

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# TODO: Do I care about enabling/disabling exception handling?
# TODO: warnings as errors?
# TODO: compiler specific bits? Disable specific errors? Move vendor specific stuff to modules?
# * MSVC
#   * Standards compliant?
#   * Check for MSVC version?
#   * Do I want VC 2022?
#   * Disable minmax?
#   * Lean + mean!
#   * PDB gen every build?
# * gcc/clang - check out USD per-platform settings: https://github.com/PixarAnimationStudios/OpenUSD/tree/dev/cmake/

if (MSVC)
    # Enable all warnings
    # TODO: maybe roll back to /W3 in the future?
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")

    # warnings as errors
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wx")

    # generate separate PDBs
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")

    # Enable multiprocess builds, and disable incremental (not compatible)
    # Also forces synchronous PDB writes, implicit to /MP
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gm-")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS")

    # don't collide with std::min/max
    add_compile_definitions(NOMINMAX)

    add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()

add_subdirectory(external/doctest)
add_subdirectory(external/quill)

# add_subdirectory(libraries)
# add_subdirectory(apps)
