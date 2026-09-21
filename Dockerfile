FROM mirror.gcr.io/library/alpine:3.24.2 AS base

FROM base AS build-porla

ARG GITVERSION_SEMVER="0.0.0"
ENV GITVERSION_SEMVER=${GITVERSION_SEMVER}

WORKDIR /src

RUN apk add --no-cache \
    boost1.84-dev \
    boost1.84-static \
    brotli-static \
    build-base \
    curl-dev \
    curl-static \
    cmake \
    git \
    icu-static \
    libmaxminddb-dev \
    libmaxminddb-static \
    libidn2-static \
    libpsl-static \
    libsodium-dev \
    libsodium-static \
    libunistring-static \
    linux-headers \
    lua5.4-dev \
    nghttp2-dev \
    nghttp2-static \
    ninja \
    openssl-dev \
    openssl-libs-static \
    sqlite-dev \
    sqlite-static \
    zlib-dev \
    zlib-static \
    zstd-dev \
    zstd-static

COPY . .

RUN cmake --preset alpine-static \
    && cmake --build --preset alpine-static

# runtime image
FROM base AS runtime

ENV PORLA_HTTP_HOST=0.0.0.0
EXPOSE 1337

WORKDIR /
COPY --from=build-porla /src/build/release-static/porla /usr/bin/porla
ENTRYPOINT [ "/usr/bin/porla" ]
