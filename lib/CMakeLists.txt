
include(prebuilt.cmake)

ADD_SUBDIRECTORY(libjpeg)

ADD_SUBDIRECTORY(zlib)

ADD_SUBDIRECTORY(libpng)

ADD_SUBDIRECTORY(lua)

include(OpenAL.cmake)

add_subdirectory(opengl)

ADD_SUBDIRECTORY(libsdl)

ADD_SUBDIRECTORY(jansson)
# These are only required for the web-ui which is non-Windows
IF(UNIX)
	ADD_SUBDIRECTORY(mongoose)
ENDIF(UNIX)

add_subdirectory(md5)

add_subdirectory(utfcpp)

include(freetype.cmake)

if (FSO_BUILD_WITH_FFMPEG)
	include(FFmpeg.cmake)
endif()

add_subdirectory(discord)

option(USE_BUNDLED_LIBROCKET "Use bundled version of librocket instead of system one" ON)

if (USE_BUNDLED_LIBROCKET)
    IF(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libRocket/Build/cmake")
        message(FATAL_ERROR "libRocket submodule could not be found. Please make sure you have updated your submodules, or set USE_BUNDLED_LIBROCKET to use system version.")
    endif()

    include(libRocket.cmake)
else(USE_BUNDLED_LIBROCKET)
    find_package(libRocket REQUIRED)
    add_library(libRocket INTERFACE)
    target_link_libraries(libRocket INTERFACE "${libRocket_LIBRARIES}")
endif(USE_BUNDLED_LIBROCKET)

add_subdirectory(libpcp)

add_subdirectory(vulkan-hpp)
