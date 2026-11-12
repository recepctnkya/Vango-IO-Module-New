@echo off
REM Derleme: idf.py build
setlocal
cd /d "%~dp0"
call "%~dp0_idf_export.cmd" || exit /b 1
echo [build] %CD%
idf.py build
set "EC=%ERRORLEVEL%"
if not "%EC%"=="0" (
  echo [build] BASARISIZ (%EC%)
  exit /b %EC%
)
set "SRC_BIN=build\mqtt_tcp_custom_outbox.bin"
set "REL_BIN=build\vango_medium_v0_60.bin"
if exist "%SRC_BIN%" (
  copy /Y "%SRC_BIN%" "%REL_BIN%" >nul
  echo [build] Platform dosyasi: %REL_BIN%
) else (
  echo [build] UYARI: %SRC_BIN% yok
  exit /b 1
)
exit /b 0
