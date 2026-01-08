# BLADE Build Configuration

## Current Build: Dynamic Linking with Qt

Your application is currently built with **dynamic linking**, which means it requires Qt DLLs and MinGW runtime DLLs to run.

### ✅ FIXED: Compiler Compatibility Issue
The error you encountered was because your system was using MinGW 14.2.0 to compile the executable, but Qt 6.8.1 was built with MinGW 13.1.0. This caused ABI incompatibility.

**Solution Applied**: The build now uses Qt's bundled MinGW 13.1.0 compiler (from `C:\Qt\Tools\mingw1310_64\`), ensuring perfect compatibility.

### Current Executable:
- **Size**: ~515 KB
- **Dependencies**: Requires DLLs in the same directory
- **Total deployment size**: ~50-60 MB (including all Qt DLLs and plugins)

### Required Files for Distribution:
```
bin/
├── blade.exe (515 KB)
├── Qt6Core.dll
├── Qt6Gui.dll
├── Qt6Widgets.dll
├── Qt6Network.dll
├── Qt6Svg.dll
├── libgcc_s_seh-1.dll
├── libstdc++-6.dll
├── libwinpthread-1.dll
├── D3Dcompiler_47.dll
├── opengl32sw.dll
├── platforms/
│   └── qwindows.dll (REQUIRED!)
├── styles/
├── imageformats/
└── web/ (your web files)
```

---

## About True Static Linking with Qt

### Why Your Qt Installation Can't Build Truly Static Executables:

1. **Qt from the official installer is pre-built with shared libraries** (DLLs)
2. The `.a` files you saw are **import libraries** for DLLs, not static libraries
3. True static linking requires **Qt built from source with `-static` flag**

### To Build Qt Statically (Advanced - Takes Hours):

You would need to:
1. Download Qt source code
2. Build Qt from source with these flags:
   ```bash
   configure -static -release -prefix "C:/Qt/6.8.1/static" -opensource -confirm-license
   mingw32-make -j8
   mingw32-make install
   ```
3. Use that static Qt build in your project
4. Result: Single executable, ~15-20 MB, no DLLs needed

**Downside**: 
- Takes 4-8 hours to compile Qt
- Larger executable size
- LGPL licensing considerations (static linking has different requirements)
- More complex deployment (all plugins must be explicitly imported in code)

---

## Recommended Approach

**Keep the current dynamic linking setup** because:

1. ✅ **Faster builds** - No need to recompile Qt
2. ✅ **Smaller executable** - 515 KB vs 15+ MB
3. ✅ **Easier updates** - Just replace DLLs
4. ✅ **LGPL compliant** - Users can replace Qt DLLs
5. ✅ **Standard Qt deployment** - Well-tested and supported

### Distribution:
- Zip the entire `bin/` folder
- Users extract and run blade.exe
- Or use an installer (NSIS, Inno Setup) to package everything

---

## Building Your Project

### Option 1: Use rebuild.bat (Recommended)
```batch
rebuild.bat
```

### Option 2: Manual Build
```batch
cd build
set PATH=C:\Qt\Tools\mingw1310_64\bin;%PATH%
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.8.1/mingw_64" -DCMAKE_BUILD_TYPE=Release ..
mingw32-make -j8
```

---

## Application Now Works!

Your application is now fully functional and runs without DLL errors. The executable and all dependencies are correctly deployed in the `bin/` folder.

