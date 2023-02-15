
include(util)

INCLUDE(source_groups.cmake)
INCLUDE(voice_rec.cmake)

ADD_LIBRARY(code STATIC ${source_files} ${VOICE_REC_FILES})

target_compile_features(code PUBLIC cxx_std_11)

if (MSVC)
	target_compile_definitions(code PUBLIC "PDB_DEBUGGING=1")
endif(MSVC)

TARGET_INCLUDE_DIRECTORIES(code PUBLIC ${CODE_HEADERS})
TARGET_INCLUDE_DIRECTORIES(code PUBLIC ${FREESPACE_HEADERS})

TARGET_INCLUDE_DIRECTORIES(code PUBLIC ${GENERATED_SOURCE_DIR})
TARGET_INCLUDE_DIRECTORIES(code PUBLIC ${GENERATED_SOURCE_DIR}/code)

if (FSO_BUILD_WITH_OPENGL)
	target_link_libraries(code PUBLIC OpenGL)
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

target_link_libraries(code PUBLIC discord-rpc)

target_link_libraries(code PUBLIC libRocket)

target_link_libraries(code PUBLIC pcp)

if (FSO_BUILD_WITH_VULKAN)
	target_link_libraries(code PUBLIC vulkan-hpp)
endif()

target_link_libraries(code PUBLIC parsers)

target_link_libraries(code PUBLIC mdns)

enable_clang_tidy(code)

IF (FSO_USE_SPEECH)
	find_package(Speech REQUIRED)
	target_link_libraries(code PUBLIC speech)
ENDIF(FSO_USE_SPEECH)

TARGET_LINK_LIBRARIES(code PUBLIC jansson)

target_link_libraries(code PUBLIC variant)

target_link_libraries(code PUBLIC any)

target_link_libraries(code PUBLIC optional)

target_link_libraries(code PUBLIC hidapi::hidapi)

target_link_libraries(code PUBLIC imgui)

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
	target_compile_definitions(code PUBLIC WITH_VULKAN)
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