
add_library(glad STATIC
	"glad/include/glad/glad.h"
	"glad/include/KHR/khrplatform.h"
	"glad/src/glad.c")
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")
set_target_properties(glad PROPERTIES FOLDER "3rdparty")
if (MSVC)
	# Suppress some warnings
	target_compile_options(glad PRIVATE "/w")
endif()


add_library(OpenGL INTERFACE)
target_link_libraries(OpenGL INTERFACE glad)
