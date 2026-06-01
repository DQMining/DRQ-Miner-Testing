if (WITH_TLS)
    if (NOT OPENSSL_ROOT_DIR)
        set(OPENSSL_ROOT_DIR ${XMRIG_DEPS})
    endif()

    if (WIN32)
        if (MSVC)
            set(OPENSSL_USE_STATIC_LIBS TRUE)
            # FALSE = link against /MD OpenSSL (lib/VC/x64/MD); TRUE = /MT — must match CMAKE_MSVC_RUNTIME_LIBRARY
            set(OPENSSL_MSVC_STATIC_RT FALSE)
        endif()

        set(EXTRA_LIBS ${EXTRA_LIBS} crypt32)
    elseif (APPLE)
        set(OPENSSL_USE_STATIC_LIBS TRUE)
    endif()

    if (BUILD_STATIC)
        set(OPENSSL_USE_STATIC_LIBS TRUE)
    endif()


    find_package(OpenSSL)

    if (OPENSSL_FOUND)
        set(TLS_SOURCES
            src/base/net/stratum/Tls.cpp
            src/base/net/stratum/Tls.h
            src/base/net/tls/ServerTls.cpp
            src/base/net/tls/ServerTls.h
            src/base/net/tls/TlsConfig.cpp
            src/base/net/tls/TlsConfig.h
            src/base/net/tls/TlsContext.cpp
            src/base/net/tls/TlsContext.h
            src/base/net/tls/TlsGen.cpp
            src/base/net/tls/TlsGen.h
            )

        include_directories(${OPENSSL_INCLUDE_DIR})

        if (WITH_HTTP)
            set(TLS_SOURCES ${TLS_SOURCES}
                src/base/net/https/HttpsClient.cpp
                src/base/net/https/HttpsClient.h
                src/base/net/https/HttpsContext.cpp
                src/base/net/https/HttpsContext.h
                src/base/net/https/HttpsServer.cpp
                src/base/net/https/HttpsServer.h
                )
        endif()
    else()
        message(FATAL_ERROR "OpenSSL NOT found: use `-DWITH_TLS=OFF` to build without TLS support")
    endif()

    add_definitions(/DXMRIG_FEATURE_TLS)
else()
    set(TLS_SOURCES "")
    set(OPENSSL_LIBRARIES "")
    remove_definitions(/DXMRIG_FEATURE_TLS)

    if (WITH_HTTP)
        set(TLS_SOURCES ${TLS_SOURCES}
            src/base/net/http/HttpServer.cpp
            src/base/net/http/HttpServer.h
            )
    endif()

    set(CMAKE_PROJECT_NAME "${CMAKE_PROJECT_NAME}-notls")
endif()
