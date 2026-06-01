# Resolves XMRIG_DEPS when not passed on the command line.
# Expected layout: https://github.com/xmrig/xmrig-deps (release zip or git clone into ./xmrig-deps).
# XMRIG_DEPS must point at the toolchain folder that contains include/ and lib/ (e.g. .../msvc2022/x64).

if(NOT XMRIG_DEPS)
    if(DEFINED ENV{XMRIG_DEPS} AND NOT "$ENV{XMRIG_DEPS}" STREQUAL "")
        set(XMRIG_DEPS "$ENV{XMRIG_DEPS}")
    elseif(WIN32)
        foreach(_xmrig_deps_try IN ITEMS
                "${CMAKE_SOURCE_DIR}/xmrig-deps/msvc2022/x64"
                "${CMAKE_SOURCE_DIR}/xmrig-deps/msvc2019/x64"
                "${CMAKE_SOURCE_DIR}/../xmrig-deps/msvc2022/x64"
                "${CMAKE_SOURCE_DIR}/../xmrig-deps/msvc2019/x64")
            if(EXISTS "${_xmrig_deps_try}/include/uv.h")
                set(XMRIG_DEPS "${_xmrig_deps_try}")
                break()
            endif()
        endforeach()
    else()
        foreach(_xmrig_deps_try IN ITEMS
                "${CMAKE_SOURCE_DIR}/xmrig-deps/gcc/x64"
                "${CMAKE_SOURCE_DIR}/xmrig-deps/clang/x64"
                "${CMAKE_SOURCE_DIR}/deps")
            if(EXISTS "${_xmrig_deps_try}/include/uv.h")
                set(XMRIG_DEPS "${_xmrig_deps_try}")
                break()
            endif()
        endforeach()
    endif()
endif()
