cmake_minimum_required(VERSION 3.10)
if (POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
project(xmrig)

# /MD everywhere: matches VS + GhostRider + bundled hwloc + nvcc; /MT causes LNK2038 vs those libs and __imp_* CRT errors.
if (MSVC AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.15)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)
endif()

option(WITH_HWLOC           "Enable hwloc support" ON)
option(WITH_CN_LITE         "Enable CryptoNight-Lite algorithms family" ON)
option(WITH_CN_HEAVY        "Enable CryptoNight-Heavy algorithms family" ON)
option(WITH_CN_PICO         "Enable CryptoNight-Pico algorithm" ON)
option(WITH_CN_FEMTO        "Enable CryptoNight-UPX2 algorithm" ON)
option(WITH_RANDOMX         "Enable RandomX algorithms family" ON)
option(WITH_ARGON2          "Enable Argon2 algorithms family" ON)
option(WITH_KAWPOW          "Enable KawPow algorithms family" ON)
option(WITH_GHOSTRIDER      "Enable GhostRider algorithm" ON)
option(WITH_VERUSHASH        "Enable VERUSHASH algorithm" ON)
option(WITH_ASTROBWT        "Enable AstroBWT algorithm" ON)
option(WITH_BC3             "Enable BC3 SHA3-256t tests (Phase 2)" ON)
option(WITH_NM              "Enable Neuromorph (nm/1) algorithm" ON)
option(WITH_HTTP            "Enable HTTP protocol support (client/server)" ON)
option(WITH_DEBUG_LOG       "Enable debug log output" OFF)
option(WITH_TLS             "Enable OpenSSL support" ON)
option(WITH_ASM             "Enable ASM PoW implementations" ON)
option(WITH_MSR             "Enable MSR mod & 1st-gen Ryzen fix" ON)
option(WITH_ENV_VARS        "Enable environment variables support in config file" ON)
option(WITH_EMBEDDED_CONFIG "Enable internal embedded JSON config" OFF)
option(WITH_OPENCL          "Enable OpenCL backend" ON)
set(WITH_OPENCL_VERSION 200 CACHE STRING "Target OpenCL version")
set_property(CACHE WITH_OPENCL_VERSION PROPERTY STRINGS 120 200 210 220)
option(WITH_CUDA            "Enable CUDA backend" ON)
option(WITH_NVML            "Enable NVML (NVIDIA Management Library) support (only if CUDA backend enabled)" ON)
option(WITH_ADL             "Enable ADL (AMD Display Library) or sysfs support (only if OpenCL backend enabled)" ON)
option(WITH_STRICT_CACHE    "Enable strict checks for OpenCL cache" ON)
option(WITH_INTERLEAVE_DEBUG_LOG "Enable debug log for threads interleave" OFF)
option(WITH_PROFILING       "Enable profiling for developers" OFF)
option(WITH_SSE4_1          "Enable SSE 4.1 for Blake2" ON)
option(WITH_AVX2            "Enable AVX2 for Blake2" ON)
option(WITH_VAES            "Enable VAES instructions for Cryptonight" ON)
option(WITH_BENCHMARK       "Enable builtin RandomX benchmark and stress test" ON)
option(WITH_SECURE_JIT      "Enable secure access to JIT memory" OFF)
option(WITH_DMI             "Enable DMI/SMBIOS reader" ON)

option(BUILD_STATIC         "Build static binary" OFF)
option(ARM_V8               "Force ARMv8 (64 bit) architecture, use with caution if automatic detection fails, but you sure it may work" OFF)
option(ARM_V7               "Force ARMv7 (32 bit) architecture, use with caution if automatic detection fails, but you sure it may work" OFF)
option(HWLOC_DEBUG          "Enable hwloc debug helpers and log" OFF)
option(XMRIG_VERUS_CUDA_LEGACY_KEPLER "Verus CUDA for Kepler/GRID (sm_35–sm_61); requires CUDA Toolkit 11.x, not 13+" OFF)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")


include (CheckIncludeFile)
include (cmake/cpu.cmake)
include (cmake/os.cmake)
include (cmake/android.cmake)
include (src/base/base.cmake)
include (src/backend/backend.cmake)


set(HEADERS
    "${HEADERS_BASE}"
    "${HEADERS_BASE_HTTP}"
    "${HEADERS_BACKEND}"
    src/App.h
    src/core/config/Config_default.h
    src/core/config/Config_platform.h
    src/core/config/Config.h
    src/core/config/ConfigTransform.h
    src/core/config/usage.h
    src/core/Controller.h
    src/core/Miner.h
    src/core/Taskbar.h
    src/core/update/UpdateApply.h
    src/core/update/UpdateCheck.h
    src/core/update/UpdateConfig.h
    src/net/interfaces/IJobResultListener.h
    src/net/JobResult.h
    src/net/JobResults.h
    src/net/Network.h
    src/net/strategies/DonateStrategy.h
    src/Summary.h
    src/version.h
   )

set(HEADERS_CRYPTO
    src/backend/common/interfaces/IMemoryPool.h
    src/crypto/cn/asm/CryptonightR_template.h
    src/crypto/cn/c_blake256.h
    src/crypto/cn/c_groestl.h
    src/crypto/cn/c_jh.h
    src/crypto/cn/c_skein.h
    src/crypto/cn/CnAlgo.h
    src/crypto/cn/CnCtx.h
    src/crypto/cn/CnHash.h
    src/crypto/cn/CryptoNight_monero.h
    src/crypto/cn/CryptoNight_test.h
    src/crypto/cn/CryptoNight.h
    src/crypto/cn/groestl_tables.h
    src/crypto/cn/hash.h
    src/crypto/cn/skein_port.h
    src/crypto/cn/soft_aes.h
    src/crypto/common/HugePagesInfo.h
    src/crypto/common/MemoryPool.h
    src/crypto/common/Nonce.h
    src/crypto/common/portable/mm_malloc.h
    src/crypto/common/VirtualMemory.h
   )

if (XMRIG_ARM OR XMRIG_RISCV)
    set(HEADERS_CRYPTO "${HEADERS_CRYPTO}" src/crypto/cn/CryptoNight_arm.h)
else()
    set(HEADERS_CRYPTO "${HEADERS_CRYPTO}" src/crypto/cn/CryptoNight_x86.h)
endif()

set(SOURCES
    "${SOURCES_BASE}"
    "${SOURCES_BASE_HTTP}"
    "${SOURCES_BACKEND}"
    src/App.cpp
    src/core/config/Config.cpp
    src/core/config/ConfigTransform.cpp
    src/core/Controller.cpp
    src/core/Miner.cpp
    src/core/Taskbar.cpp
    src/core/update/UpdateApply.cpp
    src/core/update/UpdateCheck.cpp
    src/core/update/UpdateConfig.cpp
    src/net/JobResults.cpp
    src/net/Network.cpp
    src/net/strategies/DonateStrategy.cpp
    src/Summary.cpp
    src/xmrig.cpp
   )

set(SOURCES_CRYPTO
    src/crypto/cn/c_blake256.c
    src/crypto/cn/c_groestl.c
    src/crypto/cn/c_jh.c
    src/crypto/cn/c_skein.c
    src/crypto/cn/CnCtx.cpp
    src/crypto/cn/CnHash.cpp
    src/crypto/common/HugePagesInfo.cpp
    src/crypto/common/MemoryPool.cpp
    src/crypto/common/Nonce.cpp
    src/crypto/common/VirtualMemory.cpp
   )

if (CMAKE_C_COMPILER_ID MATCHES GNU)
    set_source_files_properties(src/crypto/cn/CnHash.cpp PROPERTIES COMPILE_FLAGS "-Ofast -fno-tree-vectorize")
endif()

if (WITH_VAES)
    add_definitions(-DXMRIG_VAES)
    set(HEADERS_CRYPTO "${HEADERS_CRYPTO}" src/crypto/cn/CryptoNight_x86_vaes.h)
    set(SOURCES_CRYPTO "${SOURCES_CRYPTO}" src/crypto/cn/CryptoNight_x86_vaes.cpp)
    if (CMAKE_C_COMPILER_ID MATCHES GNU OR CMAKE_C_COMPILER_ID MATCHES Clang)
        set_source_files_properties(src/crypto/cn/CryptoNight_x86_vaes.cpp PROPERTIES COMPILE_FLAGS "-Ofast -fno-tree-vectorize -mavx2 -mvaes")
    endif()
endif()

if (WITH_HWLOC)
    list(APPEND HEADERS_CRYPTO
        src/crypto/common/NUMAMemoryPool.h
        )

    list(APPEND SOURCES_CRYPTO
        src/crypto/common/NUMAMemoryPool.cpp
        src/crypto/common/VirtualMemory_hwloc.cpp
        )
endif()

if (XMRIG_OS_WIN)
    list(APPEND SOURCES_OS
        res/app.rc
        src/App_win.cpp
        src/crypto/common/VirtualMemory_win.cpp
        )

    set(EXTRA_LIBS ws2_32 psapi iphlpapi userenv dbghelp)
elseif (XMRIG_OS_APPLE)
    list(APPEND SOURCES_OS
        src/App_unix.cpp
        src/crypto/common/VirtualMemory_unix.cpp
        )

    find_library(IOKIT_LIBRARY IOKit)
    find_library(CORESERVICES_LIBRARY CoreServices)
    set(EXTRA_LIBS ${IOKIT_LIBRARY} ${CORESERVICES_LIBRARY})
else()
    list(APPEND SOURCES_OS
        src/App_unix.cpp
        src/crypto/common/VirtualMemory_unix.cpp
        )

    if (XMRIG_OS_ANDROID)
        set(EXTRA_LIBS pthread rt dl log)
    elseif (XMRIG_OS_LINUX)
        list(APPEND SOURCES_OS
            src/crypto/common/LinuxMemory.h
            src/crypto/common/LinuxMemory.cpp
            )

        set(EXTRA_LIBS pthread rt dl)
    elseif (XMRIG_OS_FREEBSD)
        set(EXTRA_LIBS kvm pthread)
    endif()
endif()

add_definitions(-DXMRIG_MINER_PROJECT -DXMRIG_JSON_SINGLE_LINE_ARRAY)
add_definitions(-D__STDC_FORMAT_MACROS -DUNICODE -D_FILE_OFFSET_BITS=64)

include(cmake/flags.cmake)
include(cmake/deps.cmake)

option(XMRIG_USE_BUNDLED_LIBUV "Build libuv from src/3rdparty/libuv (auto-download on first configure)" ON)
if(XMRIG_USE_BUNDLED_LIBUV)
    include(cmake/bundled_libuv.cmake)
else()
    find_package(UV REQUIRED)
endif()
include(cmake/randomx.cmake)
include(cmake/argon2.cmake)
include(cmake/kawpow.cmake)
include(cmake/ghostrider.cmake)
include(cmake/verushash.cmake)

# Embedded CUDA Verus kernel (monkins1010/ccminer verus2.2gpu) — optional if toolkit present.
set(XMRIG_HAVE_CUDA_VERUS OFF)
if (WITH_CUDA AND WITH_VERUSHASH)
    set(XMRIG_CUDA_TOOLKIT_ROOT "" CACHE PATH "Optional: CUDA Toolkit root (e.g. extracted copy in src/3rdparty/cuda-toolkit-local; env XMRIG_CUDA_TOOLKIT_ROOT also works)")

    # Prefer explicit / repo-local toolkit, then CUDA_PATH / default Windows install.
    if (NOT CMAKE_CUDA_COMPILER)
        set(_xmrig_cuda_roots "")
        if (XMRIG_CUDA_TOOLKIT_ROOT)
            list(APPEND _xmrig_cuda_roots "${XMRIG_CUDA_TOOLKIT_ROOT}")
        endif()
        if (DEFINED ENV{XMRIG_CUDA_TOOLKIT_ROOT} AND NOT "$ENV{XMRIG_CUDA_TOOLKIT_ROOT}" STREQUAL "")
            list(APPEND _xmrig_cuda_roots "$ENV{XMRIG_CUDA_TOOLKIT_ROOT}")
        endif()
        list(APPEND _xmrig_cuda_roots "${CMAKE_SOURCE_DIR}/src/3rdparty/cuda-toolkit-local")

        foreach (_root IN LISTS _xmrig_cuda_roots)
            if (NOT _root)
            elseif (WIN32 AND EXISTS "${_root}/bin/nvcc.exe")
                set(CMAKE_CUDA_COMPILER "${_root}/bin/nvcc.exe" CACHE FILEPATH "CUDA compiler" FORCE)
                set(CUDAToolkit_ROOT "${_root}" CACHE PATH "NVIDIA CUDA Toolkit" FORCE)
                break()
            elseif (NOT WIN32 AND EXISTS "${_root}/bin/nvcc")
                set(CMAKE_CUDA_COMPILER "${_root}/bin/nvcc" CACHE FILEPATH "CUDA compiler" FORCE)
                set(CUDAToolkit_ROOT "${_root}" CACHE PATH "NVIDIA CUDA Toolkit" FORCE)
                break()
            endif()
        endforeach()
    endif()

    if (WIN32 AND NOT CMAKE_CUDA_COMPILER)
        if (DEFINED ENV{CUDA_PATH} AND EXISTS "$ENV{CUDA_PATH}/bin/nvcc.exe")
            set(CMAKE_CUDA_COMPILER "$ENV{CUDA_PATH}/bin/nvcc.exe" CACHE FILEPATH "CUDA compiler" FORCE)
        else()
            if (XMRIG_VERUS_CUDA_LEGACY_KEPLER)
                set(_xmrig_cuda_search 11.8 11.7 11.6 11.5 11.4 11.3 11.2 11.1 11.0 12.0 12.1 12.2)
            else()
                set(_xmrig_cuda_search 13.2 13.1 13.0 12.8 12.7 12.6 12.5 12.4 12.3 12.2 12.1 12.0 11.8 11.7)
            endif()
            foreach (_cuda_ver IN ITEMS ${_xmrig_cuda_search})
                set(_nvcc "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v${_cuda_ver}/bin/nvcc.exe")
                if (EXISTS "${_nvcc}")
                    set(CMAKE_CUDA_COMPILER "${_nvcc}" CACHE FILEPATH "CUDA compiler" FORCE)
                    break()
                endif()
            endforeach()
        endif()
    endif()
    include(CheckLanguage)
    check_language(CUDA)
    if (CMAKE_CUDA_COMPILER)
        enable_language(CUDA)
        set(CMAKE_CUDA_STANDARD 14)
        set(CMAKE_CUDA_STANDARD_REQUIRED ON)
        if (XMRIG_VERUS_CUDA_LEGACY_KEPLER)
            if (CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL 12.0 AND NOT CMAKE_CUDA_COMPILER_VERSION VERSION_LESS 13.0)
                message(WARNING "XMRIG_VERUS_CUDA_LEGACY_KEPLER: CUDA 12.x can build sm_35; CUDA 13+ cannot — install CUDA 11.8 and set XMRIG_CUDA_TOOLKIT_ROOT")
            endif()
            if (CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0)
                message(FATAL_ERROR "XMRIG_VERUS_CUDA_LEGACY_KEPLER requires CUDA 11.8 (or 12.x with sm_35 enabled), not CUDA ${CMAKE_CUDA_COMPILER_VERSION}")
            endif()
            set(CMAKE_CUDA_ARCHITECTURES 35 37 50 52 61 CACHE STRING "Kepler/Maxwell Verus CUDA arches" FORCE)
            message(STATUS "Verus CUDA: legacy Kepler/GRID arches (sm_35–sm_61)")
        elseif (NOT DEFINED CMAKE_CUDA_ARCHITECTURES OR "${CMAKE_CUDA_ARCHITECTURES}" STREQUAL "")
            # CUDA 13+ drops sm_61 (Pascal); including it makes nvcc fail the compiler test.
            if (CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0)
                set(CMAKE_CUDA_ARCHITECTURES 75 86 89 90 120)
            else()
                set(CMAKE_CUDA_ARCHITECTURES 61 75 86)
            endif()
        elseif (CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0)
            set(_xmrig_cuda_archs ${CMAKE_CUDA_ARCHITECTURES})
            list(REMOVE_ITEM _xmrig_cuda_archs 52 53 60 61)
            list(LENGTH CMAKE_CUDA_ARCHITECTURES _xmrig_olen)
            list(LENGTH _xmrig_cuda_archs _xmrig_nlen)
            if (_xmrig_nlen EQUAL 0)
                set(CMAKE_CUDA_ARCHITECTURES 75 86 89 90 120 CACHE STRING "CUDA architectures" FORCE)
                message(STATUS "CUDA ${CMAKE_CUDA_COMPILER_VERSION}: default CMAKE_CUDA_ARCHITECTURES 75 86 89 90 120 (sm_52–61 unsupported)")
            elseif (NOT _xmrig_olen EQUAL _xmrig_nlen)
                message(STATUS "CUDA ${CMAKE_CUDA_COMPILER_VERSION}: removed sm_52–61 from CMAKE_CUDA_ARCHITECTURES for nvcc compatibility")
                set(CMAKE_CUDA_ARCHITECTURES ${_xmrig_cuda_archs} CACHE STRING "CUDA architectures" FORCE)
            endif()
        endif()
        list(APPEND SOURCES_CRYPTO "${CMAKE_SOURCE_DIR}/src/crypto/verushash/cuda/ccminer_verus_gpu.cu")
        add_definitions(-DXMRIG_FEATURE_CUDA_VERUS)
        set(XMRIG_HAVE_CUDA_VERUS ON)
    endif()
endif()

include(cmake/astrobwt.cmake)
include(cmake/bc3.cmake)
if (WITH_NM AND (XMRIG_ARM OR XMRIG_RISCV))
    message(STATUS "NM (nm/1): x86-only for now — disabled on ARM/RISC-V")
    set(WITH_NM OFF CACHE BOOL "Enable Neuromorph (nm/1) algorithm" FORCE)
endif()
include(cmake/nm.cmake)
include(cmake/OpenSSL.cmake)
include(cmake/asm.cmake)

if (WITH_VERUSHASH)
    add_definitions(/DXMRIG_ALGO_VERUSHASH)

    if (MSVC)
        # x64-Debug defaults to /Od globally — Verus would sit at a few MH/s. Optimize only these files in Debug.
        set(_XMRIG_VERUS_MSVC_OPTS "")
        foreach(_cfg IN ITEMS Release RelWithDebInfo MinSizeRel)
            list(APPEND _XMRIG_VERUS_MSVC_OPTS
                "$<$<CONFIG:${_cfg}>:/fp:fast>"
                "$<$<CONFIG:${_cfg}>:/arch:AVX2>"
                "$<$<CONFIG:${_cfg}>:/Ob3>"
                "$<$<CONFIG:${_cfg}>:/Ot>"
                "$<$<CONFIG:${_cfg}>:/Oi>"
                "$<$<CONFIG:${_cfg}>:/favor:INTEL64>"
                "$<$<CONFIG:${_cfg}>:/GL>"
            )
        endforeach()
        list(APPEND _XMRIG_VERUS_MSVC_OPTS
            "$<$<CONFIG:Debug>:/O2>"
            "$<$<CONFIG:Debug>:/Oi>"
            "$<$<CONFIG:Debug>:/fp:fast>"
            "$<$<CONFIG:Debug>:/arch:AVX2>"
            "$<$<CONFIG:Debug>:/Ob2>"
        )
        # Must be one quoted argument: unquoted ${list} is parsed as extra PROPERTIES pairs.
        set_source_files_properties(
            src/xmrig.cpp
            src/backend/cpu/CpuWorker.cpp
            src/crypto/verushash/VerusHash_xmrig.cpp
            src/crypto/verushash/verus_hash.cpp
            src/crypto/verushash/verus_clhash.cpp
            src/crypto/verushash/verus_clhash_portable.cpp
            src/crypto/verushash/haraka_portable.c
            src/crypto/verushash/haraka.c
            PROPERTIES COMPILE_OPTIONS "${_XMRIG_VERUS_MSVC_OPTS}"
        )
    endif()
endif()

if (WITH_CN_LITE)
    add_definitions(/DXMRIG_ALGO_CN_LITE)
endif()

if (WITH_CN_HEAVY)
    add_definitions(/DXMRIG_ALGO_CN_HEAVY)
endif()

if (WITH_CN_PICO)
    add_definitions(/DXMRIG_ALGO_CN_PICO)
endif()

if (WITH_CN_FEMTO)
    add_definitions(/DXMRIG_ALGO_CN_FEMTO)
endif()

if (WITH_EMBEDDED_CONFIG)
    add_definitions(/DXMRIG_FEATURE_EMBEDDED_CONFIG)
endif()

include(src/hw/api/api.cmake)
include(src/hw/dmi/dmi.cmake)

include_directories(src)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(src/3rdparty)
include_directories(${UV_INCLUDE_DIR})

if (WITH_DEBUG_LOG)
    add_definitions(/DAPP_DEBUG)
endif()

add_executable(${CMAKE_PROJECT_NAME} ${HEADERS} ${SOURCES} ${SOURCES_OS} ${HEADERS_CRYPTO} ${SOURCES_CRYPTO} ${SOURCES_SYSLOG} ${TLS_SOURCES} ${XMRIG_ASM_SOURCES})
# Verus inlines SSSE3/AVX2/PCLMUL across many TUs (CpuWorker, JobResults, OpenCL runners, etc.).
if (WITH_VERUSHASH AND NOT XMRIG_ARM)
    if (CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang|AppleClang)
        target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -mssse3 -mavx2 -mpclmul -maes)
    endif()
endif()
# nvcc host pass may ignore MSVC_RUNTIME_LIBRARY; pin /MD to match the rest of the link (GhostRider, hwloc, CXX).
if (MSVC AND XMRIG_HAVE_CUDA_VERUS)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        "$<$<COMPILE_LANGUAGE:CUDA>:$<$<CONFIG:Debug>:-Xcompiler=/MDd>>"
        "$<$<COMPILE_LANGUAGE:CUDA>:$<$<NOT:$<CONFIG:Debug>>:-Xcompiler=/MD>>")
endif()
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${XMRIG_ASM_LIBRARY} ${OPENSSL_LIBRARIES} ${UV_LIBRARIES} ${EXTRA_LIBS} ${CPUID_LIB} ${ARGON2_LIBRARY} ${ETHASH_LIBRARY} ${GHOSTRIDER_LIBRARY})
if (XMRIG_ASTRO_SPSA_LIBRARY)
    if (TARGET xmrig_astro_spsa)
        add_dependencies(${CMAKE_PROJECT_NAME} xmrig_astro_spsa)
    endif()
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "${XMRIG_ASTRO_SPSA_LIBRARY}")
    if (XMRIG_ASTRO_SPSA_DLL)
        add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${XMRIG_ASTRO_SPSA_DLL}"
                "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>"
            COMMENT "Copy AstroSPSA DLL next to miner"
        )
    endif()
endif()

if (WITH_OPENCL AND WITH_VERUSHASH)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/src/backend/opencl/cl/verus/verushash.cl"
            "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/verushash.cl"
        COMMENT "Copy Verus OpenCL kernel next to miner"
    )
endif()

if (XMRIG_HAVE_CUDA_VERUS)
    find_package(CUDAToolkit REQUIRED)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE CUDA::cudart)
    set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CUDA_SEPARABLE_COMPILATION OFF)
    # Host uses dynamic cudart64_*.dll; do not also link cudart_static (heap corruption on Windows).
    set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CUDA_RUNTIME_LIBRARY Shared)
    if (WIN32)
        set(_xmrig_cuda_bins "")
        if (DEFINED CUDAToolkit_BIN_DIR AND EXISTS "${CUDAToolkit_BIN_DIR}")
            list(APPEND _xmrig_cuda_bins "${CUDAToolkit_BIN_DIR}")
        endif()
        if (DEFINED CUDAToolkit_ROOT_DIR AND EXISTS "${CUDAToolkit_ROOT_DIR}/bin")
            list(APPEND _xmrig_cuda_bins "${CUDAToolkit_ROOT_DIR}/bin")
        endif()
        if (DEFINED ENV{CUDA_PATH} AND EXISTS "$ENV{CUDA_PATH}/bin")
            list(APPEND _xmrig_cuda_bins "$ENV{CUDA_PATH}/bin")
        endif()
        if (DEFINED CUDAToolkit_INCLUDE_DIRS)
            foreach (_inc IN LISTS CUDAToolkit_INCLUDE_DIRS)
                get_filename_component(_maybe_bin "${_inc}/../bin" ABSOLUTE)
                if (EXISTS "${_maybe_bin}")
                    list(APPEND _xmrig_cuda_bins "${_maybe_bin}")
                endif()
            endforeach()
        endif()
        set(_xmrig_cudart_dlls "")
        foreach (_b IN LISTS _xmrig_cuda_bins)
            # CUDA 12+ on Windows often installs cudart64_*.dll under bin\x64, not bin\.
            file(GLOB _g "${_b}/cudart64*.dll")
            list(APPEND _xmrig_cudart_dlls ${_g})
            file(GLOB _g64 "${_b}/x64/cudart64*.dll")
            list(APPEND _xmrig_cudart_dlls ${_g64})
        endforeach()
        if (_xmrig_cudart_dlls)
            list(REMOVE_DUPLICATES _xmrig_cudart_dlls)
        endif()
        foreach (_dll IN LISTS _xmrig_cudart_dlls)
            add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_dll}" $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)
        endforeach()
        if (NOT _xmrig_cudart_dlls)
            message(WARNING "Embedded Verus: no cudart64*.dll found to copy. On CUDA 13+ check toolkit "
                "bin\\x64\\ next to DRQMiner.exe, or the miner may exit immediately on startup.")
        endif()
    endif()
endif()

if (MSVC)
    # VS multi-config normally drops xmrig.exe under .../Release/ or .../Debug/ (easy to miss).
    # Put a clearly named binary in the preset root: out/build/x64-Release/DRQMiner-Release.exe etc.
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_NAME_DEBUG "DRQMiner-Debug"
        RUNTIME_OUTPUT_NAME_RELEASE "DRQMiner-Release"
        RUNTIME_OUTPUT_NAME_RELWITHDEBINFO "DRQMiner-RelWithDebInfo"
        RUNTIME_OUTPUT_NAME_MINSIZEREL "DRQMiner-MinSizeRel"
    )
    # Enable whole-program optimization at the target level in Release.
    set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    # Debug: ensure /RTC1 never remains (incompatible with /O2 on Verus sources — cl D8016).
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE "$<$<CONFIG:Debug>:/RTC->")
    message(STATUS "MSVC output: ${CMAKE_BINARY_DIR}/DRQMiner-<Config>.exe (e.g. DRQMiner-Release.exe)")
else()
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME "drqminer")
    message(STATUS "Unix output: ${CMAKE_BINARY_DIR}/drqminer")
endif()

if (WIN32)
    if (NOT ARM_TARGET)
        add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/bin/WinRing0/WinRing0x64.sys" $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)
    endif()

    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/scripts/benchmark_1M.cmd" $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/scripts/benchmark_10M.cmd" $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/scripts/pool_mine_example.cmd" $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/scripts/solo_mine_example.cmd" $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/scripts/rtm_ghostrider_example.cmd" $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES Clang AND CMAKE_BUILD_TYPE STREQUAL Release AND NOT CMAKE_GENERATOR STREQUAL Xcode)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>")
endif()

if (WIN32 AND MSVC)
    set(_XMRIG_PACKAGE_PS1 "${CMAKE_SOURCE_DIR}/scripts/package_windows_release.ps1")
    if (EXISTS "${_XMRIG_PACKAGE_PS1}")
        add_custom_target(package-windows-release
            COMMAND powershell -NoProfile -ExecutionPolicy Bypass
                -File "${_XMRIG_PACKAGE_PS1}"
                -BuildDir "${CMAKE_BINARY_DIR}"
                -Config Release
            DEPENDS ${CMAKE_PROJECT_NAME}
            COMMENT "Assemble RELEASE folder + DRQMiner-win64.zip"
        )
        add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
            COMMAND powershell -NoProfile -ExecutionPolicy Bypass
                -File "${_XMRIG_PACKAGE_PS1}"
                -BuildDir "${CMAKE_BINARY_DIR}"
                -Config $<CONFIG>
                -Quiet
            COMMENT "Package Windows RELEASE folder (Release config only runs full staging)"
        )
    endif()
endif()
