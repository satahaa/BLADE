@echo off
setlocal

set PATH=%PATH%;E:\CLion 2025.2.3\bin\mingw\bin
cd /d "%~dp0..\cmake-build-debug" || (echo Failed to cd to build dir & pause & exit /b 1)

echo ========================================
echo   Starting BLADE...
echo   Working dir: %CD%
echo ========================================
echo.

REM If double-clicked with no args, default to no-auth so it doesn't immediately exit.
if "%~1"=="" (
  echo No args provided - defaulting to: --no-auth
  echo.
  blade.exe --no-auth
) else (
  echo Running: blade.exe %*
  echo.
  blade.exe %*
)

echo.
echo ========================================
echo   BLADE exited with code %ERRORLEVEL%
echo ========================================
pause
