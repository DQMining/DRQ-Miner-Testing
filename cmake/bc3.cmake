# Phase 2: BitcoinIII (BC3) — SHA3-256t hasher + GBT client skeleton.

if (NOT WITH_BC3)
    return()
endif()

add_definitions(-DXMRIG_ALGO_BC3)

list(APPEND HEADERS
    src/net/bc3/GbtClient.h
)

list(APPEND SOURCES
    src/net/bc3/GbtClient.cpp
)

if (NOT DEFINED XMRIG_BUILD_TESTS)
    set(XMRIG_BUILD_TESTS ON)
endif()

if (NOT XMRIG_BUILD_TESTS)
    return()
endif()

add_executable(test_sha3_256t test_vectors/test_sha3_256t.cpp)
target_sources(test_sha3_256t PRIVATE
    src/base/crypto/sha3.cpp
    src/base/crypto/keccak.cpp
)
target_include_directories(test_sha3_256t PRIVATE src)
set_target_properties(test_sha3_256t PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)
if (MSVC)
    set_target_properties(test_sha3_256t PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
