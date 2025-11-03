# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

# TODO: remaining MSVC settings to consider
#   * Standards compliant?
#   * Check for MSVC version?
#   * Do I want VC 2022?
#   * disable exception handling?

# Enable almost all warnings. I tried /Wall, but there are too many stupid (and defaulted off) warnings from MSVC
# TODO: maybe roll back to /W3 in the future?
add_compile_options(/W4)

# warnings as errors
add_compile_options(/WX)

# generate separate PDBs
add_compile_options(/Zi)

# Enable multiprocess builds, and disable incremental (not compatible)
# Also forces synchronous PDB writes, implicit to /MP
add_compile_options(/MP)
add_compile_options(/Gm-)
add_compile_options(/FS)

# don't collide with std::min/max
add_compile_definitions(NOMINMAX)

# TODO: This might be a platform thing more than a MSVC thing, because it could be tied to clang-cl as well
add_compile_definitions(WIN32_LEAN_AND_MEAN)