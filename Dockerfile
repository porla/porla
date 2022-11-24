FROM ghcr.io/porla/build-env:202211250011 AS build-env

WORKDIR /src

COPY . .
COPY ./html/webui.zip /src/build/webui.zip

RUN mkdir -p /src/build

ENV CCACHE_REMOTE_ONLY="1"
ENV CCACHE_REMOTE_STORAGE="http://ccache.blinkenlights.cloud/porla|read-only"
ENV VCPKG_FORCE_SYSTEM_BINARIES="1"

ARG GITVERSION_SEMVER="0.0.0"

RUN apk add --update ccache \
    && export GIVERSION_SEMVER=$GITVERSION_SEMVER \
    && cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    && cmake --build build \
    && ccache -s -v \
    && strip build/porla

# -- runtime layer
FROM alpine:3.17.0 AS runtime
WORKDIR /
COPY --from=build-env /src/build/porla /usr/bin/porla
ENTRYPOINT [ "/usr/bin/porla" ]
