cmake_minimum_required(VERSION 3.12)

add_definitions(-DUSETHREAD)

add_library(anl STATIC anl_impl.cpp)
target_link_libraries(anl PUBLIC compiler)
suppress_warnings(anl)

target_include_directories(anl PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")