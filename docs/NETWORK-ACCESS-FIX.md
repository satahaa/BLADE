# Network Access Fix - External Device Connection Issue

## Problem
The BLADE server web interface works on localhost (the PC running the server) but times out when accessed from other devices on the same network.

## Root Cause
**Windows Firewall is blocking incoming connections** to port 80 (HTTP) and port 8443 (main server).

## Solution

### Step 1: Configure Windows Firewall

Run the firewall setup script **as Administrator**:

1. Open PowerShell **as Administrator** (Right-click → Run as Administrator)
2. Navigate to the project directory:
   ```powershell
   cd D:\Study\3-1\Project\BLADE\bin
   ```
3. Run the firewall setup script:
   ```powershell
   powershell -ExecutionPolicy Bypass -File .\setup-firewall.ps1
   ```

This script will:
- Add firewall rules for port 80 (HTTP web interface)
- Add firewall rules for port 8443 (main server)
- Display your local IP address for access from other devices

### Step 2: Find Your Server IP Address

After running the firewall setup, note your IP address (something like `192.168.x.x`).

Or manually find it:
```powershell
ipconfig
```
Look for "IPv4 Address" under your active network adapter (WiFi or Ethernet).

### Step 3: Access from Other Devices

On your phone, tablet, or another computer:
1. Make sure the device is on the **same WiFi network**
2. Open a web browser
3. Navigate to: `http://YOUR_SERVER_IP`
   - Example: `http://192.168.1.100`

## Verification

Run the diagnostic script to verify everything is configured correctly:
```powershell
cd D:\Study\3-1\Project\BLADE\bin
powershell -ExecutionPolicy Bypass -File .\diagnose.ps1
```

This will check:
- ✓ Server is running
- ✓ Ports are listening
- ✓ Firewall rules are configured
- ✓ Network connectivity

## Common Issues

### Issue 1: Still Can't Connect
**Check:**
- Both devices are on the same WiFi/network (not one on WiFi and one on Ethernet to different networks)
- Router AP Isolation is disabled (check router settings)
- No VPN is active on the server PC
- Antivirus software isn't blocking connections

### Issue 2: Router AP Isolation
Some routers have "AP Isolation" or "Client Isolation" enabled which prevents devices from communicating with each other.

**Fix:** Access your router settings (usually `192.168.1.1` or `192.168.0.1`) and disable AP Isolation.

### Issue 3: Antivirus Blocking
Some antivirus software has additional firewall features.

**Fix:** Temporarily disable antivirus or add blade.exe to the allowed programs list.

### Issue 4: Corporate/University Network
Some networks block device-to-device communication for security.

**Fix:** Use a personal WiFi hotspot or home network instead.

## Technical Details

### What Was Changed
No code changes were needed. The issue was purely Windows Firewall configuration.

### Firewall Rules Created
- **BLADE Server - HTTP (Port 80)**
  - Direction: Inbound
  - Protocol: TCP
  - Port: 80
  - Action: Allow

- **BLADE Server - Main (Port 8443)**
  - Direction: Inbound
  - Protocol: TCP
  - Port: 8443
  - Action: Allow

### Server Binding
The server correctly binds to `0.0.0.0:80` which accepts connections from all network interfaces. This was already working correctly.

## Quick Reference

### Start Server
```batch
bin\run.bat
```

### Configure Firewall (Administrator)
```powershell
bin\setup-firewall.ps1
```

### Run Diagnostics
```powershell
bin\diagnose.ps1
```

### Access Web Interface
- **From server PC:** `http://localhost` or `http://127.0.0.1`
- **From other devices:** `http://YOUR_SERVER_IP` (e.g., `http://192.168.1.100`)

---
**Date:** January 1, 2026  
**Status:** FIREWALL CONFIGURATION REQUIRED

