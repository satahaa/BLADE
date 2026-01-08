# BLADE - Qt GUI Version

## Build Requirements
- Qt6 (6.5+) with Core, Widgets, Gui modules
- CMake 3.15+
- C++23 compiler (MinGW/GCC/MSVC)

## Quick Build

### CLion (Recommended)
1. Open project in CLion
2. CMake will auto-configure
3. Build the project (Ctrl+F9)
4. Run `blade` target

### Command Line
```powershell
# Set Qt path (adjust to your installation)
$env:CMAKE_PREFIX_PATH="C:\Qt\6.5.0\mingw_64"

# Build
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release

# Output in bin/blade.exe
```

## Running
```powershell
cd bin
.\blade.exe
```

**Must run as Administrator for port 80**

## Usage
1. Launch blade.exe
2. Enter username/password (optional)
3. Click "Start with Authentication" OR "Start without Authentication"
4. QR code appears - scan with mobile device
5. Or open the displayed URL in any browser

## Architecture
- `src/main.cpp` - 6 lines, creates Application object and runs it
- `src/Application.cpp` - Complete GUI implementation
- Backend - Shared with old console version (Server, HTTPServer, etc.)

## Ports
- **80** - Web interface (HTTP)
- **8080** - File transfer

Done. No more docs.

