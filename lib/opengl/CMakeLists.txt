
FIND_PACKAGE(OpenGL REQUIRED)

add_library(OpenGL INTERFACE)

target_link_libraries(OpenGL INTERFACE "${OPENGL_LIBRARY}")

add_subdirectory(gl)
