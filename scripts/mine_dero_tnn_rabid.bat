@echo off
REM TNN-MINER v0.8.x on Rabid DERO (port 10300).
REM Coin flag must be --DERO (uppercase); --dero clashes with --dero-benchmark on v0.8.2+.

setlocal
cd /d "%~dp0"

set "TNN_EXE=tnn-miner.exe"
if not exist "%TNN_EXE%" set "TNN_EXE=%~dp0..\..\tnn-miner\tnn-miner.exe"

if not exist "%TNN_EXE%" (
    echo ERROR: tnn-miner.exe not found. Set TNN_EXE in this script.
    exit /b 1
)

echo TNN-MINER v0.8.x DERO Rabid - CPU only, 32 threads
echo.

"%TNN_EXE%" --no-gpu ^
  --daemon-address dero.rabidmining.com ^
  --port 10300 ^
  --wallet dero1qy5c4sl5p2carlla465gyexlh6vye32v039tsxe4gp9zrha30cv9uqgr66utu ^
  --threads 32 ^
  --worker-name 14900k ^
  --no-tune wolf ^
  --DERO

pause
