# Copyright Robert Srinivasiah 2025
# Licensed under the MIT License
# PowerShell script to ensure Python 3.13.9 and CMake 4.1.2 are available locally

$PythonTargetVersion = "3.13.9"
$CMakeTargetVersion = "4.1.2"
$LocalPythonDir = Join-Path $PSScriptRoot "python_local"
$LocalPythonExe = Join-Path $LocalPythonDir "python.exe"
$LocalCMakeDir = Join-Path $PSScriptRoot "cmake_local"

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "rsbl Build Environment Setup" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

# ============================================================
# Python Setup
# ============================================================

Write-Host "--- Python Setup ---" -ForegroundColor Cyan
Write-Host ""

# Check if local Python already exists
if (Test-Path $LocalPythonExe) {
    Write-Host "[OK] Local Python installation found" -ForegroundColor Green
    Write-Host "  Location: $LocalPythonExe" -ForegroundColor Gray

    # Verify version
    try {
        $versionOutput = & $LocalPythonExe --version 2>&1
        Write-Host "  Version: $versionOutput" -ForegroundColor Gray
        Write-Host ""
    }
    catch {
        Write-Host "[WARNING] Existing installation appears corrupted, re-downloading..." -ForegroundColor Yellow
        Remove-Item $LocalPythonDir -Recurse -Force
        $needsPython = $true
    }
} else {
    $needsPython = $true
}

if ($needsPython) {
    # Download Python
    Write-Host "Downloading Python $PythonTargetVersion to local directory..." -ForegroundColor Yellow
    Write-Host ""

    # Create local directory
    if (-not (Test-Path $LocalPythonDir)) {
        New-Item -ItemType Directory -Path $LocalPythonDir | Out-Null
    }

    # Determine architecture
    $arch = if ([Environment]::Is64BitOperatingSystem) { "amd64" } else { "win32" }

    # Build download URL
    $downloadUrl = "https://www.python.org/ftp/python/$PythonTargetVersion/python-$PythonTargetVersion-embed-$arch.zip"
    $zipPath = Join-Path $LocalPythonDir "python-$PythonTargetVersion-embed-$arch.zip"

    Write-Host "  Downloading from: $downloadUrl" -ForegroundColor Gray
    Write-Host ""

    try {
        # Download with progress
        $webClient = New-Object System.Net.WebClient
        $webClient.DownloadFile($downloadUrl, $zipPath)

        Write-Host "[OK] Download complete" -ForegroundColor Green

        # Extract
        Write-Host "Extracting to $LocalPythonDir..." -ForegroundColor Yellow
        Expand-Archive -Path $zipPath -DestinationPath $LocalPythonDir -Force

        Write-Host "[OK] Extraction complete" -ForegroundColor Green

        # Clean up zip file
        Remove-Item $zipPath

        # Verify installation
        if (Test-Path $LocalPythonExe) {
            Write-Host ""
            Write-Host "[OK] Python installed at: $LocalPythonExe" -ForegroundColor Green

            # Test the installation
            Write-Host ""
            Write-Host "Testing installation..." -ForegroundColor Yellow
            & $LocalPythonExe --version
            Write-Host ""
        }
        else {
            Write-Host ""
            Write-Host "[ERROR] Python executable not found at expected location" -ForegroundColor Red
            Write-Host "  Expected: $LocalPythonExe" -ForegroundColor Gray
            exit 1
        }
    }
    catch {
        Write-Host ""
        Write-Host "[ERROR] $_" -ForegroundColor Red
        Write-Host ""
        Write-Host "Please install Python manually from: https://www.python.org/downloads/" -ForegroundColor Yellow
        exit 1
    }
}

# ============================================================
# CMake Setup
# ============================================================

Write-Host "--- CMake Setup ---" -ForegroundColor Cyan
Write-Host ""

# Determine architecture for CMake
$arch = if ([Environment]::Is64BitOperatingSystem) { "x86_64" } else { "i386" }
$LocalCMakeExe = Join-Path $LocalCMakeDir "cmake-$CMakeTargetVersion-windows-$arch\bin\cmake.exe"

# Check if local CMake already exists
if (Test-Path $LocalCMakeExe) {
    Write-Host "[OK] Local CMake installation found" -ForegroundColor Green
    Write-Host "  Location: $LocalCMakeExe" -ForegroundColor Gray

    # Verify version
    try {
        $versionOutput = & $LocalCMakeExe --version 2>&1 | Select-Object -First 1
        Write-Host "  Version: $versionOutput" -ForegroundColor Gray
        Write-Host ""
    }
    catch {
        Write-Host "[WARNING] Existing installation appears corrupted, re-downloading..." -ForegroundColor Yellow
        Remove-Item $LocalCMakeDir -Recurse -Force
        $needsCMake = $true
    }
} else {
    $needsCMake = $true
}

if ($needsCMake) {
    # Download CMake
    Write-Host "Downloading CMake $CMakeTargetVersion to local directory..." -ForegroundColor Yellow
    Write-Host ""

    # Create local directory
    if (-not (Test-Path $LocalCMakeDir)) {
        New-Item -ItemType Directory -Path $LocalCMakeDir | Out-Null
    }

    # Build download URL
    $downloadUrl = "https://github.com/Kitware/CMake/releases/download/v$CMakeTargetVersion/cmake-$CMakeTargetVersion-windows-$arch.zip"
    $zipPath = Join-Path $LocalCMakeDir "cmake-$CMakeTargetVersion-windows-$arch.zip"

    Write-Host "  Downloading from: $downloadUrl" -ForegroundColor Gray
    Write-Host ""

    try {
        # Download with progress
        $webClient = New-Object System.Net.WebClient
        $webClient.DownloadFile($downloadUrl, $zipPath)

        Write-Host "[OK] Download complete" -ForegroundColor Green

        # Extract
        Write-Host "Extracting to $LocalCMakeDir..." -ForegroundColor Yellow
        Expand-Archive -Path $zipPath -DestinationPath $LocalCMakeDir -Force

        Write-Host "[OK] Extraction complete" -ForegroundColor Green

        # Clean up zip file
        Remove-Item $zipPath

        # Verify installation
        if (Test-Path $LocalCMakeExe) {
            Write-Host ""
            Write-Host "[OK] CMake installed at: $LocalCMakeExe" -ForegroundColor Green

            # Test the installation
            Write-Host ""
            Write-Host "Testing installation..." -ForegroundColor Yellow
            & $LocalCMakeExe --version | Select-Object -First 1
            Write-Host ""
        }
        else {
            Write-Host ""
            Write-Host "[ERROR] CMake executable not found at expected location" -ForegroundColor Red
            Write-Host "  Expected: $LocalCMakeExe" -ForegroundColor Gray
            exit 1
        }
    }
    catch {
        Write-Host ""
        Write-Host "[ERROR] $_" -ForegroundColor Red
        Write-Host ""
        Write-Host "Please download CMake manually from: https://cmake.org/download/" -ForegroundColor Yellow
        exit 1
    }
}

# ============================================================
# Summary
# ============================================================

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Build Environment Setup Complete!" -ForegroundColor Green
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Python: $LocalPythonExe" -ForegroundColor Gray
Write-Host "CMake:  $LocalCMakeExe" -ForegroundColor Gray
Write-Host ""

exit 0
