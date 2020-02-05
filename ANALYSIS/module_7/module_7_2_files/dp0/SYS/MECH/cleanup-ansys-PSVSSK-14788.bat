@echo off
set LOCALHOST=%COMPUTERNAME%
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 18864)
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 14788)

del /F cleanup-ansys-PSVSSK-14788.bat
