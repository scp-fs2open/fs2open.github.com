
add_executable(cfilearchiver EXCLUDE_FROM_ALL cfilearchiver.cpp)

ADD_DEFINITIONS(-DNO_SAFE_STRINGS)
include_directories(${CODE_HEADERS})

set_target_properties(cfilearchiver
	PROPERTIES
		FOLDER "FSOTools"
)

IF(MSVC)
	set_target_properties(cfilearchiver PROPERTIES
		COMPILE_DEFINITIONS_DEBUG "_DEBUG;PDB_DEBUGGING=1"
	)
ENDIF(MSVC)
