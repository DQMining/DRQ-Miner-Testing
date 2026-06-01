@echo off
setlocal EnableExtensions
cd /d "%~dp0.."

set "BUILD_DIR=out\build\x64-Release"
if not exist "%BUILD_DIR%" set "BUILD_DIR=out\build\x64-Debug"
cd /d "%BUILD_DIR%"

set "EXE=DRQMiner-Release.exe"
if not exist "%EXE%" set "EXE=DRQMiner.exe"
if not exist "%EXE%" (
    echo [ERROR] Build first, then run from repo root: scripts\vipor_cuda_smoke.bat
    exit /b 1
)

echo === cudart DLL ===
dir /b cudart64*.dll 2>nul
if errorlevel 1 echo [WARN] Copy cudart64_*.dll from CUDA toolkit bin\x64

if exist xmrig-cuda.dll echo [WARN] Rename xmrig-cuda.dll for embedded-Verus-only test

echo === version ===
"%EXE%" -V
if errorlevel 1 exit /b 1

echo === VIPOR CUDA smoke — replace YOUR_WALLET, Ctrl+C to stop ===
"%EXE%" -a verushash -o usw.vipor.net:5040 -u YOUR_WALLET.14900K -p x --no-cpu --no-opencl --cuda --donate-level=0
set "EL=%ERRORLEVEL%"
echo Exit code: %EL%
if %EL% equ -1073740940 echo [FAIL] 0xC0000374 heap corruption
if %EL% equ -1073741819 echo [FAIL] 0xC0000005 access violation
exit /b %EL%
