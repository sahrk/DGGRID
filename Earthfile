# Earthfile
VERSION 0.6
FROM debian:bullseye

# configure apt to be noninteractive
ENV DEBIAN_FRONTEND noninteractive
ENV DEBCONF_NONINTERACTIVE_SEEN true

# install dependencies
RUN apt-get update && apt-get install -y build-essential cmake libgdal28 libgdal-dev

WORKDIR /code

code:
  COPY src src
  COPY CMakeLists.txt CMakeLists.txt

build:
  FROM +code
  RUN mkdir build
  RUN cd build && cmake -D CMAKE_BUILD_TYPE=Release ..
  # cache cmake temp files to prevent rebuilding .o files
  # when the .cpp files don't change
  RUN --mount=type=cache,target=/code/build/CMakeFiles cd build && make
  SAVE ARTIFACT build/src/apps/dggrid/dggrid AS LOCAL dggrid

docker:
  COPY +build/dggrid /bin/dggrid
  ENTRYPOINT ["/bin/dggrid"]
  SAVE IMAGE  dggrid:dev
