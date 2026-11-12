@echo off
REM USB flash — kill port hogs, sync newest app bin, flash with retries.
REM Usage: flash_usb.cmd [COMx] [baud]
setlocal EnableExtensions
cd /d "%~dp0"
call "%~dp0_idf_export.cmd" || exit /b 1

set "SRC_BIN=build\mqtt_tcp_custom_outbox.bin"
set "REL_BIN=build\vango_medium_v0_60.bin"
if not exist "%SRC_BIN%" (
  echo [flash_usb] build yok, derleniyor...
  idf.py build || exit /b 1
)
if exist "%SRC_BIN%" (
  copy /Y "%SRC_BIN%" "%REL_BIN%" >nul
  echo [flash_usb] synced %REL_BIN% from mqtt bin
)
set "APP_BIN=%REL_BIN%"
if not exist "%APP_BIN%" set "APP_BIN=%SRC_BIN%"
if not exist "%APP_BIN%" (
  echo [flash_usb] app bin yok
  exit /b 1
)

set "PORT=COM40"
if not "%~1"=="" set "PORT=%~1"
set "BAUD=115200"
if not "%~2"=="" set "BAUD=%~2"
for %%F in ("%APP_BIN%") do set "APP_NAME=%%~nxF"
cd build

echo [flash_usb] freeing %PORT% (esptool/python/DeviceMonitor)...
taskkill /F /IM DeviceMonitor.exe >nul 2>&1
for /f "tokens=2 delims=," %%P in ('tasklist /FI "IMAGENAME eq python.exe" /FO CSV /NH 2^>nul') do (
  wmic process where "ProcessId=%%~P" get CommandLine 2>nul | findstr /I "esptool %PORT%" >nul && taskkill /F /PID %%~P /T >nul 2>&1
)
REM Broad fallback: any stuck esptool cmdline
wmic process where "CommandLine like '%%esptool%%'" call terminate >nul 2>&1
ping -n 2 127.0.0.1 >nul

echo Flashing %APP_NAME% to %PORT% at %BAUD%.
echo If connect fails: unplug FTDI USB 2s, replug, retry.

set ATTEMPTS=0
:retry
set /a ATTEMPTS+=1
if %ATTEMPTS% GTR 8 (
  echo Flash failed after %ATTEMPTS% attempts.
  echo Tip: stop VSPD service temporarily, unplug FTDI, or close other serial monitors.
  exit /b 1
)
echo --- attempt %ATTEMPTS% @ %BAUD% ---
python -m esptool --chip esp32s3 -p %PORT% -b %BAUD% --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size 8MB 0x0 bootloader/bootloader.bin 0x8000 partition_table/partition-table.bin 0xf000 ota_data_initial.bin 0x20000 %APP_NAME%
if not errorlevel 1 goto ok
taskkill /F /IM DeviceMonitor.exe >nul 2>&1
wmic process where "CommandLine like '%%esptool%%'" call terminate >nul 2>&1
if %ATTEMPTS%==4 set "BAUD=57600"
if %ATTEMPTS%==6 set "BAUD=460800"
ping -n 3 127.0.0.1 >nul
goto retry

:ok
echo Flash OK.
exit /b 0
