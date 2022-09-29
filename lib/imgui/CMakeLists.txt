
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
)

ADD_LIBRARY(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Disable warnings if building from source
suppress_warnings(imgui)

set_target_properties(imgui PROPERTIES FOLDER "3rdparty")
TARGET_LINK_LIBRARIES(imgui PUBLIC sdl2)
target_link_libraries(imgui PUBLIC compiler)


