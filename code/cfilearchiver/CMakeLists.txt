
add_executable(cfilearchiver EXCLUDE_FROM_ALL cfilearchiver.cpp)

include_directories(${CODE_HEADERS})

set_target_properties(cfilearchiver
	PROPERTIES
		FOLDER "FSOTools"
)
TARGET_LINK_LIBRARIES(cfilearchiver PUBLIC sdl2)
TARGET_INCLUDE_DIRECTORIES(cfilearchiver PUBLIC ${GENERATED_SOURCE_DIR})
TARGET_INCLUDE_DIRECTORIES(cfilearchiver PUBLIC ${GENERATED_SOURCE_DIR}/code)

IF(MSVC)
	set_target_properties(cfilearchiver PROPERTIES
		COMPILE_DEFINITIONS_DEBUG "_DEBUG;PDB_DEBUGGING=1"
	)
ENDIF(MSVC)
