#!/usr/bin/env ash

function setup_boost() {
    wget https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz
    tar zxf boost_1_84_0.tar.gz

    cd boost_1_84_0 \
        && ./bootstrap.sh \
        && ./b2 link=static variant=release install \
        && cd ..
}

function setup_libtorrent() {
    wget https://github.com/arvidn/libtorrent/releases/download/v2.0.10/libtorrent-rasterbar-2.0.10.tar.gz
    tar zxf libtorrent-rasterbar-2.0.10.tar.gz

    cd libtorrent-rasterbar-2.0.10 \
        && cmake -S . -B build -G Ninja -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="-static" \
        && cmake --build build --target install \
        && cd ..
}

function setup_lua() {
    wget https://www.lua.org/ftp/lua-5.4.6.tar.gz
    tar zxf lua-5.4.6.tar.gz
    cd lua-5.4.6
    make all
    make install
}

# add required packages

echo "@edge-main https://dl-cdn.alpinelinux.org/alpine/edge/main" >> /etc/apk/repositories

apk add --no-cache \
    build-base \
    cmake \
    git \
    linux-headers \
    ninja \
    openssl-dev \
    openssl-libs-static \
    libsodium-dev@edge-main \
    libsodium-static@edge-main \
    sqlite-dev \
    sqlite-static \
    zlib-dev \
    zlib-static

cd /tmp

setup_boost
setup_libtorrent
setup_lua
