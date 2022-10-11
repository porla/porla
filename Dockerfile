FROM alpine:3.16.2 AS build-env
WORKDIR /src
COPY . .
ENV VCPKG_FORCE_SYSTEM_BINARIES="1"
RUN apk add --update --no-cache build-base cmake curl git libc6-compat linux-headers perl pkgconfig python3 unzip zip \
    && wget https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip \
    && unzip ninja-linux.zip -d /usr/bin \
    && chmod +x /usr/bin/ninja \
    && mkdir build \
    && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-linux-release -G Ninja .. \
    && ninja porla

FROM alpine:3.16.2 AS runtime
WORKDIR /
RUN apk add --update --no-cache libstdc++
COPY --from=build-env /src/build/porla /usr/bin/porla
ENTRYPOINT [ "/usr/bin/porla" ]
