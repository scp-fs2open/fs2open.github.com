
if(NOT FSO_BUILD_WITH_VULKAN)
	return()
endif()

find_program(GLSLC_PATH glslc)

# Add an option for this so that this can be disabled locally when not needed
option(SHADERS_ENABLE_COMPILATION "Enable compilation of shaders to SPIR-V" OFF)

mark_as_advanced(SHADERS_ENABLE_COMPILATION)

if (SHADERS_ENABLE_COMPILATION AND GLSLC_PATH)
	if(PLATFORM_WINDOWS)
		set(SHADERTOOL_FILENAME "shadertool-windows.tar.gz")
	elseif(PLATFORM_LINUX)
		set(SHADERTOOL_FILENAME "shadertool-linux.tar.gz")
	else()
		# Platform not supported for compiling shaders
		message("Found glslc program but platform has no shadertool binaries. Not doing shader compilation...")
		return()
	endif()

	# The existence of glslc indicated whether we can compile our shaders or not
	message(STATUS "Found glslc program. Shaders will be compiled during the build.")

	set(SHADERTOOL_VERSION "v1.0")
	set(SHADERTOOL_DIR "${CMAKE_CURRENT_BINARY_DIR}/shadertool/${SHADERTOOL_VERSION}")

	# Download correct shadertool version if we do not have it already
	if (NOT IS_DIRECTORY "${SHADERTOOL_DIR}")
		set(DOWNLOAD_URL "https://github.com/scp-fs2open/fso-shadertool/releases/download/${SHADERTOOL_VERSION}/${SHADERTOOL_FILENAME}")
		set(DOWNLOAD_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SHADERTOOL_FILENAME}")

		set(MAX_RETRIES 5)
		foreach(i RANGE 1 ${MAX_RETRIES})
			if (NOT (i EQUAL 1))
				message(STATUS "Retry after 5 seconds (attempt #${i}) ...")
				execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep "5")
			endif()

			message(STATUS "Downloading shadertool binaries from \"${DOWNLOAD_URL}\" (try ${i}/${MAX_RETRIES})")
			file(DOWNLOAD "${DOWNLOAD_URL}" "${DOWNLOAD_FILE}" SHOW_PROGRESS TLS_VERIFY ON STATUS DOWNLOAD_STATUS_LIST)

			list(GET DOWNLOAD_STATUS_LIST 0 DOWNLOAD_STATUS)
			list(GET DOWNLOAD_STATUS_LIST 1 DOWNLOAD_ERROR)
			if (DOWNLOAD_STATUS EQUAL 0)
				break()
			endif()
			message(STATUS "Download of shadertool binaries failed: ${DOWNLOAD_ERROR}!")
		endforeach()

		if (NOT (DOWNLOAD_STATUS EQUAL 0))
			message(FATAL_ERROR "${MAX_RETRIES} download attempts failed!")
			return()
		endif()

		# Make sure the directory exists
		file(MAKE_DIRECTORY "${SHADERTOOL_DIR}")

		# Extract the downloaded file
		message(STATUS "Extracting shadertool package...")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf "${DOWNLOAD_FILE}"
			WORKING_DIRECTORY "${SHADERTOOL_DIR}"
			RESULT_VARIABLE EXTRACT_RESULT
			ERROR_VARIABLE ERROR_TEXT
		)

		if (NOT (EXTRACT_RESULT EQUAL 0))
			message(FATAL_ERROR "Extracting shadertool binaries failed! Error message: ${ERROR_TEXT}")
			return()
		endif()

		file(REMOVE "${DOWNLOAD_FILE}")
	endif()

	add_executable(glslc IMPORTED GLOBAL)
	set_target_properties(glslc PROPERTIES IMPORTED_LOCATION "${GLSLC_PATH}")

	# Just use CMake for finding the shadertool binaries to avoid platform specific code here
	find_program(SHADERTOOL_PATH shadertool
		PATHS "${SHADERTOOL_DIR}/bin"
		NO_DEFAULT_PATH)

	add_executable(shadertool IMPORTED GLOBAL)
	set_target_properties(shadertool PROPERTIES IMPORTED_LOCATION "${SHADERTOOL_PATH}")
endif ()

#
# Install any required libraries
#

option(VULKAN_USE_PRECOMPILED "Force use of precompiled versions of Vulkan-Loader and Shaderc." OFF)

get_prebuilt_path(PREBUILT_PATH)

set(USING_PREBUILT_VULKAN ${VULKAN_USE_PRECOMPILED})

if(PLATFORM_WINDOWS OR PLATFORM_MAC)
	set(USING_PREBUILT_VULKAN TRUE)
elseif(PLATFORM_LINUX AND FSO_BUILD_APPIMAGE)
	set(USING_PREBUILT_VULKAN TRUE)
endif()

# Shaderc - runtime compilation of glsl to SPIRV
#
# This is dynamically loaded, so we don't have to link against it. That also
# means that we don't have to jump through hoops to make sure it's available at
# build time. We just have prebuilt libs for packaging purposes and system libs
# will automatically be used otherwise.

if(USING_PREBUILT_VULKAN)
	message(STATUS "Using pre-built Shaderc library.")

	if(PLATFORM_WINDOWS)
		file(GLOB SHADERC_LIB "${PREBUILT_PATH}/shaderc/bin/*.dll")
	else()
		file(GLOB SHADERC_LIB "${PREBUILT_PATH}/shaderc/lib/lib*")
	endif()

	add_target_copy_files("${SHADERC_LIB}")
endif()

# Vulkan loader
#
# This is dynamically loaded by SDL. It's presence doesn't necessarily mean that
# Vulkan is supported, but having it here does allow things to fail more gracefully
# than if the loader is not present at all. Prebuilt lib is for packaging purposes
# and system libs are assumed to be present otherwise.

if(USING_PREBUILT_VULKAN)
	# We use MoltenVK instead of Vulkan-Loader on Mac, but if Vulkan-Loader is
	# installed system wide then SDL will prefer using it.
	if(PLATFORM_MAC)
		message(STATUS "Using pre-built MoltenVK framework.")

		unset(MOLTENVK_LIBRARY CACHE)
		find_library(MOLTENVK_LIBRARY MoltenVK PATHS "${PREBUILT_PATH}" NO_DEFAULT_PATH)

		add_target_copy_files("${MOLTENVK_LIBRARY}")
	else()
		message(STATUS "Using pre-built Vulkan-Loader library.")

		if(PLATFORM_WINDOWS)
			file(GLOB VULKAN_LOADER_LIB "${PREBUILT_PATH}/vulkan-loader/bin/*.dll")
		else()
			file(GLOB VULKAN_LOADER_LIB "${PREBUILT_PATH}/vulkan-loader/lib/lib*")
		endif()

		add_target_copy_files("${VULKAN_LOADER_LIB}")
	endif()
endif()

# Vulkan/Shaderc headers
# 
# Use prebuilt if we should, or just as a fallback if the SDK isn't installed.
# The find_package() min version should be what is used in the prebuilt repo.
# Note that we only rely on the headers and do NOT link against the Vulkan libs!

if(NOT USING_PREBUILT_VULKAN)
	find_package(Vulkan 1.4.341)
endif()

# prebuilt/fallback
if(NOT TARGET Vulkan::Headers)
	add_library(VulkanHeaders INTERFACE)

	target_include_directories(VulkanHeaders SYSTEM INTERFACE
		"${PREBUILT_PATH}/vulkan-headers/include"
		"${PREBUILT_PATH}/shaderc/include"
	)

	add_library(Vulkan::Headers ALIAS VulkanHeaders)
endif()

# Just use our VMA
add_subdirectory(VulkanMemoryAllocator)
