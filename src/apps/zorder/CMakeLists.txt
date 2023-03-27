cmake_minimum_required (VERSION 3.9)

add_executable(zorder zorder.cpp)

target_link_libraries(zorder PRIVATE dglib)
target_compile_features(zorder PRIVATE cxx_std_11)
