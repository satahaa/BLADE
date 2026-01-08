@echo off
echo Building BLADE...
echo.

if exist build rmdir /s /q build
mkdir build
cd build

cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    cd ..
    exit /b 1
)

cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Build failed!
    cd ..
    exit /b 1
)

cd ..
echo.
echo Build complete! Run: bin\blade.exe

