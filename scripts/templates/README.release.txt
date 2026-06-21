DRQ Miner — Windows x64 Release Package
=========================================

Target: clean Windows 10/11 x64 (no Visual Studio required).

Quick start
-----------
1. Extract this entire folder to one directory (keep all files together).
2. Double-click DRQMiner-Release.exe — the built-in setup wizard runs on first launch.
   Or run:  DRQMiner-Release.exe --setup
3. Enter your wallet and pool (presets included for Verus, DERO, RandomX, Ghostrider).
4. The wizard writes config.json and start-miner.bat in this folder.
5. Next time, double-click start-miner.bat (or run the .exe again).

One-line mining (no config file):

   DRQMiner-Release.exe -a verushash -o usw.vipor.net:5040 -u YOUR_WALLET.worker -p x

Configuration
-------------
Re-run DRQMiner-Release.exe --setup to change pool or wallet.
Do not share config.json with private keys or passwords.

Files in this package
---------------------
- DRQMiner-Release.exe          Main miner
- verushash.cl                  Verus OpenCL kernel (required for --opencl Verus)
- xmrig_astro_spsa.dll          DERO AstroSPSA fast path (when built)
- cudart64_*.dll                NVIDIA CUDA runtime (embedded Verus CUDA only)
- libcrypto-3-x64.dll + libcrypto-3-x64__.dll  OpenSSL for AstroSPSA (both names required on clean Windows)
- vcruntime140*.dll / msvcp140*.dll   Microsoft VC++ runtime (/MD build)
- WinRing0x64.sys               Optional MSR driver (RandomX tuning; admin required)
- DEPENDENCY_REPORT.txt         Full DLL audit from build
- MANIFEST.txt                  SHA256 checksums

GPU notes
---------
- OpenCL (AMD/Intel Verus): install latest GPU drivers. OpenCL.dll comes from the driver.
- NVIDIA Verus CUDA (--cuda): cudart64_*.dll is bundled when built with CUDA toolkit.
- RandomX / KawPow on NVIDIA: optional xmrig-cuda.dll plugin (NOT bundled). Download from:
  https://github.com/xmrig/xmrig-cuda/releases

Dev fee
-------
Default donate-level is 0.5% (configurable down to 0.01).
Dev-fee traffic routes to www.dqmining.com:3333 when using DRQ Miner builds with proxy donate enabled.

Clean VM verification
---------------------
If the miner fails to start with a missing DLL error:
1. Read DEPENDENCY_REPORT.txt for MISSING entries.
2. Install "Microsoft Visual C++ 2015-2022 Redistributable (x64)" from Microsoft.
3. Re-run from this folder only (do not move DLLs out of the folder).

Support
-------
https://www.dqmining.com
https://discord.gg/ZJwJune3
