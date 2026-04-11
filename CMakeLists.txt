cmake_minimum_required(VERSION 3.12)

cmake_policy(SET CMP0114 NEW)
cmake_policy(SET CMP0135 NEW)

project(porla)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CMakeRC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib .so .dll)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(PkgConfig           REQUIRED)
find_package(ANTLR4              REQUIRED)
find_package(Boost               CONFIG COMPONENTS log program_options system)
find_package(CURL                REQUIRED)
find_package(LibtorrentRasterbar REQUIRED)
find_package(libzip              REQUIRED)
find_package(OpenSSL             REQUIRED)
find_package(SQLite3             REQUIRED)
find_package(ZLIB                REQUIRED)

pkg_check_modules(Lua       REQUIRED IMPORTED_TARGET lua5.4)
pkg_check_modules(MAXMINDDB REQUIRED libmaxminddb)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/_version.cpp
    COMMAND ${CMAKE_COMMAND} -P
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateVersion.cmake)


cmrc_add_resource_library(
    porla_lua_bootstrap
    ALIAS porla::lua_bootstrap
    WHENCE lua
    lua/bootstrap.lua
)

add_library(
    ${PROJECT_NAME}_core
    STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp

    src/buildinfo.cpp
    src/cmdargs.cpp
    src/config.cpp
    src/curlmulti.cpp
    src/logger.cpp

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
    src/lua/types/fs.cpp
    src/lua/types/json.cpp
    src/lua/types/libcurl.cpp
    src/lua/types/libzip.cpp
    src/lua/types/log.cpp
    src/lua/types/ltaddtorrentparams.cpp
    src/lua/types/ltalert.cpp
    src/lua/types/ltannounceendpoint.cpp
    src/lua/types/ltannounceentry.cpp
    src/lua/types/ltannounceinfohash.cpp
    src/lua/types/ltdownloadpriority.cpp
    src/lua/types/ltfilestorage.cpp
    src/lua/types/ltinfohash.cpp
    src/lua/types/ltpeerinfo.cpp
    src/lua/types/ltsession.cpp
    src/lua/types/ltsettingspack.cpp
    src/lua/types/ltstoragemode.cpp
    src/lua/types/lttorrentflags.cpp
    src/lua/types/lttorrenthandle.cpp
    src/lua/types/lttorrentinfo.cpp
    src/lua/types/lttorrentstatus.cpp
    src/lua/types/mmdb.cpp
    src/lua/types/timer.cpp
    src/lua/types/toml.cpp
    src/lua/types/uwsapp.cpp
    src/lua/host.cpp

    src/query/_aux/PorlaQueryLangBaseVisitor.cpp
    src/query/_aux/PorlaQueryLangLexer.cpp
    src/query/_aux/PorlaQueryLangParser.cpp
    src/query/_aux/PorlaQueryLangVisitor.cpp
    src/query/pql.cpp
)

target_compile_definitions(
    ${PROJECT_NAME}_core
    PUBLIC
    -DSQLITE_CORE
)

target_include_directories(
    ${PROJECT_NAME}_core
    PUBLIC
    ${ANTLR4_INCLUDE_DIRS}
    ${MAXMINDDB_INCLUDE_DIRS}
    vendor/croncpp/include
    vendor/jwt-cpp/include
    vendor/nlohmann-json/include
    vendor/sol2/include
    vendor/tomlplusplus/include
)

target_link_libraries(
    ${PROJECT_NAME}_core

    ${ANTLR4_LIBRARIES}
    ${MAXMINDDB_LIBRARIES}

    porla::lua_bootstrap

    Boost::boost Boost::log Boost::program_options
    CURL::libcurl
    libzip::zip
    PkgConfig::Lua
    sodium
    SQLite::SQLite3
    torrent-rasterbar
    -luSockets
    -lcrypto
    -lssl
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
