# Qt Setup for CLion - BLADE Project

## Qt Installation Complete!
Qt 6.8.1 has been successfully installed at: `C:\Qt\6.8.1\mingw_64`

## Configure CLion to Use Qt

### Step 1: Set CMake Options
1. Open CLion
2. Go to **File → Settings** (or **Ctrl+Alt+S**)
3. Navigate to **Build, Execution, Deployment → CMake**
4. In the **CMake options** field, add:
   ```
   -DCMAKE_PREFIX_PATH=C:/Qt/6.8.1/mingw_64
   ```

### Step 2: Set MinGW Toolchain
1. In Settings, go to **Build, Execution, Deployment → Toolchains**
2. Click **+** to add a new toolchain, select **MinGW**
3. Set the **Environment** path to: `C:\Qt\Tools\mingw1310_64`
   - If this path doesn't exist, use your existing MinGW installation
4. CLion should auto-detect:
   - C Compiler: `gcc.exe`
   - C++ Compiler: `g++.exe`
   - Make: `mingw32-make.exe`
5. Click **OK**

### Step 3: Reload CMake Project
1. Go to **File → Reload CMake Project**
2. Or click the **Reload CMake Project** icon in the CMake tool window

### Step 4: Build the Project
1. Click **Build → Build Project** (or **Ctrl+F9**)
2. The executable will be created in `cmake-build-debug/blade.exe`
3. It will also be copied to `bin/blade.exe`

## Alternative: Build from Command Line

If you prefer to build from the command line, open PowerShell and run:

```powershell
cd D:\Study\3-1\Project\BLADE
.\build_qt.bat
```

Or manually:

```powershell
$env:Path = "C:\Qt\6.8.1\mingw_64\bin;$env:Path"
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.8.1/mingw_64"
cmake --build build
```

## Running the Application

After building successfully:
1. Navigate to `bin` folder
2. Run `blade.exe`
3. The GUI will appear with login options
4. Choose "Start with Authentication" or "Start without Authentication"
5. The server will start and display a QR code for easy access from other devices

## Troubleshooting

### Qt Not Found Error
- Make sure CMAKE_PREFIX_PATH is set correctly in CMake options
- Verify Qt is installed at `C:\Qt\6.8.1\mingw_64`
- Reload CMake project after changing settings

### MinGW Compiler Not Found
- Install MinGW if not present
- Or use the MinGW that comes with Qt at `C:\Qt\Tools\mingw1310_64`

### DLL Errors When Running
- The CMake script automatically copies required Qt DLLs to the bin folder
- If you still get DLL errors, copy DLLs from `C:\Qt\6.8.1\mingw_64\bin\` to your exe directory

## Project Structure
```
BLADE/
├── bin/                    # Output directory for built exe and DLLs
│   ├── blade.exe          # Built application
│   └── web/               # Web interface files
├── src/                   # Source files
│   ├── main.cpp          # Entry point (minimal, just creates Application)
│   ├── Application.cpp   # Qt GUI implementation
│   └── ...               # Other server components
├── include/              # Header files
│   ├── Application.h     # Qt GUI header
│   └── ...
└── CMakeLists.txt       # Build configuration
```

## Next Steps
1. Configure CLion as described above
2. Reload CMake project
3. Build the project
4. Run blade.exe from the bin folder
5. Enjoy your Qt-powered file transfer application!

