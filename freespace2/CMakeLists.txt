

SET(FREESPACE_SRC freespace.cpp
				levelpaging.cpp
				freespace.h
				levelpaging.h
				SDLGraphicsOperations.cpp
				SDLGraphicsOperations.h)

IF(FSO_BUILD_PACKAGE)
	IF(FSO_INSTRUCTION_SET STREQUAL "SSE2" OR FSO_INSTRUCTION_SET STREQUAL "AVX" OR FSO_INSTRUCTION_SET STREQUAL "AVX2" )
		SET(FSO_INSTALLER_ICON "${CMAKE_CURRENT_SOURCE_DIR}/V_sse.ico" CACHE FILEPATH "The path of the installer icon")
	ELSE()
		SET(FSO_INSTALLER_ICON "${CMAKE_CURRENT_SOURCE_DIR}/V_app.ico" CACHE FILEPATH "The path of the installer icon")
	ENDIF()
ENDIF(FSO_BUILD_PACKAGE)

IF(MSVC60)
	link_directories(${STLPORT_INCLUDE_LIB_DIRS})
ENDIF(MSVC60)

# For android, build a library not a executable
if (ANDROID)
    add_library(Freespace2 SHARED ${FREESPACE_SRC})
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    target_compile_definitions(Freespace2 PRIVATE PLATFORM_ANDROID)

    # SDL2
    find_package(SDL2 REQUIRED CONFIG PATHS ${SDL2_DIR} NO_DEFAULT_PATH)
    target_link_libraries(Freespace2 PRIVATE SDL2::SDL2)

    # NDK base libs
    find_library(log-lib log)
    find_library(android-lib android)
    target_link_libraries(Freespace2 PRIVATE ${log-lib} ${android-lib})
    
    # IMGUI needs to link against OpenGL ES 3
    find_library(GLESv3_LIB GLESv3)
    target_link_libraries(Freespace2 PRIVATE ${GLESv3_LIB})
else()
    add_executable(Freespace2 ${EXE_GUI_TYPE} ${FREESPACE_SRC})
endif()

target_compile_features(Freespace2 PUBLIC cxx_std_17)

include(resources.cmake)

SET_TARGET_PROPERTIES(Freespace2 PROPERTIES OUTPUT_NAME "fs2_open_${FSO_BINARY_SUFFIX}")

if (ANDROID)
	# one call
	target_link_libraries(Freespace2
		PRIVATE
		code
		platform
		compiler
		${SDL2_LIBS}
		${ANDROID_LIBS}
	)
else()
	TARGET_LINK_LIBRARIES(Freespace2 code)
	TARGET_LINK_LIBRARIES(Freespace2 platform)
	TARGET_LINK_LIBRARIES(Freespace2 compiler)
endif()

IF(WIN32)
	# Link sdl main only here as it interferes with fred
	TARGET_LINK_LIBRARIES(Freespace2 sdlmain)
ENDIF(WIN32)

if(FSO_BUILD_WITH_OPENGL_DEBUG)
	add_definitions(-DFS_OPENGL_DEBUG)
endif()

IF(FSO_USE_VOICEREC)
	ADD_DEFINITIONS(-DFS2_VOICER)
ENDIF(FSO_USE_VOICEREC)

INCLUDE_DIRECTORIES(${FREESPACE_HEADERS})
INCLUDE_DIRECTORIES(${GENERATED_SOURCE_DIR})

IF(USE_STLPORT)
	INCLUDE_DIRECTORIES(BEFORE SYSTEM ${STLPORT_INCLUDE_DIRS})
ENDIF(USE_STLPORT)

# Do not install when building android
if (NOT ANDROID)
# Add install script to copy the executable to the FreeSpace path if possible and wanted
INSTALL(
	TARGETS Freespace2
	RUNTIME DESTINATION ${BINARY_DESTINATION} COMPONENT "Freespace2"
	BUNDLE DESTINATION ${BINARY_DESTINATION} COMPONENT "Freespace2"
)
if (FSO_INSTALL_DEBUG_FILES)
	if (MSVC)
		install(FILES "$<TARGET_PDB_FILE:Freespace2>"
				DESTINATION ${BINARY_DESTINATION}
				OPTIONAL)
	endif()
endif()
endif(NOT ANDROID)

enable_clang_tidy(Freespace2)

INCLUDE(util)
COPY_FILES_TO_TARGET(Freespace2)

# Do not launch when building android
if (NOT ANDROID)
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
endif()

# Ensure that Xcode generates debug symbols on macOS for Debug builds
set_target_properties(Freespace2 PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] "YES")
set_target_properties(Freespace2 PROPERTIES XCODE_ATTRIBUTE_COPY_PHASE_STRIP[variant=Debug] "NO")
set_target_properties(Freespace2 PROPERTIES XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT[variant=Debug] "NO")
set_target_properties(Freespace2 PROPERTIES XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN[variant=Debug] "NO")
