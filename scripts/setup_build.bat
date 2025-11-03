@echo off
REM Copyright 2025 Robert Srinivasiah
REM Licensed under the MIT License, see the LICENSE file for more info
REM Batch script to setup VS environment and configure CMake with Ninja

echo ============================================================
echo rsbl Build Setup (Windows)
echo ============================================================
echo.

REM Setup Visual Studio environment
echo Setting up Visual Studio 2022 x64 environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to setup Visual Studio environment
    exit /b 1
)
echo.

REM Get script directory and repo root
set SCRIPT_DIR=%~dp0
set REPO_ROOT=%SCRIPT_DIR%..
cd /d %REPO_ROOT%

REM Create build directory if it doesn't exist
if not exist cmake-test-ninja (
    echo Creating build directory: cmake-test-ninja
    mkdir cmake-test-ninja
)

REM Configure with CMake and Ninja
echo.
echo Configuring CMake with Ninja generator...
cd cmake-test-ninja
%REPO_ROOT%\build_env\cmake\cmake-4.1.2-windows-x86_64\bin\cmake.exe -G Ninja -DCMAKE_MAKE_PROGRAM=%REPO_ROOT%\build_env\ninja\ninja.exe ..
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed
    exit /b 1
)

REM Build all targets
echo.
echo Building all targets...
%REPO_ROOT%\build_env\cmake\cmake-4.1.2-windows-x86_64\bin\cmake.exe --build .
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed
    exit /b 1
)

echo.
echo ============================================================
echo Build complete!
echo ============================================================
echo.
echo Build directory: %REPO_ROOT%\cmake-test-ninja
echo.
echo Built executables are in: %REPO_ROOT%\cmake-test-ninja\bin
echo.
