cmake_minimum_required(VERSION 3.28)

cmake_policy(SET CMP0114 NEW)
cmake_policy(SET CMP0135 NEW)

find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

project(porla)

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.deps" CACHE PATH "")

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
    URL https://github.com/uNetworking/uSockets/archive/refs/tags/v0.8.8.tar.gz
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(
    uWebSockets
    URL https://github.com/uNetworking/uWebSockets/archive/refs/tags/v20.80.0.tar.gz
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(
    LibtorrentRasterbar
    URL https://github.com/arvidn/libtorrent/releases/download/v2.1.1/libtorrent-rasterbar-2.1.1.tar.gz
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

add_subdirectory(vendor/fmt)

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
    src/logger.cpp
    src/mmdb.cpp
    src/sessions.cpp
    src/timer.cpp
    src/zip.cpp
    src/utils/eta.cpp
    src/utils/secretkey.cpp
    src/utils/string.cpp

    src/auth/authenticator.cpp

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
    src/data/models/addtorrentparams.cpp
    src/data/models/keyvaluestore.cpp
    src/data/models/plugins.cpp
    src/data/models/presets.cpp
    src/data/models/sessions.cpp
    src/data/models/users.cpp
    src/data/statement.cpp

    src/json/_/authinit.cpp
    src/json/_/authlogin.cpp
    src/json/_/errorcode.cpp
    src/json/_/fsspace.cpp
    src/json/_/keyvalueget.cpp
    src/json/_/keyvalueset.cpp
    src/json/_/ltannounceentry.cpp
    src/json/_/ltinfohash.cpp
    src/json/_/ltpeerinfo.cpp
    src/json/_/ltqueueposition.cpp
    src/json/_/ltsettingspack.cpp
    src/json/_/lttorrentflags.cpp
    src/json/_/lttorrentstatus.cpp
    src/json/_/mmdblookup.cpp
    src/json/_/pluginsadd.cpp
    src/json/_/pluginsget.cpp
    src/json/_/pluginsinstall.cpp
    src/json/_/pluginslist.cpp
    src/json/_/pluginsreload.cpp
    src/json/_/pluginsremove.cpp
    src/json/_/pluginsupdate.cpp
    src/json/_/pluginsupgrade.cpp
    src/json/_/presetsadd.cpp
    src/json/_/presetsget.cpp
    src/json/_/presetslist.cpp
    src/json/_/presetsremove.cpp
    src/json/_/presetsupdate.cpp
    src/json/_/sessionsadd.cpp
    src/json/_/sessionsget.cpp
    src/json/_/sessionslist.cpp
    src/json/_/sessionspause.cpp
    src/json/_/sessionsremove.cpp
    src/json/_/sessionsresume.cpp
    src/json/_/sessionssettingsget.cpp
    src/json/_/sessionssettingsset.cpp
    src/json/_/sessionsupdate.cpp
    src/json/_/torrentclientdata.cpp
    src/json/_/torrentsadd.cpp
    src/json/_/torrentscount.cpp
    src/json/_/torrentsfileslist.cpp
    src/json/_/torrentsfilespriorities.cpp
    src/json/_/torrentsfilesprioritize.cpp
    src/json/_/torrentsfilesprogress.cpp
    src/json/_/torrentsfilesrename.cpp
    src/json/_/torrentsget.cpp
    src/json/_/torrentslist.cpp
    src/json/_/torrentsmigrate.cpp
    src/json/_/torrentsmove.cpp
    src/json/_/torrentspause.cpp
    src/json/_/torrentspeersadd.cpp
    src/json/_/torrentspeerslist.cpp
    src/json/_/torrentspiecesget.cpp
    src/json/_/torrentspropertiesget.cpp
    src/json/_/torrentspropertiesset.cpp
    src/json/_/torrentsqueueany.cpp
    src/json/_/torrentsqueueset.cpp
    src/json/_/torrentsrecheck.cpp
    src/json/_/torrentsremove.cpp
    src/json/_/torrentsresume.cpp
    src/json/_/torrentstrackersadd.cpp
    src/json/_/torrentstrackerslist.cpp

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
    src/lua/pluginengine.cpp
    src/lua/pluginsource.cpp

    src/rpc/jsonrpc.cpp
    src/rpc/methods/auth/authinit.cpp
    src/rpc/methods/auth/authlogin.cpp
    src/rpc/methods/fs/fsspace.cpp
    src/rpc/methods/kv/keyvalueget.cpp
    src/rpc/methods/kv/keyvalueset.cpp
    src/rpc/methods/mmdb/mmdblookup.cpp
    src/rpc/methods/plugins/pluginsadd.cpp
    src/rpc/methods/plugins/pluginsget.cpp
    src/rpc/methods/plugins/pluginsinstall.cpp
    src/rpc/methods/plugins/pluginslist.cpp
    src/rpc/methods/plugins/pluginsreload.cpp
    src/rpc/methods/plugins/pluginsremove.cpp
    src/rpc/methods/plugins/pluginsupdate.cpp
    src/rpc/methods/plugins/pluginsupgrade.cpp
    src/rpc/methods/presets/presetsadd.cpp
    src/rpc/methods/presets/presetsget.cpp
    src/rpc/methods/presets/presetslist.cpp
    src/rpc/methods/presets/presetsremove.cpp
    src/rpc/methods/presets/presetsupdate.cpp
    src/rpc/methods/sessions/sessionsadd.cpp
    src/rpc/methods/sessions/sessionsget.cpp
    src/rpc/methods/sessions/sessionslist.cpp
    src/rpc/methods/sessions/sessionspause.cpp
    src/rpc/methods/sessions/sessionsremove.cpp
    src/rpc/methods/sessions/sessionsresume.cpp
    src/rpc/methods/sessions/sessionssettingsget.cpp
    src/rpc/methods/sessions/sessionssettingsset.cpp
    src/rpc/methods/sessions/sessionsupdate.cpp
    src/rpc/methods/sys/sysstatus.cpp
    src/rpc/methods/sys/sysversions.cpp
    src/rpc/methods/torrents/torrentsadd.cpp
    src/rpc/methods/torrents/torrentscount.cpp
    src/rpc/methods/torrents/torrentsfileslist.cpp
    src/rpc/methods/torrents/torrentsfilespriorities.cpp
    src/rpc/methods/torrents/torrentsfilesprioritize.cpp
    src/rpc/methods/torrents/torrentsfilesprogress.cpp
    src/rpc/methods/torrents/torrentsfilesrename.cpp
    src/rpc/methods/torrents/torrentsget.cpp
    src/rpc/methods/torrents/torrentslist.cpp
    src/rpc/methods/torrents/torrentsmigrate.cpp
    src/rpc/methods/torrents/torrentsmove.cpp
    src/rpc/methods/torrents/torrentspause.cpp
    src/rpc/methods/torrents/torrentspeersadd.cpp
    src/rpc/methods/torrents/torrentspeerslist.cpp
    src/rpc/methods/torrents/torrentspiecesget.cpp
    src/rpc/methods/torrents/torrentspropertiesget.cpp
    src/rpc/methods/torrents/torrentsqueueany.cpp
    src/rpc/methods/torrents/torrentsqueueset.cpp
    src/rpc/methods/torrents/torrentsrecheck.cpp
    src/rpc/methods/torrents/torrentsremove.cpp
    src/rpc/methods/torrents/torrentsresume.cpp
    src/rpc/methods/torrents/torrentspropertiesset.cpp
    src/rpc/methods/torrents/torrentstrackersadd.cpp
    src/rpc/methods/torrents/torrentstrackerslist.cpp

    src/query/_aux/PorlaQueryLangBaseVisitor.cpp
    src/query/_aux/PorlaQueryLangLexer.cpp
    src/query/_aux/PorlaQueryLangParser.cpp
    src/query/_aux/PorlaQueryLangVisitor.cpp
    src/query/pql.cpp

    src/utils/ratio.cpp
    src/utils/ltsettings.cpp
)

target_compile_definitions(
    ${PROJECT_NAME}_core
    PUBLIC
    -DSOL_ALL_SAFETIES_ON=1
    -DSQLITE_CORE
)

target_include_directories(
    ${PROJECT_NAME}_core
    PUBLIC
    vendor/croncpp/include
    vendor/jwt-cpp/include
    vendor/nlohmann-json/include
    vendor/sol2/include
    vendor/tomlplusplus/include
)

target_link_libraries(
    ${PROJECT_NAME}_core
    PUBLIC

    antlr4_static
    libzip::zip
    torrent-rasterbar

    Boost::boost Boost::log Boost::program_options
    fmt::fmt
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
