FROM ghcr.io/porla/build-env:202306091551 AS build-env

WORKDIR /src

COPY . .
COPY ./html/webui.zip /src/build/webui.zip

RUN mkdir -p /src/build

ARG GITVERSION_SEMVER="0.0.0"
ARG CCACHE_REMOTE_STORAGE="http://ccache.blinkenlights.cloud/porla|read-only|connect-timeout=500"

ENV CCACHE_REMOTE_ONLY="1"
ENV CCACHE_REMOTE_STORAGE=${CCACHE_REMOTE_STORAGE}
ENV GITVERSION_SEMVER=${GITVERSION_SEMVER}
ENV VCPKG_FORCE_SYSTEM_BINARIES="1"

RUN apk add --update bash ccache \
    && cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    && cmake --build build \
    && ccache -s -v \
    && strip build/porla*

# -- runtime layer
FROM alpine:3.18.0 AS runtime

ENV PORLA_HTTP_HOST=0.0.0.0
EXPOSE 1337

RUN apk --no-cache add curl bash
WORKDIR /
COPY --from=build-env /src/build/porla* /usr/bin/porla
ENTRYPOINT [ "/usr/bin/porla" ]
