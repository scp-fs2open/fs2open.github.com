
MESSAGE(STATUS "Configuring Windows specific things and stuff...")

SET(WIN32_LIBS
	comctl32
	msacm32
	odbc32
	odbccp32
	vfw32
	winmm
	ws2_32
	psapi
)

IF (MINGW)
	SET(WIN32_LIBS ${WIN32_LIBS} mingw32)
ENDIF (MINGW)

target_link_libraries(platform INTERFACE ${WIN32_LIBS})

target_compile_definitions(platform INTERFACE WIN32 USE_OPENAL _WINDOWS)

# Specify minimum Windows version for the headers (currently this is Vista)
target_compile_definitions(platform INTERFACE NTDDI_VERSION=0x06000000 _WIN32_WINNT=0x0600)

SET(EXE_GUI_TYPE WIN32)
set(PLATFORM_WINDOWS TRUE CACHE INTERNAL "" FORCE)
