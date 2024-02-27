FROM ghcr.io/porla/alpine:3.19.1 AS build-env

WORKDIR /src

COPY . .

RUN ./scripts/setup-env.sh
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="-static -Os"
RUN cmake --build build

FROM ghcr.io/porla/alpine:3.19.1 AS runtime

ENV PORLA_HTTP_HOST=0.0.0.0
EXPOSE 1337

RUN apk --no-cache add curl bash
WORKDIR /
COPY --from=build-env /src/build/porla /usr/bin/porla
ENTRYPOINT [ "/usr/bin/porla" ]
