# BLADE GUI - Quick Reference Card

## 🚀 Launch Commands

```powershell
# GUI Version
cd bin
.\blade_gui.exe

# CLI Version
.\blade.exe                           # No auth
.\blade.exe -u admin -p password123   # With auth
```

---

## 📋 Two Ways to Start

### 🔓 Without Authentication (Quick Share)
1. Launch blade_gui.exe
2. Click **"Start without Authentication"**
3. ✅ Server starts instantly

**Use when:** Sharing with trusted friends/family

### 🔐 With Authentication (Secure)
1. Launch blade_gui.exe
2. Enter username and password
3. Click **"Start with Authentication"**
4. ✅ Server starts with login protection

**Use when:** Office/work environment or sensitive files

---

## 📱 Connect from Other Devices

### Method 1: QR Code (Mobile - Easiest)
1. Open camera on phone
2. Point at QR code on screen
3. Tap notification
4. Browser opens → BLADE interface

### Method 2: URL (Any Device)
1. Note URL from GUI (e.g., http://192.168.1.10)
2. Open browser
3. Type URL
4. Access BLADE

---

## 🎨 GUI Layout

```
┌─────────────────────────────────┐
│         BLADE LOGO              │
│           BLADE                 │
│  Local Network File Transfer    │
│                                 │
│  [Username Field] (optional)    │
│  [Password Field] (optional)    │
│                                 │
│  [Start with Authentication]    │
│  [Start without Authentication] │
└─────────────────────────────────┘
         ↓ (After Start)
┌─────────────────────────────────┐
│    Server Running ✓             │
│                                 │
│   ┌─────────────────┐           │
│   │   QR CODE       │           │
│   └─────────────────┘           │
│                                 │
│   http://192.168.1.10          │
│   Port 80 | Port 8080          │
└─────────────────────────────────┘
```

---

## ⚙️ Build Instructions

### Quick Build
```powershell
.\build_gui.bat
```

### Manual Build
```powershell
$env:CMAKE_PREFIX_PATH="C:\Qt\6.5.0\mingw_64"
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

---

## 🔧 Common Issues & Fixes

| Problem | Solution |
|---------|----------|
| **blade_gui.exe missing** | Install Qt6, set CMAKE_PREFIX_PATH, rebuild |
| **"Failed to start server"** | Run as Administrator |
| **Qt DLLs not found** | Add Qt\bin to PATH |
| **Can't connect** | Check firewall, allow blade_gui.exe |
| **QR won't scan** | Maximize window or type URL manually |

---

## 📂 File Locations

```
bin/
├── blade.exe          ← CLI version
├── blade_gui.exe      ← GUI version (run this)
├── Qt6*.dll           ← Qt dependencies
└── web/               ← Web interface files
```

---

## 🌐 Ports Used

| Port | Purpose |
|------|---------|
| **80** | Web interface (HTTP) |
| **8080** | File transfer |

*Both must be free. Port 80 needs admin privileges.*

---

## 📖 Documentation Quick Links

| Document | Purpose |
|----------|---------|
| `INSTALL-GUI.md` | Installation guide |
| `docs/GUI-QUICKSTART.md` | User quick start |
| `docs/GUI-README.md` | Technical overview |
| `GUI-COMPLETE.md` | Implementation summary |

---

## 💡 Pro Tips

1. **QR Code best** for mobile → instant connection
2. **Run as Admin** to avoid port 80 errors
3. **Check firewall** if devices can't connect
4. **Use auth mode** for sensitive files
5. **Both CLI and GUI** use same backend

---

## 🎯 Quick Decision Matrix

**Choose GUI when:**
- You want visual interface
- QR code scanning needed
- Less technical users
- Quick, easy setup

**Choose CLI when:**
- Automating with scripts
- Running as service
- Minimal resource use
- Remote/SSH sessions

---

## 📊 At a Glance

| Feature | Value |
|---------|-------|
| **Min Window Size** | 600x700px |
| **Startup Time** | 1-2 seconds |
| **Memory Use** | ~50-80 MB |
| **Qt Version** | 6.5+ |
| **C++ Standard** | C++23 |

---

## ⌨️ Keyboard Shortcuts

| Key | Action |
|-----|--------|
| **Tab** | Next field |
| **Enter** | Start server |
| **Ctrl+C** | Copy URL (when selected) |

---

## 🎨 Color Guide

| Color | Meaning |
|-------|---------|
| **Blue (#4a9eff)** | Primary actions, links |
| **Green (#4ade80)** | Success, server running |
| **Red** | Errors, warnings |
| **Gray** | Secondary info |

---

## 📱 Mobile Compatibility

✅ **Works with:**
- iPhone (iOS 12+)
- Android phones
- Tablets
- Any device with camera + browser

---

## 🔐 Security Notes

- 🔒 Local network only (not internet)
- 🔒 Use auth for sensitive files
- 🔒 Credentials stored in memory only
- 🔒 No data sent outside your network

---

## ✅ Success Checklist

Before connecting:
- [x] GUI launched without errors
- [x] "Server Running" message displayed
- [x] QR code visible and clear
- [x] URL shown in blue box
- [x] Can open URL in own browser first

---

## 🆘 Emergency Troubleshooting

```powershell
# Check what's on port 80
netstat -ano | findstr :80

# Check your IP
ipconfig

# Test with CLI version
cd bin
.\blade.exe
```

---

## 📞 Getting Help

1. Check `INSTALL-GUI.md`
2. Review error messages
3. Check firewall settings
4. Verify Qt installation
5. Test CLI version

---

## 🎉 Quick Win

**Fastest way to get started:**
```powershell
cd bin
.\blade_gui.exe
# Click "Start without Authentication"
# Scan QR code with phone
# Done! 🎉
```

---

*Print this card for quick reference! 📄*

**Version**: 1.0.0 | **Date**: January 9, 2026

