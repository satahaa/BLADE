# BLADE - Bi-Directional LAN Asset Distribution Engine

BLADE is a high-performance, C++ based local network file transfer system that enables secure data sharing between devices on the same LAN. Built with modern C++17 and featuring a beautiful web-based GUI, BLADE provides an efficient, low-level solution for PC-to-PC and PC-to-Mobile file transfers.

## Features

- 🚀 **High Performance**: Written in C++ for maximum efficiency and low-level control
- 🔐 **Secure Authentication**: Optional authentication system to protect your network
- 🌐 **Web-Based GUI**: Beautiful HTML/CSS/JavaScript interface accessible from any device
- 🔌 **Easy Connection**: Simple port-based networking with automatic discovery
- 💻 **Cross-Platform**: Works on Windows, Linux, and macOS
- 📦 **CMake Build System**: Professional build configuration for CLion and other IDEs

## Architecture

BLADE consists of several key components:

- **Server**: Main network server managing connections and authentication
- **Authentication Manager**: Handles user credentials and session tokens
- **Connection Handler**: Manages connected clients and data transfer
- **HTTP Server**: Serves the web interface to connected devices
- **Network Utils**: Low-level socket operations and networking utilities

## Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Operating System: Windows, Linux, or macOS

## Building the Project

### Using CMake

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

# The executable will be in the build directory
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

### Port Already in Use

If you get a "port already in use" error:
```bash
# Use a different port
./blade -p 9000
```

### Cannot Connect from Other Devices

1. Check firewall settings
2. Ensure devices are on the same network
3. Verify the server IP address is correct
4. Try disabling authentication: `./blade --no-auth`

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
