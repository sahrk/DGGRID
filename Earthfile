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

test:
  FROM +build
  COPY +build/dggrid /bin/dggrid
  COPY examples examples
  RUN cd /code/examples/aigenerate && /bin/dggrid aigenerate.meta > outputfiles/aigenerate.txt
  RUN cd /code/examples/binpres && /bin/dggrid binpres.meta > outputfiles/binpres.txt
  RUN cd /code/examples/binvals && /bin/dggrid binvals.meta > outputfiles/binvals.txt
  RUN cd /code/examples/determineRes && /bin/dggrid determineRes.meta > outputfiles/determineRes.txt
  RUN cd /code/examples/gdalCollection && /bin/dggrid gdalCollection.meta > outputfiles/gdalCollection.txt
  RUN cd /code/examples/gdalExample && /bin/dggrid gdalExample.meta > outputfiles/gdalExample.txt
  RUN cd /code/examples/gridgenCellClip && /bin/dggrid gridgenCellClip.meta > outputfiles/gridgenCellClip.txt
  RUN cd /code/examples/gridgenDiamond && /bin/dggrid gridgenDiamond.meta > outputfiles/gridgenDiamond.txt
  RUN cd /code/examples/gridgenGeoJSON && /bin/dggrid gridgenGeoJSON.meta > outputfiles/gridgenGeoJSON.txt
  RUN cd /code/examples/gridgenMixedSHP && /bin/dggrid gridgenMixedSHP.meta > outputfiles/gridgenMixedSHP.txt
  RUN cd /code/examples/gridgenPureKML && /bin/dggrid gridgenPureKML.meta > outputfiles/gridgenPureKML.txt
  RUN cd /code/examples/hiRes && /bin/dggrid hiRes.meta > outputfiles/hiRes.txt
  RUN cd /code/examples/holes && /bin/dggrid holes.meta > outputfiles/holes.txt
  RUN cd /code/examples/icosahedron && /bin/dggrid icosahedron.meta > outputfiles/icosahedron.txt
  RUN cd /code/examples/isea7hGen && /bin/dggrid isea7hGen.meta > outputfiles/isea7hGen.txt
  RUN cd /code/examples/mixedAperture && /bin/dggrid mixedAperture.meta > outputfiles/mixedAperture.txt
  RUN cd /code/examples/planetRiskGridGen && /bin/dggrid planetRiskGridGen.meta > outputfiles/planetRiskGridGen.txt
  RUN cd /code/examples/planetRiskGridNoWrap && /bin/dggrid planetRiskGridNoWrap.meta > outputfiles/planetRiskGridNoWrap.txt
  RUN cd /code/examples/planetRiskClipHiRes && /bin/dggrid planetRiskClipHiRes.meta > outputfiles/planetRiskClipHiRes.txt
  RUN cd /code/examples/planetRiskClipMulti && /bin/dggrid planetRiskClipMulti.meta > outputfiles/planetRiskClipMulti.txt
  RUN cd /code/examples/planetRiskTable && /bin/dggrid planetRiskTable.meta > outputfiles/planetRiskTable.txt
  RUN cd /code/examples/seqnums && /bin/dggrid seqnums.meta > outputfiles/seqnums.txt
  RUN cd /code/examples/superfundGrid && /bin/dggrid superfundGrid.meta > outputfiles/superfundGrid.txt
  RUN cd /code/examples/table && /bin/dggrid table.meta > outputfiles/table.txt
  RUN cd /code/examples/transform && /bin/dggrid transform.meta > outputfiles/transform.txt


docker:
  COPY +build/dggrid /bin/dggrid
  ENTRYPOINT ["/bin/dggrid"]
  SAVE IMAGE  dggrid:dev
