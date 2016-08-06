
add_executable(cfileextractor EXCLUDE_FROM_ALL cfileextractor.cpp)

ADD_DEFINITIONS(-DNO_SAFE_STRINGS)
include_directories(${CODE_HEADERS})

set_target_properties(cfileextractor
	PROPERTIES
		FOLDER "FSOTools"
)
TARGET_LINK_LIBRARIES(cfileextractor PUBLIC ${SDL_LIBS})
TARGET_INCLUDE_DIRECTORIES(cfileextractor PUBLIC ${GENERATED_SOURCE_DIR})
TARGET_INCLUDE_DIRECTORIES(cfileextractor PUBLIC ${GENERATED_SOURCE_DIR}/code)

IF(MSVC)
	set_target_properties(cfileextractor PROPERTIES
		COMPILE_DEFINITIONS_DEBUG "_DEBUG;PDB_DEBUGGING=1"
	)
ENDIF(MSVC)
