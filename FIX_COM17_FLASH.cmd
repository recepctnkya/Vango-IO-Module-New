@echo off
:: Run as Administrator: unlocks COM17 and flashes BLE-early firmware
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build\fix_com_flash.ps1"
echo.
pause
