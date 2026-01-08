# badCAD Development Environment Setup Script
# Run this before development sessions to ensure MSYS2 tools are in PATH

Write-Host "Setting up badCAD development environment..." -ForegroundColor Cyan

# Add MSYS2/MinGW64 to PATH
$env:Path = "C:\msys64\mingw64\bin;" + $env:Path

Write-Host "✓ Added MSYS2/MinGW64 to PATH" -ForegroundColor Green

# Verify installations
Write-Host "`nVerifying tools..." -ForegroundColor Cyan

$tools = @{
    "GCC" = "g++ --version"
    "CMake" = "cmake --version"
    "Ninja" = "ninja --version"
    "Git" = "git --version"
}

foreach ($tool in $tools.Keys) {
    try {
        $version = Invoke-Expression $tools[$tool] 2>&1 | Select-Object -First 1
        Write-Host "✓ $tool`: $version" -ForegroundColor Green
    } catch {
        Write-Host "✗ $tool`: Not found" -ForegroundColor Red
    }
}

Write-Host "`n✓ Development environment ready!" -ForegroundColor Green
Write-Host "You can now use: cmake, g++, ninja, make" -ForegroundColor Yellow
