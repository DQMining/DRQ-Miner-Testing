@echo off
setlocal EnableExtensions
cd /d "%~dp0"

echo.
echo === CUDA runtime DLL (required for embedded Verus GPU) ===
dir /b "cudart64*.dll" 2>nul
if errorlevel 1 (
    echo [MISSING] Copy cudart64_*.dll from CUDA toolkit — often under bin\x64 ^(not bin^), e.g.:
    echo   C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.2\bin\x64\cudart64_13.dll
    echo into this folder, then run this script again.
    pause
    exit /b 1
)

echo.
echo === Miner version ===
if exist "DRQMiner-Release.exe" (
    set "EXE=DRQMiner-Release.exe"
) else if exist "DRQMiner.exe" (
    set "EXE=DRQMiner.exe"
) else (
    echo [ERROR] No DRQMiner.exe or DRQMiner-Release.exe in this folder.
    pause
    exit /b 1
)
"%EXE%" -V
if errorlevel 1 (
    pause
    exit /b 1
)

echo.
echo === Starting miner (watch for errors above). Omit --opencl if you only use NVIDIA. ===
REM Wallet: confirm spelling (e.g. GDWVJ vs GDWvJ).
"%EXE%" -a verushash -o usw.vipor.net:5040 -u RGjsamSLiYGDWvJ5pf76zZXz1jQFcX5eaA.14900K -p x --no-cpu --cuda

echo.
echo Exit code: %ERRORLEVEL%
if %ERRORLEVEL% neq 0 (
    echo If code is -1073741819 ^(0xC0000005^): access violation — often missing cudart DLL, bad GPU driver, or CUDA mismatch.
)
pause
