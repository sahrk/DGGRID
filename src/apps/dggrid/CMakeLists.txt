cmake_minimum_required (VERSION 3.9)

find_package(GDAL)

add_executable(dggrid
  binpres.cpp
  binvals.cpp
  clipper.cpp
  dggrid.cpp
  DgHexSF.cpp
  gridgen.cpp
  param.cpp
  quadclip.cpp
  table.cpp
  transform.cpp
  util.cpp
)

if(GDAL_FOUND)
  target_compile_definitions(dglib PRIVATE USE_GDAL)
endif()

# If GDAL isn't present than ${GDAL_LIBRARIES} gives an empty string
target_link_libraries(dggrid PRIVATE dglib shapelib ${GDAL_LIBRARIES})
target_compile_features(dggrid PRIVATE cxx_std_11)