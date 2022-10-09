cmake_minimum_required(VERSION 3.12)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(porla)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost                      REQUIRED COMPONENTS log program_options system)
#find_package(GTest               CONFIG REQUIRED)
find_package(LibtorrentRasterbar CONFIG REQUIRED)
find_package(nlohmann_json       CONFIG REQUIRED)
find_package(tomlplusplus        CONFIG REQUIRED)
find_package(unofficial-sqlite3  CONFIG REQUIRED)

add_executable(
    ${PROJECT_NAME}
    src/config.cpp
    src/config.hpp
    src/main.cpp
    src/httpcontext.hpp
    src/httpeventstream.cpp
    src/httpeventstream.hpp
    src/httpmiddleware.hpp
    src/httpserver.cpp
    src/httpserver.hpp
    src/httpsession.cpp
    src/httpsession.hpp
    src/session.cpp
    src/session.hpp

    src/data/migrate.cpp
    src/data/migrate.hpp
    src/data/migrations/0001_initialsetup.cpp
    src/data/migrations/0001_initialsetup.hpp
    src/data/models/addtorrentparams.cpp
    src/data/models/addtorrentparams.hpp
    src/data/models/sessionparams.cpp
    src/data/models/sessionparams.hpp
    src/data/statement.cpp
    src/data/statement.hpp

    src/methods/method.hpp
    src/methods/torrentsadd.cpp
    src/methods/torrentsadd.hpp
    src/methods/torrentslist.cpp
    src/methods/torrentslist.hpp
    src/methods/torrentsquery.cpp
    src/methods/torrentsquery.hpp
    src/methods/torrentsremove.cpp
    src/methods/torrentsremove.hpp
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    -DSQLITE_CORE
)

target_link_libraries(
    ${PROJECT_NAME}
    Boost::boost
    Boost::log
    Boost::program_options
    LibtorrentRasterbar::torrent-rasterbar
    unofficial::sqlite3::sqlite3
)
