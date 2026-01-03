@echo off
echo Building BLADE...

set PATH=E:\CLion 2025.2.3\bin\mingw\bin;E:\CLion 2025.2.3\bin\cmake\win\x64\bin;%PATH%

cd ..\cmake-build-debug

echo Cleaning old build files...
if exist blade.exe del /F blade.exe
if exist CMakeCache.txt del /F CMakeCache.txt

echo Configuring with CMake...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

echo Building...
mingw32-make

if exist blade.exe (
    echo.
    echo ========================================
    echo Build successful!
    echo ========================================
    echo.
    echo To run the server, use: ..\bin\blade.exe
) else (
    echo.
    echo ========================================
    echo Build failed!
    echo ========================================
)

