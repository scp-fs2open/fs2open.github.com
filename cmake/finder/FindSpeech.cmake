
include(CheckIncludeFile)

add_library(speech INTERFACE)

if (WIN32)
	CHECK_INCLUDE_FILE_CXX("sapi.h" HAVE_SAPI_H)
	
	if (NOT HAVE_SAPI_H)
		message(SEND_ERROR "sapi.h could not be found on your platform. Please disable speech support.")
	endif()
elseif(APPLE)
	# it should just work
else()
	message(SEND_ERROR "Text to Speech is not supported on this platform!")
endif()

target_compile_definitions(speech INTERFACE -DFS2_SPEECH)
