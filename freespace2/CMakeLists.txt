

SET(FREESPACE_SRC freespace.cpp
				levelpaging.cpp
				freespace.h
				levelpaging.h
				SDLGraphicsOperations.cpp
				SDLGraphicsOperations.h)

IF(FSO_BUILD_PACKAGE)
	IF(FSO_INSTRUCTION_SET STREQUAL "SSE2" OR FSO_INSTRUCTION_SET STREQUAL "AVX")
		SET(FSO_INSTALLER_ICON "${CMAKE_CURRENT_SOURCE_DIR}/V_sse.ico" CACHE FILEPATH "The path of the installer icon")
	ELSE()
		SET(FSO_INSTALLER_ICON "${CMAKE_CURRENT_SOURCE_DIR}/V_app.ico" CACHE FILEPATH "The path of the installer icon")
	ENDIF()
ENDIF(FSO_BUILD_PACKAGE)

IF(MSVC60)
	link_directories(${STLPORT_INCLUDE_LIB_DIRS})
ENDIF(MSVC60)

ADD_EXECUTABLE(Freespace2 ${EXE_GUI_TYPE} ${FREESPACE_SRC})

include(resources.cmake)

SET_TARGET_PROPERTIES(Freespace2 PROPERTIES OUTPUT_NAME "fs2_open_${FSO_BINARY_SUFFIX}")

TARGET_LINK_LIBRARIES(Freespace2 code)
TARGET_LINK_LIBRARIES(Freespace2 platform)
TARGET_LINK_LIBRARIES(Freespace2 compiler)

IF(WIN32)
	# Link sdl main only here as it interferes with fred
	TARGET_LINK_LIBRARIES(Freespace2 sdlmain)
ENDIF(WIN32)

IF(FSO_USE_VOICEREC)
	ADD_DEFINITIONS(-DFS2_VOICER)
ENDIF(FSO_USE_VOICEREC)

INCLUDE_DIRECTORIES(${FREESPACE_HEADERS})
INCLUDE_DIRECTORIES(${GENERATED_SOURCE_DIR})

IF(USE_STLPORT)
	INCLUDE_DIRECTORIES(BEFORE SYSTEM ${STLPORT_INCLUDE_DIRS})
ENDIF(USE_STLPORT)

# Add install script to copy the executable to the FreeSpace path if possible and wanted
INSTALL(
	TARGETS Freespace2
	RUNTIME DESTINATION ${BINARY_DESTINATION}
	BUNDLE DESTINATION ${BINARY_DESTINATION}
)
if (FSO_INSTALL_DEBUG_FILES)
	if (MSVC)
		install(FILES "$<TARGET_PDB_FILE:Freespace2>"
				DESTINATION ${BINARY_DESTINATION}
				OPTIONAL)
	endif()
endif()

INCLUDE(util)
COPY_FILES_TO_TARGET(Freespace2)

include(CreateLaunchers)
create_target_launcher(Freespace2
	WORKING_DIRECTORY ${FSO_FREESPACE_PATH}
	ARGS ${FSO_RUN_ARGUMENTS})

separate_arguments(RUN_ARGS_LIST UNIX_COMMAND "${FSO_RUN_ARGUMENTS}")
message("${FSO_RUN_ARGUMENTS}")
add_custom_target(launch_fso
		COMMAND Freespace2 ${RUN_ARGS_LIST}
		WORKING_DIRECTORY "${FSO_FREESPACE_PATH}"
		COMMENT "Launching FreeSpace"
		VERBATIM)

# Ensure that Xcode generates debug symbols on macOS for Debug builds
set_target_properties(Freespace2 PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] "YES")
set_target_properties(Freespace2 PROPERTIES XCODE_ATTRIBUTE_COPY_PHASE_STRIP[variant=Debug] "NO")
set_target_properties(Freespace2 PROPERTIES XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT[variant=Debug] "NO")
set_target_properties(Freespace2 PROPERTIES XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN[variant=Debug] "NO")
