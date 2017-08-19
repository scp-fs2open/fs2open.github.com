
add_subdirectory(glm)

target_include_directories(glm SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(glm PROPERTIES FOLDER "3rdparty")

# Disable warnings in this library
suppress_warnings(glm)
target_compile_definitions(glm PUBLIC GLM_FORCE_CXX11 GLM_FORCE_RADIANS)