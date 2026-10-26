if (Test-Path build) {
    Remove-Item -Recurse -Force build
}

Write-Host "Configuring build files with GCC (MinGW)..." -ForegroundColor Cyan
cmake -B build -G "MinGW Makefiles"

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "Compiling winfetch..." -ForegroundColor Cyan
cmake --build build

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "Success! Executable created at .\build\winfetch.exe" -ForegroundColor Green
