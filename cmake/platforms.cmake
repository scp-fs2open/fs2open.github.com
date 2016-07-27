
add_library(platform INTERFACE)

IF(FSO_CMAKE_DEBUG)
	INCLUDE(debug-vars)
ENDIF(FSO_CMAKE_DEBUG)

IF(WIN32)
	INCLUDE(platform-win32)
ELSEIF(UNIX)
	INCLUDE(platform-unix)

	IF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
		INCLUDE(platform-freebsd)
	ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
		INCLUDE(platform-solaris)
	ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		include(platform-darwin)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
ELSE(WIN32)
	MESSAGE(STATUS "This platform is not supported, good luck!")
ENDIF(WIN32)
