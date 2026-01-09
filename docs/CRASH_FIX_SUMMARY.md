# BLADE Application - Crash Fix Summary

## Date: January 9, 2026

## Issues Fixed:

### 1. **QML Compilation Errors**
   - Fixed deprecated `mouse` parameter in MaterialButton.qml
   - Set Qt Quick Controls style to "Basic" to allow customization
   - Resolved "module not installed" errors by keeping essential Qt modules

### 2. **Build Deployment Optimization**
   - Reduced from ~1,200 files to manageable size
   - Automatic cleanup of unnecessary files on each build
   - QML files auto-copy to bin/qml on every build

### 3. **Exception Handling**
   - Added comprehensive exception handling in QR code generation
   - Enhanced logging throughout Application.cpp to track crashes
   - Multiple layers of try-catch to prevent silent failures

## Current Status:

### Files Modified:
1. `src/main.cpp` - Added QQuickStyle::setStyle("Basic")
2. `src/Application.cpp` - Enhanced exception handling and logging
3. `qml/MaterialButton.qml` - Fixed deprecated mouse parameter
4. `CMakeLists.txt` - Optimized deployment, auto-copy QML files
5. `bin/qt.conf` - Added QML import paths

### Build Configuration:
- Qt Quick Controls style: Basic
- Console window: Hidden (for release)
- All Qt dependencies properly linked
- Automatic file cleanup after deployment

## Testing Instructions:

### To test if the app runs:
```powershell
cd D:\Study\3-1\Project\BLADE\bin
.\blade.exe
```

### To check if app is running:
```powershell
Get-Process blade -ErrorAction SilentlyContinue
```

### To view latest log:
```powershell
Get-Content (Get-ChildItem "D:\Study\3-1\Project\BLADE\bin\logs\" | Sort-Object LastWriteTime -Descending | Select-Object -First 1).FullName | Select-Object -Last 30
```

### To rebuild the project:
```powershell
cd D:\Study\3-1\Project\BLADE
cmake --build build --config Release
```

## Known Issues:

### If app crashes after clicking "Start without Authentication":

1. **Check the log file** - It will show which step failed:
   - "Generating QR code" - QR code generation issue
   - "About to invoke showServerView" - QML method invocation issue
   - "Server view invoked successfully" - QML view transition issue

2. **Possible causes**:
   - QR code library exception (now handled with try-catch)
   - QML StackView push failure
   - Missing ServerView.qml component
   - Memory allocation issues

3. **Debug mode** - To see actual errors, temporarily edit CMakeLists.txt:
   ```cmake
   # Comment out this line:
   # target_link_options(blade PRIVATE -mwindows)
   ```
   Then rebuild and run to see console output.

## Next Steps if Crash Persists:

1. **Run with console output**:
   - Edit `CMakeLists.txt` line ~82
   - Comment out: `target_link_options(blade PRIVATE -mwindows)`
   - Rebuild
   - Run blade.exe from PowerShell
   - Observe actual error messages

2. **Check ServerView.qml**:
   - Ensure all required properties exist
   - Verify qrImage property is properly defined
   - Check for QML syntax errors

3. **Test QR code generation separately**:
   - The QR code generation might be the culprit
   - Consider using a simpler QR library or pre-generated images

4. **Memory issues**:
   - The crash might be due to memory allocation
   - Check if system has sufficient RAM
   - Monitor with Task Manager during execution

## File Locations:

- **Executable**: `D:\Study\3-1\Project\BLADE\bin\blade.exe`
- **Logs**: `D:\Study\3-1\Project\BLADE\bin\logs\blade_YYYYMMDD_HHMMSS.log`
- **QML Files**: `D:\Study\3-1\Project\BLADE\bin\qml\`
- **Web Files**: `D:\Study\3-1\Project\BLADE\bin\web\`

## Build Statistics:

- **Total files in bin**: ~200-250 files
- **Essential Qt DLLs**: ~30 DLLs
- **QML modules**: Basic, Fusion, Universal, Windows, NativeStyle
- **Total size**: ~70-75 MB


