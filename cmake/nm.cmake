if (WITH_NM)
    add_definitions(/DXMRIG_ALGO_NM)

    list(APPEND HEADERS_CRYPTO
        src/crypto/nm/nm_aes.h
        src/crypto/nm/nm_sha256.h
        src/crypto/nm/nm_params.h
        src/crypto/nm/nm_neuromorph.h
        src/crypto/nm/nm_fast.h
        src/crypto/nm/nm_fast_fill.h
        src/crypto/nm/NmShared.h
    )

    set(SOURCES_NM
        src/crypto/nm/nm_params.c
        src/crypto/nm/nm_fast.c
        src/crypto/nm/nm_sha256.c
        src/crypto/nm/NmShared.cpp
    )

    if (MSVC)
        find_program(XMRIG_CLANG_CL NAMES clang-cl
            PATHS
                "C:/Program Files/LLVM/bin"
                "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/bin"
                "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin"
        )
        if (XMRIG_CLANG_CL)
            set(_NM_VAES256_OBJ "${CMAKE_BINARY_DIR}/nm_fast_vaes256.obj")
            add_custom_command(
                OUTPUT "${_NM_VAES256_OBJ}"
                COMMAND "${XMRIG_CLANG_CL}" /nologo /O2 /arch:AVX2 /clang:-mvaes /fp:precise
                    /I "${CMAKE_SOURCE_DIR}/src"
                    /c "${CMAKE_SOURCE_DIR}/src/crypto/nm/nm_fast_vaes256.c"
                    /Fo"${_NM_VAES256_OBJ}"
                DEPENDS
                    "${CMAKE_SOURCE_DIR}/src/crypto/nm/nm_fast_vaes256.c"
                    "${CMAKE_SOURCE_DIR}/src/crypto/nm/nm_fast_fill.h"
                    "${CMAKE_SOURCE_DIR}/src/crypto/nm/nm_fast.h"
                COMMENT "Building nm_fast VAES-256 (clang-cl -mvaes)"
            )
            list(APPEND SOURCES_NM "${_NM_VAES256_OBJ}")
            message(STATUS "NM VAES-256: enabled via clang-cl (${XMRIG_CLANG_CL})")
        else()
            message(STATUS "NM VAES-256: clang-cl not found, AES-NI fill only on MSVC")
        endif()
    endif()

    list(APPEND SOURCES_CRYPTO ${SOURCES_NM})

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set_source_files_properties(${SOURCES_NM} PROPERTIES
            COMPILE_FLAGS "-O3 -maes -fno-fast-math -ffp-contract=off")
    endif()

    if (MSVC)
        set(_XMRIG_NM_MSVC_OPTS "")
        foreach(_cfg IN ITEMS Release RelWithDebInfo MinSizeRel)
            list(APPEND _XMRIG_NM_MSVC_OPTS
                "$<$<CONFIG:${_cfg}>:/O2>"
                "$<$<CONFIG:${_cfg}>:/Ob3>"
                "$<$<CONFIG:${_cfg}>:/Ot>"
                "$<$<CONFIG:${_cfg}>:/Oi>"
                "$<$<CONFIG:${_cfg}>:/GL>"
                "$<$<CONFIG:${_cfg}>:/fp:precise>"
                "$<$<CONFIG:${_cfg}>:/arch:AVX2>"
                "$<$<CONFIG:${_cfg}>:/favor:INTEL64>"
            )
        endforeach()
        set_source_files_properties(
            src/crypto/nm/nm_params.c
            src/crypto/nm/nm_fast.c
            src/crypto/nm/nm_sha256.c
            PROPERTIES COMPILE_OPTIONS "${_XMRIG_NM_MSVC_OPTS}"
        )
    endif()
else()
    remove_definitions(/DXMRIG_ALGO_NM)
endif()

if (WITH_NM AND XMRIG_BUILD_TESTS)
    add_executable(test_neuromorph test_vectors/test_neuromorph.cpp)
    target_sources(test_neuromorph PRIVATE
        src/crypto/nm/nm_params.c
        src/crypto/nm/nm_neuromorph.c
        src/crypto/nm/nm_sha256.c
        src/crypto/nm/NmShared.cpp
        src/crypto/nm/nm_fast.c
        src/base/crypto/keccak.cpp
    )
    target_include_directories(test_neuromorph PRIVATE src)
    target_compile_definitions(test_neuromorph PRIVATE XMRIG_ALGO_NM)
    set_target_properties(test_neuromorph PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)
    if (MSVC)
        set_target_properties(test_neuromorph PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    endif()
endif()
