@echo off
REM DRQMiner VerusHash (VRSC) CPU — VIPOR example pool.

setlocal
cd /d "%~dp0"

set "MINER=%~dp0..\out\build\x64-Release\DRQMiner-Release.exe"
if not exist "%MINER%" set "MINER=DRQMiner-Release.exe"

echo DRQMiner VerusHash CPU — replace YOUR_WALLET.worker
echo.

"%MINER%" -a verushash ^
  -o usw.vipor.net:5040 ^
  -u YOUR_WALLET.worker ^
  -p x --no-cuda --no-opencl -t 30

pause
