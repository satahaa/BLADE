# BLADE - Bi-Directional LAN Asset Distribution Engine

BLADE is a high-performance, C++23-based local network file transfer system for secure data sharing between devices on the same LAN. It features a modern Qt 6 GUI, a web interface, and a minimal, robust QR code generator for easy mobile access.

## Features

- 🚀 **High Performance**: Modern C++23 core for maximum efficiency
- 🖥️ **Qt 6 GUI**: Elegant, dark-themed desktop application
- 🔐 **Optional Authentication**: Secure login or guest mode
- 🌐 **Web Interface**: Access from any device via browser
- 📱 **QR Code Access**: Scan to connect from mobile (minimal QR code logic, no advanced segmenting)
- 💻 **Windows-first**: Officially supports Windows with Qt 6.8.1 + MinGW toolchain
- 📦 **CMake Build System**: Professional build configuration for CLion and other IDEs

## User Interfaces

### Qt GUI Application
- Login screen with BLADE logo
- Two authentication modes: with or without login
- Automatic QR code generation for mobile access (basic alphanumeric/byte mode only)
- Modern dark theme
- Real-time server status

### Web Interface
- Responsive design for desktop/mobile
- File upload, connection status, device management

## Architecture

- **Server**: Manages connections and authentication
- **Authentication Manager**: Handles user credentials
- **Connection Handler**: Manages clients and data transfer
- **HTTP Server**: Serves the web interface
- **Network Utils**: Low-level networking utilities

## Prerequisites

- **Windows 10/11** (primary support)
- **CMake 3.15+**
- **C++23 compatible compiler** (MinGW from Qt 6.8.1 recommended)
- **Qt 6.8.1** (Core, Widgets, Gui modules)
- Qt toolchain must be used for both build and runtime DLLs (see below)

## Building the Project

### Quick Build (GUI)

**Windows:**
```powershell
# Run the build script (auto-detects Qt)
./build_gui.bat
```

### Manual Build (Recommended for CLion)

1. Open CLion and select the BLADE directory
2. In CMake profile, set:
   - `CMAKE_PREFIX_PATH = C:/Qt/6.8.1/mingw_64`
   - `CMAKE_C_COMPILER  = C:/Qt/6.8.1/mingw_64/bin/gcc.exe`
   - `CMAKE_CXX_COMPILER= C:/Qt/6.8.1/mingw_64/bin/g++.exe`
   - Generator: MinGW Makefiles
3. Delete/clean the build directory to regenerate CMakeCache
4. Build the project
5. Ensure DLLs (`libstdc++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll`) are copied from the Qt MinGW bin directory to `bin/`

### Using the App

- Run `bin/blade.exe` (GUI)
- The web interface is available at `http://<server-ip>:8081`
- Scan the QR code in the GUI to connect from mobile (basic QR only)

## QR Code Generation (Limitations)

- Only basic alphanumeric/byte QR codes are supported (no Kanji/ECI/advanced segmenting)
- The QR code generator is minimal: no QrSegment, BitBuffer, or advanced APIs
- If you need advanced QR features, use the full Project Nayuki library

## Troubleshooting

- **App fails to start or DLL errors:**
  - Ensure you are using the MinGW toolchain from your Qt install for both build and runtime
  - All required DLLs must be present in the `bin/` directory
- **QR code not visible or cropped:**
  - Ensure the GUI is updated and the QR code widget is not clipped
- **Build errors:**
  - Use CMake 3.15+, MinGW from Qt 6.8.1, and Qt 6.8.1 only

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

---