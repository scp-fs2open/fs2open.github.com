cmake_minimum_required(VERSION 2.6)

add_definitions(-DUSETHREAD)

add_library(anl STATIC anl_impl.cpp)

target_include_directories(anl SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")