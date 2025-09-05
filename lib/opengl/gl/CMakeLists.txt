if(NOT FSO_BUILD_WITH_OPENGL_ES)
	add_library(glad STATIC
		"glad/include/glad/glad.h"
		"glad/include/KHR/khrplatform.h"
		"glad/src/glad.c")
	target_include_directories(glad SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")
	set_target_properties(glad PROPERTIES FOLDER "3rdparty")
	suppress_warnings(glad)


	add_library(OpenGL INTERFACE)
	target_link_libraries(OpenGL INTERFACE glad)
else()
    add_library(glad_gles STATIC
        "glad_es/include/glad/glad.h"
        "glad_es/include/KHR/khrplatform.h"
        "glad_es/src/glad.c")
    target_include_directories(glad_gles SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad_es/include")
    set_target_properties(glad_gles PROPERTIES FOLDER "3rdparty")
    suppress_warnings(glad_gles)
	
	add_library(OpenGL INTERFACE)
    target_link_libraries(OpenGL INTERFACE glad_gles)
endif()	

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
	if(NOT FSO_BUILD_WITH_OPENGL_ES)
		find_package(OpenGL)
		target_link_libraries(glad_wgl PUBLIC "${OPENGL_LIBRARIES}")
	else()
		# If we are building OpenGL ES, WGL need to be linked to Glad, not Glad ES
			add_library(glad STATIC
			"glad/include/glad/glad.h"
			"glad/include/KHR/khrplatform.h"
			"glad/src/glad.c")
		target_include_directories(glad SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")
		set_target_properties(glad PROPERTIES FOLDER "3rdparty")
		suppress_warnings(glad)
		target_link_libraries(glad_wgl PUBLIC glad)
	endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_library(glad_glx STATIC
			"glad_glx/include/glad/glad_glx.h"
			"glad_glx/include/KHR/khrplatform.h"
			"glad_glx/src/glad_glx.c")
	target_include_directories(glad_glx SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad_glx/include")
	set_target_properties(glad_glx PROPERTIES FOLDER "3rdparty")
	target_link_libraries(glad_glx PUBLIC glad)
	suppress_warnings(glad_glx)

	# On Linux we need GLX in addition to OpenGL, so attach GLX to OGL if we're here
	target_link_libraries(OpenGL INTERFACE glad_glx)
endif()
