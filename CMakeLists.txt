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
# Earliest H3 that reports a version with the public API DgH3System needs
# (constructCell and getIndexDigit, added in 4.4.0; but the 4.4.0 release
# identifies itself as 4.3.0, fixed in 4.4.1)
set(H3_MIN_VERSION 4.4.1)
set(H3_MIN_VERSION_NUM 40401)
if(WITH_H3)
  find_package(h3 ${H3_MIN_VERSION} CONFIG QUIET)
  if(TARGET h3::h3)
    set(H3_FOUND TRUE)
    set(H3_TARGET h3::h3)
    # Some packages (e.g. Homebrew) install the header as <h3/h3api.h> only.
    get_target_property(_h3_inc h3::h3 INTERFACE_INCLUDE_DIRECTORIES)
    find_path(H3_INCLUDE_DIR NAMES h3api.h HINTS ${_h3_inc} PATH_SUFFIXES h3)
    if(H3_INCLUDE_DIR)
      set_property(TARGET h3::h3 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${H3_INCLUDE_DIR}")
    endif()
  else()
    # Some H3 installations provide only the header and library.
    find_path(H3_INCLUDE_DIR NAMES h3api.h PATH_SUFFIXES h3)
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
    # DgH3System needs H3's internal FaceIJK functions (exported by libh3 but
    # not declared in h3api.h) and the public constructCell/getIndexDigit
    # (H3 4.4.0). Check the version and that the symbols link.
    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_LIBRARIES ${H3_TARGET})
    check_cxx_source_compiles("
      #include <h3api.h>
      #if (H3_VERSION_MAJOR*10000 + H3_VERSION_MINOR*100 + H3_VERSION_PATCH) < ${H3_MIN_VERSION_NUM}
      #error H3 is older than ${H3_MIN_VERSION}
      #endif
      int main() { return 0; }" H3_VERSION_OK)
    if(NOT H3_VERSION_OK)
      message(STATUS "H3 found is older than ${H3_MIN_VERSION}; building without H3")
      set(H3_FOUND FALSE)
    else()
      check_cxx_source_compiles("
        #include <h3api.h>
        extern \"C\" {
        typedef struct { int i; int j; int k; } CoordIJK;
        typedef struct { int face; CoordIJK coord; } FaceIJK;
        H3Index _faceIjkToH3 (const FaceIJK* fijk, int res);
        H3Error _h3ToFaceIjk (H3Index h, FaceIJK* fijk);
        }
        int main() {
          FaceIJK f = {0, {0, 0, 0}};
          H3Index h = _faceIjkToH3(&f, 0);
          int d = 0;
          getIndexDigit(h, 1, &d);
          return (int) _h3ToFaceIjk(h, &f) + d;
        }" H3_INTERNALS_OK)
      if(NOT H3_INTERNALS_OK)
        message(STATUS "H3 found but internal FaceIJK symbols are not exported; building without H3")
        set(H3_FOUND FALSE)
      endif()
    endif()
    unset(CMAKE_REQUIRED_LIBRARIES)
    if(NOT H3_FOUND)
      unset(H3_TARGET)
    endif()
  endif()
  if(H3_FOUND)
    message(STATUS "Found H3: ${H3_TARGET}")
  else()
    message(STATUS "Building without external H3")
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

# Optional tests/ directory: nothing is built or registered unless it exists.
#  - If tests/CMakeLists.txt exists it takes over (full control).
#  - Otherwise the contents are discovered at configure time:
#      tests/*.cpp      -> an executable per file, linked against the libs
#      tests/test_*.cpp -> also registered with CTest
#      tests/test_*.py  -> registered with CTest (run with Python3, if found)
#    CONFIGURE_DEPENDS re-runs the glob on build so added files are picked up.
set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
if(IS_DIRECTORY ${TESTS_DIR})
  enable_testing()
  if(EXISTS ${TESTS_DIR}/CMakeLists.txt)
    add_subdirectory(tests)
  else()
    file(GLOB _test_cpps CONFIGURE_DEPENDS ${TESTS_DIR}/*.cpp)
    foreach(_src IN LISTS _test_cpps)
      get_filename_component(_name ${_src} NAME_WE)
      add_executable(${_name} ${_src})
      target_link_libraries(${_name} PRIVATE dglib dgaplib shapelib ${GDAL_LIBRARIES} ${H3_TARGET})
      target_compile_features(${_name} PRIVATE cxx_std_11)
      if(GDAL_FOUND)
        target_compile_definitions(${_name} PRIVATE USE_GDAL)
      endif()
      if(H3_FOUND)
        target_compile_definitions(${_name} PRIVATE USE_H3)
      endif()
      if(_name MATCHES "^test_")
        add_test(NAME ${_name} COMMAND ${_name})
      endif()
    endforeach()

    file(GLOB _test_pys CONFIGURE_DEPENDS ${TESTS_DIR}/test_*.py)
    if(_test_pys)
      find_package(Python3 COMPONENTS Interpreter QUIET)
    endif()
    if(Python3_Interpreter_FOUND)
      foreach(_src IN LISTS _test_pys)
        get_filename_component(_name ${_src} NAME_WE)
        add_test(NAME ${_name}
                 COMMAND ${Python3_EXECUTABLE} ${_src} $<TARGET_FILE:dggrid>
                 WORKING_DIRECTORY ${TESTS_DIR})
      endforeach()
    endif()
  endif()
  message(STATUS "Found tests/ directory; building and registering its tests")
endif()

if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.in @ONLY)
  add_custom_target(docs
    ${DOXYGEN_EXECUTABLE}
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.in
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
endif(DOXYGEN_FOUND)
