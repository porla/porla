cmake_minimum_required(VERSION 3.28)

cmake_policy(SET CMP0114 NEW)
cmake_policy(SET CMP0135 NEW)

find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

project(porla)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CMakeRC)
include(FetchContent)

set(CMAKE_CXX_STANDARD              20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT PORLA_CORE_TYPE)
    if(BUILD_SHARED_LIBS)
        set(PORLA_CORE_TYPE SHARED)
    else()
        set(PORLA_CORE_TYPE STATIC)
    endif()
endif()

find_package(PkgConfig REQUIRED)

# pkg-config's Libs.private (nghttp2, psl, brotli, ...) is only needed when
# linking statically, and CMAKE_FIND_LIBRARY_SUFFIXES does not reach pkg-config.
function(porla_link_pkg target prefix)
    if(BUILD_SHARED_LIBS)
        target_link_libraries(${target} PUBLIC PkgConfig::${prefix})
    else()
        target_include_directories(${target} PUBLIC ${${prefix}_STATIC_INCLUDE_DIRS})
        target_link_libraries(${target}      PUBLIC ${${prefix}_STATIC_LDFLAGS})
    endif()
endfunction()

FetchContent_Declare(
    ANTLR4
    URL https://github.com/antlr/antlr4/archive/refs/tags/4.13.2.tar.gz
    SOURCE_SUBDIR runtime/Cpp
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(
    uSockets
    URL https://github.com/uNetworking/uSockets/archive/7a7c820db4740c2a2a4faf0d918c4eec2ac1fac3.tar.gz
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(
    uWebSockets
    URL https://github.com/uNetworking/uWebSockets/archive/refs/tags/v20.80.0.tar.gz
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(
    LibtorrentRasterbar
    URL https://github.com/arvidn/libtorrent/releases/download/v2.1.2/libtorrent-rasterbar-2.1.2.tar.gz
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(
    Libzip
    URL https://github.com/nih-at/libzip/releases/download/v1.11.4/libzip-1.11.4.tar.xz
    EXCLUDE_FROM_ALL
)

# antlr4
set(ANTLR4_INSTALL        OFF CACHE BOOL "" FORCE)
set(ANTLR_BUILD_CPP_TESTS OFF CACHE BOOL "" FORCE)
set(ANTLR_BUILD_SHARED    OFF CACHE BOOL "" FORCE)
set(ANTLR_BUILD_STATIC    ON  CACHE BOOL "" FORCE)
set(WITH_DEMO             OFF CACHE BOOL "" FORCE)

# libzip options
set(BUILD_OSSFUZZ   OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES  OFF CACHE BOOL "" FORCE)
set(BUILD_DOC       OFF CACHE BOOL "" FORCE)
set(BUILD_TOOLS     OFF CACHE BOOL "" FORCE)
set(BUILD_REGRESS   OFF CACHE BOOL "" FORCE)
set(ENABLE_BZIP2 OFF CACHE BOOL "" FORCE)
set(ENABLE_LZMA  OFF CACHE BOOL "" FORCE)

# lt options
set(deprecated-functions OFF CACHE BOOL "" FORCE)
set(build_examples       OFF CACHE BOOL "" FORCE)
set(build_tests          OFF CACHE BOOL "" FORCE)
set(build_tools          OFF CACHE BOOL "" FORCE)
set(python-bindings      OFF CACHE BOOL "" FORCE)
set(webtorrent           OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(
    ANTLR4
    LibtorrentRasterbar
    Libzip
    uSockets
    uWebSockets)

find_package(Boost               CONFIG COMPONENTS log program_options system)
find_package(OpenSSL             REQUIRED)
find_package(SQLite3             REQUIRED)
find_package(ZLIB                REQUIRED)

pkg_check_modules(Curl      REQUIRED IMPORTED_TARGET libcurl)
pkg_check_modules(Sodium    REQUIRED IMPORTED_TARGET libsodium)
pkg_check_modules(Lua       REQUIRED IMPORTED_TARGET lua5.4)
pkg_check_modules(MAXMINDDB REQUIRED IMPORTED_TARGET libmaxminddb)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/_version.cpp
    COMMAND ${CMAKE_COMMAND} -P
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateVersion.cmake)

file(GLOB_RECURSE usockets_sources ${usockets_SOURCE_DIR}/src/*.c)
list(FILTER usockets_sources EXCLUDE REGEX "(libuv|gcd|wolfssl|epoll_kqueue)\\.c$")

add_library(uSockets STATIC
    ${usockets_sources}
    ${usockets_SOURCE_DIR}/src/crypto/sni_tree.cpp
    ${usockets_SOURCE_DIR}/src/eventing/asio.cpp
)

target_include_directories(uSockets PUBLIC ${usockets_SOURCE_DIR}/src)
target_compile_definitions(uSockets PUBLIC LIBUS_USE_OPENSSL LIBUS_USE_ASIO)
target_link_libraries(uSockets PUBLIC OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB Boost::boost)

file(GLOB uws_headers ${uwebsockets_SOURCE_DIR}/src/*.h ${uwebsockets_SOURCE_DIR}/src/*/*.h)

file(COPY ${uwebsockets_SOURCE_DIR}/src/
     DESTINATION ${CMAKE_BINARY_DIR}/uws-include/uWebSockets)

add_library(uWebSockets INTERFACE)
target_include_directories(uWebSockets INTERFACE ${CMAKE_BINARY_DIR}/uws-include)
target_link_libraries(uWebSockets INTERFACE uSockets)

cmrc_add_resource_library(
    porla-lua-resources
    ALIAS porla_lua
    NAMESPACE porla_lua
    WHENCE lua
    lua/plugin.lua
    lua/jsonrpc/webui_install.lua
    lua/webui/init.lua)

add_library(
    ${PROJECT_NAME}_core
    ${PORLA_CORE_TYPE}
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp

    src/buildinfo.cpp
    src/cmdargs.cpp
    src/config.cpp
    src/curlmulti.cpp
    src/events.cpp
    src/fields.cpp
    src/httphostguard.cpp
    src/keyvalue.cpp
    src/logger.cpp
    src/mmdb.cpp
    src/plugins.cpp
    src/presets.cpp
    src/sessions/handlers/session.cpp
    src/sessions/handlers/storage.cpp
    src/sessions/handlers/torrents.cpp
    src/sessions/jobs/loadtorrents.cpp
    src/sessions/jobs/savestate.cpp
    src/sessions/scheduler.cpp
    src/sessions/session.cpp
    src/sessions/sessions.cpp
    src/timer.cpp
    src/torrentclientdata.cpp
    src/torrents.cpp
    src/zip.cpp
    src/utils/eta.cpp
    src/utils/string.cpp

    src/auth/authenticator.cpp
    src/auth/password.cpp
    src/auth/tokens.cpp

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
    src/data/migrations/0010_sessions.cpp
    src/data/migrations/0011_presets.cpp
    src/data/migrations/0012_alterplugins.cpp
    src/data/migrations/0013_addkeyvaluestore.cpp
    src/data/migrations/0014_morepresets.cpp
    src/data/migrations/0015_addtorrentparamsindex.cpp
    src/data/migrations/0016_addapikeys.cpp
    src/data/migrations/0017_torrentinfos.cpp
    src/data/migrations/0018_addcompletedat.cpp
    src/data/migrations/0019_addautoincrementids.cpp
    src/data/migrations/0020_tokens.cpp
    src/data/models/addtorrentparams.cpp
    src/data/models/keyvaluestore.cpp
    src/data/models/plugins.cpp
    src/data/models/presets.cpp
    src/data/models/sessions.cpp
    src/data/models/tokens.cpp
    src/data/models/users.cpp
    src/data/statement.cpp
    src/data/transaction.cpp

    src/json/_/errorcode.cpp
    src/json/_/ltalerts.cpp
    src/json/_/ltannounceentry.cpp
    src/json/_/ltfileflags.cpp
    src/json/_/ltinfohash.cpp
    src/json/_/ltpeerinfo.cpp
    src/json/_/ltqueueposition.cpp
    src/json/_/ltsettingspack.cpp
    src/json/_/lttorrentflags.cpp
    src/json/_/lttorrentstatus.cpp
    src/json/_/lttypedbitfield.cpp

    src/lua/packages/codec.cpp
    src/lua/packages/crypto.cpp
    src/lua/packages/events.cpp
    src/lua/packages/filesystem.cpp
    src/lua/packages/httpclient.cpp
    src/lua/packages/httpserver.cpp
    src/lua/packages/jsonrpc.cpp
    src/lua/packages/kv.cpp
    src/lua/packages/presets.cpp
    src/lua/packages/runtime.cpp
    src/lua/packages/sessions.cpp
    src/lua/packages/sockets.cpp
    src/lua/packages/timers.cpp
    src/lua/packages/zip.cpp

    src/lua/types/ltaddtorrentparams.cpp
    src/lua/types/ltannounceendpoint.cpp
    src/lua/types/ltannounceentry.cpp
    src/lua/types/ltannounceinfohash.cpp
    src/lua/types/ltfilenames.cpp
    src/lua/types/ltfilestorage.cpp
    src/lua/types/ltinfohash.cpp
    src/lua/types/ltopenfilestate.cpp
    src/lua/types/ltpeerinfo.cpp
    src/lua/types/ltrenamedfiles.cpp
    src/lua/types/ltsettingspack.cpp
    src/lua/types/lttorrentflags.cpp
    src/lua/types/lttorrentinfo.cpp
    src/lua/types/lttorrenthandle.cpp
    src/lua/types/lttorrentstatus.cpp
    src/lua/types/poauthcontext.cpp
    src/lua/types/pocancellable.cpp
    src/lua/types/poerror.cpp
    src/lua/types/pojson.cpp
    src/lua/types/pohttpformfile.cpp
    src/lua/types/pohttpserverresponse.cpp
    src/lua/types/pojsonrpcresponse.cpp
    src/lua/types/popreset.cpp
    src/lua/types/poquery.cpp
    src/lua/types/posessionhandle.cpp
    src/lua/types/posessionsiterator.cpp
    src/lua/types/potcpclient.cpp
    src/lua/types/potorrentdata.cpp
    src/lua/types/potorrentshandle.cpp
    src/lua/types/potorrentsiterator.cpp
    src/lua/types/sohashstate.cpp

    src/lua/print.cpp
    src/lua/plugin.cpp
    src/lua/pluginmanifest.cpp
    src/lua/pluginsource.cpp

    src/rpc/jsonrpc.cpp
    src/rpc/methods/auth.cpp
    src/rpc/methods/fs.cpp
    src/rpc/methods/kv.cpp
    src/rpc/methods/mmdb.cpp
    src/rpc/methods/plugins.cpp
    src/rpc/methods/presets.cpp
    src/rpc/methods/resolve.cpp
    src/rpc/methods/sessions.cpp
    src/rpc/methods/sys.cpp
    src/rpc/methods/torrents.cpp

    src/query/_aux/PqlLexer.cpp
    src/query/_aux/PqlParser.cpp
    src/query/pql.cpp

    src/utils/ltflags.cpp
    src/utils/ltsettings.cpp
    src/utils/ratio.cpp
)

target_compile_definitions(
    ${PROJECT_NAME}_core
    PUBLIC
    -DSOL_ALL_SAFETIES_ON=1
)

target_include_directories(
    ${PROJECT_NAME}_core
    PUBLIC
    vendor/croncpp/include
    vendor/nlohmann-json/include
    vendor/sol2/include
)

target_link_libraries(
    ${PROJECT_NAME}_core
    PUBLIC

    antlr4_static
    libzip::zip
    torrent-rasterbar

    Boost::boost Boost::log Boost::program_options
    OpenSSL::SSL
    OpenSSL::Crypto
    SQLite::SQLite3
    uWebSockets
    ZLIB::ZLIB
)

porla_link_pkg(${PROJECT_NAME}_core Curl)
porla_link_pkg(${PROJECT_NAME}_core Lua)
porla_link_pkg(${PROJECT_NAME}_core MAXMINDDB)
porla_link_pkg(${PROJECT_NAME}_core Sodium)

add_executable(
    ${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    ${PROJECT_NAME}_core
    porla_lua
)

option(PORLA_BUILD_TESTS "Build porla unit tests" OFF)

if(PORLA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
