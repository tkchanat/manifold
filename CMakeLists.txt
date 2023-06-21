cmake_minimum_required(VERSION 3.15)
project(manifold)
set(CMAKE_CXX_STANDARD 17)

add_library(manifold STATIC
  src/mesh.cpp
)
target_include_directories(manifold PRIVATE include)

option(BUILD_EXAMPLES "Build example executables" ON)
if(BUILD_EXAMPLES)
  message("Building examples")
  add_subdirectory(examples)
endif()