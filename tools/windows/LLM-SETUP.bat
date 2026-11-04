@echo off
setlocal
powershell.exe -NoLogo -NoProfile -ExecutionPolicy RemoteSigned -File "%~dp0LLM-SETUP.ps1" %*
set "setup_result=%errorlevel%"
if "%~1"=="" pause
exit /b %setup_result%
