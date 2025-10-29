# PowerShell script to ensure Python 3.13.9 is available locally

$TargetVersion = "3.13.9"
$LocalPythonDir = Join-Path $PSScriptRoot "python_local"
$LocalPythonExe = Join-Path $LocalPythonDir "python.exe"

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "rsbl Python Environment Setup" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

# Check if local Python already exists
if (Test-Path $LocalPythonExe) {
    Write-Host "[OK] Local Python installation found" -ForegroundColor Green
    Write-Host "  Location: $LocalPythonExe" -ForegroundColor Gray
    Write-Host ""

    # Verify version
    try {
        $versionOutput = & $LocalPythonExe --version 2>&1
        Write-Host "  Version: $versionOutput" -ForegroundColor Gray
        Write-Host ""
        Write-Host "[OK] Python environment is ready!" -ForegroundColor Green
        exit 0
    }
    catch {
        Write-Host "[WARNING] Existing installation appears corrupted, re-downloading..." -ForegroundColor Yellow
        Remove-Item $LocalPythonDir -Recurse -Force
    }
}

# Download Python
Write-Host "Downloading Python $TargetVersion to local directory..." -ForegroundColor Yellow
Write-Host ""

# Create local directory
if (-not (Test-Path $LocalPythonDir)) {
    New-Item -ItemType Directory -Path $LocalPythonDir | Out-Null
}

# Determine architecture
$arch = if ([Environment]::Is64BitOperatingSystem) { "amd64" } else { "win32" }

# Build download URL
$downloadUrl = "https://www.python.org/ftp/python/$TargetVersion/python-$TargetVersion-embed-$arch.zip"
$zipPath = Join-Path $LocalPythonDir "python-$TargetVersion-embed-$arch.zip"

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
    }
    else {
        Write-Host ""
        Write-Host "[ERROR] Python executable not found at expected location" -ForegroundColor Red
        Write-Host "  Expected: $LocalPythonExe" -ForegroundColor Gray
        exit 1
    }

    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Cyan
    Write-Host "Setup complete!" -ForegroundColor Green
    Write-Host "============================================================" -ForegroundColor Cyan

    exit 0
}
catch {
    Write-Host ""
    Write-Host "[ERROR] $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install Python manually from: https://www.python.org/downloads/" -ForegroundColor Yellow
    exit 1
}
