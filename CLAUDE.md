# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**rsbl** is a real-time rendering framework. This is a new project with minimal code currently in place.

## Build Environment Setup

This project uses local installations of Python 3.13.9 and CMake 4.1.2 for development. Both are managed locally in the `python_local/` and `cmake_local/` directories.

### Initial Setup

Run the unified setup script to ensure both Python and CMake are available:

**Windows (PowerShell):**
```powershell
.\scripts\setup_build_env.ps1
```

**Windows (Git Bash), Linux, or macOS:**
```bash
./scripts/setup_build_env.sh
```

These scripts will:
- Check if Python 3.13.9 is already downloaded to `python_local/`
- Download and extract Python 3.13.9 if not present
- Check if CMake 4.1.2 is already downloaded to `cmake_local/`
- Download and extract CMake 4.1.2 if not present
- Verify both installations

### Using Local Tools

After setup, use the local installations:

**Python:**
```powershell
.\python_local\python.exe your_script.py
```

**CMake:**
```powershell
.\cmake_local\cmake-4.1.2-windows-x86_64\bin\cmake.exe [args]
```

## Repository Status

This repository is in its initial stages. The build environment setup is complete, ready for rendering framework development.
