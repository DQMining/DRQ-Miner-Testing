if (WITH_ASTROBWT)
    include(cmake/astro_spsa.cmake)
    add_definitions(/DXMRIG_ALGO_ASTROBWT)
    # Uncomment to A/B test SA-IS vs divsufsort (must pass test_astrobwt):
    # add_definitions(/DXMRIG_ASTROBWT_USE_SAIS)

    list(APPEND HEADERS_CRYPTO
        src/crypto/astrobwt/AstroBWT.h
        src/crypto/astrobwt/sha256_utils.h
        src/crypto/astrobwt/hash_utils.h
        src/crypto/astrobwt/sais.h
        src/crypto/astrobwt/Salsa20.hpp
        src/crypto/astrobwt/divsufsort.h
        src/crypto/astrobwt/divsufsort_private.h
    )

    list(APPEND SOURCES_CRYPTO
        src/crypto/astrobwt/AstroBWT.cpp
        src/crypto/astrobwt/Salsa20.cpp
        src/crypto/astrobwt/sha256_utils.cpp
        src/crypto/astrobwt/wolf/wolf_tables.cpp
        src/crypto/astrobwt/wolf/wolf_worker.cpp
        src/crypto/astrobwt/wolf/wolf_compute.cpp
        src/crypto/astrobwt/wolf/wolf_stub.cpp
        src/crypto/astrobwt/astrobwt_suffix.cpp
        src/crypto/astrobwt/divsufsort.c
        src/crypto/astrobwt/sssort.c
        src/crypto/astrobwt/trsort.c
        src/crypto/astrobwt/divsufsort_utils.c
        src/crypto/astrobwt/spsa/spsa_finalize.cpp
    )

    if (XMRIG_ASTRO_SPSA_BRIDGE_SOURCES)
        list(APPEND SOURCES_CRYPTO ${XMRIG_ASTRO_SPSA_BRIDGE_SOURCES})
        if (XMRIG_ASTRO_SPSA_INCLUDE_DIR)
            include_directories(BEFORE "${XMRIG_ASTRO_SPSA_INCLUDE_DIR}")
        endif()
        set_source_files_properties(${XMRIG_ASTRO_SPSA_BRIDGE_SOURCES} PROPERTIES
            COMPILE_DEFINITIONS "USE_ASTRO_SPSA"
        )
        add_definitions(-DXMRIG_ASTROBWT_USE_SPSA)
    endif()

    if (XMRIG_ARM AND XMRIG_ASTRO_SPSA_ENABLED)
        set(XMRIG_ASTRO_AARCH64_ENABLED ON)
        add_definitions(-DXMRIG_ASTRO_AARCH64_ENABLED)
        include_directories(BEFORE "${CMAKE_SOURCE_DIR}/lib/tnn_compat")
        list(APPEND SOURCES_CRYPTO
            src/crypto/astrobwt/aarch64/astro_aarch64.cpp
            src/crypto/astrobwt/aarch64/astrobwt_aarch64.cpp
        )
        set_source_files_properties(
            src/crypto/astrobwt/aarch64/astro_aarch64.cpp
            src/crypto/astrobwt/aarch64/astrobwt_aarch64.cpp
            PROPERTIES COMPILE_DEFINITIONS "USE_ASTRO_SPSA"
        )
        if (CMAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
            set_source_files_properties(
                src/crypto/astrobwt/aarch64/astro_aarch64.cpp
                PROPERTIES COMPILE_FLAGS "-O3 -flax-vector-conversions"
            )
        endif()
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
        if (XMRIG_ARM_CRYPTO)
            set_source_files_properties(
                src/crypto/astrobwt/sha256_utils.cpp
                src/crypto/astrobwt/sha256_arm_crypto.cpp
                PROPERTIES COMPILE_FLAGS "${ARM8_CXX_FLAGS}")
        else()
            set_source_files_properties(src/crypto/astrobwt/sha256_utils.cpp PROPERTIES COMPILE_FLAGS "-msha -msse4.1 -mssse3")
        endif()
        if (NOT XMRIG_ARM)
            set_source_files_properties(
                src/crypto/astrobwt/AstroBWT.cpp
                src/crypto/astrobwt/wolf/wolf_compute.cpp
                src/crypto/astrobwt/wolf/wolf_tables.cpp
                PROPERTIES COMPILE_FLAGS "-O3 -funroll-loops -mavx2"
            )
        else()
            set_source_files_properties(
                src/crypto/astrobwt/AstroBWT.cpp
                src/crypto/astrobwt/wolf/wolf_compute.cpp
                src/crypto/astrobwt/wolf/wolf_tables.cpp
                PROPERTIES COMPILE_FLAGS "-O3"
            )
        endif()
    elseif (MSVC)
        set(_XMRIG_ASTROBWT_MSVC_OPTS "")
        foreach(_cfg IN ITEMS Release RelWithDebInfo MinSizeRel)
            list(APPEND _XMRIG_ASTROBWT_MSVC_OPTS
                "$<$<CONFIG:${_cfg}>:/O2>"
                "$<$<CONFIG:${_cfg}>:/Ob3>"
                "$<$<CONFIG:${_cfg}>:/Oi>"
                "$<$<CONFIG:${_cfg}>:/Ot>"
                "$<$<CONFIG:${_cfg}>:/fp:fast>"
                "$<$<CONFIG:${_cfg}>:/arch:AVX2>"
            )
        endforeach()
        set_source_files_properties(
            src/crypto/astrobwt/AstroBWT.cpp
            src/crypto/astrobwt/sha256_utils.cpp
            src/crypto/astrobwt/wolf/wolf_compute.cpp
            src/crypto/astrobwt/wolf/wolf_tables.cpp
            PROPERTIES COMPILE_OPTIONS "${_XMRIG_ASTROBWT_MSVC_OPTS}"
        )
    endif()

    if (XMRIG_ARM_CRYPTO)
        list(APPEND SOURCES_CRYPTO src/crypto/astrobwt/sha256_arm_crypto.cpp)
    endif()

    if (NOT DEFINED XMRIG_BUILD_TESTS)
        set(XMRIG_BUILD_TESTS ON)
    endif()

    if (NOT XMRIG_BUILD_TESTS)
        return()
    endif()

    add_executable(test_astrobwt
        test_vectors/test_astrobwt.cpp
        src/crypto/astrobwt/AstroBWT.cpp
        src/crypto/astrobwt/Salsa20.cpp
        src/crypto/astrobwt/sha256_utils.cpp
        src/crypto/astrobwt/wolf/wolf_tables.cpp
        src/crypto/astrobwt/wolf/wolf_worker.cpp
        src/crypto/astrobwt/wolf/wolf_compute.cpp
        src/crypto/astrobwt/wolf/wolf_stub.cpp
        src/crypto/astrobwt/astrobwt_suffix.cpp
        src/crypto/astrobwt/spsa/spsa_finalize.cpp
        src/crypto/astrobwt/divsufsort.c
        src/crypto/astrobwt/sssort.c
        src/crypto/astrobwt/trsort.c
        src/crypto/astrobwt/divsufsort_utils.c
    )
    if (XMRIG_ASTRO_SPSA_BRIDGE_SOURCES)
        target_sources(test_astrobwt PRIVATE ${XMRIG_ASTRO_SPSA_BRIDGE_SOURCES})
        if (XMRIG_ASTRO_SPSA_INCLUDE_DIR)
            target_include_directories(test_astrobwt BEFORE PRIVATE "${XMRIG_ASTRO_SPSA_INCLUDE_DIR}")
        endif()
        set_source_files_properties(${XMRIG_ASTRO_SPSA_BRIDGE_SOURCES} PROPERTIES
            COMPILE_DEFINITIONS "USE_ASTRO_SPSA"
        )
    endif()
    if (XMRIG_ASTRO_SPSA_LIBRARY)
        if (TARGET xmrig_astro_spsa)
            add_dependencies(test_astrobwt xmrig_astro_spsa)
        endif()
        target_link_libraries(test_astrobwt PRIVATE "${XMRIG_ASTRO_SPSA_LIBRARY}")
    endif()
    target_include_directories(test_astrobwt PRIVATE src)
    set_target_properties(test_astrobwt PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)
    target_compile_definitions(test_astrobwt PRIVATE XMRIG_ALGO_ASTROBWT)
    if (XMRIG_ASTRO_SPSA_ENABLED)
        target_compile_definitions(test_astrobwt PRIVATE XMRIG_ASTROBWT_USE_SPSA)
    endif()
    if (MSVC)
        target_compile_options(test_astrobwt PRIVATE /arch:AVX2 /O2)
        set_source_files_properties(
            src/crypto/astrobwt/wolf/wolf_compute.cpp
            src/crypto/astrobwt/wolf/wolf_tables.cpp
            PROPERTIES COMPILE_OPTIONS "$<$<CONFIG:Release>:/arch:AVX2>;$<$<CONFIG:RelWithDebInfo>:/arch:AVX2>"
        )
        set_target_properties(test_astrobwt PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
        set_source_files_properties(
            src/crypto/astrobwt/wolf/wolf_compute.cpp
            src/crypto/astrobwt/wolf/wolf_tables.cpp
            PROPERTIES COMPILE_FLAGS "-mavx2 -O3"
        )
    endif()
else()
    remove_definitions(/DXMRIG_ALGO_ASTROBWT)
endif()
