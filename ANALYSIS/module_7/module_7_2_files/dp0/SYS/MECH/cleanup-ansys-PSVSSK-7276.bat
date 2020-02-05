@echo off
set LOCALHOST=%COMPUTERNAME%
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 9124)
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 7276)

del /F cleanup-ansys-PSVSSK-7276.bat
