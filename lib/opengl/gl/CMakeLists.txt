
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

if (WIN32)	
	add_library(glad_wgl STATIC
		"glad_wgl/include/glad/glad_wgl.h"
		"glad_wgl/include/KHR/khrplatform.h"
		"glad_wgl/src/glad_wgl.c")
	target_include_directories(glad_wgl PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad_wgl/include")
	set_target_properties(glad_wgl PROPERTIES FOLDER "3rdparty")
	target_link_libraries(glad_wgl PUBLIC glad)
	if (MSVC)
		# Suppress some warnings
		target_compile_options(glad_wgl PRIVATE "/w")
	endif()
	
	# On Windows we need WGL which requires the OpenGL libraries
	find_package(OpenGL)
	target_link_libraries(glad_wgl PUBLIC "${OPENGL_LIBRARIES}")
endif()
