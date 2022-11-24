FROM ghcr.io/porla/build-env:202211091940 AS build-env

WORKDIR /src
COPY . .

RUN mkdir -p /src/build

ENV CCACHE_DEBUG=1
ENV CCACHE_REMOTE_STORAGE=http://ccache.blinkenlights.cloud
ENV VCPKG_FORCE_SYSTEM_BINARIES="1"

ARG GITVERSION_SEMVER="0.0.0"

RUN apk add --update ccache \
    && cd build \
    && cmake -DCMAKE_CXX_COMPILER_LAUNCHER=ccache --build . \
    && GIVERSION_SEMVER=$GITVERSION_SEMVER ninja porla \
    && strip porla

# -- runtime layer
FROM alpine:3.16.2 AS runtime
WORKDIR /
COPY --from=build-env /src/build/porla /usr/bin/porla
ENTRYPOINT [ "/usr/bin/porla" ]
