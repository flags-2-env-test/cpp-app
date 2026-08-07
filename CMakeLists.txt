cmake_minimum_required(VERSION 3.16)
project(demo VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The dependency, pulled straight out of the directory .zpkg.toml's
# [install].dir names. The C++ client is an INTERFACE target over a static
# library built from the same src/parser.c every other client binds to.
add_subdirectory(
  .vendor/.zed/oresoftware/flags-2-env/clients/cpp
  ${CMAKE_BINARY_DIR}/flags2env
  EXCLUDE_FROM_ALL)

add_executable(demo src/demo.cpp)
target_link_libraries(demo PRIVATE flags2env_cpp)
