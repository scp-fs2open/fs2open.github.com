add_library(ffx_portable_arm_asr STATIC
    src/shared/ffxm_assert.cpp
    src/shared/ffxm_object_management.cpp
    src/components/fsr2/ffxm_fsr2.cpp
    src/backends/vk/ffxm_hash.cpp
    src/backends/vk/ffxm_vk.cpp
    src/backends/shared/ffxm_shader_blobs.cpp
    src/backends/shared/blob_accessors/ffxm_fsr2_shaderblobs.cpp
)

target_include_directories(ffx_portable_arm_asr SYSTEM PUBLIC include)
target_include_directories(ffx_portable_arm_asr PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    include/host
    include/gpu
    src/components
    src/backends/vk
    src/backends/shared
    src/backends/shared/blob_accessors/prebuilt_shaders
)
target_compile_definitions(ffx_portable_arm_asr PRIVATE FFXM_FSR VK_NO_PROTOTYPES)
# Standalone builds of ffx-portable do not include FSO's CMake helpers.
if(COMMAND suppress_warnings)
    suppress_warnings(ffx_portable_arm_asr)
elseif(MSVC)
    target_compile_options(ffx_portable_arm_asr PRIVATE /W0)
else()
    target_compile_options(ffx_portable_arm_asr PRIVATE -w)
endif()
if(TARGET Vulkan::Headers)
    target_link_libraries(ffx_portable_arm_asr PRIVATE Vulkan::Headers)
endif()

set_target_properties(ffx_portable_arm_asr PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
