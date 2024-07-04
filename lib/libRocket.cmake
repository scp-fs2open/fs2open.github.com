#===================================
# Build script for libRocket =======
#===================================

set(LIBROCKET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libRocket")

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0015 NEW)
endif(COMMAND cmake_policy)

# Search in the 'cmake' directory for additional CMake modules.
list(APPEND CMAKE_MODULE_PATH ${LIBROCKET_DIR}/Build/cmake)
set(PROJECT_SOURCE_DIR "${LIBROCKET_DIR}")

#===================================
# Environment tests ================
#===================================

include(TestForANSIForScope)
include(TestForANSIStreamHeaders)
include(TestForSTDNamespace)

#===================================
# Plaform specific global hacks ====
#===================================

if(APPLE)
	# Disables naked builtins from AssertMacros.h which
	# This prevents naming collisions such as those from the check()
	# function macro with LuaType::check
	add_definitions(-D__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES=0)
endif(APPLE)

#===================================
# Find dependencies ================
#===================================

# FreeType
#find_package(Freetype REQUIRED)7

#===================================
# Setup paths ======================
#===================================

# Include list of source files
include(FileList)

#===================================
# Build libraries ==================
#===================================
set(LIBRARIES Core Controls Debugger)

foreach(library ${LIBRARIES})
    set(NAME Rocket${library})

    add_library(${NAME}
                ${${library}_HDR_FILES}
                ${${library}_PUB_HDR_FILES}
                ${MASTER_${library}_PUB_HDR_FILES}
                ${${library}_SRC_FILES}
    )
    target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/Include")
    target_link_libraries(${NAME} PUBLIC compiler)
    suppress_warnings(${NAME})

    if(NOT BUILD_SHARED_LIBS)
    	target_compile_definitions(${NAME} PUBLIC ROCKET_STATIC_LIB)
        target_compile_definitions(${NAME} PRIVATE ROCKET_VERSION="FSO")
    endif()

    set_target_properties(${NAME}
            PROPERTIES
            FOLDER "3rdparty/libRocket"
            )
endforeach(library)

# Build Lua bindings
set(LIBRARIES Core Controls)

foreach(library ${LIBRARIES})
    set(NAME Rocket${library}Lua)

    add_library(${NAME} ${Lua${library}_SRC_FILES}
                        ${Lua${library}_HDR_FILES}
                        ${Lua${library}_PUB_HDR_FILES}
    )
    target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/Include")
    target_link_libraries(${NAME} PUBLIC compiler)
    suppress_warnings(${NAME})

    if(NOT BUILD_SHARED_LIBS)
        target_compile_definitions(${NAME} PUBLIC ROCKET_STATIC_LIB)
        target_compile_definitions(${NAME} PRIVATE ROCKET_VERSION="FSO")
    endif()

    set_target_properties(${NAME}
            PROPERTIES
            FOLDER "3rdparty/libRocket/Lua"
            )
endforeach(library)


#===================================
# Link libraries ===================
#===================================

target_link_libraries(RocketCore PUBLIC freetype)
target_link_libraries(RocketControls PUBLIC RocketCore)
target_link_libraries(RocketDebugger PUBLIC RocketCore)

target_link_libraries(RocketCoreLua PUBLIC RocketCore ${LUA_LIBS})
target_link_libraries(RocketControlsLua PUBLIC RocketControls RocketCoreLua ${LUA_LIBS})

add_library(libRocket INTERFACE)
target_link_libraries(libRocket INTERFACE RocketCore RocketControls RocketDebugger RocketCoreLua RocketControlsLua)
