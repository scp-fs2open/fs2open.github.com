
FIND_PACKAGE(OpenGL REQUIRED)

add_library(OpenGL INTERFACE)

if (APPLE)
    target_include_directories(OpenGL INTERFACE "${OPENGL_INCLUDE_DIR}/Headers")
else()
    target_include_directories(OpenGL INTERFACE "${OPENGL_INCLUDE_DIR}/GL")
endif()

target_link_libraries(OpenGL INTERFACE "${OPENGL_LIBRARY}")

add_subdirectory(gl)
