FROM alpine:3.16.2 AS build-env
WORKDIR /src
COPY . .
ENV VCPKG_FORCE_SYSTEM_BINARIES="1"
RUN apk add --update --no-cache cmake curl g++ git libc6-compat linux-headers make perl pkgconfig python3 unzip zip \
    && wget https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip \
    && unzip ninja-linux.zip -d /usr/bin \
    && chmod +x /usr/bin/ninja \
    && mkdir build \
    && cd build \
    && cmake -DCMAKE_EXE_LINKER_FLAGS="-static -Os" -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-linux-musl-release -G Ninja .. \
    && ninja porla \
    && strip porla

# -- runtime layer
FROM alpine:3.16.2 AS runtime
WORKDIR /
COPY --from=build-env /src/build/porla /usr/bin/porla
ENTRYPOINT [ "/usr/bin/porla" ]
