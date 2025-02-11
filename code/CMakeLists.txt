
include(util)

INCLUDE(source_groups.cmake)
INCLUDE(voice_rec.cmake)

ADD_LIBRARY(code STATIC ${source_files} ${VOICE_REC_FILES})

target_compile_features(code PUBLIC cxx_std_17)

if (MSVC)
	target_compile_definitions(code PUBLIC "PDB_DEBUGGING=1")
endif(MSVC)

TARGET_INCLUDE_DIRECTORIES(code PUBLIC ${CODE_HEADERS})
TARGET_INCLUDE_DIRECTORIES(code PUBLIC ${FREESPACE_HEADERS})

TARGET_INCLUDE_DIRECTORIES(code PUBLIC ${GENERATED_SOURCE_DIR})
TARGET_INCLUDE_DIRECTORIES(code PUBLIC ${GENERATED_SOURCE_DIR}/code)

if (FSO_BUILD_WITH_OPENGL)
	target_link_libraries(code PUBLIC OpenGL)
	if(FSO_BUILD_WITH_OPENGL_DEBUG)
		add_definitions(-DFS_OPENGL_DEBUG)
	endif()
endif()

if(FSO_BUILD_WITH_OPENXR)
	add_definitions(-DFS_OPENXR)
endif()

if(FSO_BUILD_WITH_OPENXR_DEBUG AND FSO_BUILD_WITH_OPENXR)
	add_definitions(-DFS_OPENXR_DEBUG)
endif()

TARGET_LINK_LIBRARIES(code PUBLIC openal)
TARGET_LINK_LIBRARIES(code PUBLIC ${LUA_LIBS})
TARGET_LINK_LIBRARIES(code PUBLIC ${PNG_LIBS})
TARGET_LINK_LIBRARIES(code PUBLIC ${JPEG_LIBS})
TARGET_LINK_LIBRARIES(code PUBLIC lz4)

TARGET_LINK_LIBRARIES(code PUBLIC sdl2)

if (FSO_BUILD_WITH_FFMPEG)
	TARGET_LINK_LIBRARIES(code PUBLIC ffmpeg)
endif()

TARGET_LINK_LIBRARIES(code PUBLIC utfcpp)

TARGET_LINK_LIBRARIES(code PUBLIC platform)
TARGET_LINK_LIBRARIES(code PUBLIC compiler)

target_link_libraries(code PUBLIC md5)

if (FSO_BUILD_WITH_DISCORD)
	target_link_libraries(code PUBLIC discord-rpc)
	add_definitions(-DWITH_DISCORD)
endif()

target_link_libraries(code PUBLIC libRocket)

target_link_libraries(code PUBLIC pcp)

target_link_libraries(code PUBLIC parsers)

target_link_libraries(code PUBLIC mdns)

enable_clang_tidy(code)

IF (FSO_USE_SPEECH)
	find_package(Speech REQUIRED)
	target_link_libraries(code PUBLIC speech)
ENDIF(FSO_USE_SPEECH)

TARGET_LINK_LIBRARIES(code PUBLIC jansson)

target_link_libraries(code PUBLIC anl)

target_link_libraries(code PUBLIC hidapi::hidapi)

target_link_libraries(code PUBLIC imgui)

IF(FSO_BUILD_WITH_OPENXR)
	target_link_libraries(code PUBLIC OpenXR::openxr_loader)
	target_include_directories(code PUBLIC OpenXR::Headers)
ENDIF()
IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
	# On Linux, we need to access X11 window data.
	# On Windows, SDL already provides everything, so no need to do anything
	find_package(X11)
	target_link_libraries(code PUBLIC ${X11_LIBRARIES})
ENDIF()

# Headers for standalone "gui" on UNIX
IF(UNIX)
	TARGET_LINK_LIBRARIES(code PUBLIC ${MONGOOSE_LIBS})
ENDIF(UNIX)

IF(FSO_BUILD_TOOLS)
	ADD_SUBDIRECTORY(cfilearchiver)

	ADD_SUBDIRECTORY(cfileextractor)
ENDIF(FSO_BUILD_TOOLS)

if (FSO_RELEASE_LOGGING)
	target_compile_definitions(code PUBLIC SCP_RELEASE_LOGGING)
endif()

if (FSO_BUILD_WITH_FFMPEG)
	target_compile_definitions(code PUBLIC WITH_FFMPEG)
endif()
if (FSO_BUILD_WITH_OPENGL)
	target_compile_definitions(code PUBLIC WITH_OPENGL)
endif()
if (FSO_BUILD_WITH_VULKAN)
	find_package(Vulkan REQUIRED)
	if (Vulkan_FOUND)
		target_compile_definitions(code PUBLIC WITH_VULKAN)
		target_link_libraries(code PRIVATE Vulkan::Vulkan)

		target_compile_definitions(code PUBLIC WITH_VULKAN VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VK_NO_PROTOTYPES)
	else()
		message(WARNING "FSO_BUILD_WITH_VULKAN was set, but the package was unable to be found. Forcing OFF.")
		set(FSO_BUILD_WITH_VULKAN OFF CACHE BOOL "Enable compilation of the Vulkan renderer" FORCE)
	endif()
endif()

include(shaders.cmake)

target_embed_files(code FILES ${default_files_files} RELATIVE_TO "def_files")

handle_embedded_files(code)

set_precompiled_header(code "${CMAKE_CURRENT_SOURCE_DIR}/prefix_header.h")

# Ensure that Xcode generates debug symbols on macOS for Debug builds
set_target_properties(code PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] "YES")
set_target_properties(code PROPERTIES XCODE_ATTRIBUTE_COPY_PHASE_STRIP[variant=Debug] "NO")
set_target_properties(code PROPERTIES XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT[variant=Debug] "NO")
set_target_properties(code PROPERTIES XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN[variant=Debug] "NO")

# Prevent GCC warnings in third-party BitOp... see GitHub #4366
suppress_file_warnings(scripting/lua/bitop/bit.c)
