@echo off
set LOCALHOST=%COMPUTERNAME%
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 17168)
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 13740)

del /F cleanup-ansys-PSVSSK-13740.bat
