FROM alpine:3.16.2 AS build-env
WORKDIR /src
COPY . .
ENV VCPKG_FORCE_SYSTEM_BINARIES="1"
RUN apk add --update --no-cache build-base cmake curl git ninja unzip zip \
    && mkdir build \
    && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-linux-release -G Ninja .. \
    && ninja porla
