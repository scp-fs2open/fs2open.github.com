
include(opengl.cmake)

IF(APPLE)
	ADD_SUBDIRECTORY(mac)
ENDIF(APPLE)

ADD_SUBDIRECTORY(libjpeg)

ADD_SUBDIRECTORY(zlib)

ADD_SUBDIRECTORY(libpng)

ADD_SUBDIRECTORY(lua)

ADD_SUBDIRECTORY(oggvorbis)

ADD_SUBDIRECTORY(openal)

ADD_SUBDIRECTORY(libsdl)

# These are only required for the web-ui which is non-Windows
IF(UNIX)
	ADD_SUBDIRECTORY(jansson)

	ADD_SUBDIRECTORY(mongoose)
ENDIF(UNIX)
