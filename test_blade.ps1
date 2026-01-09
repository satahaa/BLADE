# BLADE Test Script
# Run this to test the application and see detailed diagnostics

Write-Output "======================================"
Write-Output "  BLADE Application Diagnostic Tool"
Write-Output "======================================"
Write-Output ""

# Stop any running instances
Write-Output "[1/5] Checking for running instances..."
$existing = Get-Process blade -ErrorAction SilentlyContinue
if ($existing) {
    Write-Output "      Found running instance (PID: $($existing.Id)), stopping it..."
    Stop-Process -Name blade -Force
    Start-Sleep -Seconds 1
}
Write-Output "      ✓ No running instances"
Write-Output ""

# Check if executable exists
Write-Output "[2/5] Checking executable..."
$exePath = "D:\Study\3-1\Project\BLADE\bin\blade.exe"
if (Test-Path $exePath) {
    $exeInfo = Get-Item $exePath
    Write-Output "      ✓ Found: $($exeInfo.Name)"
    Write-Output "      Size: $([math]::Round($exeInfo.Length/1MB, 2)) MB"
    Write-Output "      Modified: $($exeInfo.LastWriteTime)"
}
else {
    Write-Output "      ✗ NOT FOUND: $exePath"
    Write-Output "      Please rebuild the project first!"
    exit 1
}
Write-Output ""

# Check QML files
Write-Output "[3/5] Checking QML files..."
$qmlPath = "D:\Study\3-1\Project\BLADE\bin\qml\"
if (Test-Path "$qmlPath\main.qml") {
    Write-Output "      ✓ main.qml"
} else {
    Write-Output "      ✗ MISSING: main.qml"
}
if (Test-Path "$qmlPath\LoginView.qml") {
    Write-Output "      ✓ LoginView.qml"
} else {
    Write-Output "      ✗ MISSING: LoginView.qml"
}
if (Test-Path "$qmlPath\ServerView.qml") {
    Write-Output "      ✓ ServerView.qml"
} else {
    Write-Output "      ✗ MISSING: ServerView.qml"
}
if (Test-Path "$qmlPath\MaterialButton.qml") {
    Write-Output "      ✓ MaterialButton.qml"
} else {
    Write-Output "      ✗ MISSING: MaterialButton.qml"
}
Write-Output ""

# Start the application
Write-Output "[4/5] Starting application..."
$process = Start-Process -FilePath $exePath -PassThru
Start-Sleep -Seconds 3

# Check if it's running
$running = Get-Process -Id $process.Id -ErrorAction SilentlyContinue
if ($running) {
    Write-Output "      ✓ App started successfully!"
    Write-Output "      Process ID: $($running.Id)"
    Write-Output "      Memory: $([math]::Round($running.WorkingSet64/1MB, 2)) MB"
    Write-Output ""
    Write-Output "======================================"
    Write-Output "  APP IS RUNNING!"
    Write-Output "======================================"
    Write-Output ""
    Write-Output "INSTRUCTIONS:"
    Write-Output "1. Look for the BLADE window"
    Write-Output "2. Click 'Start without Authentication'"
    Write-Output "3. Wait 5 seconds"
    Write-Output "4. Press Enter here to check status..."
    Write-Output ""

    Read-Host "Press Enter after testing"

    # Check if still running
    $stillRunning = Get-Process blade -ErrorAction SilentlyContinue
    if ($stillRunning) {
        Write-Output ""
        Write-Output "✓ SUCCESS! App is still running!"
        Write-Output ""

        # Get latest log
        $latestLog = Get-ChildItem "D:\Study\3-1\Project\BLADE\bin\logs\" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
        if ($latestLog) {
            Write-Output "[5/5] Latest log ($($latestLog.Name)):"
            Write-Output "--------------------------------------"
            Get-Content $latestLog.FullName | Select-Object -Last 20
        }

        Write-Output ""
        Write-Output "Stopping app..."
        Stop-Process -Name blade -Force
    }
    else {
        Write-Output ""
        Write-Output "✗ APP CRASHED!"
        Write-Output ""
        Write-Output "[5/5] Crash log:"
        Write-Output "--------------------------------------"
        $latestLog = Get-ChildItem "D:\Study\3-1\Project\BLADE\bin\logs\" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
        if ($latestLog) {
            Get-Content $latestLog.FullName | Select-Object -Last 30
        }
        Write-Output ""
        Write-Output "TROUBLESHOOTING:"
        Write-Output "1. Check if error mentions 'QR code'"
        Write-Output "2. Check if error mentions 'QML'"
        Write-Output "3. Try rebuilding: cmake --build build --config Release"
    }
}
else {
    Write-Output "      ✗ App failed to start or crashed immediately"
    Write-Output ""
    Write-Output "[5/5] Checking logs..."
    $latestLog = Get-ChildItem "D:\Study\3-1\Project\BLADE\bin\logs\" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($latestLog) {
        Write-Output "Latest log ($($latestLog.Name)):"
        Write-Output "--------------------------------------"
        Get-Content $latestLog.FullName | Select-Object -Last 20
    }
}

Write-Output ""
Write-Output "======================================"
Write-Output "  Diagnostic Complete"
Write-Output "======================================"

