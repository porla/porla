FROM alpine:edge AS base

FROM base AS build-base
ARG GITVERSION_SEMVER="0.0.0"
ARG CCACHE_REMOTE_STORAGE="http://ccache.porla.org/cache|read-only|connect-timeout=500"
ENV CCACHE_REMOTE_ONLY="1"
ENV CCACHE_REMOTE_STORAGE=${CCACHE_REMOTE_STORAGE}
ENV GITVERSION_SEMVER=${GITVERSION_SEMVER}
WORKDIR /src
RUN apk add --no-cache \
    build-base \
    boost1.84-dev \
    boost1.84-static \
    ccache \
    cmake \
    linux-headers \
    ninja \
    openssl-dev \
    openssl-libs-static \
    zlib-dev \
    zlib-static \
    zstd-dev \
    zstd-static \
    git \
    icu-static \
    libsodium-dev \
    libsodium-static \
    libtorrent-rasterbar-dev \
    libtorrent-rasterbar-static \
    lua5.4-dev \
    sqlite-dev \
    sqlite-static

# antlr4
FROM build-base AS build-antlr4
RUN wget -O antlr4-4.13.2.tar.gz https://github.com/antlr/antlr4/archive/refs/tags/4.13.2.tar.gz
RUN tar zxf antlr4-4.13.2.tar.gz
RUN cd antlr4-4.13.2/runtime/Cpp \
    && cmake -S . -B build -G Ninja \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --target install

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
        -DLINK_WITH_STATIC_LIBRARIES=ON \
        -DUSE_NTLMCLIENT=OFF \
    && cmake --build build --target install

# libzip
FROM build-base AS build-libzip
RUN wget https://github.com/nih-at/libzip/releases/download/v1.11.2/libzip-1.11.2.tar.gz
RUN tar zxf libzip-1.11.2.tar.gz
RUN cd libzip-1.11.2 \
    && cmake -S . -B build -G Ninja \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        -DBUILD_SHARED_LIBS=OFF \
        -DBUILD_TOOLS=OFF \
        -DBUILD_DOCS=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DENABLE_BZIP2=OFF \
        -DENABLE_LZMA=OFF \
    && cmake --build build --target install

# uWebSockets
FROM build-base AS build-uwebsockets
RUN wget -O uSockets-0.8.8.tar.gz https://github.com/uNetworking/uSockets/archive/refs/tags/v0.8.8.tar.gz
RUN tar zxf uSockets-0.8.8.tar.gz
RUN cd uSockets-0.8.8 \
    && WITH_ASIO=1 WITH_OPENSSL=1 make CC="ccache gcc"
RUN wget -O uWebSockets-20.70.0.tar.gz https://github.com/uNetworking/uWebSockets/archive/refs/tags/v20.70.0.tar.gz
RUN tar zxf uWebSockets-20.70.0.tar.gz

FROM build-base AS build-porla
# antlr4
COPY --from=build-antlr4 /usr/local/include/antlr4-runtime /usr/local/include/antlr4-runtime
COPY --from=build-antlr4 /usr/local/lib/libantlr4* /usr/local/lib
# libcurl
COPY --from=build-curl /usr/local/include/curl /usr/local/include/curl
COPY --from=build-curl /usr/local/lib/cmake /usr/local/lib/cmake
COPY --from=build-curl /usr/local/lib/libcurl* /usr/local/lib
# libgit2
COPY --from=build-libgit2 /usr/local/include/git2 /usr/local/include/git2
COPY --from=build-libgit2 /usr/local/include/git2.h /usr/local/include/
COPY --from=build-libgit2 /usr/local/lib/libgit2* /usr/local/lib
COPY --from=build-libgit2 /usr/local/lib/pkgconfig/libgit2.pc /usr/local/lib/pkgconfig/libgit2.pc
# libzip
COPY --from=build-libzip /usr/local/include/* /usr/local/include/
COPY --from=build-libzip /usr/local/lib/cmake /usr/local/lib/cmake
COPY --from=build-libzip /usr/local/lib/libzip* /usr/local/lib
# uwebsockets
COPY --from=build-uwebsockets /src/uSockets-0.8.8/src/libusockets.h /usr/local/include/libusockets.h
COPY --from=build-uwebsockets /src/uSockets-0.8.8/uSockets.a /usr/local/lib/libuSockets.a
COPY --from=build-uwebsockets /src/uWebSockets-20.70.0/src/* /usr/local/include/uWebSockets/

COPY . .

RUN cmake -S . -B build -G Ninja \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXE_LINKER_FLAGS="-static -Os" \
    -DBUILD_SHARED_LIBS=OFF \
    -DLINK_WITH_STATIC_LIBRARIES=ON \
    -DOPENSSL_USE_STATIC_LIBS=TRUE \
    # boost
    -DBoost_USE_STATIC_LIBS=ON \
    # libzip
    -DENABLE_BZIP2=OFF \
    -DENABLE_LZMA=OFF

RUN cmake --build build

# runtime image
FROM base AS runtime

ENV PORLA_HTTP_HOST=0.0.0.0
EXPOSE 1337

RUN apk --no-cache add curl bash
WORKDIR /
COPY --from=build-porla /src/build/porla /usr/bin/porla
ENTRYPOINT [ "/usr/bin/porla" ]
