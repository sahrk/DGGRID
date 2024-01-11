cmake_minimum_required (VERSION 3.9)

project (dggrid
  VERSION 8.1
  LANGUAGES C CXX
)

find_package(Doxygen)

#Adds aggressive warnings to all compilation
if(MSVC)
  # add_compile_options(/W4 /WX)
  add_compile_options(/W2)
else()
  add_compile_options(-Wall -pedantic)
endif()


option(WITH_GDAL "Build with external GDAL" ON)
option(WITH_EXT_SHAPELIB "Build with external SHAPELIB" OFF)

if(WITH_GDAL)
    find_package(GDAL)
endif()

if(WITH_EXT_SHAPELIB)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/src/lib/shapelib_ext)
  find_package(shapelib REQUIRED)
else()
  add_subdirectory(src/lib/shapelib)
endif()

add_subdirectory(src/lib/dglib)
add_subdirectory(src/lib/dgaplib)
add_subdirectory(src/lib/proj4lib)
# add_subdirectory(src/lib/shapelib)

add_subdirectory(src/apps/dggrid)
add_subdirectory(src/apps/appex)

if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.in @ONLY)
  add_custom_target(docs
    ${DOXYGEN_EXECUTABLE}
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.in
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
endif(DOXYGEN_FOUND)
