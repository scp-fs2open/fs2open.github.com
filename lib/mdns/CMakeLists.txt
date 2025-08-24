add_library(mdns
	INTERFACE
	mdns.h
)

target_include_directories(mdns SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(mdns PROPERTIES FOLDER "3rdparty")