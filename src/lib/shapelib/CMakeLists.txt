cmake_minimum_required (VERSION 3.9)

add_library(shapelib
  lib/dbfopen.c
  lib/safileio.c
  lib/sbnsearch.c
  lib/shpopen.c
  lib/shptree.c
  lib/shputils.c
)

target_include_directories(shapelib PUBLIC include)
