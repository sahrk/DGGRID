cmake_minimum_required (VERSION 3.9)

add_library(proj4lib
  lib/adjlon.cpp
  lib/pj_auth.cpp
  lib/pj_mlfn.cpp
  lib/pj_msfn.cpp
  lib/pj_phi2.cpp
  lib/pj_qsfn.cpp
  lib/pj_tsfn.cpp
)

target_include_directories(proj4lib PUBLIC include)
