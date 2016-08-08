
include(source_groups.cmake)

IF(UNIX)
	SET(wxWidgets_USE_DEBUG OFF)
	SET(wxWidgets_USE_UNICODE ON)
	SET(wxWidgets_USE_UNIVERSAL OFF)
	SET(wxWidgets_USE_STATIC OFF)
	set(wxWidgets_CONFIG_OPTIONS --version=3.0)
ELSEIF(WIN32)
	set(wxWidgets_CONFIGURATION "mswu" CACHE STRING "Release WxWidgets configuration" FORCE)
ENDIF(UNIX)

find_package(wxWidgets COMPONENTS gl core base REQUIRED)

include(${wxWidgets_USE_FILE})

add_executable(wxfred2 ${EXE_GUI_TYPE} ${file_root})

include(resources.cmake)

target_compile_definitions(wxfred2 PUBLIC USING_THIRD_PARTY_LIBS FRED)

SET_TARGET_PROPERTIES(wxfred2 PROPERTIES OUTPUT_NAME "wxfred2_${FSO_BINARY_SUFFIX}")

TARGET_INCLUDE_DIRECTORIES(wxfred2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_INCLUDE_DIRECTORIES(wxfred2 PUBLIC ${wxWidgets_INCLUDE_DIRS})
TARGET_INCLUDE_DIRECTORIES(wxfred2 PUBLIC ${GENERATED_SOURCE_DIR})
TARGET_INCLUDE_DIRECTORIES(wxfred2 PUBLIC ${GENERATED_SOURCE_DIR}/wxfred2)
TARGET_INCLUDE_DIRECTORIES(wxfred2 PUBLIC ${GENERATED_SOURCE_DIR}/code)

TARGET_LINK_LIBRARIES(wxfred2 code)
TARGET_LINK_LIBRARIES(wxfred2 ${wxWidgets_LIBRARIES})

if (MSVC)
	set_target_properties(wxfred2 PROPERTIES LINK_FLAGS "/FORCE:MULTIPLE")
endif()

# Add install script to copy the executable to the FreeSpace path if possible and wanted
INSTALL(
	TARGETS wxfred2
	RUNTIME DESTINATION ${BINARY_DESTINATION}
	BUNDLE DESTINATION ${BINARY_DESTINATION}
)
if (FSO_INSTALL_DEBUG_FILES)
	if (MSVC)
		install(FILES "$<TARGET_PDB_FILE:wxfred2>"
				DESTINATION ${BINARY_DESTINATION}
				OPTIONAL)
	endif()
endif()

INCLUDE(util)
COPY_FILES_TO_TARGET(wxfred2)

include(CreateLaunchers)
create_target_launcher(wxfred2
	WORKING_DIRECTORY ${FSO_FREESPACE_PATH}
	ARGS ${FSO_RUN_ARGUMENTS})
