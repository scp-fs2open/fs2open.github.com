
SET(IMPLOT_SOURCES
	implot.h
	implot_internal.h
	implot.cpp
	implot_items.cpp
)

ADD_LIBRARY(implot STATIC ${IMPLOT_SOURCES})

target_include_directories(implot SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Disable warnings if building from source
suppress_warnings(implot)

set_target_properties(implot PROPERTIES FOLDER "3rdparty")
TARGET_LINK_LIBRARIES(implot PUBLIC imgui)
