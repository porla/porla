cmake_minimum_required(VERSION 3.12)
set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)

project(porla)

include(FetchContent)

# cmake options
set(CMAKE_CXX_STANDARD 20)

# boost options
set(BOOST_INCLUDE_LIBRARIES asio headers log program_options system)
set(BOOST_ENABLE_CMAKE      ON)
set(Boost_USE_STATIC_LIBS   ON)

FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
    FIND_PACKAGE_ARGS NAMES asio headers log program_options system
)

FetchContent_Declare(
    libtorrent
    URL https://github.com/arvidn/libtorrent/releases/download/v2.0.10/libtorrent-rasterbar-2.0.10.tar.gz
)

FetchContent_MakeAvailable(
    Boost
    libtorrent
)

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
    Boost
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
    vendor/sol2/include
)

target_link_libraries(
    ${PROJECT_NAME}_core

    ${PROJECT_NAME}_query
    ${PROJECT_NAME}_utils

    ${LUA_LIBRARIES}

    Boost
    CURL::libcurl
    libzip::zip
    torrent-rasterbar
    PkgConfig::LIBGIT2
    PkgConfig::tomlplusplus
    unofficial-sodium::sodium
    unofficial-sodium::sodium_config_public
    unofficial::sqlite3::sqlite3
    uriparser::uriparser
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

