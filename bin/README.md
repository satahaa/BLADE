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

### setup-firewall.bat / setup-firewall.ps1
**⚠️ IMPORTANT:** Configures Windows Firewall to allow external device connections.

**Usage:**
```batch
cd bin
setup-firewall.bat    # Automatically requests Administrator privileges
```
Or run the PowerShell script directly as Administrator:
```powershell
powershell -ExecutionPolicy Bypass -File .\setup-firewall.ps1
```

**What it does:**
- Adds Windows Firewall rule for port 80 (HTTP web interface)
- Adds Windows Firewall rule for port 8443 (main server)
- Displays your local IP address for external access
- **Must be run as Administrator**

**Why is this needed?**
By default, Windows Firewall blocks incoming connections. Without configuring firewall rules, external devices (phones, tablets, other PCs) cannot connect to your BLADE server.

---

### diagnose.ps1
Diagnoses network and connectivity issues.

**Usage:**
```powershell
cd bin
powershell -ExecutionPolicy Bypass -File .\diagnose.ps1
```

**What it checks:**
- Server is running
- Ports 80 and 8443 are listening
- Network adapters and IP addresses
- Windows Firewall rules are configured
- Localhost connectivity test

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

2. **Configure firewall (REQUIRED for external devices):**
   ```batch
   setup-firewall.bat
   ```
   ⚠️ **Must run as Administrator** - Right-click and select "Run as Administrator"

3. **Run the server:**
   ```batch
   run.bat
   ```

4. **Access the web interface:**
   - **From server PC:** `http://localhost` or `http://127.0.0.1`
   - **From other devices:** `http://YOUR_SERVER_IP` (e.g., `http://192.168.1.100`)
   - Find your IP: Run `ipconfig` and look for "IPv4 Address"
   - Default credentials (if auth enabled): admin / admin123

---

## Troubleshooting External Device Connections

If other devices can't connect (timeout error):

1. **Run diagnostics:**
   ```powershell
   powershell -ExecutionPolicy Bypass -File .\diagnose.ps1
   ```

2. **Configure firewall rules:**
   ```batch
   setup-firewall.bat
   ```
   (Must run as Administrator)

3. **Common issues:**
   - ❌ Devices not on same WiFi/network
   - ❌ Router AP Isolation enabled
   - ❌ VPN active on server PC
   - ❌ Antivirus blocking connections
   - ❌ Windows Firewall not configured

4. **Detailed troubleshooting:**
   See: `docs/NETWORK-ACCESS-FIX.md`

---

## Notes

- All scripts automatically handle relative paths
- Scripts must be run from the `bin` folder
- The executable is built in `../cmake-build-debug/`
- Required DLLs are automatically copied during build

