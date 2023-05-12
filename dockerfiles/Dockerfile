# FROM debian:bullseye AS base
# debian libgdal28
FROM ubuntu:jammy AS base
# ubuntu libgdal30

RUN set -ex;         \
    apt-get update && apt-get install -y libgdal30

# The first stage will install build dependencies on top of the
# runtime dependencies, and then compile

FROM base AS builder

RUN set -ex;                                                                      \
    apt-get update && apt-get install -y g++ curl cmake libgdal-dev ;                                \
    mkdir -p /usr/src;                                                            \
    cd /usr/src;                                                                  \
    curl -L https://github.com/sahrk/DGGRID/archive/refs/tags/v7.8.tar.gz | tar -zxf -;  \
    cd DGGRID-7.8;                                                     \
    mkdir build; \
    cd build; \
    cmake -D CMAKE_BUILD_TYPE=Release .. ; make; ls -ltr src/apps/dggrid; cp src/apps/dggrid/dggrid /usr/local/bin/dggrid


# The second stage will already contain all dependencies, just copy
# the compiled executables

FROM base AS runtime

COPY --from=builder /usr/local/bin/dggrid /usr/local/bin/dggrid
