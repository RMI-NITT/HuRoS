@echo off
set LOCALHOST=%COMPUTERNAME%
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 13628)
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 19100)

del /F cleanup-ansys-PSVSSK-19100.bat
