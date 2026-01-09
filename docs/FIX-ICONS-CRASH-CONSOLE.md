# BLADE Application Fixes - January 9, 2026

## Issues Fixed

### 1. ✅ Console Window Removed
**Problem:** Console window was appearing when launching the application.

**Solution:** Uncommented the `-mwindows` linker flag in CMakeLists.txt:
```cmake
target_link_options(blade PRIVATE -mwindows)
```

This flag tells the linker to create a Windows GUI application instead of a console application.

---

### 2. ✅ Icons Now Load Properly (No More Emojis)
**Problem:** The application was showing emoji characters (●, —) instead of actual SVG icons.

**Root Causes:**
- MaterialButton.qml file was corrupted/reversed
- IconImage.qml was using Text component with unicode symbols instead of actual images
- Icon paths were using relative paths (`../icons/`) instead of qrc paths
- Missing proper Image components

**Solutions:**

#### a) Recreated MaterialButton.qml
- Replaced corrupted file with proper implementation
- Uses actual `Image` component to load SVG icons
- Icon paths use `qrc:/icons/` prefix

#### b) Recreated IconImage.qml
- Changed from Text component to proper Image component
- Now renders actual SVG files

#### c) Updated All Icon Paths
- Changed from `"../icons/file.svg"` to `"qrc:/icons/file.svg"`
- Applied to LoginView.qml, ServerView.qml
- All icons now load from Qt resources

#### d) Removed ColorOverlay Dependencies
- Removed `import Qt5Compat.GraphicalEffects` (not needed for Qt 6)
- Removed all `ColorOverlay` wrappers around images
- Qt 6 can render SVG icons directly without color overlays

**Files Updated:**
- `qml/MaterialButton.qml` - Recreated
- `qml/IconImage.qml` - Recreated  
- `qml/LoginView.qml` - Updated icon paths
- `qml/ServerView.qml` - Updated icon paths, removed ColorOverlay

---

### 3. ✅ Application No Longer Crashes on Button Click
**Problem:** App crashed immediately when clicking "Start with Authentication" or "Continue without Authentication"

**Root Cause:** The view switching mechanism was broken. The old main.qml had a stub `showServerView()` function that did nothing, causing the app to fail when trying to navigate.

**Solutions:**

#### a) Reimplemented main.qml with StackView
- Created proper view navigation using Qt's StackView
- LoginView loads as initial view
- ServerView pushed onto stack when server starts

```qml
StackView {
    id: stackView
    anchors.fill: parent
    initialItem: loginViewComponent
    
    Component {
        id: loginViewComponent
        LoginView { objectName: "loginView" }
    }
    
    Component {
        id: serverViewComponent
        ServerView { objectName: "serverView" }
    }
}
```

#### b) Fixed Application.cpp View Switching
- Updated `showServerView()` to properly invoke QML functions
- Added proper error checking and logging
- Uses `QMetaObject::invokeMethod()` to call QML functions
- Uses `QTimer::singleShot()` to set QR image after view loads
- Added QTimer include

**Before:**
```cpp
// Old broken code that tried to find non-existent objects
QObject* serverView = mainWindow_->findChild<QObject*>("serverView");
```

**After:**
```cpp
// Proper invocation of QML functions
QMetaObject::invokeMethod(mainWindow_, "showServerView",
                          Q_ARG(QVariant, QString::fromStdString(url)));

// Set QR image after view loads
QTimer::singleShot(100, [this, tempPath]() {
    QMetaObject::invokeMethod(mainWindow_, "setQRImage",
                            Q_ARG(QVariant, tempPath));
});
```

---

## Technical Details

### Qt 6 Compatibility
- Removed Qt5Compat.GraphicalEffects imports
- ColorOverlay not needed in Qt 6 for simple icon rendering
- SVG icons render directly with proper sourceSize

### Resource System
All icons and assets now properly loaded from Qt Resource System:
- Icons: `qrc:/icons/*.svg`
- App icon: `qrc:/blade.ico`
- Compiled into executable via resources.qrc

### View Navigation
- **StackView Pattern:** Proper Qt Quick navigation
- **Component Loading:** Views loaded as components on demand
- **Property Binding:** Properties set via QML functions after load

---

## Build Instructions

```bash
cd D:\Study\3-1\Project\BLADE
cd build
cmake --build . --config Release

# Remove unwanted DLLs
cd ..\bin
del libgcc_s_seh-1.dll
del libstdc++-6.dll
```

---

## Testing Checklist

- ✅ Application launches without console window
- ✅ Lock icon shows in "Start with Authentication" button
- ✅ Rocket icon shows in "Continue without Authentication" button
- ✅ Person icon shows in username field
- ✅ Lock icon shows in password field
- ✅ Eye icons work for password visibility toggle
- ✅ Clicking buttons doesn't crash the app
- ✅ Server view loads when starting server
- ✅ QR code displays in server view
- ✅ Server URL displays correctly
- ✅ All icons render as SVG (no emojis)

---

## Files Modified

### CMakeLists.txt
- Uncommented `-mwindows` flag

### src/Application.cpp
- Added `#include <QTimer>`
- Rewrote `showServerView()` function
- Added proper QML function invocation

### qml/main.qml
- Complete rewrite with StackView
- Proper view navigation
- QML functions for C++ to call

### qml/LoginView.qml
- Updated all icon paths to use `qrc:/`
- Removed ColorOverlay wrappers

### qml/ServerView.qml
- Removed Qt5Compat import
- Updated all icon paths to use `qrc:/`
- Removed ColorOverlay wrappers
- Removed DropShadow effect

### qml/MaterialButton.qml
- Complete rewrite (file was corrupted)
- Proper Image component for icons
- Correct icon path handling

### qml/IconImage.qml
- Complete rewrite
- Changed from Text to Image component

---

## Date
January 9, 2026

## Status
✅ **ALL ISSUES RESOLVED**

