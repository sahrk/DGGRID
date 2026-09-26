cmake_minimum_required (VERSION 3.20)

project (dggrid
  VERSION 9.0
  LANGUAGES C CXX
)

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

include(CTest)
if(BUILD_TESTING)
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  add_test(NAME geographic_modes
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/test_geographic_modes.py
            $<TARGET_FILE:dggrid>)
  set(geographic_output_test_options)
  if(GDAL_FOUND)
    list(APPEND geographic_output_test_options --gdal)
  endif()
  add_test(NAME geographic_output
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/test_geographic_output.py
            $<TARGET_FILE:dggrid> ${geographic_output_test_options})
  add_executable(test_authalic_converter tests/core/test_authalic_converter.cpp)
  target_link_libraries(test_authalic_converter PRIVATE dglib)
  target_compile_features(test_authalic_converter PRIVATE cxx_std_11)
  add_test(NAME authalic_converter COMMAND test_authalic_converter)

  add_executable(dggrid_isea_probe tests/reference/dggrid_isea_probe.cpp)
  target_link_libraries(dggrid_isea_probe PRIVATE dglib)
  target_compile_features(dggrid_isea_probe PRIVATE cxx_std_11)
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
