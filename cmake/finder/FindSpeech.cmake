
add_library(speech INTERFACE)

if (WIN32)
	find_package(WindowsSDK REQUIRED)

	foreach(dir ${WINDOWSSDK_DIRS})
		get_windowssdk_library_dirs("${dir}" LIB_DIRS)
	
		find_library(SPEECH_LIBRARY
			NAMES sapi
			PATHS ${LIB_DIRS}
			NO_DEFAULT_PATH)

		if (SPEECH_LIBRARY)
			set(SDK_DIR "${dir}")
			break()
		endif()
	endforeach()

	if (NOT SPEECH_LIBRARY)
		message(SEND_ERROR "Text to speech library could not be found! Either install it or disable the speech option.")
		return()
	endif()
	
	get_windowssdk_include_dirs("${SDK_DIR}" INCLUDE_DIRS)
	
	target_include_directories(speech INTERFACE ${INCLUDE_DIRS})
else()
	message(SEND_ERROR "Text to Speech is not supported on this platform!")
endif()

target_compile_definitions(speech INTERFACE -DFS2_SPEECH)