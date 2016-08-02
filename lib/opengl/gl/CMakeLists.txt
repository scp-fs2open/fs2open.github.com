
add_library(glad_12 STATIC 
	"release/include/glad/glad.h"
	"release/include/KHR/khrplatform.h"
	"release/src/glad.c")
target_include_directories(glad_12 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/release/include")
set_target_properties(glad_12 PROPERTIES FOLDER "3rdparty")
if (MSVC)
	# Suppress some warnings
	target_compile_options(glad_12 PRIVATE "/wd4090" "/wd4055" "/wd4054")
endif()
	
add_library(glad_12_debug STATIC 
	"debug/include/glad/glad.h"
	"debug/include/KHR/khrplatform.h"
	"debug/src/glad.c")
target_include_directories(glad_12_debug PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/debug/include")
set_target_properties(glad_12_debug PROPERTIES FOLDER "3rdparty")
if (MSVC)
	# Suppress some warnings
	target_compile_options(glad_12_debug PRIVATE "/wd4090" "/wd4055" "/wd4054" "/wd4100")
endif()


add_library(gl_12 INTERFACE)
target_link_libraries(gl_12 INTERFACE "$<$<CONFIG:Release>:glad_12>" "$<$<NOT:$<CONFIG:Release>>:glad_12_debug>")

target_link_libraries(gl_12 INTERFACE OpenGL)
