# Copyright Robert Srinivasiah 2025
# Licensed under the MIT License
# PowerShell wrapper to run the CMake setup Python script

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$PythonExe = Join-Path $ScriptDir "python_local\python.exe"
$PythonScript = Join-Path $ScriptDir "setup_cmake.py"

# Check if local Python exists
if (-not (Test-Path $PythonExe)) {
    Write-Host "[ERROR] Local Python not found at: $PythonExe" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please run setup_python.ps1 first to install Python:" -ForegroundColor Yellow
    Write-Host "  .\setup_python.ps1" -ForegroundColor White
    exit 1
}

# Run the Python setup script
Write-Host "Running CMake setup script..." -ForegroundColor Cyan
Write-Host ""

& $PythonExe $PythonScript

exit $LASTEXITCODE
