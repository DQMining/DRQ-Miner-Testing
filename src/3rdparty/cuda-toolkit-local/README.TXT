Optional local CUDA Toolkit (extracted install only)
====================================================

1. Run the official NVIDIA CUDA Toolkit installer (or extract the payload) so you
   have a normal toolkit layout with bin/nvcc(.exe), include/, lib/, etc.

2. Copy or point that root HERE as the contents of this folder, e.g.:
      cuda-toolkit-local/bin/nvcc.exe
      cuda-toolkit-local/include/...
      cuda-toolkit-local/lib/...

3. This directory is gitignored. Do not commit the toolkit or the installer .exe:
   size is large and redistribution is governed by the NVIDIA EULA.

4. CMake checks this path automatically. You can also set cache/env:
      XMRIG_CUDA_TOOLKIT_ROOT=C:/path/to/cuda

End users only need cudart64_*.dll next to DRQMiner.exe (copied on build when
embedded Verus is enabled), not the full toolkit.
