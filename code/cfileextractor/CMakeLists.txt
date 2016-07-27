
add_executable(cfileextractor EXCLUDE_FROM_ALL cfileextractor.cpp)

ADD_DEFINITIONS(-DNO_SAFE_STRINGS)
include_directories(${CODE_HEADERS})

set_target_properties(cfileextractor
	PROPERTIES
		FOLDER "FSOTools"
)

IF(MSVC)
	set_target_properties(cfileextractor PROPERTIES
		COMPILE_DEFINITIONS_DEBUG "_DEBUG;PDB_DEBUGGING=1"
	)
ENDIF(MSVC)
