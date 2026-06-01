@echo off
REM DRQMiner embedded CUDA VerusHash — NVIDIA only, no xmrig-cuda.dll.

setlocal
cd /d "%~dp0"

set "BUILD=%~dp0..\out\build\x64-Release"
set "MINER=%BUILD%\DRQMiner-Release.exe"
if not exist "%MINER%" set "MINER=DRQMiner-Release.exe"

cd /d "%BUILD%"
if not exist cudart64*.dll (
    echo [WARN] cudart64_*.dll missing next to miner — copy from CUDA toolkit bin\x64
)

if exist xmrig-cuda.dll (
    echo [WARN] Rename xmrig-cuda.dll for embedded-Verus-only test
)

echo DRQMiner VerusHash CUDA — replace YOUR_WALLET.worker
echo.

"%MINER%" -a verushash ^
  -o usw.vipor.net:5040 ^
  -u YOUR_WALLET.worker ^
  -p x --no-cpu --no-opencl --cuda --donate-level=0

pause
