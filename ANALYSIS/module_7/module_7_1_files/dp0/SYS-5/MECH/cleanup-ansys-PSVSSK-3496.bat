@echo off
set LOCALHOST=%COMPUTERNAME%
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 13640)
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 3496)

del /F cleanup-ansys-PSVSSK-3496.bat
