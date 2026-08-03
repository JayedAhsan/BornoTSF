@echo off
:: Self-elevation snippet
%1 mshta vbscript:CreateObject("Shell.Application").ShellExecute("cmd.exe","/c %~s0 ::","","runas",1)(window.close)&&exit

cd /d "%~dp0"

echo =======================================================
echo        BornoTSF Keyboard IME Unregister Script
echo =======================================================
echo.

set HAS_UNREGISTERED=0


if exist "%windir%\System32\BornoTSF.dll" (
    echo Unregistering System32 BornoTSF.dll...
    %windir%\System32\regsvr32.exe /u /s "%windir%\System32\BornoTSF.dll"
    del /f /q "%windir%\System32\BornoTSF.dll" >nul 2>&1
    set HAS_UNREGISTERED=1
)

if exist "%windir%\SysWOW64\BornoTSF.dll" (
    echo Unregistering SysWOW64 BornoTSF.dll...
    %windir%\SysWOW64\regsvr32.exe /u /s "%windir%\SysWOW64\BornoTSF.dll"
    del /f /q "%windir%\SysWOW64\BornoTSF.dll" >nul 2>&1
    set HAS_UNREGISTERED=1
)


if exist "%~dp0BornoTSF.dll" (
    echo Unregistering local BornoTSF.dll...
    regsvr32.exe /u /s "%~dp0BornoTSF.dll"
    set HAS_UNREGISTERED=1
)


echo Cleaning installed dictionaries and layouts...
if exist "%APPDATA%\BornoTSF\dictionaries\ac.json" del /f /q "%APPDATA%\BornoTSF\dictionaries\ac.json" >nul 2>&1
if exist "%ProgramData%\BornoTSF\dictionaries\ac.json" del /f /q "%ProgramData%\BornoTSF\dictionaries\ac.json" >nul 2>&1
if exist "%APPDATA%\BornoTSF\layouts\bn-khipro.mim" del /f /q "%APPDATA%\BornoTSF\layouts\bn-khipro.mim" >nul 2>&1
if exist "%ProgramData%\BornoTSF\layouts\bn-khipro.mim" del /f /q "%ProgramData%\BornoTSF\layouts\bn-khipro.mim" >nul 2>&1

echo.
if %HAS_UNREGISTERED%==1 (
    echo [SUCCESS] BornoTSF Keyboard IME unregistered and cleaned!
) else (
    echo [INFO] BornoTSF Keyboard IME was not active or DLLs were not found. Cleaned up remaining configuration files.
)

echo.
pause
