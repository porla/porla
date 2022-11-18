cmake_minimum_required(VERSION 3.12)

set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg-triplets)
set(CMAKE_TOOLCHAIN_FILE   ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(porla)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost                      REQUIRED COMPONENTS log program_options system)
#find_package(GTest               CONFIG REQUIRED)
find_package(libzip              CONFIG REQUIRED)
find_package(LibtorrentRasterbar CONFIG REQUIRED)
find_package(nlohmann_json       CONFIG REQUIRED)
find_package(tomlplusplus        CONFIG REQUIRED)
find_package(unofficial-sodium   CONFIG REQUIRED)
find_package(unofficial-sqlite3  CONFIG REQUIRED)
find_package(uriparser           CONFIG REQUIRED)

find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/base.h")

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
    src/config.cpp
    src/config.hpp
    src/embeddedwebuihandler.cpp
    src/embeddedwebuihandler.hpp
    src/main.cpp
    src/httpauthtokenhandler.cpp
    src/httpauthtokenhandler.hpp
    src/httpcontext.hpp
    src/httpeventstream.cpp
    src/httpeventstream.hpp
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
    src/torrentsvt.cpp
    src/torrentsvt.hpp

    src/data/migrate.cpp
    src/data/migrate.hpp
    src/data/migrations/0001_initialsetup.cpp
    src/data/migrations/0001_initialsetup.hpp
    src/data/migrations/0002_addsessionsettings.cpp
    src/data/migrations/0002_addsessionsettings.hpp
    src/data/migrations/0003_users.cpp
    src/data/migrations/0003_users.hpp
    src/data/models/addtorrentparams.cpp
    src/data/models/addtorrentparams.hpp
    src/data/models/sessionparams.cpp
    src/data/models/sessionparams.hpp
    src/data/models/sessionsettings.cpp
    src/data/models/sessionsettings.hpp
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
    src/methods/torrentsmove.cpp
    src/methods/torrentsmove.hpp
    src/methods/torrentspause.cpp
    src/methods/torrentspause.hpp
    src/methods/torrentspeersadd.cpp
    src/methods/torrentspeersadd.hpp
    src/methods/torrentspeerslist.cpp
    src/methods/torrentspeerslist.hpp
    src/methods/torrentsquery.cpp
    src/methods/torrentsquery.hpp
    src/methods/torrentsremove.cpp
    src/methods/torrentsremove.hpp
    src/methods/torrentsresume.cpp
    src/methods/torrentsresume.hpp
    src/methods/torrentstrackerslist.cpp
    src/methods/torrentstrackerslist.hpp
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
    libzip::zip
    LibtorrentRasterbar::torrent-rasterbar
    unofficial-sodium::sodium
    unofficial-sodium::sodium_config_public
    unofficial::sqlite3::sqlite3
    uriparser::uriparser
)
