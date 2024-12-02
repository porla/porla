FROM ghcr.io/porla/alpine:3.19.1 AS base

FROM base AS build-base
ARG GITVERSION_SEMVER="0.0.0"
ARG CCACHE_REMOTE_STORAGE="http://ccache.porla.org/cache|read-only|connect-timeout=500"
ENV CCACHE_REMOTE_ONLY="1"
ENV CCACHE_REMOTE_STORAGE=${CCACHE_REMOTE_STORAGE}
ENV GITVERSION_SEMVER=${GITVERSION_SEMVER}
WORKDIR /src
RUN apk add --no-cache \
    build-base \
    ccache \
    cmake \
    linux-headers \
    ninja \
    openssl-dev \
    openssl-libs-static

# boost
FROM build-base AS build-boost
RUN echo "using gcc : : ccache g++ ;" >> ~/user-config.jam
RUN wget https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz
RUN tar zxf boost_1_86_0.tar.gz
RUN cd boost_1_86_0 && ./bootstrap.sh && ./b2 link=static variant=release install

# libcurl
FROM build-base AS build-curl
RUN wget https://github.com/curl/curl/releases/download/curl-8_11_0/curl-8.11.0.tar.gz
RUN tar zxf curl-8.11.0.tar.gz
RUN cd curl-8.11.0 \
    && cmake -S . -B build -G Ninja \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        -DBUILD_CURL_EXE=OFF \
        -DBUILD_SHARED_LIBS=OFF \
        -DBUILD_STATIC_LIBS=ON \
    && cmake --build build --target install

# libtorrent
FROM build-base AS build-libtorrent
COPY --from=build-boost /usr/local/include/boost /usr/local/include/boost
COPY --from=build-boost /usr/local/lib/cmake /usr/local/lib/cmake
COPY --from=build-boost /usr/local/lib/libboost* /usr/local/lib
RUN wget https://github.com/arvidn/libtorrent/releases/download/v2.0.10/libtorrent-rasterbar-2.0.10.tar.gz
RUN tar zxf libtorrent-rasterbar-2.0.10.tar.gz
RUN cd libtorrent-rasterbar-2.0.10 \
    && cmake -S . -B build -G Ninja \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_EXE_LINKER_FLAGS="-static" \
    && cmake --build build --target install

# libgit2
FROM build-base AS build-libgit2
RUN wget -O libgit2-1.8.4.tar.gz https://github.com/libgit2/libgit2/archive/refs/tags/v1.8.4.tar.gz
RUN tar zxf libgit2-1.8.4.tar.gz
RUN cd libgit2-1.8.4 \
    && cmake -S . -B build -G Ninja \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        -DBUILD_SHARED_LIBS=OFF \
        -DBUILD_TESTS=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_EXE_LINKER_FLAGS="-static" \
        -DLINK_WITH_STATIC_LIBRARIES=ON \
        -DUSE_NTLMCLIENT=OFF \
    && cmake --build build --target install

# lua
FROM build-base AS build-lua
RUN wget https://www.lua.org/ftp/lua-5.4.6.tar.gz
RUN tar zxf lua-5.4.6.tar.gz
RUN cd lua-5.4.6 && make CC="ccache gcc" all && make install

FROM build-base AS build-porla
# boost
COPY --from=build-boost /usr/local/include/boost /usr/local/include/boost
COPY --from=build-boost /usr/local/lib/cmake /usr/local/lib/cmake
COPY --from=build-boost /usr/local/lib/libboost* /usr/local/lib
# libcurl
COPY --from=build-curl /usr/local/include/curl /usr/local/include/curl
COPY --from=build-curl /usr/local/lib/cmake /usr/local/lib/cmake
COPY --from=build-curl /usr/local/lib/libcurl* /usr/local/lib
# libgit2
COPY --from=build-libgit2 /usr/local/include/git2 /usr/local/include/git2
COPY --from=build-libgit2 /usr/local/include/git2.h /usr/local/include/
COPY --from=build-libgit2 /usr/local/lib/libgit2* /usr/local/lib
COPY --from=build-libgit2 /usr/local/lib/pkgconfig/libgit2.pc /usr/local/lib/pkgconfig/libgit2.pc
# libtorrent
COPY --from=build-libtorrent /usr/local/include/libtorrent /usr/local/include/libtorrent
COPY --from=build-libtorrent /usr/local/lib/cmake /usr/local/lib/cmake
COPY --from=build-libtorrent /usr/local/lib/libtorrent* /usr/local/lib
# lua
COPY --from=build-lua /usr/local/include/* /usr/local/include/
COPY --from=build-lua /usr/local/lib/liblua* /usr/local/lib
COPY . .

RUN echo "@edge-main https://dl-cdn.alpinelinux.org/alpine/edge/main" >> /etc/apk/repositories

RUN apk add --no-cache \
    git \
    libsodium-dev@edge-main \
    libsodium-static@edge-main \
    sqlite-dev \
    sqlite-static \
    zlib-dev \
    zlib-static

RUN cmake -S . -B build -G Ninja \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXE_LINKER_FLAGS="-static -Os" \
    -DBUILD_SHARED_LIBS=OFF \
    -DLINK_WITH_STATIC_LIBRARIES=ON \
    -DOPENSSL_USE_STATIC_LIBS=TRUE \
    -DURIPARSER_SHARED_LIBS=OFF

RUN cmake --build build

# runtime image
FROM base AS runtime

ENV PORLA_HTTP_HOST=0.0.0.0
EXPOSE 1337

RUN apk --no-cache add curl bash
WORKDIR /
COPY --from=build-porla /src/build/porla /usr/bin/porla
ENTRYPOINT [ "/usr/bin/porla" ]
