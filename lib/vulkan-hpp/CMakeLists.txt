

add_library(vulkan-hpp INTERFACE)

target_include_directories(vulkan-hpp SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(vulkan-hpp INTERFACE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VK_NO_PROTOTYPES)
