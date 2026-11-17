
include(prebuilt.cmake)

ADD_SUBDIRECTORY(libjpeg)

ADD_SUBDIRECTORY(zlib)

ADD_SUBDIRECTORY(libpng)

ADD_SUBDIRECTORY(lua)

include(OpenAL.cmake)

if (FSO_BUILD_WITH_OPENGL)
	add_subdirectory(opengl)
endif()

include(SDL3.cmake)

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

add_subdirectory(libpcpnatpmp)

include(antlr4.cmake)

include(vulkan.cmake)

if (FSO_BUILD_WITH_VULKAN AND FSO_BUILD_WITH_FFX)
	# ffx-portable's AUTO picks by OS and CMAKE_SYSTEM_PROCESSOR, which Visual Studio generators don't set per
	# architecture, so it would fetch x64-only DLLs for FSO's Win32 and ARM64 builds.
	if (WIN32 AND (NOT IS_64BIT OR IS_ARM64))
		set(FFX_PORTABLE_BACKEND "UNAVAILABLE" CACHE STRING "")
	endif()
	add_subdirectory(ffx)
	ffx_portable_get_runtime_files(FFX_RUNTIME_FILES)
	foreach(ffx_runtime_file IN LISTS FFX_RUNTIME_FILES)
		add_target_copy_files("${ffx_runtime_file}")
	endforeach()
	if (FFX_RUNTIME_FILES)
		# AMD's license has to ship with its binaries. Nothing else from the SDK is redistributed.
		set(ffx_license "${CMAKE_CURRENT_SOURCE_DIR}/ffx/third_party/amd/Kits/FidelityFX/docs/license.md")
		set_property(GLOBAL PROPERTY FSO_FFX_LICENSE_FILE "${ffx_license}")
		install(FILES "${ffx_license}" DESTINATION "${BINARY_DESTINATION}/license" RENAME amd_ffx.txt)
	endif()
endif()

add_subdirectory(mdns)

add_subdirectory(accidental-noise)

ADD_SUBDIRECTORY(lz4)

ADD_SUBDIRECTORY(imgui)
ADD_SUBDIRECTORY(implot)

ADD_SUBDIRECTORY(etcpak)

if(FSO_BUILD_WITH_OPENXR)
	add_subdirectory(openxr EXCLUDE_FROM_ALL)
endif()
