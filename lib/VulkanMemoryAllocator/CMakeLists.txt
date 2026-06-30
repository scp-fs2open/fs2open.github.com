add_library(VulkanMemoryAllocator INTERFACE)
target_include_directories(VulkanMemoryAllocator SYSTEM INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
