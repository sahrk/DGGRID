cmake_minimum_required (VERSION 3.20)

add_executable(appex appex.cpp)

target_link_libraries(appex PRIVATE dglib)
target_compile_features(appex PRIVATE cxx_std_11)
