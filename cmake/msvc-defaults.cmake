# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

# TODO: remaining MSVC settings to consider
#   * Standards compliant?
#   * Check for MSVC version?
#   * Do I want VC 2022?

# Enable all warnings
# TODO: maybe roll back to /W3 in the future?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")

# warnings as errors
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")

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