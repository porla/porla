cmake_minimum_required(VERSION 3.12)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(porla)

find_package(LibtorrentRasterbar CONFIG REQUIRED)

add_executable(
    ${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    LibtorrentRasterbar::torrent-rasterbar
)
