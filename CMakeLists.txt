cmake_minimum_required(VERSION 3.12)

cmake_policy(SET CMP0114 NEW)
cmake_policy(SET CMP0135 NEW)

project(porla)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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

add_library(
    ${PROJECT_NAME}_core
    STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp

    src/buildinfo.cpp
    src/cmdargs.cpp
    src/config.cpp
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
    src/data/models/presets.cpp
    src/data/models/sessions.cpp
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

    src/json/_/errorcode.cpp
    src/json/_/fsspace.cpp
    src/json/_/keyvalueget.cpp
    src/json/_/keyvalueset.cpp
    src/json/_/ltannounceentry.cpp
    src/json/_/ltinfohash.cpp
    src/json/_/ltpeerinfo.cpp
    src/json/_/ltsettingspack.cpp
    src/json/_/lttorrentstatus.cpp
    src/json/_/mmdblookup.cpp
    src/json/_/pluginsadd.cpp
    src/json/_/pluginsget.cpp
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
    src/json/_/sessionslist.cpp
    src/json/_/sessionspause.cpp
    src/json/_/sessionsremove.cpp
    src/json/_/sessionsresume.cpp
    src/json/_/sessionssettingsget.cpp
    src/json/_/sessionssettingsset.cpp
    src/json/_/torrentclientdata.cpp
    src/json/_/torrentsadd.cpp
    src/json/_/torrentsfileslist.cpp
    src/json/_/torrentsfilesprogress.cpp
    src/json/_/torrentsget.cpp
    src/json/_/torrentslist.cpp
    src/json/_/torrentsmetadatalist.cpp
    src/json/_/torrentsmove.cpp
    src/json/_/torrentsoverview.cpp
    src/json/_/torrentspause.cpp
    src/json/_/torrentspeersadd.cpp
    src/json/_/torrentspeerslist.cpp
    src/json/_/torrentspropertiesget.cpp
    src/json/_/torrentspropertiesset.cpp
    src/json/_/torrentsrecheck.cpp
    src/json/_/torrentsremove.cpp
    src/json/_/torrentsresume.cpp
    src/json/_/torrentstrackerslist.cpp
    src/json/_/webuiinstall.cpp

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
    src/methods/keyvalueget.cpp
    src/methods/keyvalueset.cpp
    src/methods/mmdblookup.cpp
    src/methods/plugins/pluginsadd.cpp
    src/methods/plugins/pluginsget.cpp
    src/methods/plugins/pluginslist.cpp
    src/methods/plugins/pluginsreload.cpp
    src/methods/plugins/pluginsremove.cpp
    src/methods/plugins/pluginsupdate.cpp
    src/methods/presets/presetsadd.cpp
    src/methods/presets/presetsget.cpp
    src/methods/presets/presetslist.cpp
    src/methods/presets/presetsremove.cpp
    src/methods/presets/presetsupdate.cpp
    src/methods/sessions/sessionsadd.cpp
    src/methods/sessions/sessionslist.cpp
    src/methods/sessions/sessionspause.cpp
    src/methods/sessions/sessionsremove.cpp
    src/methods/sessions/sessionsresume.cpp
    src/methods/sessions/sessionssettingslist.cpp
    src/methods/sessions/sessionssettingsset.cpp
    src/methods/sysversions.cpp
    src/methods/torrents/torrentsadd.cpp
    src/methods/torrents/torrentsfileslist.cpp
    src/methods/torrents/torrentsfilesprogress.cpp
    src/methods/torrents/torrentsget.cpp
    src/methods/torrents/torrentslist.cpp
    src/methods/torrents/torrentsmetadatalist.cpp
    src/methods/torrents/torrentsmove.cpp
    src/methods/torrents/torrentspause.cpp
    src/methods/torrents/torrentspeersadd.cpp
    src/methods/torrents/torrentspeerslist.cpp
    src/methods/torrents/torrentspropertiesget.cpp
    src/methods/torrents/torrentsrecheck.cpp
    src/methods/torrents/torrentsremove.cpp
    src/methods/torrents/torrentsresume.cpp
    src/methods/torrents/torrentspropertiesset.cpp
    src/methods/torrents/torrentstrackerslist.cpp
    src/methods/torrents/torrentsoverview.cpp
    src/methods/webui/webuiinstall.cpp

    src/query/_aux/PorlaQueryLangBaseVisitor.cpp
    src/query/_aux/PorlaQueryLangLexer.cpp
    src/query/_aux/PorlaQueryLangParser.cpp
    src/query/_aux/PorlaQueryLangVisitor.cpp
    src/query/pql.cpp

    src/tools/authtoken.cpp
    src/tools/generatesecretkey.cpp
    src/tools/versionjson.cpp

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
