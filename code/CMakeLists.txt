
INCLUDE(source_groups.cmake)

INCLUDE(voice_rec.cmake)

ADD_LIBRARY(code STATIC ${file_root} ${VOICE_REC_FILES})

target_compile_features(code PUBLIC ${REQUIRED_CXX_FEATURES})

INCLUDE(default_files.cmake)

if (MSVC)
	target_compile_definitions(code PUBLIC "PDB_DEBUGGING=1")
endif(MSVC)

TARGET_INCLUDE_DIRECTORIES(code PUBLIC ${CODE_HEADERS})
TARGET_INCLUDE_DIRECTORIES(code PUBLIC ${FREESPACE_HEADERS})

TARGET_INCLUDE_DIRECTORIES(code PUBLIC ${GENERATED_SOURCE_DIR})
TARGET_INCLUDE_DIRECTORIES(code PUBLIC ${GENERATED_SOURCE_DIR}/code)

target_link_libraries(code PUBLIC OpenGL)

TARGET_LINK_LIBRARIES(code PUBLIC ${OPENAL_LIBS})
TARGET_LINK_LIBRARIES(code PUBLIC ${LUA_LIBS})
TARGET_LINK_LIBRARIES(code PUBLIC ${PNG_LIBS})
TARGET_LINK_LIBRARIES(code PUBLIC ${JPEG_LIBS})

TARGET_LINK_LIBRARIES(code PUBLIC sdl2)

TARGET_LINK_LIBRARIES(code PUBLIC ffmpeg)

TARGET_LINK_LIBRARIES(code PUBLIC utfcpp)

TARGET_LINK_LIBRARIES(code PUBLIC platform)
TARGET_LINK_LIBRARIES(code PUBLIC compiler)

target_link_libraries(code PUBLIC md5)

IF (FSO_USE_SPEECH)
	find_package(Speech REQUIRED)
	target_link_libraries(code PUBLIC speech)
ENDIF(FSO_USE_SPEECH)

TARGET_LINK_LIBRARIES(code PUBLIC jansson)

# Headers for standalone "gui" on UNIX
IF(UNIX)
	TARGET_LINK_LIBRARIES(code PUBLIC ${MONGOOSE_LIBS})
ENDIF(UNIX)

IF(FSO_BUILD_TOOLS)
	ADD_SUBDIRECTORY(cfilearchiver)

	ADD_SUBDIRECTORY(cfileextractor)
ENDIF(FSO_BUILD_TOOLS)

set_target_properties(code PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "prefix_header.h")

include(util)
configure_cotire(code)

# Ensure that Xcode generates debug symbols on macOS for Debug builds
set_target_properties(code PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] "YES")
set_target_properties(code PROPERTIES XCODE_ATTRIBUTE_COPY_PHASE_STRIP[variant=Debug] "NO")
set_target_properties(code PROPERTIES XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT[variant=Debug] "NO")
set_target_properties(code PROPERTIES XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN[variant=Debug] "NO")
