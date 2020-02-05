@echo off
set LOCALHOST=%COMPUTERNAME%
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 12892)
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 17580)

del /F cleanup-ansys-PSVSSK-17580.bat
