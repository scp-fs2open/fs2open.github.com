
add_library(glad STATIC
	"glad/include/glad/glad.h"
	"glad/include/KHR/khrplatform.h"
	"glad/src/glad.c")
target_include_directories(glad SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")
set_target_properties(glad PROPERTIES FOLDER "3rdparty")
suppress_warnings(glad)


add_library(OpenGL INTERFACE)
target_link_libraries(OpenGL INTERFACE glad)

if (WIN32)	
	add_library(glad_wgl STATIC
		"glad_wgl/include/glad/glad_wgl.h"
		"glad_wgl/include/KHR/khrplatform.h"
		"glad_wgl/src/glad_wgl.c")
	target_include_directories(glad_wgl SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad_wgl/include")
	set_target_properties(glad_wgl PROPERTIES FOLDER "3rdparty")
	target_link_libraries(glad_wgl PUBLIC glad)
	suppress_warnings(glad_wgl)
	
	# On Windows we need WGL which requires the OpenGL libraries
	find_package(OpenGL)
	target_link_libraries(glad_wgl PUBLIC "${OPENGL_LIBRARIES}")
endif()
