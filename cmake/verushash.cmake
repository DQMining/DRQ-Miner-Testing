if (WITH_VERUSHASH)
    add_definitions(/DXMRIG_ALGO_VERUSHASH)

    list(APPEND HEADERS_CRYPTO
        src/crypto/verushash/verus_hash.h
        src/crypto/verushash/verus_clhash.h
        src/crypto/verushash/VerusHash_xmrig.h
        src/crypto/verushash/sse2neon.h
        src/crypto/verushash/sha512.h
        src/crypto/verushash/sha256.h
        src/crypto/verushash/sha1.h
        src/crypto/verushash/ripemd160.h
        src/crypto/verushash/hmac_sha512.h
        src/crypto/verushash/hmac_sha256.h
        src/crypto/verushash/haraka_portable.h
        src/crypto/verushash/haraka.h
        src/crypto/verushash/common.h
        src/crypto/verushash/chacha20.h
        src/crypto/verushash/compat/sse2neon.h
    )

    list(APPEND SOURCES_CRYPTO
        src/crypto/verushash/verus_hash.cpp
        src/crypto/verushash/VerusHash_xmrig.cpp
        src/crypto/verushash/verus_clhash_portable.cpp
        src/crypto/verushash/verus_clhash.cpp
        src/crypto/verushash/sha512.cpp
        src/crypto/verushash/sha256.cpp
        src/crypto/verushash/sha1.cpp
        src/crypto/verushash/ripemd160.cpp
        src/crypto/verushash/hmac_sha512.cpp
        src/crypto/verushash/hmac_sha256.cpp
        src/crypto/verushash/uint256.cpp
        src/crypto/verushash/chacha20.cpp
        src/crypto/verushash/haraka_portable.c
        src/crypto/verushash/haraka.c
        src/crypto/verushash/cuda/VerusCudaBridge.cpp
    )

    if (CMAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
        if (NOT XMRIG_ARM)
            set_source_files_properties(
                src/crypto/verushash/verus_clhash.cpp
                src/crypto/verushash/verus_clhash_portable.cpp
                PROPERTIES COMPILE_FLAGS "-mavx2 -O3"
            )
        endif()
    endif()

    if (NOT DEFINED XMRIG_BUILD_TESTS)
        set(XMRIG_BUILD_TESTS ON)
    endif()

    if (NOT XMRIG_BUILD_TESTS)
        return()
    endif()

    add_executable(test_verushash
        test_vectors/test_verushash.cpp
        src/crypto/verushash/VerusHash_xmrig.cpp
        src/crypto/verushash/verus_hash.cpp
        src/crypto/verushash/verus_clhash_portable.cpp
        src/crypto/verushash/verus_clhash.cpp
        src/crypto/verushash/sha512.cpp
        src/crypto/verushash/sha256.cpp
        src/crypto/verushash/sha1.cpp
        src/crypto/verushash/ripemd160.cpp
        src/crypto/verushash/hmac_sha512.cpp
        src/crypto/verushash/hmac_sha256.cpp
        src/crypto/verushash/uint256.cpp
        src/crypto/verushash/chacha20.cpp
        src/crypto/verushash/haraka_portable.c
        src/crypto/verushash/haraka.c
    )
    target_include_directories(test_verushash PRIVATE src)
    target_compile_definitions(test_verushash PRIVATE XMRIG_ALGO_VERUSHASH)
    if (MSVC)
        target_compile_options(test_verushash PRIVATE /arch:AVX2 /O2)
        set_target_properties(test_verushash PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
        set_source_files_properties(
            test_vectors/test_verushash.cpp
            src/crypto/verushash/VerusHash_xmrig.cpp
            src/crypto/verushash/verus_hash.cpp
            src/crypto/verushash/verus_clhash.cpp
            src/crypto/verushash/verus_clhash_portable.cpp
            src/crypto/verushash/haraka_portable.c
            src/crypto/verushash/haraka.c
            PROPERTIES COMPILE_OPTIONS "$<$<CONFIG:Release>:/arch:AVX2>;$<$<CONFIG:RelWithDebInfo>:/arch:AVX2>"
        )
    elseif (CMAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
        set_source_files_properties(
            test_vectors/test_verushash.cpp
            src/crypto/verushash/verus_clhash.cpp
            src/crypto/verushash/verus_clhash_portable.cpp
            PROPERTIES COMPILE_FLAGS "-mavx2 -O3"
        )
    endif()
else()
    remove_definitions(/DXMRIG_ALGO_VERUSHASH)
endif()
