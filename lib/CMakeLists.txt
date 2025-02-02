
include(prebuilt.cmake)

ADD_SUBDIRECTORY(libjpeg)

ADD_SUBDIRECTORY(zlib)

ADD_SUBDIRECTORY(libpng)

ADD_SUBDIRECTORY(lua)

include(OpenAL.cmake)

if (FSO_BUILD_WITH_OPENGL)
	add_subdirectory(opengl)
endif()

include(SDL2.cmake)

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

if (FSO_BUILD_WITH_DISCORD)
	add_subdirectory(discord)
endif()

include(libRocket.cmake)

add_subdirectory(libpcp)

include(antlr4.cmake)

include(vulkan.cmake)

add_subdirectory(mdns)

add_subdirectory(accidental-noise)

ADD_SUBDIRECTORY(lz4)

set(HIDAPI_WITH_LIBUSB OFF)
set(HIDAPI_WITH_HIDRAW ON)
add_subdirectory(hidapi)

ADD_SUBDIRECTORY(imgui)

if(FSO_BUILD_WITH_OPENXR)
	add_subdirectory(openxr)
endif()
