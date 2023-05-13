# Earthfile
VERSION 0.7
# FROM debian:bullseye
# debian libgdal28
FROM ubuntu:jammy
# ubuntu libgdal30

# configure apt to be noninteractive
ENV DEBIAN_FRONTEND noninteractive
ENV DEBCONF_NONINTERACTIVE_SEEN true

# install dependencies
RUN apt-get update && apt-get install -y libgdal30

WORKDIR /code

code:
  COPY src src
  COPY CMakeLists.txt CMakeLists.txt

build:
  FROM +code
  RUN mkdir build
  RUN apt-get install -y build-essential cmake libgdal-dev
  RUN cd build && cmake -D CMAKE_BUILD_TYPE=Release ..
  # cache cmake temp files to prevent rebuilding .o files
  # when the .cpp files don't change
  RUN --mount=type=cache,target=/code/build/CMakeFiles cd build && make
  SAVE ARTIFACT build/src/apps/dggrid/dggrid AS LOCAL dggrid


test:
  FROM +build
  COPY +build/dggrid /bin/dggrid
  COPY examples examples
  RUN cd /code/examples/aigenerate && /bin/dggrid aigenerate.meta | tee -a outputfiles/aigenerate.txt
  RUN cd /code/examples/binpres && /bin/dggrid binpres.meta | tee -a outputfiles/binpres.txt
  RUN cd /code/examples/binvals && /bin/dggrid binvals.meta | tee -a outputfiles/binvals.txt
  RUN cd /code/examples/determineRes && /bin/dggrid determineRes.meta | tee -a outputfiles/determineRes.txt
  RUN cd /code/examples/gdalCollection && /bin/dggrid gdalCollection.meta | tee -a outputfiles/gdalCollection.txt
  RUN cd /code/examples/gdalExample && /bin/dggrid gdalExample.meta | tee -a outputfiles/gdalExample.txt
  RUN cd /code/examples/gridgenCellClip && /bin/dggrid gridgenCellClip.meta | tee -a outputfiles/gridgenCellClip.txt
  RUN cd /code/examples/gridgenDiamond && /bin/dggrid gridgenDiamond.meta | tee -a outputfiles/gridgenDiamond.txt
  RUN cd /code/examples/gridgenGeoJSON && /bin/dggrid gridgenGeoJSON.meta | tee -a outputfiles/gridgenGeoJSON.txt
  RUN cd /code/examples/gridgenMixedSHP && /bin/dggrid gridgenMixedSHP.meta | tee -a outputfiles/gridgenMixedSHP.txt
  RUN cd /code/examples/gridgenPureKML && /bin/dggrid gridgenPureKML.meta | tee -a outputfiles/gridgenPureKML.txt
  RUN cd /code/examples/hiRes && /bin/dggrid hiRes.meta | tee -a outputfiles/hiRes.txt
  RUN cd /code/examples/holes && /bin/dggrid holes.meta | tee -a outputfiles/holes.txt
  RUN cd /code/examples/icosahedron && /bin/dggrid icosahedron.meta | tee -a outputfiles/icosahedron.txt
  RUN cd /code/examples/isea4d && /bin/dggrid isea4d.meta | tee -a outputfiles/isea4d.txt 
  RUN cd /code/examples/isea4t && /bin/dggrid isea4t.meta | tee -a outputfiles/isea4t.txt
  RUN cd /code/examples/isea7hGen && /bin/dggrid isea7hGen.meta | tee -a outputfiles/isea7hGen.txt
  RUN cd /code/examples/mixedAperture && /bin/dggrid mixedAperture.meta | tee -a outputfiles/mixedAperture.txt
  RUN cd /code/examples/planetRiskGridGen && /bin/dggrid planetRiskGridGen.meta | tee -a outputfiles/planetRiskGridGen.txt
  RUN cd /code/examples/planetRiskGridNoWrap && /bin/dggrid planetRiskGridNoWrap.meta | tee -a outputfiles/planetRiskGridNoWrap.txt
  RUN cd /code/examples/planetRiskClipHiRes && /bin/dggrid planetRiskClipHiRes.meta | tee -a outputfiles/planetRiskClipHiRes.txt
  RUN cd /code/examples/planetRiskClipMulti && /bin/dggrid planetRiskClipMulti.meta | tee -a outputfiles/planetRiskClipMulti.txt
  RUN cd /code/examples/planetRiskTable && /bin/dggrid planetRiskTable.meta | tee -a outputfiles/planetRiskTable.txt
  RUN cd /code/examples/seqnums && /bin/dggrid seqnums.meta | tee -a outputfiles/seqnums.txt
  RUN cd /code/examples/superfundGrid && /bin/dggrid superfundGrid.meta | tee -a outputfiles/superfundGrid.txt
  RUN cd /code/examples/table && /bin/dggrid table.meta | tee -a outputfiles/table.txt
  RUN cd /code/examples/transform && /bin/dggrid transform.meta | tee -a outputfiles/transform.txt
  RUN cd /code/examples/z3Collection && /bin/dggrid z3Collection.meta | tee -a outputfiles/z3Collection.txt
  RUN cd /code/examples/z3Nums && /bin/dggrid z3Nums.meta | tee -a outputfiles/z3Nums.txt
  RUN cd /code/examples/z3Transform && /bin/dggrid z3Transform.meta | tee -a outputfiles/z3Transform.txt
  RUN cd /code/examples/z3CellClip && /bin/dggrid z3CellClip.meta | tee -a outputfiles/z3CellClip.txt
  RUN cd /code/examples/z3WholeEarth && /bin/dggrid z3WholeEarth.meta | tee -a outputfiles/z3WholeEarth.txt
  RUN cd /code/examples/zCellClip && /bin/dggrid zCellClip.meta | tee -a outputfiles/zCellClip.txt
  RUN cd /code/examples/zCollection && /bin/dggrid zCollection.meta | tee -a outputfiles/zCollection.txt
  RUN cd /code/examples/zNums && /bin/dggrid zNums.meta | tee -a outputfiles/zNums.txt
  RUN cd /code/examples/zTransform && /bin/dggrid zTransform.meta | tee -a outputfiles/zTransform.txt


docker-local:
  COPY +build/dggrid /bin/dggrid
  ENTRYPOINT ["/bin/dggrid"]
  SAVE IMAGE  dggrid:latest

docker-github:
  ARG imagetag
  COPY +build/dggrid /bin/dggrid
  ENTRYPOINT ["/bin/dggrid"]
  RUN echo "trying to push to $imagetag"
  SAVE IMAGE --push ${imagetag}

