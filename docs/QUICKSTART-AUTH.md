# Quick Start Guide - BLADE Authentication

## ✅ Implementation Complete

The BLADE server now supports **dynamic authentication** where the web interface automatically adapts based on how you start the server.

---

## 🚀 How to Run

### Option 1: No Authentication (Default - Quick Start)
```bash
blade.exe
```

**What happens:**
- ✅ Server starts on port 443 (HTTPS)
- ✅ Web interface on port 444 (HTTPS)
- ✅ **No login page** - dashboard loads immediately
- ✅ Anyone on your network can access files
- ✅ Perfect for trusted networks or testing

**Access:**
- Local: `https://localhost:444`
- Network: `https://<your-ip>:444`

---

### Option 2: With Authentication (Secure)
```bash
blade.exe -u admin -p mypassword
```

**What happens:**
- ✅ Server starts on port 443 (HTTPS)
- ✅ Web interface on port 444 (HTTPS)
- ✅ **Login page appears** - credentials required
- ✅ Users must enter: username=`admin`, password=`mypassword`
- ✅ Logout button visible for ending sessions
- ✅ Perfect for shared/untrusted networks

**Access:**
- Local: `https://localhost:444`
- Network: `https://<your-ip>:444`
- **Must login with the username and password you specified**

---

## 📋 Requirements

### Administrator Privileges
Port 443 requires admin rights on Windows:

1. **Right-click** on `blade.exe` or the terminal
2. Select **"Run as Administrator"**
3. Click **"Yes"** on the UAC prompt

### Browser Security Warning
Since we use a self-signed certificate for local HTTPS:

1. Browser will show **"Your connection is not private"**
2. Click **"Advanced"**
3. Click **"Proceed to localhost (unsafe)"**
4. This is normal for local development!

---

## 🔧 Build Instructions

If you need to rebuild:

```bash
# Set PATH for CLion tools
$env:PATH = "E:\CLion 2025.2.3\bin\mingw\bin;E:\CLion 2025.2.3\bin\cmake\win\x64\bin;$env:PATH"

# Navigate to build directory
cd D:\Study\3-1\Project\BLADE\cmake-build-debug

# Configure
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

# Build
mingw32-make
```

---

## 🎯 Key Features

### ✨ No Authentication Mode
- **No login page** - instant access
- **No credentials needed** - just open the URL
- **Logout button hidden** - no need for sessions
- **Perfect for:** Local testing, trusted home networks

### 🔒 Authentication Mode
- **Login page required** - security enabled
- **Custom credentials** - you choose username/password
- **Session management** - logout when done
- **Perfect for:** Shared networks, office environments

---

## 📝 Command Line Arguments

```
BLADE - Bi-Directional LAN Asset Distribution Engine

Usage: blade.exe [options]

Options:
  -u, --username <user>  Username for authentication (requires -p)
  -p, --password <pass>  Password for authentication (requires -u)
  -h, --help             Show this help message

Examples:
  blade.exe                                    (no authentication)
  blade.exe -u admin -p mypassword             (with authentication)
  blade.exe --username admin --password secret (with authentication)

Note: Server runs on port 443 (HTTPS) and requires administrator/root privileges
      Authentication is disabled by default - web interface will load directly
      When -u and -p are provided, web interface will show a login page
```

---

## 🔍 Troubleshooting

### Problem: "No output when running blade.exe"
**Solution:** You need administrator privileges. Right-click and "Run as Administrator"

### Problem: "Browser shows security warning"
**Solution:** This is normal for self-signed certificates. Click "Advanced" → "Proceed"

### Problem: "Can't access from other devices"
**Solution:** 
1. Check your firewall - allow port 443 and 444
2. Make sure you're using your actual IP (not localhost)
3. Try: `https://<your-lan-ip>:444`

### Problem: "Login page won't accept credentials"
**Solution:** Make sure you're entering the EXACT username and password you used with `-u` and `-p`

### Problem: "Port 443 already in use"
**Solution:** Another service is using HTTPS. Stop it or change BLADE's port in the code

---

## 📊 What Changed

### Before (Old Behavior)
- ❌ Hardcoded username/password in JavaScript
- ❌ Login page always shown, even with `-n` flag
- ❌ Confusing flag behavior

### After (New Behavior)
- ✅ No hardcoded credentials
- ✅ Server controls authentication via API
- ✅ Login page shown ONLY when `-u` and `-p` are used
- ✅ Direct dashboard access when no auth
- ✅ Clean, intuitive behavior

---

## 🎓 Technical Details

### API Endpoint: `/api/auth-config`

The web interface fetches this JSON endpoint to determine authentication:

**No Auth Response:**
```json
{
  "authEnabled": false
}
```

**Auth Enabled Response:**
```json
{
  "authEnabled": true,
  "username": "admin",
  "password": "secret"
}
```

The JavaScript uses this to:
1. Hide/show login page
2. Validate credentials (if auth enabled)
3. Hide/show logout button

---

## 📚 Additional Documentation

- `AUTH-CONFIG.md` - Detailed authentication configuration guide
- `IMPLEMENTATION-SUMMARY.md` - Technical implementation details
- `HTTPS-ADMIN-SETUP.md` - HTTPS and admin privileges setup

---

## ✅ Testing Checklist

- [ ] Run without args - dashboard loads immediately
- [ ] Run with `-u` and `-p` - login page appears
- [ ] Login with correct credentials - dashboard loads
- [ ] Login with wrong credentials - error shown
- [ ] Logout button hidden in no-auth mode
- [ ] Logout button visible in auth mode
- [ ] Access from another device on LAN

---

## 🎉 Summary

**Your BLADE server is ready!**

- **Quick test?** Just run `blade.exe` (as admin) and open `https://localhost:444`
- **Need security?** Run `blade.exe -u yourusername -p yourpassword` (as admin)
- **Share with others?** Give them `https://<your-ip>:444` and credentials (if auth enabled)

**That's it! Your server intelligently adapts to your security needs.**

