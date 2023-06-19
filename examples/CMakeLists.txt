cmake_minimum_required(VERSION 3.15)

add_executable(example main.cpp)
target_include_directories(example PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(example PRIVATE manifold)