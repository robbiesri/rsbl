# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**rsbl** is a real-time rendering framework. This is a new project with minimal code currently in place.

## Python Environment Setup

This project uses Python 3.13.9 as its local development environment. Python is managed locally in the `python_local/` directory.

### Initial Setup

Run the setup script to ensure Python is available:

```powershell
.\setup_python.ps1
```

This script will:
- Check if Python 3.13.9 is already downloaded to `python_local/`
- Download and extract Python 3.13.9 if not present
- Verify the installation

### Using Local Python

After setup, use the local Python installation:

```powershell
.\python_local\python.exe your_script.py
```

## Repository Status

This repository is in its initial stages. The Python environment setup is complete, ready for rendering framework development.
