# Debug Guide - Authentication Issue

## Problem
Login page shows even when running without authentication flags.

## Debug Steps Added

### 1. Server-Side Logging

Added debug output in three places:

**Server.cpp Constructor:**
```
[DEBUG] Server constructor - useAuth: true/false, username: '...', password: '...'
```

**HTTPServer.cpp Constructor:**
```
[DEBUG] HTTPServer constructor - useAuth_: true/false, username_: '...', password_: '...'
```

**HTTPServer.cpp getAuthConfig():**
```
[DEBUG] Auth config: {"authEnabled":true/false,"username":"...","password":"..."}
```

### 2. Client-Side Logging

Added console.log in `script.js`:
```javascript
console.log('Auth config received:', this.authConfig);
console.log('Auth disabled - bypassing login');  // or
console.log('Auth enabled - showing login page');
```

## How to Test

### Test 1: No Authentication (Default)

```bash
cd D:\Study\3-1\Project\BLADE\cmake-build-debug
blade.exe
```

**Expected Server Output:**
```
[DEBUG] Server constructor - useAuth: false, username: '', password: ''
[DEBUG] HTTPServer constructor - useAuth_: false, username_: '', password_: ''
Initializing HTTPS with self-signed certificate...
========================================
   BLADE Server Started Successfully
========================================

Web Interface Access:
  https://192.168.x.x:444

Authentication: DISABLED

Waiting for client connections...
========================================

[DEBUG] Auth config: {"authEnabled":false}
```

**Expected Browser Console:**
```
Auth config received: {authEnabled: false}
Auth disabled - bypassing login
```

**Expected Behavior:**
- ✅ Login page should NOT show
- ✅ Dashboard should load immediately
- ✅ Status should show "Connected"

### Test 2: With Authentication

```bash
cd D:\Study\3-1\Project\BLADE\cmake-build-debug
blade.exe -u admin -p secret123
```

**Expected Server Output:**
```
[DEBUG] Server constructor - useAuth: true, username: 'admin', password: 'secret123'
[DEBUG] HTTPServer constructor - useAuth_: true, username_: 'admin', password_: 'secret123'
Initializing HTTPS with self-signed certificate...
========================================
   BLADE Server Started Successfully
========================================

Web Interface Access:
  https://192.168.x.x:444

Authentication: ENABLED

Waiting for client connections...
========================================

[DEBUG] Auth config: {"authEnabled":true,"username":"admin","password":"secret123"}
```

**Expected Browser Console:**
```
Auth config received: {authEnabled: true, username: "admin", password: "secret123"}
Auth enabled - showing login page
```

**Expected Behavior:**
- ✅ Login page SHOULD show
- ✅ Must enter username: admin, password: secret123
- ✅ After login, dashboard loads

## Troubleshooting

### If login page shows when running without -u and -p:

Check the server output:

1. **If you see `useAuth: true`** → Something is wrong with argument parsing in main.cpp
2. **If you see `useAuth: false` but auth config shows `true`** → Problem in getAuthConfig()
3. **If you see `authEnabled: false` but login shows** → Problem in JavaScript logic

### If dashboard loads when running with -u and -p:

Check the server output:

1. **If you see `useAuth: false`** → Arguments not being parsed correctly
2. **If you see empty username/password** → Arguments not being captured
3. **If auth config is correct** → JavaScript might have logic error

## Next Steps

Run the server with debug logging and share:
1. The complete server terminal output
2. The browser console output (F12 → Console tab)
3. What you see on screen (login page or dashboard)

This will help identify exactly where the issue is occurring.

