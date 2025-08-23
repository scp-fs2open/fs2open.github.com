
# We have precompiled libs for windows
IF (WIN32)
    get_prebuilt_path(PREBUILT_PATH)

    add_library(openal SHARED IMPORTED GLOBAL)

    set(dll_name "${PREBUILT_PATH}/openal/bin/OpenAL32.dll")

    set_target_properties(openal
            PROPERTIES
            IMPORTED_LOCATION "${dll_name}"
            IMPORTED_IMPLIB "${PREBUILT_PATH}/openal/libs/${CMAKE_IMPORT_LIBRARY_PREFIX}OpenAL32${CMAKE_IMPORT_LIBRARY_SUFFIX}"
            INTERFACE_INCLUDE_DIRECTORIES "${PREBUILT_PATH}/openal/include"
            )

    add_target_copy_files("${dll_name}")
ELSEIF(PLATFORM_MAC)
    # use prebuilt openal-soft framework on Mac since the system version is
    # deprecated and lacking features
    get_prebuilt_path(PREBUILT_PATH)

    add_library(openal INTERFACE)
    unset(OPENAL_LIBRARY CACHE)
    find_library(OPENAL_LIBRARY OpenAL PATHS "${PREBUILT_PATH}" NO_DEFAULT_PATH)

    target_link_libraries(openal INTERFACE "${OPENAL_LIBRARY}")
    target_include_directories(openal SYSTEM INTERFACE "${OPENAL_LIBRARY}/Headers")

    add_target_copy_files("${OPENAL_LIBRARY}")
ELSE(WIN32)
    option(OPENAL_USE_PRECOMPILED "Use precompiled version of OpenAL. If disabled the system libraries will be used." OFF)

    set(USING_PREBUILT_LIBS FALSE)
    set(OpenAL_ROOT_DIR)

    include(util)

    if(OPENAL_USE_PRECOMPILED)
        get_prebuilt_path(PREBUILT_PATH)
        set(OpenAL_ROOT_DIR "${PREBUILT_PATH}/openal")
        set(USING_PREBUILT_LIBS TRUE)
    else()
		if(ANDROID)
			# ugly workaround, it was not being detected
			get_prebuilt_path(PREBUILT_PATH)
    		set(OPENAL_LIBRARY "${PREBUILT_PATH}/openal/lib/libopenal.so")
    		set(OPENAL_INCLUDE_DIR "${PREBUILT_PATH}/openal/include")
    	endif()
    	FIND_PACKAGE(OpenAL)
        if(OpenAL_FOUND)
            ADD_IMPORTED_LIB(openal "${OPENAL_INCLUDE_DIR}" "${OPENAL_LIBRARY}")
        else()
            message("OpenAL library could not be found. Using prebuilt library...")

            get_prebuilt_path(PREBUILT_PATH)
            set(OpenAL_ROOT_DIR "${PREBUILT_PATH}/openal")
            set(USING_PREBUILT_LIBS TRUE)
        endif()
    endif()

    if(USING_PREBUILT_LIBS)
        message(STATUS "Using pre-built OpenAL library.")

        unset(OpenAL_LOCATION CACHE)
        find_library(OpenAL_LOCATION openal PATHS "${OpenAL_ROOT_DIR}/lib" NO_DEFAULT_PATH)

        get_filename_component(FULL_LIB_PATH "${OpenAL_LOCATION}" REALPATH)
        ADD_IMPORTED_LIB(openal "${OpenAL_ROOT_DIR}/include" "${FULL_LIB_PATH}")

        file(GLOB OpenAL_LIBS "${OpenAL_ROOT_DIR}/lib/libopenal*")
        add_target_copy_files("${OpenAL_LIBS}")
    endif()
ENDIF(WIN32)
