
SET(IMGUI_SOURCES
	imconfig.h
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_internal.h
	imgui_tables.cpp
	imgui_widgets.cpp
	imgui.cpp
	imgui.h
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h
    backends/imgui_impl_opengl3.h
    backends/imgui_impl_opengl3.cpp
    backends/imgui_impl_sdl.h
    backends/imgui_impl_sdl.cpp
    extensions/imgui_sugar.hpp
    extensions/ImGuizmo.cpp
    extensions/ImGuizmo.h
)

if (FSO_BUILD_WITH_VULKAN)
    list(APPEND IMGUI_SOURCES
        backends/imgui_impl_vulkan.h
        backends/imgui_impl_vulkan.cpp
    )
endif()

ADD_LIBRARY(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Disable warnings if building from source
suppress_warnings(imgui)

set_target_properties(imgui PROPERTIES FOLDER "3rdparty")
TARGET_LINK_LIBRARIES(imgui PUBLIC sdl2)
target_link_libraries(imgui PUBLIC compiler)

if(FSO_BUILD_WITH_OPENGL_ES)
    # IMGUI needs to link against OpenGL ES 3, the NDK already has it
    # For Linux this adds a dependency: libgles2-mesa-dev
    if(ANDROID OR MAKE_SYSTEM_NAME MATCHES "Linux")
    	add_definitions(-DIMGUI_IMPL_OPENGL_ES3)
    	find_library(GLESv3_LIB GLESv3)
    	target_link_libraries(imgui PRIVATE ${GLESv3_LIB})
	elseif(WIN32)
		# Windows, optional Angle, without Angle it will compile and run, but it will crash if you use imgui
		set(ANGLE_ROOT "" CACHE PATH "Folder with ANGLE include/ and lib/")
		if(ANGLE_ROOT AND EXISTS "${ANGLE_ROOT}")
			find_path(angle_include NAMES GLES3/gl3.h HINTS "${ANGLE_ROOT}/include" NO_DEFAULT_PATH)
			find_library(angle_gles2 NAMES libGLESv2.dll.lib HINTS "${ANGLE_ROOT}/lib" NO_DEFAULT_PATH)
			if(angle_include AND angle_gles2)
				message(STATUS "ANGLE found on: ${ANGLE_ROOT}")
				target_include_directories(imgui PRIVATE "${angle_include}")
				target_link_libraries(imgui PRIVATE "${angle_gles2}")
				add_definitions(-DIMGUI_IMPL_OPENGL_ES3)
			else()
				message(WARNING "ANGLE lib is needed to use imgui with the OpenGL ES backend. Without it FSO will compile and run, but imgui will crash. Please specify a path to a prebuilt version of the ANGLE using ANGLE_ROOT")
			endif()
		endif()
    endif()
endif()

if (FSO_BUILD_WITH_VULKAN)
	find_package(Vulkan REQUIRED)
    target_compile_definitions(imgui INTERFACE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VK_NO_PROTOTYPES)
	target_link_libraries(imgui PRIVATE Vulkan::Vulkan)
endif()
