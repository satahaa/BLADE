# BLADE - Bi-Directional LAN Asset Distribution Engine

BLADE is a high-performance, C++ based local network file transfer system that enables secure data sharing between devices on the same LAN. Built with modern C++23 and featuring both a beautiful web-based interface and an elegant Qt GUI, BLADE provides an efficient, low-level solution for PC-to-PC and PC-to-Mobile file transfers.

## Features

- 🚀 **High Performance**: Written in C++ for maximum efficiency and low-level control
- 🖥️ **Dual Interface**: Choose between CLI or elegant Qt GUI application
- 🔐 **Secure Authentication**: Optional authentication system to protect your network
- 🌐 **Web-Based GUI**: Beautiful HTML/CSS/JavaScript interface accessible from any device
- 📱 **QR Code Access**: Scan QR code to instantly connect from mobile devices
- 🔌 **Easy Connection**: Simple port-based networking with automatic discovery
- 💻 **Cross-Platform**: Works on Windows, Linux, and macOS
- 📦 **CMake Build System**: Professional build configuration for CLion and other IDEs

## User Interfaces

### Qt GUI Application
- **Elegant login screen** with BLADE logo
- **Two authentication modes**: With or without login
- **Automatic QR code generation** for easy mobile access
- **Modern design** with dark theme and glassmorphic effects
- **Real-time server status** display

**📖 See [GUI-README.md](docs/GUI-README.md) for detailed GUI documentation**

### Web Interface
- **Responsive design** for desktop and mobile
- **File type icons** for images, videos, audio, documents
- **Drag-and-drop** file upload
- **Connection status** indicator
- **Device management** view

### Command Line Interface
- **Traditional CLI** for server management
- **Debug logging** for troubleshooting
- **Command-line arguments** for automation

## Architecture

BLADE consists of several key components:

- **Server**: Main network server managing connections and authentication
- **Authentication Manager**: Handles user credentials and session tokens
- **Connection Handler**: Manages connected clients and data transfer
- **HTTP Server**: Serves the web interface to connected devices
- **Network Utils**: Low-level socket operations and networking utilities

## Prerequisites

### For CLI Version
- CMake 3.15 or higher
- C++23 compatible compiler (GCC 11+, Clang 14+, MSVC 2019+)
- Operating System: Windows, Linux, or macOS

### For GUI Version (Optional)
- All CLI prerequisites
- Qt6 (6.5 or later) with Core, Widgets, and Gui modules
- Qt should be in PATH or CMAKE_PREFIX_PATH set

## Building the Project

### Quick Build (GUI + CLI)

**Windows:**
```powershell
# Run the build script (auto-detects Qt)
.\build_gui.bat
```

**Manual Build:**
```bash
# Configure with Qt auto-detection
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release

# Outputs:
# - bin/blade.exe (CLI version - always built)
# - bin/blade_gui.exe (GUI version - if Qt6 found)
```

### Building CLI Only

```bash
# Clone the repository
git clone https://github.com/satahaa/BLADE.git
cd BLADE

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .

# The executable will be in bin/blade.exe
```

### Using CLion

1. Open CLion
2. Select "Open" and choose the BLADE directory
3. CLion will automatically detect the CMakeLists.txt
4. Click "Build" -> "Build Project"
5. Run the application using the Run configuration

## Usage

### Basic Usage

```bash
# Start server on default port (8080)
./blade

# Specify custom port
./blade -p 9000

# Disable authentication
./blade --no-auth

# Show help
./blade --help
```

### Command Line Options

- `-p, --port <port>`: Specify server port (default: 8080)
- `-n, --no-auth`: Disable authentication
- `-h, --help`: Display help message

### Accessing the Web Interface

1. Start the BLADE server
2. Note the IP address and HTTP port displayed (typically port 8081)
3. Open a web browser on any device connected to the same network
4. Navigate to: `http://<server-ip>:8081`
5. Login with credentials (default: admin/admin123)

### Default Credentials

When authentication is enabled:
- **Username**: admin
- **Password**: admin123

⚠️ **Security Note**: Change the default credentials in production environments!

## Project Structure

```
BLADE/
├── CMakeLists.txt          # CMake build configuration
├── README.md               # This file
├── LICENSE                 # GPL-3.0 License
├── include/                # Header files
│   ├── Server.h
│   ├── AuthenticationManager.h
│   ├── ConnectionHandler.h
│   ├── HTTPServer.h
│   └── NetworkUtils.h
├── src/                    # Source files
│   ├── main.cpp
│   ├── Server.cpp
│   ├── AuthenticationManager.cpp
│   ├── ConnectionHandler.cpp
│   ├── HTTPServer.cpp
│   └── NetworkUtils.cpp
└── web/                    # Web interface
    ├── index.html
    ├── style.css
    └── script.js
```

## Network Ports

- **Main Server Port**: Configurable (default: 8080)
- **HTTP Server Port**: Main port + 1 (default: 8081)

Make sure these ports are not blocked by your firewall.

## Security Features

- Password-based authentication
- Token-based session management
- Secure password hashing
- Per-client connection tracking
- Optional authentication bypass for trusted networks

## Development

### Adding New Features

1. Add header declarations in `include/`
2. Implement functionality in `src/`
3. Update CMakeLists.txt if adding new files
4. Rebuild the project

### Code Style

- Follow C++17 standards
- Use meaningful variable names
- Document public APIs with Doxygen-style comments
- Keep functions focused and modular

## Troubleshooting

### External Devices Cannot Connect (Connection Timeout)

**Problem:** Web interface works on localhost but times out from phones/tablets/other PCs.

**Solution:** Configure Windows Firewall (or your OS firewall) to allow incoming connections.

**On Windows:**
1. Run the firewall setup script **as Administrator**:
   ```batch
   cd bin
   setup-firewall.bat
   ```
2. Find your server IP address:
   ```batch
   ipconfig
   ```
   Look for "IPv4 Address" (e.g., 192.168.1.100)
3. Access from other devices: `http://YOUR_SERVER_IP`

**Common Issues:**
- ❌ Devices not on same WiFi/network
- ❌ Router AP Isolation enabled (check router settings)
- ❌ VPN active on server PC
- ❌ Antivirus blocking connections
- ❌ Firewall not configured

**Diagnostic Tool:**
```powershell
cd bin
powershell -ExecutionPolicy Bypass -File .\diagnose.ps1
```

📖 **Detailed Guide:** See `docs/NETWORK-ACCESS-FIX.md`

### Port Already in Use

If you get a "port already in use" error:
```bash
# Use a different port
./blade -p 9000
```


### Build Errors

- Ensure you have a C++17 compatible compiler
- Update CMake to version 3.15 or higher
- Check that all dependencies are installed

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built as an academic project to demonstrate C++ and network programming
- Designed for educational purposes and real-world LAN file transfer needs

## Support

For issues, questions, or suggestions, please open an issue on GitHub.

---

**BLADE** - Making local network file transfers fast, secure, and beautiful! 🚀
