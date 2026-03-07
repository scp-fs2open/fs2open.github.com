add_library(VulkanHeaders INTERFACE)
target_include_directories(VulkanHeaders SYSTEM INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
