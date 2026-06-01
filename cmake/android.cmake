# Android defaults (NDK cross-build). Included when CMAKE_SYSTEM_NAME is Android.

if (NOT ANDROID AND NOT CMAKE_SYSTEM_NAME MATCHES "Android")
    return()
endif()

message(STATUS "Android build: trimming desktop-only backends")

set(WITH_CUDA OFF CACHE BOOL "CUDA is not available on Android" FORCE)
set(WITH_NVML OFF CACHE BOOL "" FORCE)
set(WITH_MSR OFF CACHE BOOL "" FORCE)
set(WITH_DMI OFF CACHE BOOL "" FORCE)
set(WITH_ADL OFF CACHE BOOL "" FORCE)
set(WITH_GHOSTRIDER OFF CACHE BOOL "GhostRider is not built for Android in DRQ Miner" FORCE)
set(WITH_KAWPOW OFF CACHE BOOL "" FORCE)
set(WITH_SSE4_1 OFF CACHE BOOL "" FORCE)
set(WITH_AVX2 OFF CACHE BOOL "" FORCE)
set(WITH_VAES OFF CACHE BOOL "" FORCE)

# Verus + OpenCL (Adreno/Mali) + CPU remain ON via top-level options.
