#!/usr/bin/env python3
# Copyright Robert Srinivasiah 2025
# Licensed under the MIT License
"""
CMake setup script for rsbl.
Downloads CMake 4.1.2 to local cmake_local directory.
"""

import os
import sys
import platform
import urllib.request
import zipfile
import tarfile
from pathlib import Path

TARGET_VERSION = "4.1.2"
SCRIPT_DIR = Path(__file__).parent
LOCAL_CMAKE_DIR = SCRIPT_DIR / "cmake_local"


def get_download_url():
    """Get the appropriate CMake download URL for the current platform."""
    system = platform.system()
    machine = platform.machine().lower()

    base_url = f"https://github.com/Kitware/CMake/releases/download/v{TARGET_VERSION}"

    if system == "Windows":
        if "amd64" in machine or "x86_64" in machine:
            return f"{base_url}/cmake-{TARGET_VERSION}-windows-x86_64.zip", "zip"
        else:
            return f"{base_url}/cmake-{TARGET_VERSION}-windows-i386.zip", "zip"
    elif system == "Darwin":  # macOS
        return f"{base_url}/cmake-{TARGET_VERSION}-macos-universal.tar.gz", "tar.gz"
    elif system == "Linux":
        if "x86_64" in machine or "amd64" in machine:
            return f"{base_url}/cmake-{TARGET_VERSION}-linux-x86_64.tar.gz", "tar.gz"
        elif "aarch64" in machine or "arm64" in machine:
            return f"{base_url}/cmake-{TARGET_VERSION}-linux-aarch64.tar.gz", "tar.gz"
        else:
            raise RuntimeError(f"Unsupported Linux architecture: {machine}")
    else:
        raise RuntimeError(f"Unsupported platform: {system}")


def check_existing_installation():
    """Check if CMake is already installed locally."""
    if system := platform.system() == "Windows":
        cmake_exe = LOCAL_CMAKE_DIR / f"cmake-{TARGET_VERSION}-windows-x86_64" / "bin" / "cmake.exe"
    else:
        # For Linux/macOS, check in the extracted directory structure
        possible_paths = [
            LOCAL_CMAKE_DIR / f"cmake-{TARGET_VERSION}-linux-x86_64" / "bin" / "cmake",
            LOCAL_CMAKE_DIR / f"cmake-{TARGET_VERSION}-linux-aarch64" / "bin" / "cmake",
            LOCAL_CMAKE_DIR / f"cmake-{TARGET_VERSION}-macos-universal" / "CMake.app" / "Contents" / "bin" / "cmake",
        ]
        cmake_exe = next((p for p in possible_paths if p.exists()), None)

    if cmake_exe and cmake_exe.exists():
        print(f"[OK] CMake {TARGET_VERSION} already installed")
        print(f"  Location: {cmake_exe}")
        return True

    return False


def download_cmake():
    """Download and extract CMake to local directory."""
    print(f"Downloading CMake {TARGET_VERSION} to local directory...")
    print()

    # Create local directory
    LOCAL_CMAKE_DIR.mkdir(exist_ok=True)

    url, archive_type = get_download_url()
    archive_path = LOCAL_CMAKE_DIR / f"cmake-{TARGET_VERSION}.{archive_type}"

    print(f"  Downloading from: {url}")

    try:
        # Download with progress
        def progress_hook(block_num, block_size, total_size):
            downloaded = block_num * block_size
            if total_size > 0:
                percent = min(downloaded * 100 / total_size, 100)
                print(f"\r  Progress: {percent:.1f}%", end="", flush=True)

        urllib.request.urlretrieve(url, archive_path, progress_hook)
        print("\n[OK] Download complete")

        # Extract
        print(f"Extracting to {LOCAL_CMAKE_DIR}...")
        if archive_type == "zip":
            with zipfile.ZipFile(archive_path, 'r') as zip_ref:
                zip_ref.extractall(LOCAL_CMAKE_DIR)
        elif archive_type == "tar.gz":
            with tarfile.open(archive_path, 'r:gz') as tar_ref:
                tar_ref.extractall(LOCAL_CMAKE_DIR)

        print("[OK] Extraction complete")

        # Clean up archive
        archive_path.unlink()

        # Find and report CMake executable location
        system = platform.system()
        if system == "Windows":
            machine = platform.machine().lower()
            if "amd64" in machine or "x86_64" in machine:
                cmake_exe = LOCAL_CMAKE_DIR / f"cmake-{TARGET_VERSION}-windows-x86_64" / "bin" / "cmake.exe"
            else:
                cmake_exe = LOCAL_CMAKE_DIR / f"cmake-{TARGET_VERSION}-windows-i386" / "bin" / "cmake.exe"
        elif system == "Darwin":
            cmake_exe = LOCAL_CMAKE_DIR / f"cmake-{TARGET_VERSION}-macos-universal" / "CMake.app" / "Contents" / "bin" / "cmake"
        else:  # Linux
            machine = platform.machine().lower()
            if "aarch64" in machine or "arm64" in machine:
                cmake_exe = LOCAL_CMAKE_DIR / f"cmake-{TARGET_VERSION}-linux-aarch64" / "bin" / "cmake"
            else:
                cmake_exe = LOCAL_CMAKE_DIR / f"cmake-{TARGET_VERSION}-linux-x86_64" / "bin" / "cmake"

        if cmake_exe.exists():
            print()
            print(f"[OK] CMake installed at: {cmake_exe}")
            print()
            print("To use CMake, run:")
            print(f"  {cmake_exe}")
        else:
            print()
            print("[WARNING] CMake extracted but executable not found at expected location")
            print(f"  Expected: {cmake_exe}")
            print(f"  Check {LOCAL_CMAKE_DIR} for CMake files")

        return True

    except Exception as e:
        print(f"\n[ERROR] Error downloading CMake: {e}")
        print("\nPlease download CMake manually from: https://cmake.org/download/")
        return False


def main():
    """Main setup function."""
    print("=" * 60)
    print("rsbl CMake Setup")
    print("=" * 60)
    print()

    # Check if already installed
    if check_existing_installation():
        print()
        print("[OK] CMake environment is ready!")
        return 0

    # Download CMake
    if download_cmake():
        print()
        print("=" * 60)
        print("Setup complete!")
        print("=" * 60)
        return 0
    else:
        return 1


if __name__ == "__main__":
    sys.exit(main())
