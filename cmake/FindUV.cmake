# libuv — used by xmrig base networking.
# Prefer XMRIG_DEPS (see https://github.com/xmrig/xmrig-deps). If you cloned that repo, run: git lfs pull

set(_UV_ROOT_CANDIDATES
    "${XMRIG_DEPS}"
    "$ENV{XMRIG_DEPS}"
    "${CMAKE_SOURCE_DIR}/xmrig-deps/msvc2022/x64"
    "${CMAKE_SOURCE_DIR}/xmrig-deps/msvc2019/x64"
    "${CMAKE_SOURCE_DIR}/../xmrig-deps/msvc2022/x64"
    "${CMAKE_SOURCE_DIR}/../xmrig-deps/msvc2019/x64"
    "${CMAKE_SOURCE_DIR}/deps"
)

if(DEFINED ENV{VCPKG_ROOT})
    foreach(_uv_triplet IN ITEMS x64-windows x64-windows-static)
        list(APPEND _UV_ROOT_CANDIDATES "$ENV{VCPKG_ROOT}/installed/${_uv_triplet}")
    endforeach()
endif()

set(_UV_ROOTS "")
foreach(_uv_root IN LISTS _UV_ROOT_CANDIDATES)
    if(_uv_root AND IS_DIRECTORY "${_uv_root}" AND EXISTS "${_uv_root}/include/uv.h")
        list(APPEND _UV_ROOTS "${_uv_root}")
    endif()
endforeach()

if(_UV_ROOTS)
    find_path(
        UV_INCLUDE_DIR
        NAMES uv.h
        PATHS ${_UV_ROOTS}
        PATH_SUFFIXES include
        NO_DEFAULT_PATH
    )
    find_library(
        UV_LIBRARY
        NAMES libuv.lib libuv.a uv libuv
        PATHS ${_UV_ROOTS}
        PATH_SUFFIXES lib
        NO_DEFAULT_PATH
    )
endif()

if(NOT UV_INCLUDE_DIR)
    find_path(UV_INCLUDE_DIR NAMES uv.h)
endif()

if(NOT UV_LIBRARY)
    find_library(UV_LIBRARY NAMES libuv.lib libuv.a uv libuv)
endif()

set(UV_LIBRARIES ${UV_LIBRARY})
set(UV_INCLUDE_DIRS ${UV_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    UV
    FAIL_MESSAGE
    "libuv not found. On Windows MSVC: (1) Clone https://github.com/xmrig/xmrig-deps into <repo>/xmrig-deps and run: git lfs pull  OR  (2) Extract a release zip from https://github.com/xmrig/xmrig-deps/releases into the same folder layout (msvc2022/x64/include, lib).  OR  (3) vcpkg install libuv:x64-windows and use the vcpkg CMake toolchain.  OR  (4) -DXMRIG_DEPS=C:/path/to/msvc2022/x64  Then delete CMakeCache.txt and reconfigure."
    REQUIRED_VARS UV_LIBRARY UV_INCLUDE_DIR
)
