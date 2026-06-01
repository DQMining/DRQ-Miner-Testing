# Bundled libuv under src/3rdparty/libuv (downloaded on first configure; see src/3rdparty/README.libuv.txt).

set(_XMRIG_LIBUV_ROOT "${CMAKE_SOURCE_DIR}/src/3rdparty/libuv")
set(_XMRIG_LIBUV_TAR "${CMAKE_BINARY_DIR}/libuv-v1.51.0.tar.gz")
set(_XMRIG_LIBUV_SHA256 "5f0557b90b1106de71951a3c3931de5e0430d78da1d9a10287ebc7a3f78ef8eb")

if(NOT EXISTS "${_XMRIG_LIBUV_ROOT}/CMakeLists.txt")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/src/3rdparty")
    message(STATUS "Downloading libuv 1.51.0 into src/3rdparty/libuv ...")
    file(DOWNLOAD
        "https://dist.libuv.org/dist/v1.51.0/libuv-v1.51.0.tar.gz"
        "${_XMRIG_LIBUV_TAR}"
        EXPECTED_HASH SHA256=${_XMRIG_LIBUV_SHA256}
        TLS_VERIFY ON
        SHOW_PROGRESS
    )
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E tar xzf "${_XMRIG_LIBUV_TAR}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src/3rdparty"
        RESULT_VARIABLE _xmrig_libuv_extract_rc
    )
    if(NOT _xmrig_libuv_extract_rc EQUAL 0)
        message(FATAL_ERROR "Extracting libuv tarball failed (code ${_xmrig_libuv_extract_rc}).")
    endif()
    if(EXISTS "${CMAKE_SOURCE_DIR}/src/3rdparty/libuv-v1.51.0/CMakeLists.txt")
        file(RENAME "${CMAKE_SOURCE_DIR}/src/3rdparty/libuv-v1.51.0" "${_XMRIG_LIBUV_ROOT}")
    endif()
endif()

if(NOT EXISTS "${_XMRIG_LIBUV_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR
        "Bundled libuv missing at ${_XMRIG_LIBUV_ROOT}. "
        "Delete src/3rdparty/libuv if it is incomplete and reconfigure (needs network for download).")
endif()

set(LIBUV_BUILD_SHARED OFF CACHE BOOL "Build libuv shared library" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

add_subdirectory("${_XMRIG_LIBUV_ROOT}" "${CMAKE_BINARY_DIR}/3rdparty-libuv" EXCLUDE_FROM_ALL)

if(MSVC AND TARGET uv_a)
    set_property(TARGET uv_a PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(UV_INCLUDE_DIR "${_XMRIG_LIBUV_ROOT}/include" CACHE PATH "libuv headers" FORCE)
set(UV_LIBRARIES libuv::libuv)
set(UV_INCLUDE_DIRS ${UV_INCLUDE_DIR})
set(UV_FOUND TRUE)

message(STATUS "Using bundled libuv: ${_XMRIG_LIBUV_ROOT}")
