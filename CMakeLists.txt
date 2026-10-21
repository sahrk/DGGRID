cmake_minimum_required (VERSION 3.20)

project (dggrid
  VERSION 9.1
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
option(WITH_H3 "Build with external H3 when available" ON)
option(WITH_EXT_SHAPELIB "Build with external SHAPELIB" OFF)

if(WITH_GDAL)
    find_package(GDAL)
endif()

set(H3_FOUND FALSE)
if(WITH_H3)
  find_package(h3 CONFIG QUIET)
  if(TARGET h3::h3)
    set(H3_FOUND TRUE)
    set(H3_TARGET h3::h3)
  else()
    # Some H3 installations provide only the header and library.
    find_path(H3_INCLUDE_DIR NAMES h3api.h)
    find_library(H3_LIBRARY NAMES h3)
    if(H3_INCLUDE_DIR AND H3_LIBRARY)
      add_library(H3::H3 UNKNOWN IMPORTED)
      set_target_properties(H3::H3 PROPERTIES
        IMPORTED_LOCATION "${H3_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${H3_INCLUDE_DIR}")
      set(H3_FOUND TRUE)
      set(H3_TARGET H3::H3)
    endif()
  endif()
  if(H3_FOUND)
    message(STATUS "Found H3: ${H3_TARGET}")
  else()
    message(STATUS "H3 not found; building without external H3")
  endif()
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
