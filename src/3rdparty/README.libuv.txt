libuv (https://libuv.org) — bundled build
========================================

When XMRIG_USE_BUNDLED_LIBUV is ON (default), CMake downloads libuv v1.51.0
(verified SHA-256) into this folder as src/3rdparty/libuv on the first
configure. Internet access is required once; afterwards the tree is reused.

To use prebuilt xmrig-deps or system libuv instead:
  cmake ... -DXMRIG_USE_BUNDLED_LIBUV=OFF
and set XMRIG_DEPS or UV_INCLUDE_DIR / UV_LIBRARY as documented in
doc/build/CMAKE_OPTIONS.md.

The libuv/ directory is listed in .gitignore so it is not committed.
