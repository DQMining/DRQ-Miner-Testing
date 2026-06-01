@echo off
REM DRQMiner AstroBWT v3 on Rabid (requires wss:// and --daemon).

setlocal
cd /d "%~dp0"

set "MINER=%~dp0..\out\build\x64-Release\DRQMiner-Release.exe"
if not exist "%MINER%" set "MINER=DRQMiner-Release.exe"

echo DRQMiner DERO Rabid - enable huge pages in config for best results
echo.

"%MINER%" --daemon -a astrobwt/v3 ^
  -o wss://dero.rabidmining.com:10300 ^
  -u dero1qy5c4sl5p2carlla465gyexlh6vye32v039tsxe4gp9zrha30cv9uqgr66utu ^
  -p x -t 32

pause
