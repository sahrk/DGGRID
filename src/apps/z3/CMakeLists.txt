cmake_minimum_required (VERSION 3.9)

add_executable(z3 z3.cpp)

target_link_libraries(z3 PRIVATE dglib)
target_compile_features(z3 PRIVATE cxx_std_11)
