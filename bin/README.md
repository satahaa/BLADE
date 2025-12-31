# BLADE Utility Scripts

This folder contains utility scripts for building and running BLADE.

## Scripts

### build.bat
Compiles the BLADE project using CMake and MinGW.

**Usage:**
```batch
cd bin
build.bat
```

**What it does:**
- Configures the project with CMake
- Compiles all source files
- Links the executable
- Copies required DLL files to cmake-build-debug
- Creates blade.exe in cmake-build-debug folder

---

### run.bat
Runs the BLADE server.

**Usage:**
```batch
cd bin
run.bat [options]

# Examples:
run.bat                    # Run with default settings (port 8080, auth enabled)
run.bat -p 9000           # Run on port 9000
run.bat --no-auth         # Run without authentication
run.bat --help            # Show help message
```

**What it does:**
- Sets up the PATH for MinGW DLLs
- Changes to cmake-build-debug directory
- Runs blade.exe with any provided arguments

---

### test-ip.bat
Quick test script to verify IP address detection.

**Usage:**
```batch
cd bin
test-ip.bat
```

**What it does:**
- Starts the BLADE server
- Waits 2 seconds (check console for IP output)
- Automatically stops the server

---

## Quick Start

1. **First time setup:**
   ```batch
   cd bin
   build.bat
   ```

2. **Run the server:**
   ```batch
   run.bat
   ```

3. **Access the web interface:**
   - Open browser and go to the IP address shown in console
   - Default port: 8081 (HTTP web interface)
   - Default credentials: admin / admin123

---

## Notes

- All scripts automatically handle relative paths
- Scripts must be run from the `bin` folder
- The executable is built in `../cmake-build-debug/`
- Required DLLs are automatically copied during build

