set(XMRIG_ASTRO_SPSA_ENABLED OFF)
set(XMRIG_ASTRO_SPSA_LIBRARY "")

if (NOT WITH_ASTROBWT)
    return()
endif()

option(WITH_ASTRO_SPSA "Use AstroSPSA suffix-array library (TNN path, ~2x SA speed)" ON)

if (NOT WITH_ASTRO_SPSA)
    return()
endif()

# AstroSPSA prebuild is x86_64 Windows (DLL) only; ARM/phone uses divsufsort (correct, slower).
if (XMRIG_ARM OR XMRIG_OS_ANDROID OR CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64|armv8)")
    message(STATUS "AstroSPSA: skipped on ARM/Android (DERO uses divsufsort CPU path)")
    return()
endif()

set(_SPSA_LIB_DIR "${CMAKE_SOURCE_DIR}/lib/astrospsa")
set(_SPSA_GNU_LIB "${_SPSA_LIB_DIR}/libastroSPSA_gnu.a")
if (NOT EXISTS "${_SPSA_GNU_LIB}")
    set(_SPSA_GNU_LIB "${_SPSA_LIB_DIR}/libastroSPSA_win_amd64_clang_18_x86-64-v4.a")
endif()
if (NOT EXISTS "${_SPSA_GNU_LIB}")
    message(STATUS "AstroSPSA: prebuilt library not found in ${_SPSA_LIB_DIR}, using divsufsort")
    return()
endif()

find_program(XMRIG_CLANG_PP NAMES clang++
    PATHS
        "C:/Program Files/LLVM/bin"
        "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/bin"
        "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin"
)

if (NOT XMRIG_CLANG_PP)
    message(STATUS "AstroSPSA: clang++ not found, using divsufsort")
    return()
endif()

message(STATUS "AstroSPSA: enabled (${_SPSA_GNU_LIB}, ${XMRIG_CLANG_PP})")

set(XMRIG_ASTRO_SPSA_ENABLED ON)
add_definitions(/DXMRIG_ASTROBWT_USE_SPSA)

set(_SPSA_BRIDGE_SRC "${CMAKE_SOURCE_DIR}/src/crypto/astrobwt/spsa/spsa_bridge.cpp")
set(_SPSA_STUBS_SRC "${CMAKE_SOURCE_DIR}/src/crypto/astrobwt/spsa/spsa_tnn_stubs.cpp")
set(_SPSA_BRIDGE_OBJ "${CMAKE_CURRENT_BINARY_DIR}/spsa_bridge_gnu.obj")
set(_SPSA_STUBS_OBJ "${CMAKE_CURRENT_BINARY_DIR}/spsa_tnn_stubs_gnu.obj")
set(_SPSA_DLL "${CMAKE_CURRENT_BINARY_DIR}/xmrig_astro_spsa.dll")
set(_SPSA_IMPLIB "${CMAKE_CURRENT_BINARY_DIR}/xmrig_astro_spsa.lib")

set(_SPSA_CL_INC -I"${_SPSA_LIB_DIR}" -I"${CMAKE_SOURCE_DIR}/src")
if (OPENSSL_INCLUDE_DIR)
    list(APPEND _SPSA_CL_INC -I"${OPENSSL_INCLUDE_DIR}")
endif()

set(_SPSA_LINK_DIRS "")
set(_SPSA_LINK_LIBS "")
if (OPENSSL_LIBRARIES)
    foreach(_lib IN LISTS OPENSSL_LIBRARIES)
        get_filename_component(_libdir "${_lib}" DIRECTORY)
        get_filename_component(_name "${_lib}" NAME_WE)
        if (_libdir)
            list(APPEND _SPSA_LINK_DIRS -L"${_libdir}")
        endif()
        if (_name MATCHES "^libcrypto")
            list(APPEND _SPSA_LINK_LIBS -lcrypto)
        elseif(_name MATCHES "^libssl")
            list(APPEND _SPSA_LINK_LIBS -lssl)
        endif()
    endforeach()
endif()
if (NOT _SPSA_LINK_LIBS)
    list(APPEND _SPSA_LINK_DIRS -L"C:/OpenSSL-Win64/lib")
    list(APPEND _SPSA_LINK_LIBS -lcrypto)
endif()

add_custom_command(
    OUTPUT "${_SPSA_DLL}" "${_SPSA_IMPLIB}"
    COMMAND "${XMRIG_CLANG_PP}" --target=x86_64-w64-windows-gnu -std=c++17 -O2 -mavx2
        -DUSE_ASTRO_SPSA ${_SPSA_CL_INC}
        -c "${_SPSA_BRIDGE_SRC}" -o "${_SPSA_BRIDGE_OBJ}"
    COMMAND "${XMRIG_CLANG_PP}" --target=x86_64-w64-windows-gnu -std=c++17 -O2
        -c "${_SPSA_STUBS_SRC}" -o "${_SPSA_STUBS_OBJ}"
    COMMAND "${XMRIG_CLANG_PP}" --target=x86_64-w64-windows-gnu -shared -O2
        -fuse-ld=lld
        -o "${_SPSA_DLL}"
        "${_SPSA_BRIDGE_OBJ}" "${_SPSA_STUBS_OBJ}" "${_SPSA_GNU_LIB}"
        ${_SPSA_LINK_DIRS} ${_SPSA_LINK_LIBS}
        -lstdc++ -lpthread -lwinpthread
        -Wl,--out-implib,"${_SPSA_IMPLIB}"
    DEPENDS "${_SPSA_BRIDGE_SRC}" "${_SPSA_STUBS_SRC}" "${_SPSA_GNU_LIB}"
    COMMENT "Building AstroSPSA DLL (clang++ gnu ABI)"
)

add_custom_target(xmrig_astro_spsa DEPENDS "${_SPSA_DLL}" "${_SPSA_IMPLIB}")

# spsa_finalize.cpp is always linked from cmake/astrobwt.cmake (stubs when DLL absent).
list(APPEND HEADERS_CRYPTO src/crypto/astrobwt/spsa/spsa_finalize.h)

set(XMRIG_ASTRO_SPSA_LIBRARY "${_SPSA_IMPLIB}")
set(XMRIG_ASTRO_SPSA_DLL "${_SPSA_DLL}")
