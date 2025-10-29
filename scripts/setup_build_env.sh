#!/bin/bash
# Copyright Robert Srinivasiah 2025
# Licensed under the MIT License
# Bash script to ensure Python 3.13.9 and CMake 4.1.2 are available locally

set -e

PYTHON_TARGET_VERSION="3.13.9"
CMAKE_TARGET_VERSION="4.1.2"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
LOCAL_PYTHON_DIR="$REPO_ROOT/python_local"
LOCAL_CMAKE_DIR="$REPO_ROOT/cmake_local"

# Color codes
CYAN='\033[0;36m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
GRAY='\033[0;90m'
NC='\033[0m' # No Color

echo -e "${CYAN}============================================================${NC}"
echo -e "${CYAN}rsbl Build Environment Setup${NC}"
echo -e "${CYAN}============================================================${NC}"
echo ""

# Detect OS and architecture
OS="$(uname -s)"
ARCH="$(uname -m)"

# ============================================================
# Python Setup
# ============================================================

echo -e "${CYAN}--- Python Setup ---${NC}"
echo ""

case "$OS" in
    Linux)
        if [ "$ARCH" = "x86_64" ]; then
            PYTHON_PLATFORM="linux-x86_64"
            LOCAL_PYTHON_EXE="$LOCAL_PYTHON_DIR/bin/python3"
        elif [ "$ARCH" = "aarch64" ]; then
            PYTHON_PLATFORM="linux-aarch64"
            LOCAL_PYTHON_EXE="$LOCAL_PYTHON_DIR/bin/python3"
        else
            echo -e "${RED}[ERROR] Unsupported architecture: $ARCH${NC}"
            exit 1
        fi
        PYTHON_ARCHIVE_EXT="tar.gz"
        ;;
    Darwin)
        if [ "$ARCH" = "x86_64" ]; then
            PYTHON_PLATFORM="macos-x86_64"
        elif [ "$ARCH" = "arm64" ]; then
            PYTHON_PLATFORM="macos-arm64"
        else
            echo -e "${RED}[ERROR] Unsupported architecture: $ARCH${NC}"
            exit 1
        fi
        LOCAL_PYTHON_EXE="$LOCAL_PYTHON_DIR/bin/python3"
        PYTHON_ARCHIVE_EXT="tar.gz"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        # Windows (Git Bash)
        if [[ "$ARCH" == "x86_64" || "$ARCH" == "amd64" ]]; then
            PYTHON_ARCH="amd64"
        else
            PYTHON_ARCH="win32"
        fi
        LOCAL_PYTHON_EXE="$LOCAL_PYTHON_DIR/python.exe"
        PYTHON_ARCHIVE_EXT="zip"
        ;;
    *)
        echo -e "${RED}[ERROR] Unsupported operating system: $OS${NC}"
        exit 1
        ;;
esac

NEEDS_PYTHON=false

# Check if local Python already exists
if [ -f "$LOCAL_PYTHON_EXE" ]; then
    echo -e "${GREEN}[OK] Local Python installation found${NC}"
    echo -e "${GRAY}  Location: $LOCAL_PYTHON_EXE${NC}"

    # Verify version
    if VERSION_OUTPUT=$("$LOCAL_PYTHON_EXE" --version 2>&1); then
        echo -e "${GRAY}  Version: $VERSION_OUTPUT${NC}"
        echo ""
    else
        echo -e "${YELLOW}[WARNING] Existing installation appears corrupted, re-downloading...${NC}"
        rm -rf "$LOCAL_PYTHON_DIR"
        NEEDS_PYTHON=true
    fi
else
    NEEDS_PYTHON=true
fi

if [ "$NEEDS_PYTHON" = true ]; then
    echo -e "${YELLOW}Downloading Python $PYTHON_TARGET_VERSION to local directory...${NC}"
    echo ""

    # Create local directory
    mkdir -p "$LOCAL_PYTHON_DIR"

    # Build download URL based on platform
    case "$OS" in
        MINGW*|MSYS*|CYGWIN*)
            # Windows - use official python.org embedded distribution
            PYTHON_DOWNLOAD_URL="https://www.python.org/ftp/python/$PYTHON_TARGET_VERSION/python-$PYTHON_TARGET_VERSION-embed-$PYTHON_ARCH.zip"
            PYTHON_ARCHIVE="$LOCAL_PYTHON_DIR/python-$PYTHON_TARGET_VERSION.zip"
            ;;
        *)
            # Linux/macOS - use python-build-standalone
            PYTHON_DOWNLOAD_URL="https://github.com/indygreg/python-build-standalone/releases/download/20241002/cpython-$PYTHON_TARGET_VERSION+20241002-$ARCH-unknown-$PYTHON_PLATFORM-install_only_stripped.tar.gz"
            PYTHON_ARCHIVE="$LOCAL_PYTHON_DIR/python-$PYTHON_TARGET_VERSION.tar.gz"
            ;;
    esac

    echo -e "${GRAY}  Downloading from: $PYTHON_DOWNLOAD_URL${NC}"
    echo ""

    # Download
    if command -v curl &> /dev/null; then
        curl -L -o "$PYTHON_ARCHIVE" "$PYTHON_DOWNLOAD_URL"
    elif command -v wget &> /dev/null; then
        wget -O "$PYTHON_ARCHIVE" "$PYTHON_DOWNLOAD_URL"
    else
        echo -e "${RED}[ERROR] Neither curl nor wget found. Please install one of them.${NC}"
        exit 1
    fi

    echo -e "${GREEN}[OK] Download complete${NC}"

    # Extract
    echo -e "${YELLOW}Extracting to $LOCAL_PYTHON_DIR...${NC}"
    case "$PYTHON_ARCHIVE_EXT" in
        zip)
            # Windows - use unzip
            if command -v unzip &> /dev/null; then
                unzip -q "$PYTHON_ARCHIVE" -d "$LOCAL_PYTHON_DIR"
            else
                echo -e "${RED}[ERROR] unzip command not found. Please ensure Git Bash is properly installed.${NC}"
                exit 1
            fi
            ;;
        tar.gz)
            # Linux/macOS - use tar
            tar -xzf "$PYTHON_ARCHIVE" -C "$LOCAL_PYTHON_DIR" --strip-components=1
            ;;
    esac

    echo -e "${GREEN}[OK] Extraction complete${NC}"

    # Clean up archive
    rm "$PYTHON_ARCHIVE"

    # Verify installation
    if [ -f "$LOCAL_PYTHON_EXE" ]; then
        echo ""
        echo -e "${GREEN}[OK] Python installed at: $LOCAL_PYTHON_EXE${NC}"

        # Test the installation
        echo ""
        echo -e "${YELLOW}Testing installation...${NC}"
        "$LOCAL_PYTHON_EXE" --version
        echo ""
    else
        echo ""
        echo -e "${RED}[ERROR] Python executable not found at expected location${NC}"
        echo -e "${GRAY}  Expected: $LOCAL_PYTHON_EXE${NC}"
        exit 1
    fi
fi

# ============================================================
# CMake Setup
# ============================================================

echo -e "${CYAN}--- CMake Setup ---${NC}"
echo ""

case "$OS" in
    Linux)
        if [ "$ARCH" = "x86_64" ]; then
            CMAKE_PLATFORM="linux-x86_64"
        elif [ "$ARCH" = "aarch64" ]; then
            CMAKE_PLATFORM="linux-aarch64"
        else
            echo -e "${RED}[ERROR] Unsupported architecture: $ARCH${NC}"
            exit 1
        fi
        CMAKE_ARCHIVE_EXT="tar.gz"
        LOCAL_CMAKE_EXE="$LOCAL_CMAKE_DIR/cmake-$CMAKE_TARGET_VERSION-$CMAKE_PLATFORM/bin/cmake"
        ;;
    Darwin)
        CMAKE_PLATFORM="macos-universal"
        CMAKE_ARCHIVE_EXT="tar.gz"
        LOCAL_CMAKE_EXE="$LOCAL_CMAKE_DIR/cmake-$CMAKE_TARGET_VERSION-$CMAKE_PLATFORM/CMake.app/Contents/bin/cmake"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        # Windows (Git Bash)
        if [[ "$ARCH" == "x86_64" || "$ARCH" == "amd64" ]]; then
            CMAKE_ARCH="x86_64"
        else
            CMAKE_ARCH="i386"
        fi
        CMAKE_PLATFORM="windows-$CMAKE_ARCH"
        CMAKE_ARCHIVE_EXT="zip"
        LOCAL_CMAKE_EXE="$LOCAL_CMAKE_DIR/cmake-$CMAKE_TARGET_VERSION-$CMAKE_PLATFORM/bin/cmake.exe"
        ;;
esac

NEEDS_CMAKE=false

# Check if local CMake already exists
if [ -f "$LOCAL_CMAKE_EXE" ]; then
    echo -e "${GREEN}[OK] Local CMake installation found${NC}"
    echo -e "${GRAY}  Location: $LOCAL_CMAKE_EXE${NC}"

    # Verify version
    if VERSION_OUTPUT=$("$LOCAL_CMAKE_EXE" --version 2>&1 | head -n 1); then
        echo -e "${GRAY}  Version: $VERSION_OUTPUT${NC}"
        echo ""
    else
        echo -e "${YELLOW}[WARNING] Existing installation appears corrupted, re-downloading...${NC}"
        rm -rf "$LOCAL_CMAKE_DIR"
        NEEDS_CMAKE=true
    fi
else
    NEEDS_CMAKE=true
fi

if [ "$NEEDS_CMAKE" = true ]; then
    echo -e "${YELLOW}Downloading CMake $CMAKE_TARGET_VERSION to local directory...${NC}"
    echo ""

    # Create local directory
    mkdir -p "$LOCAL_CMAKE_DIR"

    # Build download URL
    CMAKE_DOWNLOAD_URL="https://github.com/Kitware/CMake/releases/download/v$CMAKE_TARGET_VERSION/cmake-$CMAKE_TARGET_VERSION-$CMAKE_PLATFORM.$CMAKE_ARCHIVE_EXT"
    CMAKE_ARCHIVE="$LOCAL_CMAKE_DIR/cmake-$CMAKE_TARGET_VERSION.$CMAKE_ARCHIVE_EXT"

    echo -e "${GRAY}  Downloading from: $CMAKE_DOWNLOAD_URL${NC}"
    echo ""

    # Download
    if command -v curl &> /dev/null; then
        curl -L -o "$CMAKE_ARCHIVE" "$CMAKE_DOWNLOAD_URL"
    elif command -v wget &> /dev/null; then
        wget -O "$CMAKE_ARCHIVE" "$CMAKE_DOWNLOAD_URL"
    else
        echo -e "${RED}[ERROR] Neither curl nor wget found. Please install one of them.${NC}"
        exit 1
    fi

    echo -e "${GREEN}[OK] Download complete${NC}"

    # Extract
    echo -e "${YELLOW}Extracting to $LOCAL_CMAKE_DIR...${NC}"
    case "$CMAKE_ARCHIVE_EXT" in
        zip)
            # Windows - use unzip
            if command -v unzip &> /dev/null; then
                unzip -q "$CMAKE_ARCHIVE" -d "$LOCAL_CMAKE_DIR"
            else
                echo -e "${RED}[ERROR] unzip command not found. Please ensure Git Bash is properly installed.${NC}"
                exit 1
            fi
            ;;
        tar.gz)
            # Linux/macOS - use tar
            tar -xzf "$CMAKE_ARCHIVE" -C "$LOCAL_CMAKE_DIR"
            ;;
    esac

    echo -e "${GREEN}[OK] Extraction complete${NC}"

    # Clean up archive
    rm "$CMAKE_ARCHIVE"

    # Verify installation
    if [ -f "$LOCAL_CMAKE_EXE" ]; then
        echo ""
        echo -e "${GREEN}[OK] CMake installed at: $LOCAL_CMAKE_EXE${NC}"

        # Test the installation
        echo ""
        echo -e "${YELLOW}Testing installation...${NC}"
        "$LOCAL_CMAKE_EXE" --version | head -n 1
        echo ""
    else
        echo ""
        echo -e "${RED}[ERROR] CMake executable not found at expected location${NC}"
        echo -e "${GRAY}  Expected: $LOCAL_CMAKE_EXE${NC}"
        exit 1
    fi
fi

# ============================================================
# Summary
# ============================================================

echo -e "${CYAN}============================================================${NC}"
echo -e "${GREEN}Build Environment Setup Complete!${NC}"
echo -e "${CYAN}============================================================${NC}"
echo ""
echo -e "${GRAY}Python: $LOCAL_PYTHON_EXE${NC}"
echo -e "${GRAY}CMake:  $LOCAL_CMAKE_EXE${NC}"
echo ""

exit 0
