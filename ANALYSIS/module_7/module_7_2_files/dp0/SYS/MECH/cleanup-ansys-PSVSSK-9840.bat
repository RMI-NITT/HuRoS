@echo off
set LOCALHOST=%COMPUTERNAME%
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 17368)
if /i "%LOCALHOST%"=="PSVSSK" (taskkill /f /pid 9840)

del /F cleanup-ansys-PSVSSK-9840.bat
