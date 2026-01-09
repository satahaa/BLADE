# BLADE GUI Quick Start Guide

## Installation & Setup

### Step 1: Build the Application

#### Option A: Using Build Script (Recommended)
```powershell
# Run from project root
.\build_gui.bat
```

#### Option B: Manual Build
```powershell
# Set Qt path (adjust to your installation)
$env:CMAKE_PREFIX_PATH="C:\Qt\6.5.0\mingw_64"

# Build
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release
cd ..
```

### Step 2: Run the Application

#### From bin folder:
```powershell
cd bin
.\blade_gui.exe
```

**OR** use the helper script:
```powershell
cd bin
.\run_gui.bat
```

## Using the GUI

### Starting without Authentication (Quick Start)

1. **Launch** `blade_gui.exe`
2. **Click** "Start without Authentication"
3. **Server starts** immediately
4. **QR Code appears** - scan with mobile device
5. **URL displayed** - open in any browser on the network

```
┌─────────────────────────┐
│      BLADE Logo         │
│                         │
│  [Start with Auth]      │
│  [Start without Auth]   │ ← Click this
└─────────────────────────┘
           ↓
┌─────────────────────────┐
│   Server Running ✓      │
│                         │
│     [QR CODE HERE]      │
│                         │
│  http://192.168.1.10    │
│  Web: Port 80           │
│  Transfer: Port 8080    │
└─────────────────────────┘
```

### Starting with Authentication (Secure)

1. **Launch** `blade_gui.exe`
2. **Enter** username (e.g., "admin")
3. **Enter** password (e.g., "secure123")
4. **Click** "Start with Authentication"
5. **Server starts** with login protection
6. **QR Code appears** with URL
7. **Clients must login** with the username/password

```
┌─────────────────────────┐
│      BLADE Logo         │
│                         │
│  Username: [admin___]   │
│  Password: [•••••••]    │
│                         │
│  [Start with Auth]      │ ← Click this
│  [Start without Auth]   │
└─────────────────────────┘
           ↓
┌─────────────────────────┐
│   Server Running ✓      │
│   (Auth Required)       │
│                         │
│     [QR CODE HERE]      │
│                         │
│  http://192.168.1.10    │
│  Login: admin           │
└─────────────────────────┘
```

## Connecting from Other Devices

### Mobile Device (Recommended)
1. **Open camera app** on your phone
2. **Scan the QR code** displayed in GUI
3. **Tap the notification** or link
4. **Browser opens** to BLADE web interface
5. **Start transferring files!**

### Desktop/Laptop
1. **Note the URL** from GUI (e.g., http://192.168.1.10)
2. **Open browser** on any device
3. **Type the URL** in address bar
4. **Press Enter**
5. **Web interface loads**

### If Authentication is Enabled
- Web interface will show **login page**
- Enter the **same username/password** you set in GUI
- Click **Login**
- Web interface unlocks

## Common Scenarios

### Scenario 1: Share files with friends (No security needed)
```
1. Start without Authentication
2. Show QR code to friends
3. They scan and access instantly
4. Share files freely
```

### Scenario 2: Office/work environment (Security required)
```
1. Enter username and password
2. Start with Authentication
3. Share URL with colleagues
4. They login with credentials
5. Secure file sharing
```

### Scenario 3: Quick phone-to-PC transfer
```
1. Start without Authentication (faster)
2. Scan QR with phone
3. Upload files from phone
4. Files appear on PC
```

## Troubleshooting

### "Failed to start server" Error
**Problem**: Port 80 or 8080 already in use

**Solutions**:
1. Close any web servers (IIS, Apache, XAMPP)
2. Check Task Manager for processes using ports
3. Run as Administrator
4. Restart computer if needed

```powershell
# Check what's using port 80
netstat -ano | findstr :80
```

### GUI Won't Start
**Problem**: Qt DLLs missing

**Solution**: Add Qt to PATH
```powershell
$env:PATH += ";C:\Qt\6.5.0\mingw_64\bin"
```

### Can't Connect from Other Devices
**Problem**: Firewall blocking

**Solution**: Allow BLADE through firewall
```
1. Windows Settings
2. Firewall & network protection
3. Allow an app through firewall
4. Add blade_gui.exe
5. Enable for Private networks
```

### QR Code Won't Scan
**Problem**: QR code too small or blurry

**Solutions**:
1. Maximize the window
2. Take screenshot and zoom
3. Manually type the URL instead

### Wrong IP Address Displayed
**Problem**: Multiple network adapters

**Solution**: Check your actual IP
```powershell
ipconfig
```
Look for "IPv4 Address" under your active network adapter.

## Tips & Best Practices

### 🔐 Security
- Use authentication when sharing sensitive files
- Don't share credentials over insecure channels
- Server only accessible on local network (not internet)

### 🚀 Performance
- Both devices should be on same network
- Close unnecessary applications for better speed
- Wired connection is faster than WiFi

### 📱 Mobile Access
- QR codes are the fastest way to connect
- Bookmark the URL on mobile for quick access
- Use WiFi (not mobile data) for transfers

### 💡 General
- Keep the GUI window open while transferring
- Don't close until all transfers complete
- Server stops when you close the GUI

## Advanced Usage

### Running as Administrator
For port 80 access (standard HTTP port):
```powershell
# Right-click blade_gui.exe
# Select "Run as administrator"
```

### Command Line Alternative
If you prefer terminal/console:
```powershell
.\blade.exe -u admin -p password123
```

## Feature Overview

| Feature | GUI Version | CLI Version |
|---------|-------------|-------------|
| Visual QR Code | ✅ | ❌ (Text only) |
| Easy Auth Setup | ✅ | ⚠️ (Flags needed) |
| Status Display | ✅ | ⚠️ (Console) |
| User-Friendly | ✅ | ❌ |
| Automated | ⚠️ | ✅ |
| Resource Usage | Higher | Lower |

## Next Steps

After starting the server:

1. **Test the connection**
   - Open the URL in your own browser first
   - Verify the web interface loads

2. **Try file transfer**
   - Click "Choose Files" or drag files
   - Click "Send"
   - Check files received

3. **Connect mobile device**
   - Scan QR code
   - Test upload from mobile

4. **Share with others**
   - Show QR code or share URL
   - Guide them through upload process

## Getting Help

- **Documentation**: See `docs/GUI-README.md`
- **Web Interface**: See `docs/HTTP-SIMPLE-GUIDE.md`
- **Authentication**: See `docs/QUICKSTART-AUTH.md`
- **Network Issues**: See `docs/NETWORK-ACCESS-FIX.md`

## Screenshots Reference

### Login View
```
┌───────────────────────────────┐
│                               │
│         [BLADE LOGO]          │
│            BLADE              │
│  Local Network File Transfer  │
│                               │
│  ╔═══════════════════════╗   │
│  ║ Username (Optional)   ║   │
│  ║ [________________]    ║   │
│  ║                       ║   │
│  ║ Password (Optional)   ║   │
│  ║ [________________]    ║   │
│  ╚═══════════════════════╝   │
│                               │
│  [Start with Authentication]  │
│  [Start without Auth]         │
│                               │
└───────────────────────────────┘
```

### Server Running View
```
┌───────────────────────────────┐
│     Server Running ✓          │
│                               │
│  ┌─────────────────────┐     │
│  │                     │     │
│  │    █████  █████    │     │
│  │    QR CODE HERE    │     │
│  │    █████  █████    │     │
│  │                     │     │
│  └─────────────────────┘     │
│                               │
│     http://192.168.1.10      │
│                               │
│  Web Interface: Port 80       │
│  File Transfer: Port 8080     │
│                               │
│  Scan QR or visit URL from    │
│  any device on your network   │
│                               │
└───────────────────────────────┘
```

---

**Enjoy using BLADE! 🚀**

