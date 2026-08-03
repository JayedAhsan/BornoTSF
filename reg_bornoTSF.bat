@echo off
:: Self-elevation snippet
%1 mshta vbscript:CreateObject("Shell.Application").ShellExecute("cmd.exe","/c %~s0 ::","","runas",1)(window.close)&&exit

cd /d "%~dp0"

echo =======================================================
echo          BornoTSF Keyboard IME Register Script
echo =======================================================
echo.

:: Copy ac.json
if exist "%~dp0assets\entries\ac.json" (
    echo Copying auto-correct dictionary ^(ac.json^)...
    if not exist "%APPDATA%\BornoTSF\dictionaries" mkdir "%APPDATA%\BornoTSF\dictionaries" >nul 2>&1
    copy /y "%~dp0assets\entries\ac.json" "%APPDATA%\BornoTSF\dictionaries\ac.json" >nul 2>&1
    if not exist "%ProgramData%\BornoTSF\dictionaries" mkdir "%ProgramData%\BornoTSF\dictionaries" >nul 2>&1
    copy /y "%~dp0assets\entries\ac.json" "%ProgramData%\BornoTSF\dictionaries\ac.json" >nul 2>&1
)

:: Copy bn-khipro.mim
if exist "%~dp0layoutParsers\khipro-port\bn-khipro.mim" (
    echo Copying Khipro layout ^(bn-khipro.mim^)...
    if not exist "%APPDATA%\BornoTSF\layouts" mkdir "%APPDATA%\BornoTSF\layouts" >nul 2>&1
    copy /y "%~dp0layoutParsers\khipro-port\bn-khipro.mim" "%APPDATA%\BornoTSF\layouts\bn-khipro.mim" >nul 2>&1
    if not exist "%ProgramData%\BornoTSF\layouts" mkdir "%ProgramData%\BornoTSF\layouts" >nul 2>&1
    copy /y "%~dp0layoutParsers\khipro-port\bn-khipro.mim" "%ProgramData%\BornoTSF\layouts\bn-khipro.mim" >nul 2>&1
)

set HAS_REGISTERED=0

:: Check for 64-bit OS
if exist "%windir%\SysWOW64" (
    echo Detecting 64-bit Windows OS...
    
    :: 1. Copy & Register 64-bit DLL
    if exist "%~dp0x64\Release\BornoTSF.dll" (
        echo Copying 64-bit Release DLL to System32...
        copy /y "%~dp0x64\Release\BornoTSF.dll" "%windir%\System32\BornoTSF.dll"
        echo Registering 64-bit BornoTSF.dll...
        %windir%\System32\regsvr32.exe /s "%windir%\System32\BornoTSF.dll"
        set HAS_REGISTERED=1
    ) else if exist "%~dp0x64\Debug\BornoTSF.dll" (
        echo Copying 64-bit Debug DLL to System32...
        copy /y "%~dp0x64\Debug\BornoTSF.dll" "%windir%\System32\BornoTSF.dll"
        echo Registering 64-bit BornoTSF.dll...
        %windir%\System32\regsvr32.exe /s "%windir%\System32\BornoTSF.dll"
        set HAS_REGISTERED=1
    )

    :: 2. Copy & Register 32-bit DLL for WOW64 apps
    if exist "%~dp0Release\BornoTSF.dll" (
        echo Copying 32-bit Release DLL to SysWOW64...
        copy /y "%~dp0Release\BornoTSF.dll" "%windir%\SysWOW64\BornoTSF.dll"
        echo Registering 32-bit BornoTSF.dll...
        %windir%\SysWOW64\regsvr32.exe /s "%windir%\SysWOW64\BornoTSF.dll"
        set HAS_REGISTERED=1
    ) else if exist "%~dp0Debug\BornoTSF.dll" (
        echo Copying 32-bit Debug DLL to SysWOW64...
        copy /y "%~dp0Debug\BornoTSF.dll" "%windir%\SysWOW64\BornoTSF.dll"
        echo Registering 32-bit BornoTSF.dll...
        %windir%\SysWOW64\regsvr32.exe /s "%windir%\SysWOW64\BornoTSF.dll"
        set HAS_REGISTERED=1
    )
) else (
    echo Detecting 32-bit Windows OS...
    if exist "%~dp0Release\BornoTSF.dll" (
        echo Copying 32-bit Release DLL to System32...
        copy /y "%~dp0Release\BornoTSF.dll" "%windir%\System32\BornoTSF.dll"
        echo Registering BornoTSF.dll...
        %windir%\System32\regsvr32.exe /s "%windir%\System32\BornoTSF.dll"
        set HAS_REGISTERED=1
    ) else if exist "%~dp0Debug\BornoTSF.dll" (
        echo Copying 32-bit Debug DLL to System32...
        copy /y "%~dp0Debug\BornoTSF.dll" "%windir%\System32\BornoTSF.dll"
        echo Registering BornoTSF.dll...
        %windir%\System32\regsvr32.exe /s "%windir%\System32\BornoTSF.dll"
        set HAS_REGISTERED=1
    )
)

:: Fallback
if %HAS_REGISTERED%==0 (
    if exist "%~dp0BornoTSF.dll" (
        echo Registering local BornoTSF.dll...
        regsvr32.exe /s "%~dp0BornoTSF.dll"
        set HAS_REGISTERED=1
    )
)

if %HAS_REGISTERED%==1 (
    echo.
    echo [SUCCESS] BornoTSF Keyboard IME installed and registered!
    echo.
) else (
    echo.
    echo [ERROR] BornoTSF.dll not found in Release or Debug folders!
    echo Please build the project in Visual Studio first.
)

echo.
pause
