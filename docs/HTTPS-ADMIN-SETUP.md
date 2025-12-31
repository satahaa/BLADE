# HTTPS Configuration - Administrator Privileges

## Overview
The BLADE server now **automatically requests administrator privileges** when launched. This ensures HTTPS is always enabled with proper SSL certificate creation.

## What Changed

### 1. **Manifest File Embedded**
- Created `blade.manifest` with `requireAdministrator` execution level
- The manifest is compiled into the executable as a resource

### 2. **CMakeLists.txt Updated**
- Enabled RC (Resource Compiler) language for Windows
- Automatically generates `blade.rc` during configuration
- Compiles manifest into the executable

### 3. **Result**
When you run `blade.exe`:
1. Windows UAC prompt will **automatically appear**
2. User clicks "Yes" to grant administrator privileges
3. Server starts with **full HTTPS support**
4. Self-signed certificate is created successfully
5. **No HTTP fallback** needed

## Usage

### Running the Server
```bash
# Simply run the executable - UAC will prompt automatically
blade.exe -u admin -p mypassword

# Or without authentication
blade.exe --no-auth
```

### Expected Behavior
```
[UAC Prompt appears - User clicks "Yes"]

========================================
  BLADE - Local Network File Transfer
  Version 1.0.0
========================================

Initializing HTTPS with self-signed certificate...
BLADE Server started on port 443
Web interface available at https://192.168.1.100:444
Authentication: ENABLED

Press Ctrl+C to stop the server...
```

## Technical Details

### Files Modified
1. **`blade.manifest`** (NEW) - Defines admin privilege requirement
2. **`CMakeLists.txt`** - Embeds manifest via RC compilation
3. **`HTTPServer.cpp`** - Always attempts HTTPS (no fallback needed with admin rights)

### Build Process
The build now includes:
```
[ 87%] Building RC object CMakeFiles/blade.dir/blade.rc.obj
[100%] Linking CXX executable blade.exe
```

The RC object contains the embedded manifest.

## Benefits
✅ **Always HTTPS** - No more HTTP fallback
✅ **Automatic UAC** - No manual "Run as Administrator"
✅ **Seamless Experience** - One-click execution
✅ **Secure by Default** - Certificate creation always succeeds

## Notes
- The UAC prompt is a **one-time per execution** prompt
- Once elevated, the server runs with full privileges
- This is standard Windows security for applications requiring admin access
- The manifest is permanently embedded in the executable

