# BLADE Qt GUI - Installation & Usage Guide

## 🎯 Quick Start

### If You Have Qt6 Installed
```powershell
# 1. Build the project
.\build_gui.bat

# 2. Run the GUI
cd bin
.\blade_gui.exe
```

### If You Don't Have Qt6
You have two options:
1. **Install Qt6** and build the GUI version (recommended for best experience)
2. **Use CLI version** (already built, no Qt needed)

---

## 📦 Installing Qt6

### Option 1: Online Installer (Recommended)
1. Download from: https://www.qt.io/download-qt-installer
2. Run the installer
3. Select Qt 6.5 or later
4. Choose components:
   - Qt 6.x for MinGW (or MSVC)
   - CMake integration
5. Install to default location (C:\Qt\)

### Option 2: Package Manager
```powershell
# Using Chocolatey
choco install qt6

# Or using winget
winget install --id=Qt.Qt
```

### After Installation
Add Qt to your PATH or set CMAKE_PREFIX_PATH:
```powershell
# Temporary (current session)
$env:CMAKE_PREFIX_PATH="C:\Qt\6.5.0\mingw_64"

# Permanent (system-wide)
[System.Environment]::SetEnvironmentVariable("CMAKE_PREFIX_PATH", "C:\Qt\6.5.0\mingw_64", "User")
```

---

## 🏗️ Building the Project

### Automated Build (Recommended)
```powershell
# From project root directory
.\build_gui.bat
```
This script will:
- Auto-detect Qt installation
- Configure CMake
- Build both CLI and GUI versions
- Copy files to bin/ folder
- Report any errors

### Manual Build
```powershell
# Set Qt path (if not in PATH)
$env:CMAKE_PREFIX_PATH="C:\Qt\6.5.0\mingw_64"

# Configure
mkdir build
cd build
cmake .. -G "MinGW Makefiles"

# Build
cmake --build . --config Release

# Go back to project root
cd ..
```

### Expected Output
```
bin/
├── blade.exe          ✅ CLI version
├── blade_gui.exe      ✅ GUI version (if Qt found)
├── Qt6Core.dll        ✅ Qt dependency
├── Qt6Gui.dll         ✅ Qt dependency
├── Qt6Widgets.dll     ✅ Qt dependency
├── blade.ico          ✅ Icon file
└── web/               ✅ Web interface files
```

---

## 🚀 Running the Application

### GUI Version (Recommended)
```powershell
cd bin
.\blade_gui.exe
```

**Or double-click** `blade_gui.exe` in File Explorer.

**Note**: May need to run as Administrator for port 80 access.

### CLI Version (Traditional)
```powershell
# Without authentication
cd bin
.\blade.exe

# With authentication
.\blade.exe -u admin -p password123
```

---

## 💻 Using the GUI

### Starting Without Authentication (Quick Mode)
Perfect for: Sharing with trusted friends/family

1. Launch `blade_gui.exe`
2. Click **"Start without Authentication"**
3. Server starts immediately
4. QR code appears
5. Scan QR with mobile device OR open URL in browser

### Starting With Authentication (Secure Mode)
Perfect for: Office/work environment

1. Launch `blade_gui.exe`
2. Enter **username** (e.g., "admin")
3. Enter **password** (e.g., "secure123")
4. Click **"Start with Authentication"**
5. Server starts with login protection
6. Share credentials with authorized users
7. QR code appears
8. Users must login to access

---

## 📱 Connecting from Other Devices

### Mobile Device (Easiest)
1. Open camera app
2. Point at QR code on screen
3. Tap notification/link
4. Browser opens to BLADE
5. Upload/download files

### Computer/Tablet
1. Note the URL from GUI (e.g., http://192.168.1.10)
2. Open browser
3. Type the URL
4. Press Enter
5. Access BLADE web interface

### If Authentication is Enabled
- You'll see a login page
- Enter the username and password you set
- Click Login
- Now you can transfer files

---

## 🎨 GUI Features

### Beautiful Design
- **Dark gradient background** - Modern and easy on eyes
- **Glassmorphic effects** - Translucent containers
- **Professional typography** - Segoe UI font family
- **Smooth animations** - Elegant transitions
- **Responsive layout** - Adapts to window size

### Smart Features
- **Auto-generate QR codes** - Instant mobile access
- **Copy-paste friendly URLs** - Selectable text
- **Clear status indicators** - Know when server is running
- **Port information** - See web and transfer ports
- **Error handling** - Helpful messages if something goes wrong

---

## 🔧 Troubleshooting

### "blade_gui.exe not found after build"
**Cause**: Qt6 not found during build

**Solution**:
```powershell
# Install Qt6 (see above)
# Set PATH
$env:CMAKE_PREFIX_PATH="C:\Qt\6.5.0\mingw_64"
# Rebuild
.\build_gui.bat
```

### "Failed to start server"
**Cause**: Port 80 or 8080 in use

**Solutions**:
1. Run as Administrator (Right-click → Run as administrator)
2. Close other web servers (IIS, Apache, XAMPP)
3. Check Task Manager for processes on ports
4. Restart computer

**Check what's using port 80:**
```powershell
netstat -ano | findstr :80
```

### "Qt DLLs not found" or "Missing Qt6Core.dll"
**Cause**: Qt DLLs not in PATH

**Solutions**:
1. Add Qt bin folder to PATH:
   ```powershell
   $env:PATH += ";C:\Qt\6.5.0\mingw_64\bin"
   ```
2. Or manually copy DLLs to bin/ folder
3. Or rebuild - CMake should auto-copy them

### "Can't connect from other device"
**Cause**: Windows Firewall blocking

**Solution**:
1. Windows Settings → Firewall & network protection
2. Allow an app through firewall
3. Click "Change settings"
4. Click "Allow another app"
5. Browse to blade_gui.exe
6. Check "Private" networks
7. Click Add

### QR Code won't scan
**Solutions**:
1. Maximize window for larger QR code
2. Screenshot and zoom
3. Adjust phone camera distance
4. Ensure good lighting
5. Manually type URL instead

### Wrong IP address shown
**Check your actual IP:**
```powershell
ipconfig
```
Look for "IPv4 Address" under your active network adapter (usually Wi-Fi or Ethernet).

If showing 0.0.0.0 or wrong IP:
- Check network connection
- Restart application
- Check network adapter settings

---

## 📋 System Requirements

### Minimum
- Windows 10 (64-bit)
- 4 GB RAM
- 100 MB disk space
- Network adapter

### Recommended
- Windows 10/11 (64-bit)
- 8 GB RAM
- Network connection (WiFi or Ethernet)
- Administrator privileges

### For Building
- CMake 3.15+
- C++23 compiler (GCC 11+, MSVC 2019+)
- Qt6 (6.5+) with Core, Widgets, Gui modules

---

## 📚 Additional Documentation

- **GUI Overview**: `docs/GUI-README.md`
- **Quick Start Guide**: `docs/GUI-QUICKSTART.md`
- **Implementation Details**: `docs/GUI-IMPLEMENTATION.md`
- **Web Interface**: `docs/HTTP-SIMPLE-GUIDE.md`
- **Authentication**: `docs/QUICKSTART-AUTH.md`
- **Network Issues**: `docs/NETWORK-ACCESS-FIX.md`

---

## 🎯 Common Use Cases

### Use Case 1: Share Photos with Family
```
1. Start without authentication (quick mode)
2. Show QR code to family members
3. They scan and open in browser
4. Upload/download photos
5. No login needed - trusted environment
```

### Use Case 2: Office Document Sharing
```
1. Enter username and password
2. Start with authentication
3. Share URL with colleagues
4. They login with credentials
5. Secure file transfer
6. Track who's connected
```

### Use Case 3: Phone to PC Transfer
```
1. Start without authentication (faster)
2. Scan QR code with phone
3. Select files on phone
4. Upload to PC
5. Files appear in PC directory
```

---

## 🚨 Important Notes

### Security
- Server only accessible on local network (not internet)
- Use authentication for sensitive files
- Don't share credentials over insecure channels
- Close server when not in use

### Performance
- Both devices should be on same network
- WiFi: Close to router for best speed
- Wired: Ethernet is faster than WiFi
- Close unnecessary applications

### Port Usage
- **Port 80**: Web interface (HTTP)
- **Port 8080**: File transfer
- Both ports must be free
- Administrator needed for port 80

---

## ❓ FAQ

**Q: Do I need Qt to use BLADE?**
A: No, but GUI version is much easier. CLI version works without Qt.

**Q: Can I use BLADE over the internet?**
A: No, BLADE is designed for local network only. Don't expose to internet.

**Q: What file types can I transfer?**
A: Any file type - images, videos, documents, archives, etc.

**Q: Is there a file size limit?**
A: No hard limit, but large files (>1GB) may take time depending on network speed.

**Q: Can multiple people connect at once?**
A: Yes, BLADE supports multiple simultaneous connections.

**Q: Does BLADE work on Linux/Mac?**
A: CLI version yes (needs recompile). GUI version should work if Qt6 installed.

**Q: Can I customize the port numbers?**
A: Currently hardcoded (80 and 8080). Future versions may add configuration.

**Q: Where do uploaded files go?**
A: By default to the directory where blade_gui.exe is running.

---

## 🆘 Getting Help

If you encounter issues:

1. **Check documentation** in `docs/` folder
2. **Review error messages** carefully
3. **Check system logs** if server crashes
4. **Test with CLI version** to isolate GUI issues
5. **Verify network connectivity** with `ping` and `ipconfig`
6. **Check firewall settings** if connection fails

---

## 🎉 Success Indicators

You'll know everything is working when:

✅ GUI launches without errors
✅ Server starts (see "Server Running" message)
✅ QR code displays clearly
✅ URL shown in blue box
✅ Can open URL in your own browser
✅ Web interface loads
✅ Can upload/download test file
✅ Mobile device can scan QR code

---

**Enjoy using BLADE! 🚀**

For the best experience, use the Qt GUI version. For automation/scripting, use the CLI version.

