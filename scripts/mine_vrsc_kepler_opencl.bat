@echo off
REM Verus on older NVIDIA (GRID K2 / Kepler) via OpenCL — no Kepler CUDA toolkit required.
REM Run from the folder that contains DRQMiner-Release.exe and verushash.cl

setlocal
cd /d "%~dp0..\out\build\x64-Release" 2>nul || cd /d "%~dp0.."

if not exist DRQMiner-Release.exe (
  echo DRQMiner-Release.exe not found. Build: cmake --preset x64-Release ^&^& cmake --build out\build\x64-Release --config Release
  exit /b 1
)

if not exist verushash.cl (
  echo verushash.cl missing beside exe — rebuild Release or copy from src\backend\opencl\cl\verus\
  exit /b 1
)

echo OpenCL Verus (GRID K2 / low-VRAM GPUs). Set POOL and WALLET below.
set POOL=stratum+tcp://127.0.0.1:9999
set WALLET=YOUR_VRSC_ADDRESS

DRQMiner-Release.exe --no-cpu --no-cuda --opencl -a verus -o %POOL% -u %WALLET% -p x %*

endlocal
