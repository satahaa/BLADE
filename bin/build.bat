@echo off
setlocal

REM Set build directory name (at project root)
set BUILD_DIR=..\build

REM Remove previous build directory if it exists
if exist "%BUILD_DIR%" (
    echo Deleting previous build directory...
    rmdir /s /q "%BUILD_DIR%"
)

REM Remove any CMakeCache.txt or CMakeFiles in bin (cleanup)
if exist "CMakeCache.txt" del /f /q "CMakeCache.txt"
if exist "CMakeFiles" rmdir /s /q "CMakeFiles"

REM Create build directory
mkdir "%BUILD_DIR%"

REM Change to build directory
cd "%BUILD_DIR%"

REM Run CMake to generate build files (adjust generator if needed)
cmake -G "MinGW Makefiles" ..

REM Build the project
mingw32-make

REM Go back to bin directory
cd ..\bin

echo.
echo ===== Rebuild complete! =====
