
FIND_PACKAGE(Threads)

SET(MONGOOSE_SOURCES
	mongoose.c
	mongoose.h)

ADD_LIBRARY(mongoose STATIC ${MONGOOSE_SOURCES})
	
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(mongoose ${CMAKE_THREAD_LIBS_INIT})

IF(UNIX)
	# Also add dynamic linking libs
	CMAKE_SEARCH_LIBS(HAVE_DLSYM LIB_DLSYM dlsym dl)

	IF(NOT HAVE_DLSYM)
		MESSAGE(FATAL_ERROR "Failed to find library providing dlsym!")
	ENDIF(NOT HAVE_DLSYM)

	TARGET_LINK_LIBRARIES(mongoose ${LIB_DLSYM})
ENDIF(UNIX)

set_target_properties(mongoose
	PROPERTIES
		FOLDER "3rdparty"
		INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
)
	
if (MSVC)
	# Disable warnings if building from source
	target_compile_options(mongoose PRIVATE "/W0")
endif()

SET(MONGOOSE_LIBS mongoose CACHE INTERNAL "mongoose library")

