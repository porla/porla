FROM ghcr.io/porla/build-env:202211091940 AS build-env
WORKDIR /src
COPY . .
RUN mkdir -p /src/build
COPY ./html/webui.zip /src/build/webui.zip
ARG GITVERSION_SEMVER="0.0.0"
ENV VCPKG_FORCE_SYSTEM_BINARIES="1"
RUN cd build && GIVERSION_SEMVER=$GITVERSION_SEMVER ninja porla && strip porla

# -- runtime layer
FROM alpine:3.16.2 AS runtime
WORKDIR /
COPY --from=build-env /src/build/porla /usr/bin/porla
ENTRYPOINT [ "/usr/bin/porla" ]
