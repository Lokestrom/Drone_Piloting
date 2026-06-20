@echo off
setlocal

pushd "%~dp0" >nul

where cargo >nul 2>nul
if errorlevel 1 (
    echo cargo was not found in PATH.
    popd >nul
    exit /b 1
)

echo Building debug control.dll...
cargo build
if errorlevel 1 (
    popd >nul
    exit /b 1
)

echo Building release control.dll...
cargo build --release
if errorlevel 1 (
    popd >nul
    exit /b 1
)

if not exist "..\..\Debug" mkdir "..\..\Debug"
if not exist "..\..\Release" mkdir "..\..\Release"

copy /Y "target\debug\control.dll" "..\..\Debug\control.dll" >nul
if errorlevel 1 (
    echo Failed to copy debug control.dll.
    popd >nul
    exit /b 1
)

copy /Y "target\release\control.dll" "..\..\Release\control.dll" >nul
if errorlevel 1 (
    echo Failed to copy release control.dll.
    popd >nul
    exit /b 1
)

echo Installed Rust control.dll files to:
echo   assets\Drones\testDrone\Debug\control.dll
echo   assets\Drones\testDrone\Release\control.dll

popd >nul
endlocal
pause