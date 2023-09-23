cmake_minimum_required (VERSION 3.9)

add_library(dgaplib
  lib/DgApParamList.cpp
  lib/DgApSubOperation.cpp
)

if(GDAL_FOUND)
  target_compile_definitions(dgaplib PRIVATE USE_GDAL)
endif()

# If GDAL isn't present than ${GDAL_INCLUDE_DIRS} and ${GDAL_LIBRARIES} give
# empty strings
target_include_directories(dgaplib PUBLIC include ../dglib/include ${GDAL_INCLUDE_DIRS})
target_link_libraries(dgaplib PRIVATE ${GDAL_LIBRARIES} shapelib proj4lib)
target_compile_features(dgaplib PRIVATE cxx_std_11)
