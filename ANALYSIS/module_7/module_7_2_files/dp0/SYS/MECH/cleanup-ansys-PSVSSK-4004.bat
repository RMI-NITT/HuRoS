@echo off
set LOCALHOST=%COMPUTERNAME%
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 17436)
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 4004)

del /F cleanup-ansys-PSVSSK-4004.bat
