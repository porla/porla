cmake_minimum_required(VERSION 3.12)

set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg-triplets)
set(CMAKE_TOOLCHAIN_FILE   ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(porla)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost                      REQUIRED COMPONENTS log program_options system)
find_package(libzip              CONFIG REQUIRED)
find_package(LibtorrentRasterbar CONFIG REQUIRED)
find_package(nlohmann_json       CONFIG REQUIRED)
find_package(ryml                CONFIG REQUIRED)
find_package(tomlplusplus        CONFIG REQUIRED)
find_package(unofficial-sodium   CONFIG REQUIRED)
find_package(unofficial-sqlite3  CONFIG REQUIRED)
find_package(uriparser           CONFIG REQUIRED)

find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/base.h")

add_subdirectory(vendor/jsonnet)

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

add_executable(to_c_array vendor/jsonnet/stdlib/to_c_array.cpp)

add_executable(
    ${PROJECT_NAME}

    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/webui.cpp

    src/authinithandler.cpp
    src/authinithandler.hpp
    src/authloginhandler.cpp
    src/authloginhandler.hpp
    src/buildinfo.cpp
    src/buildinfo.hpp
    src/cmdargs.cpp
    src/cmdargs.hpp
    src/config.cpp
    src/config.hpp
    src/embeddedwebuihandler.cpp
    src/embeddedwebuihandler.hpp
    src/logger.cpp
    src/logger.hpp
    src/main.cpp
    src/httpcontext.hpp
    src/httpeventstream.cpp
    src/httpeventstream.hpp
    src/httpjwtauth.cpp
    src/httpjwtauth.hpp
    src/httpmiddleware.hpp
    src/httpserver.cpp
    src/httpserver.hpp
    src/httpsession.cpp
    src/httpsession.hpp
    src/jsonrpchandler.cpp
    src/jsonrpchandler.hpp
    src/metricshandler.cpp
    src/metricshandler.hpp
    src/session.cpp
    src/session.hpp
    src/systemhandler.cpp
    src/systemhandler.hpp
    src/torrentsvt.cpp
    src/torrentsvt.hpp
    src/uri.cpp
    src/uri.hpp
    src/utils/eta.cpp
    src/utils/eta.hpp
    src/utils/ratio.cpp
    src/utils/ratio.hpp
    src/utils/secretkey.cpp
    src/utils/secretkey.hpp
    src/webhookclient.cpp
    src/webhookclient.hpp

    src/actions/executor.cpp
    src/actions/executor.hpp
    src/actions/forcereannounce.cpp
    src/actions/forcereannounce.hpp
    src/actions/log.cpp
    src/actions/log.hpp
    src/actions/move.cpp
    src/actions/move.hpp
    src/actions/sleep.cpp
    src/actions/sleep.hpp

    src/data/migrate.cpp
    src/data/migrate.hpp
    src/data/migrations/0001_initialsetup.cpp
    src/data/migrations/0001_initialsetup.hpp
    src/data/migrations/0002_addsessionsettings.cpp
    src/data/migrations/0002_addsessionsettings.hpp
    src/data/migrations/0003_users.cpp
    src/data/migrations/0003_users.hpp
    src/data/migrations/0004_removesessionparams.cpp
    src/data/migrations/0004_removesessionparams.hpp
    src/data/migrations/0005_metadata.cpp
    src/data/migrations/0005_metadata.hpp
    src/data/models/addtorrentparams.cpp
    src/data/models/addtorrentparams.hpp
    src/data/models/sessionsettings.cpp
    src/data/models/sessionsettings.hpp
    src/data/models/torrentsmetadata.cpp
    src/data/models/torrentsmetadata.hpp
    src/data/models/users.cpp
    src/data/models/users.hpp
    src/data/statement.cpp
    src/data/statement.hpp

    src/methods/method.hpp
    src/methods/presetslist.cpp
    src/methods/presetslist.hpp
    src/methods/sessionpause.cpp
    src/methods/sessionpause.hpp
    src/methods/sessionresume.cpp
    src/methods/sessionresume.hpp
    src/methods/sessionsettingslist.cpp
    src/methods/sessionsettingslist.hpp
    src/methods/sessionsettingsupdate.cpp
    src/methods/sessionsettingsupdate.hpp
    src/methods/sysversions.cpp
    src/methods/sysversions.hpp
    src/methods/torrentsadd.cpp
    src/methods/torrentsadd.hpp
    src/methods/torrentsfileslist.cpp
    src/methods/torrentsfileslist.hpp
    src/methods/torrentslist.cpp
    src/methods/torrentslist.hpp
    src/methods/torrentsmetadatalist.cpp
    src/methods/torrentsmetadatalist.hpp
    src/methods/torrentsmove.cpp
    src/methods/torrentsmove.hpp
    src/methods/torrentspause.cpp
    src/methods/torrentspause.hpp
    src/methods/torrentspeersadd.cpp
    src/methods/torrentspeersadd.hpp
    src/methods/torrentspeerslist.cpp
    src/methods/torrentspeerslist.hpp
    src/methods/torrentspropertiesget.cpp
    src/methods/torrentspropertiesget.hpp
    src/methods/torrentsquery.cpp
    src/methods/torrentsquery.hpp
    src/methods/torrentsremove.cpp
    src/methods/torrentsremove.hpp
    src/methods/torrentsresume.cpp
    src/methods/torrentsresume.hpp
    src/methods/torrentspropertiesset.cpp
    src/methods/torrentspropertiesset.hpp
    src/methods/torrentstrackerslist.cpp
    src/methods/torrentstrackerslist.hpp

    src/tools/authtoken.cpp
    src/tools/authtoken.hpp
    src/tools/generatesecretkey.cpp
    src/tools/generatesecretkey.hpp
    src/tools/versionjson.cpp
    src/tools/versionjson.hpp
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    -DSQLITE_CORE
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${JWT_CPP_INCLUDE_DIRS}
)

target_link_libraries(
    ${PROJECT_NAME}

    Boost::boost
    Boost::log
    Boost::program_options
    jsonnet
    libzip::zip
    LibtorrentRasterbar::torrent-rasterbar
    unofficial-sodium::sodium
    unofficial-sodium::sodium_config_public
    unofficial::sqlite3::sqlite3
    uriparser::uriparser
)
