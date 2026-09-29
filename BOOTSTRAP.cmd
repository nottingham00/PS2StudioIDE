@echo off
setlocal
cd /d "%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\Bootstrap-Windows.ps1" -Mode All
exit /b %errorlevel%
