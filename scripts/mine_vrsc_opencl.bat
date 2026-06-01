@echo off
REM DRQMiner VerusHash OpenCL (AMD / Intel) — NVIDIA users should prefer --cuda.

setlocal
cd /d "%~dp0"

set "BUILD=%~dp0..\out\build\x64-Release"
set "MINER=%BUILD%\DRQMiner-Release.exe"
if not exist "%MINER%" set "MINER=DRQMiner-Release.exe"

cd /d "%BUILD%"

echo DRQMiner VerusHash OpenCL — replace YOUR_WALLET.worker
echo.

"%MINER%" -a verushash ^
  -o usw.vipor.net:5040 ^
  -u YOUR_WALLET.worker ^
  -p x --no-cpu --no-cuda --opencl --donate-level=0

pause
