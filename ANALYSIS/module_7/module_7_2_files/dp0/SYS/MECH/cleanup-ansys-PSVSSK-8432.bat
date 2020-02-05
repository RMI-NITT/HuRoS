@echo off
set LOCALHOST=%COMPUTERNAME%
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 12140)
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 8432)

del /F cleanup-ansys-PSVSSK-8432.bat
