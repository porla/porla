cmake_minimum_required(VERSION 3.12)

cmake_policy(SET CMP0114 NEW)
cmake_policy(SET CMP0135 NEW)

project(porla)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib .so .dll)

find_package(PkgConfig           REQUIRED)
find_package(ANTLR4              REQUIRED)
find_package(Boost               CONFIG COMPONENTS log program_options system)
find_package(CURL                REQUIRED)
find_package(libzip              REQUIRED)
find_package(LibtorrentRasterbar REQUIRED)
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

add_library(
    ${PROJECT_NAME}_core
    STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp

    src/buildinfo.cpp
    src/cmdargs.cpp
    src/config.cpp
    src/curlmulti.cpp
    src/logger.cpp
    src/mmdb.cpp
    src/sessions.cpp
    src/timer.cpp
    src/webui.cpp
    src/zip.cpp
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
    src/data/migrations/0010_sessions.cpp
    src/data/migrations/0011_presets.cpp
    src/data/migrations/0012_alterplugins.cpp
    src/data/migrations/0013_addkeyvaluestore.cpp
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
    src/json/_/torrentsrecheck.cpp
    src/json/_/torrentsremove.cpp
    src/json/_/torrentsresume.cpp
    src/json/_/torrentstrackerslist.cpp
    src/json/_/webuiinstall.cpp

    src/lua/globals/http.cpp
    src/lua/globals/sleep.cpp
    src/lua/types/ltaddtorrentparams.cpp
    src/lua/types/ltannounceendpoint.cpp
    src/lua/types/ltannounceentry.cpp
    src/lua/types/ltannounceinfohash.cpp
    src/lua/types/ltdownloadpriority.cpp
    src/lua/types/ltfilestorage.cpp
    src/lua/types/ltinfohash.cpp
    src/lua/types/ltpeerinfo.cpp
    src/lua/types/ltsettingspack.cpp
    src/lua/types/ltstoragemode.cpp
    src/lua/types/lttorrentflags.cpp
    src/lua/types/lttorrenthandle.cpp
    src/lua/types/lttorrentinfo.cpp
    src/lua/types/lttorrentstatus.cpp
    src/lua/types/session.cpp
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
    src/rpc/methods/torrents/torrentsrecheck.cpp
    src/rpc/methods/torrents/torrentsremove.cpp
    src/rpc/methods/torrents/torrentsresume.cpp
    src/rpc/methods/torrents/torrentspropertiesset.cpp
    src/rpc/methods/torrents/torrentstrackerslist.cpp
    src/rpc/methods/webui/webuiinstall.cpp

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

    Boost::boost Boost::log Boost::program_options
    CURL::libcurl
    libzip::zip
    LibtorrentRasterbar::torrent-rasterbar
    PkgConfig::Lua
    sodium
    SQLite::SQLite3
    -luSockets
    -lcrypto
    -lssl
    -lunistring
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
