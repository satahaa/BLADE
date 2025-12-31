# BLADE - IP Address Detection Fix

## Problem
When running `blade.exe`, the server was displaying IP address `192.168.57.1` (a virtual adapter IP from VMware/VirtualBox) instead of the actual network IP `10.5.0.216` shown in `ipconfig`.

## Solution Applied

### Modified `NetworkUtils::getLocalIPAddress()` in `src/NetworkUtils.cpp`

The function now:
1. **Iterates through all network interfaces** instead of just taking the first one
2. **Filters out virtual adapter IP ranges:**
   - `192.168.x.x` (VMware, VirtualBox host-only adapters)
   - `172.16.x.x` - `172.31.x.x` (Docker, some VPNs)
   - `169.254.x.x` (APIPA/Link-local addresses)
   - `127.x.x.x` (Loopback)

3. **Prioritizes actual network IPs** like `10.x.x.x` (your network)
4. **Has fallback logic** - If no preferred IP is found, uses virtual adapter IP

### Key Code Changes

```cpp
// Before: Took first IP found
return firstIP;

// After: Filters and prioritizes
for (all interfaces) {
    if (not loopback and not virtual) {
        return actualNetworkIP;  // e.g., 10.5.0.216
    }
}
```

## How to Use

### Building the Project

Use the provided `build.bat` script:
```batch
cd D:\Study\3-1\Project\BLADE
build.bat
```

This script:
- Uses CLion's MinGW toolchain (GCC 13.1.0)
- Builds in Release mode
- Copies necessary DLL files
- Creates standalone executable

### Running the Server

```batch
cd cmake-build-debug
blade.exe
```

Or use the run script:
```batch
cd cmake-build-debug
run.bat
```

### Command Line Options

```
blade.exe [options]

Options:
  -p, --port <port>      Server port (default: 8080)
  -n, --no-auth          Disable authentication
  -h, --help             Show this help message

Examples:
  blade.exe -p 9000           # Run on port 9000
  blade.exe --no-auth         # Run without authentication
```

## Expected Output

When you run `blade.exe`, you should now see:

```
========================================
  BLADE - Local Network File Transfer  
  Version 1.0.0                         
========================================

BLADE Server started on port 8080
Web interface available at http://10.5.0.216:8081
Authentication: ENABLED
Default credentials - Username: admin, Password: admin123

Press Ctrl+C to stop the server...
```

**Note:** It should now show `10.5.0.216` (your actual network IP) instead of `192.168.57.1` (virtual adapter).

## Files Modified

1. **src/NetworkUtils.cpp** - Updated `getLocalIPAddress()` function
2. **CMakeLists.txt** - Added pthread flag for MinGW
3. **Created build.bat** - Build automation script
4. **Created run.bat** - Run helper script (already existed in cmake-build-debug)

## Testing

To verify the IP detection:
1. Run `ipconfig` in cmd to see your network IP
2. Run `blade.exe`
3. Compare the IP shown by BLADE with your actual network IP from ipconfig
4. Other devices on your network can now access: `http://10.5.0.216:8081`

## Technical Notes

- The function now correctly handles multiple network interfaces
- Virtual adapters are deprioritized but kept as fallback
- Works on both Windows and Linux/Unix systems
- IPv4 only (IPv6 support can be added if needed)

## Troubleshooting

**If BLADE still shows wrong IP:**
- Check if you have multiple network interfaces active
- Temporarily disable virtual adapters (VMware, VirtualBox) if not needed
- The function will use the first non-virtual IP it finds

**If the app doesn't run:**
- Make sure you're in the `cmake-build-debug` directory
- Required DLL files must be in the same directory as blade.exe:
  - libstdc++-6.dll
  - libgcc_s_seh-1.dll
  - libwinpthread-1.dll
- Use `run.bat` which sets up the PATH correctly

