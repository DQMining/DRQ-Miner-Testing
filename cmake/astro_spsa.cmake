set(XMRIG_ASTRO_SPSA_ENABLED OFF)
set(XMRIG_ASTRO_SPSA_LIBRARY "")
set(XMRIG_ASTRO_SPSA_BRIDGE_SOURCES "")
set(XMRIG_ASTRO_SPSA_INCLUDE_DIR "")

if (NOT WITH_ASTROBWT)
    return()
endif()

option(WITH_ASTRO_SPSA "Use AstroSPSA suffix-array library (TNN path, ~2x SA speed)" ON)

if (NOT WITH_ASTRO_SPSA)
    return()
endif()

set(_SPSA_LIB_DIR "${CMAKE_SOURCE_DIR}/lib/astrospsa")
set(_SPSA_FETCH_TAG "8938667bfa3253b52c622daf575fc11674d7067b")

function(xmrig_astro_spsa_fetch_lib_dir OUT_VAR)
    if (EXISTS "${_SPSA_LIB_DIR}/spsa.hpp")
        file(GLOB _local_libs "${_SPSA_LIB_DIR}/libastroSPSA_*.a")
        if (_local_libs)
            set(${OUT_VAR} "${_SPSA_LIB_DIR}" PARENT_SCOPE)
            return()
        endif()
    endif()

    include(FetchContent)
    FetchContent_Declare(
        xmrig_astrospsa
        GIT_REPOSITORY https://gitlab.com/Tritonn204/astro-spsa.git
        GIT_TAG        ${_SPSA_FETCH_TAG}
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(xmrig_astrospsa)
    set(${OUT_VAR} "${xmrig_astrospsa_SOURCE_DIR}" PARENT_SCOPE)
endfunction()

function(xmrig_astro_spsa_pick_lib OUT_VAR LIB_DIR OS_PREFIX TARGET_ARCH)
    set(_suffixes
        "_clang_20_armv8-a+crypto.a"
        "_clang_19_armv8-a+crypto.a"
        "_clang_18_armv8-a+crypto.a"
        "_clang_15_armv8-a+crypto.a"
        "_clang_14_armv8-a+crypto.a"
        "_clang_20_armv8-a+aes.a"
        "_clang_19_armv8-a+aes.a"
        "_clang_18_armv8-a+aes.a"
        "_clang_20_x86-64-v4.a"
        "_clang_19_x86-64-v4.a"
        "_clang_18_x86-64-v4.a"
        "_clang_18_znver4.a"
        "_clang_18_znver3.a"
        "_clang_18_x86-64-v3.a"
        "_clang_18_x86-64-v2.a"
        ".a"
    )

    foreach(_suffix IN LISTS _suffixes)
        set(_candidate "${LIB_DIR}/libastroSPSA_${OS_PREFIX}_${TARGET_ARCH}${_suffix}")
        if (EXISTS "${_candidate}")
            set(${OUT_VAR} "${_candidate}" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    set(_fallback_gnu "${LIB_DIR}/libastroSPSA_gnu.a")
    if (EXISTS "${_fallback_gnu}")
        set(${OUT_VAR} "${_fallback_gnu}" PARENT_SCOPE)
        return()
    endif()

    set(${OUT_VAR} "" PARENT_SCOPE)
endfunction()

function(xmrig_astro_spsa_enable_static BRIDGE_SRC LIB_PATH)
    set(XMRIG_ASTRO_SPSA_ENABLED ON PARENT_SCOPE)
    set(XMRIG_ASTRO_SPSA_LIBRARY "${LIB_PATH}" PARENT_SCOPE)
    set(XMRIG_ASTRO_SPSA_BRIDGE_SOURCES
        "${BRIDGE_SRC}"
        "${CMAKE_SOURCE_DIR}/src/crypto/astrobwt/spsa/spsa_tnn_stubs.cpp"
        PARENT_SCOPE
    )
    set(XMRIG_ASTRO_SPSA_INCLUDE_DIR "${_SPSA_LIB_DIR}" PARENT_SCOPE)
endfunction()

list(APPEND HEADERS_CRYPTO src/crypto/astrobwt/spsa/spsa_finalize.h)

# Phone / ARM: static link AstroSPSA + POSIX bridge (headers always from lib/astrospsa).
if (XMRIG_ARM OR XMRIG_OS_ANDROID OR CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64|armv8)")
    xmrig_astro_spsa_fetch_lib_dir(_SPSA_FETCHED_DIR)
    if (APPLE)
        set(_SPSA_OS_PREFIX "macos")
    else()
        set(_SPSA_OS_PREFIX "linux")
    endif()
    xmrig_astro_spsa_pick_lib(_SPSA_GNU_LIB "${_SPSA_FETCHED_DIR}" "${_SPSA_OS_PREFIX}" "aarch64")

    if (NOT _SPSA_GNU_LIB)
        message(STATUS "AstroSPSA: aarch64 prebuilt library not found, using divsufsort")
        return()
    endif()

    message(STATUS "AstroSPSA: enabled for ARM (${_SPSA_GNU_LIB}, headers ${_SPSA_LIB_DIR})")
    xmrig_astro_spsa_enable_static(
        "${CMAKE_SOURCE_DIR}/src/crypto/astrobwt/spsa/spsa_bridge_posix.cpp"
        "${_SPSA_GNU_LIB}"
    )
    return()
endif()

# Windows x64 only: AstroSPSA DLL via clang++ (linux CI must not enter this path).
if (NOT WIN32)
    message(STATUS "AstroSPSA: skipped on non-Windows desktop (phone uses ARM static path)")
    return()
endif()

set(_SPSA_GNU_LIB "${_SPSA_LIB_DIR}/libastroSPSA_gnu.a")
if (NOT EXISTS "${_SPSA_GNU_LIB}")
    set(_SPSA_GNU_LIB "${_SPSA_LIB_DIR}/libastroSPSA_win_amd64_clang_18_x86-64-v4.a")
endif()
if (NOT EXISTS "${_SPSA_GNU_LIB}")
    xmrig_astro_spsa_fetch_lib_dir(_SPSA_FETCHED_DIR)
    xmrig_astro_spsa_pick_lib(_SPSA_GNU_LIB "${_SPSA_FETCHED_DIR}" "win" "amd64")
endif()
if (NOT _SPSA_GNU_LIB)
    message(STATUS "AstroSPSA: prebuilt library not found, using divsufsort")
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
set(_SPSA_CRYPTO_LIB "")
if (OPENSSL_LIBRARIES)
    foreach(_lib IN LISTS OPENSSL_LIBRARIES)
        get_filename_component(_name "${_lib}" NAME)
        if (_name MATCHES "^libcrypto.*\\.lib$")
            set(_SPSA_CRYPTO_LIB "${_lib}")
            break()
        endif()
    endforeach()
endif()
if (NOT _SPSA_CRYPTO_LIB AND OPENSSL_CRYPTO_LIBRARY)
    set(_SPSA_CRYPTO_LIB "${OPENSSL_CRYPTO_LIBRARY}")
endif()
if (NOT _SPSA_CRYPTO_LIB AND EXISTS "C:/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib")
    set(_SPSA_CRYPTO_LIB "C:/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib")
endif()
if (_SPSA_CRYPTO_LIB)
    # Static-link libcrypto into the DLL so clean Windows does not need libcrypto-3-x64__.dll
    # (clang++ -lcrypto + lld incorrectly imports libcrypto-3-x64__.dll from the MSVC import lib).
    list(APPEND _SPSA_LINK_LIBS "${_SPSA_CRYPTO_LIB}" -lcrypt32 -lws2_32)
else()
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

set(XMRIG_ASTRO_SPSA_LIBRARY "${_SPSA_IMPLIB}")
set(XMRIG_ASTRO_SPSA_DLL "${_SPSA_DLL}")
