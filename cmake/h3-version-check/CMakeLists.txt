# h3-version-check: developer tool that checks DGGRID's minimum H3 version.
#
# For each version in H3_TEST_VERSIONS this builds that version of the H3
# library as a static library from its GitHub source release, builds DGGRID
# against it in a separate build directory, and runs the checks in check.cmake:
#   - a version at least H3_MIN_VERSION (4.4.1; constructCell and getIndexDigit,
#     which DgH3System uses, are in 4.4.0, but that release reports its
#     version as 4.3.0) must give "built with H3 version
#     <that version>" and run the H3 examples (examples/examplesH3.lst) with
#     output identical to examples/sampleOutput;
#   - an older version must give "built without H3";
#   - DGGRID must not link a shared libh3 (so it cannot pick up another H3,
#     such as one installed by a package manager, at run time);
#   - the examples that do not need H3 but use the H3 grid and the GNOMONIC
#     projection (gnomonicGen, h3Geom) must also match sampleOutput.
#
# This is not part of the default DGGRID build and needs network access (the
# H3 source is downloaded) and about 10 minutes. The data files in
# examples/sampleOutput are compared exactly; they were created on macOS, and
# rounding differences on another platform can show up as failures.
#
# usage:
#   cmake -S cmake/h3-version-check -B build-h3check
#   cmake --build build-h3check --target check
#
# options:
#   -DH3_TEST_VERSIONS="4.4.1;4.4.0;4.5.0"   the H3 versions to test (default:
#       the minimum, the release before it, and the newest tested release)
#   -DH3_MIN_VERSION=4.4.1                   the version that must work; must
#       match H3_MIN_VERSION in the top-level CMakeLists.txt and in
#       src/MakeIncludes.noCMake

cmake_minimum_required(VERSION 3.20)
project(h3-version-check NONE)

include(ExternalProject)

set(H3_MIN_VERSION 4.4.1 CACHE STRING "Minimum H3 version DGGRID supports")
set(H3_TEST_VERSIONS "4.4.1;4.4.0;4.5.0" CACHE STRING "H3 versions to test")

# the 4.4.0 release has the API but reports itself as 4.3.0 (fixed in 4.4.1),
# so DGGRID treats it as too old
set(H3_REPORTED_4.4.0 4.3.0)

# SHA256 of the GitHub source archives
# (https://github.com/uber/h3/archive/refs/tags/v<version>.tar.gz)
set(H3_SHA256_4.3.0 a47cfa36e255e4bf16c63015aaff8e6fe2afc14ffaa3f6b281718158446c0e9b)
set(H3_SHA256_4.4.0 b88de0212058812f560c21517e62cd6b2d146f5382b9cc1316bd45c1a3334239)
set(H3_SHA256_4.4.1 9df719eb878f218c203e424dc5ffca9b98eca4d78ba83928773987649ead404d)
set(H3_SHA256_4.5.0 0da8a392a6ff77e76b60e6a331a49497d0935b6b7b6899da7a3e2786139b0441)

get_filename_component(DGGRID_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(check_targets)
foreach(v IN LISTS H3_TEST_VERSIONS)
  if(NOT DEFINED H3_SHA256_${v})
    message(FATAL_ERROR "No SHA256 known for H3 ${v}; add H3_SHA256_${v} to ${CMAKE_CURRENT_LIST_FILE}")
  endif()

  set(h3_prefix ${CMAKE_BINARY_DIR}/h3-${v})

  # a static library guarantees the internal symbols are present and stops
  # another installed libh3 from being loaded at run time
  ExternalProject_Add(h3_${v}
    URL https://github.com/uber/h3/archive/refs/tags/v${v}.tar.gz
    URL_HASH SHA256=${H3_SHA256_${v}}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=Release
      -DCMAKE_INSTALL_PREFIX=${h3_prefix}
      -DBUILD_SHARED_LIBS=OFF
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
      -DBUILD_TESTING=OFF
      -DBUILD_BENCHMARKS=OFF
      -DBUILD_FUZZERS=OFF
      -DBUILD_FILTERS=OFF
      -DBUILD_GENERATORS=OFF
      -DENABLE_DOCS=OFF
      -DENABLE_FORMAT=OFF
      -DENABLE_LINTING=OFF
    BINARY_DIR ${CMAKE_BINARY_DIR}/h3-${v}-build
  )

  # DGGRID, pointed at this H3 only (GDAL and the rest still come from the
  # system, so /opt/homebrew etc. is not ignored; the H3 locations are given
  # explicitly so another H3 cannot be found)
  set(dggrid_bin ${CMAKE_BINARY_DIR}/dggrid-${v})
  ExternalProject_Add(dggrid_${v}
    SOURCE_DIR ${DGGRID_ROOT}
    BINARY_DIR ${dggrid_bin}
    DEPENDS h3_${v}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=Release
      -DWITH_H3=ON
      -Dh3_DIR=${h3_prefix}/lib/cmake/h3
      -DH3_INCLUDE_DIR=${h3_prefix}/include/h3
      -DH3_LIBRARY=${h3_prefix}/lib/libh3.a
    INSTALL_COMMAND ""
    BUILD_ALWAYS TRUE
    # DGGRID's own source tree is the source here; nothing to update
    UPDATE_COMMAND ""
  )

  # an H3 older than the minimum must give a DGGRID built without H3
  if(DEFINED H3_REPORTED_${v})
    set(reported ${H3_REPORTED_${v}})
  else()
    set(reported ${v})
  endif()
  if("${reported}" VERSION_LESS "${H3_MIN_VERSION}")
    set(expect none)
  else()
    set(expect ${v})
  endif()

  ExternalProject_Add_Step(dggrid_${v} check
    DEPENDEES build
    ALWAYS TRUE
    COMMAND ${CMAKE_COMMAND}
      -DDGGRID=${dggrid_bin}/src/apps/dggrid/dggrid
      -DEXPECT_H3=${expect}
      -DEXAMPLES=${DGGRID_ROOT}/examples
      -DWORK=${CMAKE_BINARY_DIR}/work-${v}
      -P ${CMAKE_CURRENT_LIST_DIR}/check.cmake
    COMMENT "Checking DGGRID built against H3 ${v} (expecting ${expect})"
    USES_TERMINAL TRUE
  )
  ExternalProject_Add_StepTargets(dggrid_${v} check)
  list(APPEND check_targets dggrid_${v}-check)
endforeach()

# run the checks in order, so output stays readable
add_custom_target(check DEPENDS ${check_targets}
  COMMENT "H3 version checks complete")
