cmake_minimum_required (VERSION 3.20)

project (dggrid
  VERSION 9.0
  LANGUAGES C CXX
)

# Default to an optimized build when no build type is given (single-config
# generators only; an explicit -DCMAKE_BUILD_TYPE=... still wins, and
# multi-config generators such as Xcode and MSVC are unaffected).
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  message(STATUS "No build type specified; defaulting to Release")
endif()

find_package(Doxygen)

#Adds aggressive warnings to all compilation
if(MSVC)
  # add_compile_options(/W4 /WX)
  add_compile_options(/W2)
  add_compile_definitions(_USE_MATH_DEFINES)
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
