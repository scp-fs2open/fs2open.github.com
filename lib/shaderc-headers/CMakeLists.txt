add_library(ShadercHeaders INTERFACE)
target_include_directories(ShadercHeaders SYSTEM INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
