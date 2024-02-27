cmake_minimum_required(VERSION 3.12)

cmake_policy(SET CMP0114 NEW)
cmake_policy(SET CMP0135 NEW)

project(porla)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ExternalAntlr4Cpp)
include(ExternalProject)
include(FetchContent)

# cmake options
set(CMAKE_CXX_STANDARD 20)

# common options
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)
set(BUILD_STATIC_LIBS ON  CACHE BOOL "Build static libs" FORCE)

# boost options
set(Boost_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/boost-1.84-install)

# curl opts
set(BUILD_LIBCURL_DOCS OFF CACHE BOOL "Build curl docs" FORCE)

# libzip optons
set(BUILD_REGRESS OFF CACHE BOOL "Build regression tests" FORCE)

# sodium opts
set(VERSION 1.0.19)
set(SODIUM_LIBRARY_VERSION_MAJOR 10)
set(SODIUM_LIBRARY_VERSION_MINOR 3)

# uriparser options
set(URIPARSER_BUILD_DOCS OFF CACHE BOOL "Build docs" FORCE)
set(URIPARSER_BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)
set(URIPARSER_BUILD_TOOLS OFF CACHE BOOL "Build tools" FORCE)
set(URIPARSER_BUILD_WCHAR_T OFF CACHE BOOL "Build wchar_t" FORCE)
set(URIPARSER_SHARED_LIBS OFF CACHE BOOL "Shared libs" FORCE)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)

FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    curl
    URL https://github.com/curl/curl/releases/download/curl-8_6_0/curl-8.6.0.tar.xz
)

FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp
    GIT_TAG v0.7.0
)

FetchContent_Declare(
    libsodium
    URL https://github.com/jedisct1/libsodium/releases/download/1.0.19-RELEASE/libsodium-1.0.19.tar.gz
)

FetchContent_Declare(
    libtorrent
    URL https://github.com/arvidn/libtorrent/releases/download/v2.0.10/libtorrent-rasterbar-2.0.10.tar.gz
)

FetchContent_Declare(
    libzip
    URL https://github.com/nih-at/libzip/releases/download/v1.10.1/libzip-1.10.1.tar.xz
)

FetchContent_Declare(
    lua
    URL https://github.com/lua/lua/archive/refs/tags/v5.4.6.tar.gz
)

FetchContent_Declare(
    sqlite3
    URL "https://sqlite.org/2024/sqlite-amalgamation-3450100.zip"
)

FetchContent_Declare(
    tomlplusplus
    URL https://github.com/marzer/tomlplusplus/archive/refs/tags/v3.4.0.tar.gz
)

FetchContent_Declare(
    uriparser
    URL https://github.com/uriparser/uriparser/releases/download/uriparser-0.9.7/uriparser-0.9.7.tar.xz
)

FetchContent_MakeAvailable(
    curl
    jwt-cpp
    libtorrent
    libzip
    tomlplusplus
    uriparser
)

find_package(Boost REQUIRED COMPONENTS log program_options system)

if (NOT libsodium_POPULATED)
  FetchContent_Populate(libsodium)
endif()

if (NOT lua_POPULATED)
  FetchContent_Populate(lua)
endif()

if (NOT sqlite3_POPULATED)
  FetchContent_Populate(sqlite3)
endif()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/_version.cpp
    COMMAND ${CMAKE_COMMAND} -P
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateVersion.cmake)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/webui.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/_webui.cpp
    COMMAND ${CMAKE_COMMAND} -P
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FileEmbed.cmake)

add_library(
    uSockets
    STATIC
    vendor/uWebSockets/uSockets/src/crypto/openssl.c
    vendor/uWebSockets/uSockets/src/eventing/asio.cpp
    vendor/uWebSockets/uSockets/src/bsd.c
    vendor/uWebSockets/uSockets/src/context.c
    vendor/uWebSockets/uSockets/src/loop.c
    vendor/uWebSockets/uSockets/src/quic.c
    vendor/uWebSockets/uSockets/src/socket.c
    vendor/uWebSockets/uSockets/src/udp.c
)

target_compile_definitions(
    uSockets
    PRIVATE
    -DLIBUS_NO_SSL
    -DLIBUS_USE_ASIO
)

target_include_directories(
    uSockets
    PUBLIC
    vendor/uWebSockets/uSockets/src
)

target_link_libraries(
    uSockets
    PRIVATE
    Boost::boost
)

add_library(
    lua
    STATIC
    ${lua_SOURCE_DIR}/lapi.c
    ${lua_SOURCE_DIR}/lauxlib.c
    ${lua_SOURCE_DIR}/lbaselib.c
    ${lua_SOURCE_DIR}/lcode.c
    ${lua_SOURCE_DIR}/lcorolib.c
    ${lua_SOURCE_DIR}/lctype.c
    ${lua_SOURCE_DIR}/ldblib.c
    ${lua_SOURCE_DIR}/ldebug.c
    ${lua_SOURCE_DIR}/ldo.c
    ${lua_SOURCE_DIR}/ldump.c
    ${lua_SOURCE_DIR}/lfunc.c
    ${lua_SOURCE_DIR}/lgc.c
    ${lua_SOURCE_DIR}/liolib.c
    ${lua_SOURCE_DIR}/llex.c
    ${lua_SOURCE_DIR}/loadlib.c
    ${lua_SOURCE_DIR}/lmathlib.c
    ${lua_SOURCE_DIR}/lmem.c
    ${lua_SOURCE_DIR}/lobject.c
    ${lua_SOURCE_DIR}/lopcodes.c
    ${lua_SOURCE_DIR}/loslib.c
    ${lua_SOURCE_DIR}/lparser.c
    ${lua_SOURCE_DIR}/lstate.c
    ${lua_SOURCE_DIR}/lstring.c
    ${lua_SOURCE_DIR}/lstrlib.c
    ${lua_SOURCE_DIR}/ltable.c
    ${lua_SOURCE_DIR}/ltablib.c
    ${lua_SOURCE_DIR}/ltm.c
    ${lua_SOURCE_DIR}/lundump.c
    ${lua_SOURCE_DIR}/lutf8lib.c
    ${lua_SOURCE_DIR}/lvm.c
    ${lua_SOURCE_DIR}/lzio.c
    ${lua_SOURCE_DIR}/ltests.c
)

target_include_directories(
    lua
    PUBLIC ${lua_SOURCE_DIR}
)

add_library(
    sodium
    STATIC
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_aead/aegis128l/aead_aegis128l.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_aead/aegis128l/aegis128l_aesni.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_aead/aegis128l/aegis128l_armcrypto.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_aead/aegis128l/aegis128l_soft.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_aead/aegis256/aead_aegis256.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_aead/aegis256/aegis256_aesni.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_aead/aegis256/aegis256_armcrypto.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_aead/aegis256/aegis256_soft.c

    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_core/ed25519/ref10/ed25519_ref10.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_core/ed25519/core_ed25519.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_core/salsa/ref/core_salsa_ref.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_core/softaes/softaes.c

    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/ref/blake2b-compress-avx2.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/ref/blake2b-compress-ref.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/ref/blake2b-compress-sse41.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/ref/blake2b-compress-ssse3.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/ref/blake2b-ref.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/ref/generichash_blake2b.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_generichash/blake2b/generichash_blake2.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_generichash/crypto_generichash.c

    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_onetimeauth/poly1305/donna/poly1305_donna.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_onetimeauth/poly1305/sse2/poly1305_sse2.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_onetimeauth/poly1305/onetimeauth_poly1305.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_onetimeauth/crypto_onetimeauth.c

    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-core.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-encoding.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-avx2.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-avx512f.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-ref.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-ssse3.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/argon2.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/blake2b-long.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/pwhash_argon2i.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_pwhash/argon2/pwhash_argon2id.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_pwhash/crypto_pwhash.c

    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_scalarmult/curve25519/ref10/x25519_ref10.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_scalarmult/curve25519/scalarmult_curve25519.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_scalarmult/crypto_scalarmult.c

    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_stream/chacha20/dolbeau/chacha20_dolbeau-avx2.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_stream/chacha20/dolbeau/chacha20_dolbeau-ssse3.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_stream/chacha20/ref/chacha20_ref.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_stream/chacha20/stream_chacha20.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_stream/salsa20/ref/salsa20_ref.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_stream/salsa20/stream_salsa20.c
    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_stream/crypto_stream.c

    ${libsodium_SOURCE_DIR}/src/libsodium/crypto_verify/verify.c

    ${libsodium_SOURCE_DIR}/src/libsodium/randombytes/internal/randombytes_internal_random.c
    ${libsodium_SOURCE_DIR}/src/libsodium/randombytes/sysrandom/randombytes_sysrandom.c
    ${libsodium_SOURCE_DIR}/src/libsodium/randombytes/randombytes.c

    ${libsodium_SOURCE_DIR}/src/libsodium/sodium/codecs.c
    ${libsodium_SOURCE_DIR}/src/libsodium/sodium/core.c
    ${libsodium_SOURCE_DIR}/src/libsodium/sodium/runtime.c
    ${libsodium_SOURCE_DIR}/src/libsodium/sodium/utils.c
    ${libsodium_SOURCE_DIR}/src/libsodium/sodium/version.c
)

configure_file(
    ${libsodium_SOURCE_DIR}/src/libsodium/include/sodium/version.h.in
    ${libsodium_SOURCE_DIR}/src/libsodium/include/sodium/version.h
)

target_include_directories(
    sodium
    PUBLIC ${libsodium_SOURCE_DIR}/src/libsodium/include
    PRIVATE ${libsodium_SOURCE_DIR}/src/libsodium/include/sodium
)

target_include_directories(
    sodium
    INTERFACE
    ${libsodium_SOURCE_DIR}/src/include
)

add_library(
    ${PROJECT_NAME}_utils
    src/utils/ratio.cpp
)

target_link_libraries(
    ${PROJECT_NAME}_utils
    PRIVATE
    torrent-rasterbar
)

add_library(
    ${PROJECT_NAME}_query
    STATIC
    src/query/_aux/PorlaQueryLangBaseVisitor.cpp
    src/query/_aux/PorlaQueryLangLexer.cpp
    src/query/_aux/PorlaQueryLangParser.cpp
    src/query/_aux/PorlaQueryLangVisitor.cpp
    src/query/pql.cpp
)

target_include_directories(
    ${PROJECT_NAME}_query
    PRIVATE
    ${ANTLR4_INCLUDE_DIRS}
)

target_link_libraries(
    ${PROJECT_NAME}_query
    PRIVATE
    ${PROJECT_NAME}_utils
    antlr4_static
    nlohmann_json::nlohmann_json
    torrent-rasterbar
)

add_library(
    ${PROJECT_NAME}_core
    STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/webui.cpp

    src/buildinfo.cpp
    src/cmdargs.cpp
    src/config.cpp
    src/logger.cpp
    src/sessions.cpp
    src/uri.cpp
    src/utils/eta.cpp
    src/utils/secretkey.cpp
    src/utils/string.cpp

    src/data/migrate.cpp
    src/data/migrations/0001_initialsetup.cpp
    src/data/migrations/0002_addsessionsettings.cpp
    src/data/migrations/0003_users.cpp
    src/data/migrations/0004_removesessionparams.cpp
    src/data/migrations/0005_metadata.cpp
    src/data/migrations/0006_clientdata.cpp
    src/data/migrations/0007_removesessionsettings.cpp
    src/data/migrations/0008_plugins.cpp
    src/data/migrations/0009_multisessions.cpp
    src/data/models/addtorrentparams.cpp
    src/data/models/users.cpp
    src/data/statement.cpp

    src/http/authinithandler.cpp
    src/http/authloginhandler.cpp
    src/http/eventshandler.cpp
    src/http/jsonrpchandler.cpp
    src/http/jwthandler.cpp
    src/http/metricshandler.cpp
    src/http/systemhandler.cpp
    src/http/webuihandler.cpp

    src/lua/packages/config.cpp
    src/lua/packages/cron.cpp
    src/lua/packages/events.cpp
    src/lua/packages/filesystem.cpp
    src/lua/packages/httpclient.cpp
    src/lua/packages/json.cpp
    src/lua/packages/log.cpp
    src/lua/packages/pql.cpp
    src/lua/packages/presets.cpp
    src/lua/packages/process.cpp
    src/lua/packages/sessions.cpp
    src/lua/packages/sqlite.cpp
    src/lua/packages/timers.cpp
    src/lua/packages/torrents.cpp
    src/lua/packages/workflows.cpp
    src/lua/plugin.cpp
    src/lua/pluginengine.cpp

    src/methods/fsspace.cpp
    # src/methods/plugins/pluginsconfigure.cpp
    # src/methods/plugins/pluginsget.cpp
    # src/methods/plugins/pluginsinstall.cpp
    # src/methods/plugins/pluginslist.cpp
    # src/methods/plugins/pluginsreload.cpp
    # src/methods/plugins/pluginsuninstall.cpp
    # src/methods/plugins/pluginsupdate.cpp
    src/methods/sessions/sessionslist.cpp
    src/methods/sessions/sessionspause.cpp
    src/methods/sessions/sessionsresume.cpp
    src/methods/sessions/sessionssettingslist.cpp
    src/methods/presetslist.cpp
    src/methods/sysversions.cpp
    src/methods/torrentsadd.cpp
    src/methods/torrentsfileslist.cpp
    src/methods/torrentslist.cpp
    src/methods/torrentsmetadatalist.cpp
    src/methods/torrentsmove.cpp
    src/methods/torrentspause.cpp
    src/methods/torrentspeersadd.cpp
    src/methods/torrentspeerslist.cpp
    src/methods/torrentspropertiesget.cpp
    src/methods/torrentsrecheck.cpp
    src/methods/torrentsremove.cpp
    src/methods/torrentsresume.cpp
    src/methods/torrentspropertiesset.cpp
    src/methods/torrentstrackerslist.cpp

    src/tools/authtoken.cpp
    src/tools/generatesecretkey.cpp
    src/tools/versionjson.cpp

    ${sqlite3_SOURCE_DIR}/sqlite3.c
)

target_compile_definitions(
    ${PROJECT_NAME}_core
    PUBLIC
    -DSQLITE_CORE
)

target_include_directories(
    ${PROJECT_NAME}_core
    PUBLIC
    ${JWT_CPP_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIR}
    vendor/croncpp/include
    vendor/sol2/include
)

target_link_libraries(
    ${PROJECT_NAME}_core

    ${PROJECT_NAME}_query
    ${PROJECT_NAME}_utils

    Boost::boost Boost::log Boost::program_options
    CURL::libcurl
    jwt-cpp::jwt-cpp
    libzip::zip
    lua
    nlohmann_json::nlohmann_json
    sodium
    torrent-rasterbar
    tomlplusplus::tomlplusplus
    uriparser
    uSockets
)

add_executable(
    ${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    ${PROJECT_NAME}_core
    uSockets
)

