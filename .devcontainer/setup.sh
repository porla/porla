#!/usr/bin/env ash

function setup_boost() {
    wget https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz
    tar -xvzf boost_1_84_0.tar.gz
    cd boost_1_84_0 && ./bootstrap.sh && cd ..
}

# add required packages
apk add --no-cache \
    build-base \
    cmake \
    git \
    ninja \
    openssl-dev \
    openssl-libs-static \
    zlib-static

setup_boost
