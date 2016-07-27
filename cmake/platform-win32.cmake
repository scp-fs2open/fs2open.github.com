
MESSAGE(STATUS "Configuring Windows specific things and stuff...")

target_link_libraries(platform INTERFACE comctl32.lib
	msacm32.lib
	odbc32.lib
	odbccp32.lib
	vfw32.lib
	winmm.lib
	wsock32.lib)

target_compile_definitions(platform INTERFACE WIN32 USE_OPENAL _WINDOWS)

# Specify minimum Windows version for the headers (currently this is XP)
target_compile_definitions(platform INTERFACE NTDDI_VERSION=0x05010000 _WIN32_WINNT=0x0501)

SET(EXE_GUI_TYPE WIN32)
