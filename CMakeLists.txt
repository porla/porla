cmake_minimum_required(VERSION 3.12)

set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg-triplets)
set(CMAKE_TOOLCHAIN_FILE   ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(porla)

set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(antlr4-runtime      CONFIG REQUIRED)
find_package(Boost                      REQUIRED COMPONENTS log program_options system)
find_package(CURL                CONFIG REQUIRED)
find_package(GTest               CONFIG REQUIRED)
find_package(libzip              CONFIG REQUIRED)
find_package(LibtorrentRasterbar CONFIG REQUIRED)
find_package(Lua                        REQUIRED)
find_package(nlohmann_json       CONFIG REQUIRED)
find_package(tomlplusplus        CONFIG REQUIRED)
find_package(unofficial-sodium   CONFIG REQUIRED)
find_package(unofficial-sqlite3  CONFIG REQUIRED)
find_package(uriparser           CONFIG REQUIRED)

find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/base.h")

include_directories(${ANTLR4_INCLUDE_DIR})

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
    ${PROJECT_NAME}_utils
    src/utils/ratio.cpp
)

target_link_libraries(
    ${PROJECT_NAME}_utils
    PRIVATE
    LibtorrentRasterbar::torrent-rasterbar
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
    LibtorrentRasterbar::torrent-rasterbar
)

add_library(
    ${PROJECT_NAME}_core
    STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/webui.cpp

    src/authinithandler.cpp
    src/authloginhandler.cpp
    src/buildinfo.cpp
    src/cmdargs.cpp
    src/config.cpp
    src/embeddedwebuihandler.cpp
    src/logger.cpp
    src/httpclient.cpp
    src/httpeventstream.cpp
    src/httpjwtauth.cpp
    src/httpserver.cpp
    src/httpsession.cpp
    src/jsonrpchandler.cpp
    src/metricshandler.cpp
    src/session.cpp
    src/systemhandler.cpp
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
    src/data/models/addtorrentparams.cpp
    src/data/models/users.cpp
    src/data/statement.cpp

    src/lua/usertypes/lttorrenthandle.cpp
    src/lua/usertypes/torrent.cpp
    src/lua/usertypes/workflow.cpp
    src/lua/usertypes/workflowactionexec.cpp
    src/lua/usertypes/workflowactionhttprequest.cpp
    src/lua/usertypes/workflowactionlog.cpp
    src/lua/usertypes/workflowactionpushdiscord.cpp
    src/lua/usertypes/workflowactionpushntfy.cpp
    src/lua/usertypes/workflowactionsleep.cpp
    src/lua/usertypes/workflowactiontorrentflags.cpp
    src/lua/usertypes/workflowactiontorrentmove.cpp
    src/lua/usertypes/workflowactiontorrentpause.cpp
    src/lua/usertypes/workflowactiontorrentreannounce.cpp
    src/lua/usertypes/workflowactiontorrentremove.cpp
    src/lua/usertypes/workflowtriggercron.cpp
    src/lua/usertypes/workflowtriggerinterval.cpp
    src/lua/usertypes/workflowtriggertorrentadded.cpp
    src/lua/usertypes/workflowtriggertorrentfinished.cpp
    src/lua/usertypes/workflowtriggertorrentmoved.cpp
    src/lua/workflows/workflowengine.cpp
    src/lua/workflows/workflowfilter.cpp
    src/lua/workflows/workflowrunner.cpp
    src/lua/workflows/actions/exec.cpp
    src/lua/workflows/actions/function.cpp
    src/lua/workflows/actions/httprequest.cpp
    src/lua/workflows/actions/log.cpp
    src/lua/workflows/actions/pushdiscord.cpp
    src/lua/workflows/actions/pushntfy.cpp
    src/lua/workflows/actions/sleep.cpp
    src/lua/workflows/actions/torrentflags.cpp
    src/lua/workflows/actions/torrentmove.cpp
    src/lua/workflows/actions/torrentpause.cpp
    src/lua/workflows/actions/torrentreannounce.cpp
    src/lua/workflows/actions/torrentremove.cpp
    src/lua/workflows/triggers/cron.cpp
    src/lua/workflows/triggers/interval.cpp
    src/lua/workflows/triggers/torrentadded.cpp
    src/lua/workflows/triggers/torrentfinished.cpp
    src/lua/workflows/triggers/torrentmoved.cpp

    src/methods/fsspace.cpp
    src/methods/presetslist.cpp
    src/methods/sessionpause.cpp
    src/methods/sessionresume.cpp
    src/methods/sessionsettingslist.cpp
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

    Boost::boost
    Boost::log
    Boost::program_options
    CURL::libcurl
    libzip::zip
    LibtorrentRasterbar::torrent-rasterbar
    unofficial-sodium::sodium
    unofficial-sodium::sodium_config_public
    unofficial::sqlite3::sqlite3
    uriparser::uriparser
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  set(ARCH_NAME "x64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
  set(ARCH_NAME "arm64")
else()
  set(ARCH_NAME ${CMAKE_SYSTEM_PROCESSOR})
endif()

add_executable(
    ${PROJECT_NAME}_${ARCH_NAME}
    src/main.cpp
)

target_link_libraries(
    ${PROJECT_NAME}_${ARCH_NAME}
    ${PROJECT_NAME}_core
)

add_executable(
    ${PROJECT_NAME}_tests
    tests/inmemorysession.cpp
    tests/main.cpp
    tests/query/pql.cpp
    tests/utils/string.cpp
)

target_link_libraries(
    ${PROJECT_NAME}_tests
    ${PROJECT_NAME}_core
    ${PROJECT_NAME}_query
    GTest::gmock
    GTest::gtest
    GTest::gmock_main
)
