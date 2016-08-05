
add_library(glad STATIC
	"glad/include/glad/glad.h"
	"glad/include/KHR/khrplatform.h"
	"glad/src/glad.c")
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")
set_target_properties(glad PROPERTIES FOLDER "3rdparty")
if (MSVC)
	# Suppress some warnings
	target_compile_options(glad PRIVATE "/wd4090" "/wd4055" "/wd4054")
endif()


add_library(gl_12 INTERFACE)
target_link_libraries(gl_12 INTERFACE glad)

target_link_libraries(gl_12 INTERFACE OpenGL)
