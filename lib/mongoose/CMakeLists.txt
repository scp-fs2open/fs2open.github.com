
FIND_PACKAGE(Threads)

SET(MONGOOSE_SOURCES
	mongoose.c
	mongoose.h)

ADD_LIBRARY(mongoose STATIC ${MONGOOSE_SOURCES})
	
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(mongoose PUBLIC ${CMAKE_THREAD_LIBS_INIT})

if (UNIX)
	target_link_libraries(mongoose PUBLIC ${CMAKE_DL_LIBS})
endif()

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

