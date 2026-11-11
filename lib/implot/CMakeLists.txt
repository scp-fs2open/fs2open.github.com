
SET(IMPLOT_SOURCES
	implot.h
	implot_internal.h
	implot.cpp
	implot_items.cpp
)

ADD_LIBRARY(implot STATIC ${IMPLOT_SOURCES})

target_include_directories(implot SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# ImPlot uses imgui's ImVec2/ImVec4 math operators, which imgui only defines when
# IMGUI_DEFINE_MATH_OPERATORS is set before imgui.h is first included. implot_items.cpp
# pulls in imgui.h (via implot.h) before implot_internal.h's own #ifndef guard runs, so
# the guard loses the race -- define it on the target instead. PUBLIC so any consumer that
# includes implot.h after imgui.h stays consistent.
target_compile_definitions(implot PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

# Disable warnings if building from source
suppress_warnings(implot)

set_target_properties(implot PROPERTIES FOLDER "3rdparty")
TARGET_LINK_LIBRARIES(implot PUBLIC imgui)
