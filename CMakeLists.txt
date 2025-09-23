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

pkg_check_modules(Lua     REQUIRED IMPORTED_TARGET lua5.4)

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
    src/sessions.cpp
    src/webui.cpp
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
    vendor/croncpp/include
    vendor/jwt-cpp/include
    vendor/nlohmann-json/include
    vendor/sol2/include
    vendor/tomlplusplus/include
)

target_link_libraries(
    ${PROJECT_NAME}_core

    ${ANTLR4_LIBRARIES}

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
