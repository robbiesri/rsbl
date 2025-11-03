# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**rsbl** is a real-time rendering framework. This is a new project with minimal code currently in place.

## Build Environment Setup

This project uses local installations of Python 3.13.9, CMake 4.1.2, and Ninja 1.13.1 for development. All tools are managed locally in the `build_env/` directory.

### Initial Setup

Run the unified setup script to ensure all build tools are available:

```bash
./scripts/setup_build_env.sh
```

**Note:** On Windows, use Git Bash to run this script.

This script will:
- Check if Python 3.13.9 is already downloaded to `build_env/python/`
- Download and extract Python 3.13.9 if not present
- Check if CMake 4.1.2 is already downloaded to `build_env/cmake/`
- Download and extract CMake 4.1.2 if not present
- Check if Ninja 1.13.1 is already downloaded to `build_env/ninja/`
- Download and extract Ninja 1.13.1 if not present
- Check if CLI11 v2.6.1 is already downloaded to `external/CLI11/`
- Download CLI11.hpp if not present
- Verify all installations

### Downloading Sample Assets

The project includes sample assets (glTF models, etc.) for testing. Download them using:

```bash
./build_env/python/python.exe scripts/download_assets.py
```

Assets are defined in `asset_listing.toml` (tracked by git) and downloaded to `sample_assets/` (ignored by git).

The script supports:
- **glb files**: Single binary file download
- **gltf files**: Full directory download including textures and associated files

### Using Local Tools

After setup, use the local installations:

**Python:**
```powershell
.\build_env\python\python.exe your_script.py
```

**CMake:**
```powershell
.\build_env\cmake\cmake-4.1.2-windows-x86_64\bin\cmake.exe [args]
```

**Ninja:**
```powershell
.\build_env\ninja\ninja.exe [args]
```

## Building the Project

### Windows

Use the `setup_build.bat` script which handles Visual Studio environment setup, CMake configuration, and building:

```bash
./scripts/setup_build.bat
```

This script will:
1. Initialize the Visual Studio 2022 x64 environment (vcvars64.bat)
2. Configure CMake with the Ninja generator using local tools from `build_env/`
3. Build all targets

**Build output location:** `cmake-test-ninja/bin/`

**Note:** The script must be run from a fresh command prompt to ensure the Visual Studio environment is properly configured. Running it from Git Bash will work, but subsequent builds need the VS environment.

### Running Tests

After running `setup_build.bat`, test executables are in `cmake-test-ninja/bin/`:

```bash
# Run Result tests
./cmake-test-ninja/bin/rsbl-result-test.exe

# Run DynamicArray tests
./cmake-test-ninja/bin/rsbl-dynamic-array-test.exe
```

All tests use the doctest framework and will report pass/fail status.

## Repository Status

This repository is in its initial stages. The build environment setup is complete, ready for rendering framework development.

### Current Components

- **rsbl-core**: Core utilities including Result<T> for error handling and DynamicArray<T> for dynamic arrays
- **rsbl-platform**: Platform-specific implementations (currently Windows file I/O)
- **gltf-viewer**: Example application
- **Tests**: Comprehensive test suites for core components

### External Dependencies

- **doctest**: Testing framework (v2.4.12)
- **quill**: High-performance logging library
- **CLI11**: Command-line parsing library (v2.6.1, single-header at `external/CLI11/CLI11.hpp`)

To use CLI11 in your code:
```cpp
#include <CLI11/CLI11.hpp>
```

Then in CMakeLists.txt, add the include directory:
```cmake
target_include_directories(your_target PRIVATE ${CMAKE_SOURCE_DIR}/external)
```
