set(ETCPAK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/etcpak")

set(ETCPAK_SOURCES
	"${ETCPAK_DIR}/ProcessRGB.cpp"
	"${ETCPAK_DIR}/Tables.cpp"
	"${ETCPAK_DIR}/Dither.cpp")

add_library(etcpak STATIC ${ETCPAK_SOURCES})

target_include_directories(etcpak SYSTEM PUBLIC "${ETCPAK_DIR}")

suppress_warnings(etcpak)

set_target_properties(etcpak PROPERTIES FOLDER "3rdparty")

target_link_libraries(etcpak PUBLIC compiler)