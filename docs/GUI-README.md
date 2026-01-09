# BLADE GUI Application

## Overview
This is the graphical user interface (GUI) version of BLADE - Local Network File Transfer application. It provides an elegant, user-friendly interface for starting the server with or without authentication.

## Features

### Elegant Login Screen
- **BLADE Logo**: Centered logo display
- **Authentication Options**: 
  - Start with Authentication (requires username and password)
  - Start without Authentication (instant access)
- **Modern Design**: 
  - Dark gradient background
  - Glassmorphic input fields
  - Smooth animations
  - Professional typography (Segoe UI)

### Server Running Screen
- **QR Code Display**: Automatically generates a scannable QR code
- **Connection URL**: Displays the HTTP URL for easy access
- **Port Information**: Shows web interface and file transfer ports
- **Status Indicator**: Visual confirmation that server is running
- **Clean Layout**: Centered, elegant design

## Requirements

### System Requirements
- Windows 10/11 (64-bit)
- Administrator privileges (for port 80 access)
- Minimum 4GB RAM
- Network connection

### Build Requirements
- CMake 3.15 or higher
- C++23 compatible compiler (GCC 11+, MSVC 2019+)
- Qt6 (Core, Widgets, Gui modules)
- Windows SDK

## Building the GUI Version

### Prerequisites
1. **Install Qt6**:
   - Download Qt6 from: https://www.qt.io/download
   - Install Qt 6.5 or later
   - Make sure to include Qt6 Core, Widgets, and Gui modules
   - Add Qt to your system PATH or set CMAKE_PREFIX_PATH

2. **Set Environment Variables** (if Qt is not in PATH):
   ```powershell
   $env:CMAKE_PREFIX_PATH="C:\Qt\6.5.0\mingw_64"
   ```

### Build Steps

#### Using CMake (Command Line)
```powershell
# Navigate to project directory
cd D:\Study\3-1\Project\BLADE

# Create build directory
mkdir build
cd build

# Configure with CMake (Qt will be auto-detected)
cmake .. -G "MinGW Makefiles"

# Build
cmake --build .

# The executables will be in bin/ folder:
# - blade.exe (CLI version)
# - blade_gui.exe (GUI version, if Qt was found)
```

#### Using CLion
1. Open the project in CLion
2. CLion will automatically detect Qt if it's in your PATH
3. Select the build configuration
4. Build the project (Ctrl+F9)
5. Run `blade_gui` target

### Building Without Qt
If Qt is not available, CMake will automatically skip the GUI build and only build the CLI version:
```
Qt6 not found - Skipping GUI version. Install Qt6 to build the GUI.
```

## Running the Application

### GUI Version
```powershell
cd bin
.\blade_gui.exe
```

**Note**: Run as Administrator to access port 80 for the web interface.

### Features at a Glance

#### Starting with Authentication:
1. Enter username and password
2. Click "Start with Authentication"
3. Server starts with login protection
4. QR code and URL displayed

#### Starting without Authentication:
1. Click "Start without Authentication"
2. Server starts immediately
3. QR code and URL displayed
4. Anyone can access without login

## Troubleshooting

### "Failed to start server" Error
- **Cause**: Port 80 or 8080 already in use, or insufficient privileges
- **Solution**: 
  - Run as Administrator
  - Close any applications using port 80 (IIS, Apache, etc.)
  - Check Windows Firewall settings

### Qt DLLs Not Found
- **Cause**: Qt DLLs not in PATH or not copied to bin folder
- **Solution**:
  - Make sure Qt bin directory is in PATH
  - Or manually copy Qt6Core.dll, Qt6Gui.dll, Qt6Widgets.dll to bin folder

### QR Code Not Displaying
- **Cause**: QRCodeGen library issue
- **Solution**: 
  - Check that QRCodeGen.cpp is compiled correctly
  - Ensure UTF-8 encoding is supported in console

### Network Detection Issues
- **Symptom**: IP address shows as 0.0.0.0 or incorrect
- **Solution**:
  - Check network adapter settings
  - Ensure you're connected to the network
  - Run `ipconfig` to verify your IP address

## Architecture

### Components
- **MainWindow.h/.cpp**: Main GUI window implementation
- **main_gui.cpp**: Qt application entry point
- **Server**: Backend server logic (shared with CLI)
- **QRCodeGen**: QR code generation library
- **NetworkUtils**: Network utility functions

### Design Philosophy
- **Minimalist**: Clean, uncluttered interface
- **Monochrome**: Professional dark theme
- **Modern**: Contemporary UI patterns
- **Responsive**: Adapts to different window sizes
- **Intuitive**: Clear visual hierarchy

## File Structure
```
BLADE/
├── bin/
│   ├── blade.exe          (CLI version)
│   ├── blade_gui.exe      (GUI version)
│   ├── Qt6Core.dll
│   ├── Qt6Gui.dll
│   ├── Qt6Widgets.dll
│   └── web/               (Web interface files)
├── src/
│   ├── main_gui.cpp       (GUI entry point)
│   ├── MainWindow.cpp     (GUI implementation)
│   └── ...
├── include/
│   ├── MainWindow.h       (GUI header)
│   └── ...
├── resources/
│   └── resources.qrc      (Qt resources)
└── CMakeLists.txt         (Build configuration)
```

## Technical Details

### Qt Modules Used
- **Qt6::Core**: Core functionality
- **Qt6::Widgets**: UI widgets
- **Qt6::Gui**: GUI utilities and painting

### Key Features
- **Async Server Start**: Non-blocking server startup
- **QR Code Generation**: Automatic QR code from URL
- **Smooth Transitions**: Elegant view switching
- **Error Handling**: Comprehensive error messages
- **Resource Management**: Proper cleanup on exit

### Styling
- Custom stylesheets for all widgets
- Gradient backgrounds
- Glassmorphic effects
- Custom button states (hover, pressed)
- Professional color scheme

## Performance
- **Startup Time**: < 2 seconds
- **Memory Usage**: ~50-80 MB (with Qt runtime)
- **CPU Usage**: Minimal when idle
- **Network Overhead**: None until server starts

## Security Considerations
- Credentials are stored in memory only
- No credential persistence
- HTTPS support via HTTPServer
- Authentication tokens for secure sessions
- No external dependencies beyond Qt

## Future Enhancements
- [ ] System tray integration
- [ ] Auto-start on system boot
- [ ] Connection statistics display
- [ ] File transfer history
- [ ] Settings persistence
- [ ] Theme customization
- [ ] Multi-language support

## Support
For issues, questions, or contributions, please refer to the main BLADE documentation.

## License
Same as BLADE project license.

