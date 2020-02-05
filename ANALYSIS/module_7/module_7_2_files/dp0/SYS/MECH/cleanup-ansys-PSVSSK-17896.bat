@echo off
set LOCALHOST=%COMPUTERNAME%
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 7500)
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 17896)

del /F cleanup-ansys-PSVSSK-17896.bat
