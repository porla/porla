cmake_minimum_required(VERSION 3.12)

cmake_policy(SET CMP0114 NEW)
cmake_policy(SET CMP0135 NEW)

project(porla)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)

include(ExternalAntlr4Cpp)
include(FetchContent)

set(BUILD_TESTS                 OFF CACHE BOOL "Build tests" FORCE)
set(BUILD_TOOLS         OFF CACHE BOOL "Build tools" FORCE)
set(BUILD_REGRESS       OFF CACHE BOOL "Build regression tests" FORCE)
set(BUILD_OSSFUZZ       OFF CACHE BOOL "Build fuzzers for ossfuzz" FORCE)
set(BUILD_EXAMPLES      OFF CACHE BOOL "Build examples" FORCE)
set(BUILD_DOC           OFF CACHE BOOL "Build docs" FORCE)
set(ENABLE_BZIP2        OFF)
set(ENABLE_ZSTD         OFF)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib .so .dll)

# uriparser options
set(URIPARSER_BUILD_DOCS    OFF CACHE BOOL "Build docs"    FORCE)
set(URIPARSER_BUILD_TESTS   OFF CACHE BOOL "Build tests"   FORCE)
set(URIPARSER_BUILD_TOOLS   OFF CACHE BOOL "Build tools"   FORCE)
set(URIPARSER_BUILD_WCHAR_T OFF CACHE BOOL "Build wchar_t" FORCE)

find_package(Boost               REQUIRED COMPONENTS log program_options system)
find_package(LibtorrentRasterbar REQUIRED)
find_package(Lua                 REQUIRED)
find_package(OpenSSL             REQUIRED)
find_package(SQLite3             REQUIRED)
find_package(ZLIB                REQUIRED)

FetchContent_Declare(curl          URL https://github.com/curl/curl/archive/refs/tags/curl-8_6_0.tar.gz)
FetchContent_Declare(jwt-cpp       URL https://github.com/Thalhammer/jwt-cpp/archive/refs/tags/v0.7.0.tar.gz)
FetchContent_Declare(libzip        URL https://github.com/nih-at/libzip/archive/refs/tags/v1.10.1.tar.gz)
FetchContent_Declare(tomlplusplus  URL https://github.com/marzer/tomlplusplus/archive/refs/tags/v3.4.0.tar.gz)
FetchContent_Declare(uriparser     URL https://github.com/uriparser/uriparser/archive/refs/tags/uriparser-0.9.7.tar.gz)

FetchContent_MakeAvailable(
    curl
    jwt-cpp
    libzip
    tomlplusplus
    uriparser)

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
    ${PROJECT_NAME}_utils
    STATIC
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
    vendor/nlohmann-json/include
)

target_link_libraries(
    ${PROJECT_NAME}_query
    PRIVATE
    ${PROJECT_NAME}_utils
    antlr4_static
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
    src/methods/plugins/pluginsconfigure.cpp
    src/methods/plugins/pluginsget.cpp
    src/methods/plugins/pluginsinstall.cpp
    src/methods/plugins/pluginslist.cpp
    src/methods/plugins/pluginsreload.cpp
    src/methods/plugins/pluginsuninstall.cpp
    src/methods/plugins/pluginsupdate.cpp
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
    vendor/nlohmann-json/include
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
    sodium
    SQLite::SQLite3
    torrent-rasterbar
    tomlplusplus::tomlplusplus
    uriparser
    uSockets
    -lcrypto
    -lssl
    -lgit2
    -lz
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
