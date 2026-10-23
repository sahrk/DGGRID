# Checks of one dggrid executable (see CMakeLists.txt); run with cmake -P.
#   DGGRID    the dggrid executable
#   EXPECT_H3 the H3 version it must report, or "none" for built without H3
#   EXAMPLES  DGGRID's examples directory
#   WORK      a scratch directory

function(fail msg)
  message(FATAL_ERROR "FAIL (${DGGRID}): ${msg}")
endfunction()

# 1. the H3 version dggrid reports
execute_process(COMMAND ${DGGRID} -v OUTPUT_VARIABLE out ERROR_VARIABLE err
                RESULT_VARIABLE rc)
set(all "${out}${err}")
if(EXPECT_H3 STREQUAL "none")
  if(NOT all MATCHES "built without H3")
    fail("expected 'built without H3', got: ${all}")
  endif()
else()
  if(NOT all MATCHES "built with H3 version ${EXPECT_H3}")
    fail("expected 'built with H3 version ${EXPECT_H3}', got: ${all}")
  endif()
endif()
message(STATUS "dggrid -v: OK (${EXPECT_H3})")

# 2. no shared libh3
if(APPLE)
  execute_process(COMMAND otool -L ${DGGRID} OUTPUT_VARIABLE libs)
else()
  execute_process(COMMAND ldd ${DGGRID} OUTPUT_VARIABLE libs)
endif()
if(libs MATCHES "libh3")
  fail("dggrid links a shared libh3:\n${libs}")
endif()
message(STATUS "no shared libh3 linked: OK")

# 3. examples: the H3 examples if built with H3, and always the H3 geometry
# and GNOMONIC examples, which do not need H3
set(examples gnomonicGen h3Geom)
if(NOT EXPECT_H3 STREQUAL "none")
  file(STRINGS ${EXAMPLES}/examplesH3.lst h3examples)
  list(APPEND examples ${h3examples})
endif()

file(REMOVE_RECURSE ${WORK})
foreach(ex IN LISTS examples)
  file(MAKE_DIRECTORY ${WORK}/${ex})
  file(COPY ${EXAMPLES}/${ex}/ DESTINATION ${WORK}/${ex}
       PATTERN outputfiles EXCLUDE)
  file(MAKE_DIRECTORY ${WORK}/${ex}/outputfiles)

  file(GLOB metas RELATIVE ${WORK}/${ex} ${WORK}/${ex}/${ex}*.meta)
  foreach(meta IN LISTS metas)
    get_filename_component(name ${meta} NAME_WE)
    execute_process(COMMAND ${DGGRID} ${meta}
                    WORKING_DIRECTORY ${WORK}/${ex}
                    OUTPUT_FILE ${WORK}/${ex}/outputfiles/${name}.txt
                    ERROR_FILE ${WORK}/${ex}/outputfiles/${name}.err
                    RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
      fail("${ex}/${meta} exited with status ${rc}")
    endif()
  endforeach()

  # compare every reference file except the logs (their headers depend on the
  # platform and on the GDAL and H3 versions)
  file(GLOB refs RELATIVE ${EXAMPLES}/sampleOutput/${ex}
       ${EXAMPLES}/sampleOutput/${ex}/*)
  foreach(ref IN LISTS refs)
    if(ref STREQUAL ".keep")
      continue()
    endif()
    set(isLog FALSE)
    foreach(meta IN LISTS metas)
      get_filename_component(name ${meta} NAME_WE)
      if(ref STREQUAL "${name}.txt")
        set(isLog TRUE)
      endif()
    endforeach()
    if(isLog)
      continue()
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files
        ${EXAMPLES}/sampleOutput/${ex}/${ref} ${WORK}/${ex}/outputfiles/${ref}
        RESULT_VARIABLE differ)
    if(NOT differ EQUAL 0)
      fail("${ex}/outputfiles/${ref} differs from examples/sampleOutput/${ex}/${ref}")
    endif()
  endforeach()
  message(STATUS "example ${ex}: OK")
endforeach()

message(STATUS "ALL CHECKS PASSED for H3 ${EXPECT_H3}")
