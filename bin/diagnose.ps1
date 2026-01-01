# BLADE Server - Network Diagnostics
# This script checks network configuration and helps troubleshoot connection issues

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  BLADE Server - Network Diagnostics" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# 1. Check if server is running
Write-Host "[1] Checking if BLADE server is running..." -ForegroundColor Yellow
$serverProcess = Get-Process -Name "blade" -ErrorAction SilentlyContinue
if ($serverProcess) {
    Write-Host "    ✓ BLADE server is running (PID: $($serverProcess.Id))`n" -ForegroundColor Green
} else {
    Write-Host "    ✗ BLADE server is NOT running" -ForegroundColor Red
    Write-Host "    Start the server using: bin\run.bat`n" -ForegroundColor Yellow
}

# 2. Check ports
Write-Host "[2] Checking if ports are listening..." -ForegroundColor Yellow
$port80 = netstat -an | Select-String ":80 " | Select-String "LISTENING"
$port8443 = netstat -an | Select-String ":8443 " | Select-String "LISTENING"

if ($port80) {
    Write-Host "    ✓ Port 80 (HTTP) is LISTENING" -ForegroundColor Green
} else {
    Write-Host "    ✗ Port 80 (HTTP) is NOT listening" -ForegroundColor Red
}

if ($port8443) {
    Write-Host "    ✓ Port 8443 (Main) is LISTENING`n" -ForegroundColor Green
} else {
    Write-Host "    ✗ Port 8443 (Main) is NOT listening`n" -ForegroundColor Red
}

# 3. Get local IP address
Write-Host "[3] Network Information:" -ForegroundColor Yellow
$networkAdapters = Get-NetIPAddress -AddressFamily IPv4 | Where-Object {$_.IPAddress -notlike "127.*"}

foreach ($adapter in $networkAdapters) {
    $adapterInfo = Get-NetAdapter | Where-Object {$_.ifIndex -eq $adapter.InterfaceIndex}
    if ($adapterInfo.Status -eq "Up") {
        Write-Host "    Interface: $($adapterInfo.Name)" -ForegroundColor Cyan
        Write-Host "    IP Address: $($adapter.IPAddress)" -ForegroundColor White
        Write-Host "    Status: $($adapterInfo.Status)" -ForegroundColor Green
        Write-Host ""
    }
}

# 4. Check firewall rules
Write-Host "[4] Checking Windows Firewall rules..." -ForegroundColor Yellow
$httpRule = Get-NetFirewallRule -DisplayName "BLADE Server - HTTP (Port 80)" -ErrorAction SilentlyContinue
$mainRule = Get-NetFirewallRule -DisplayName "BLADE Server - Main (Port 8443)" -ErrorAction SilentlyContinue

if ($httpRule -and $httpRule.Enabled -eq $true) {
    Write-Host "    ✓ HTTP firewall rule exists and is ENABLED" -ForegroundColor Green
} else {
    Write-Host "    ✗ HTTP firewall rule is MISSING or DISABLED" -ForegroundColor Red
    Write-Host "      Run: bin\setup-firewall.ps1 (as Administrator)" -ForegroundColor Yellow
}

if ($mainRule -and $mainRule.Enabled -eq $true) {
    Write-Host "    ✓ Main firewall rule exists and is ENABLED`n" -ForegroundColor Green
} else {
    Write-Host "    ✗ Main firewall rule is MISSING or DISABLED" -ForegroundColor Red
    Write-Host "      Run: bin\setup-firewall.ps1 (as Administrator)`n" -ForegroundColor Yellow
}

# 5. Test localhost connection
Write-Host "[5] Testing localhost connection..." -ForegroundColor Yellow
try {
    $response = Invoke-WebRequest -Uri "http://127.0.0.1:80" -TimeoutSec 3 -UseBasicParsing -ErrorAction Stop
    Write-Host "    ✓ Localhost connection works (Status: $($response.StatusCode))`n" -ForegroundColor Green
} catch {
    Write-Host "    ✗ Localhost connection failed: $($_.Exception.Message)`n" -ForegroundColor Red
}

# 6. Summary and recommendations
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Recommendations" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$primaryIP = ($networkAdapters | Where-Object {$_.PrefixOrigin -eq "Dhcp"} | Select-Object -First 1).IPAddress

if ($primaryIP) {
    Write-Host "Access URL from other devices:" -ForegroundColor Yellow
    Write-Host "  http://$primaryIP`n" -ForegroundColor White
}

Write-Host "If connection still fails, check:" -ForegroundColor Yellow
Write-Host "  1. Other device is on the same WiFi/network" -ForegroundColor White
Write-Host "  2. Router AP isolation is disabled" -ForegroundColor White
Write-Host "  3. No VPN is active on server PC" -ForegroundColor White
Write-Host "  4. Antivirus/security software allows connections" -ForegroundColor White
Write-Host "  5. Windows Firewall rules are configured (run setup-firewall.ps1)`n" -ForegroundColor White

Write-Host "Press any key to exit..." -ForegroundColor Gray
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

