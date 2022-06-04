
set(PREBUILT_VERSION_NAME "8a54f1d")

set(FSO_PREBUILT_OVERRIDE "" CACHE PATH "Path to the prebuilt binaries, if empty the binaries will be downloaded.")
set(PREBUILT_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/prebuilt")
set(CURRENT_ROOT "${CMAKE_CURRENT_BINARY_DIR}")

function(get_prebuilt_path OUT_VAR)
    if (IS_DIRECTORY "${PREBUILT_LIB_DIR}")
        if (NOT "${FSO_PREBUILT_OVERRIDE}" STREQUAL "")
            set(${OUT_VAR} "${FSO_PREBUILT_OVERRIDE}" PARENT_SCOPE)
            return()
        endif()

        if ("${DOWNLOADED_PREBUILT_VERSION}" STREQUAL "${PREBUILT_VERSION_NAME}")
            # Libraries already downloaded and up-to-date
            set(${OUT_VAR} "${PREBUILT_LIB_DIR}" PARENT_SCOPE)
            return()
        endif()
    endif()
    
    set(PREBUILT_PATH)
    set(TAG_NAME "bin-${PREBUILT_VERSION_NAME}")
    if(PLATFORM_WINDOWS)
        if (IS_64BIT)
            set(FILENAME "bin-win64.zip")
        else()
            set(FILENAME "bin-win32.zip")
        endif()
    elseif(PLATFORM_MAC)
        set(FILENAME "bin-mac.tar.gz")
    else()
        # Use Linux binaries...
        set(FILENAME "bin-linux.tar.gz")
    endif()
    set(DOWNLOAD_URL "https://github.com/scp-fs2open/scp-prebuilt/releases/download/${TAG_NAME}/${FILENAME}")
    set(DOWNLOAD_FILE "${CURRENT_ROOT}/${FILENAME}")

    set(MAX_RETRIES 5)
    foreach(i RANGE 1 ${MAX_RETRIES})
        if (NOT (i EQUAL 1))
            message(STATUS "Retry after 5 seconds (attempt #${i}) ...")
            execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep "5")
        endif()

        message(STATUS "Downloading prebuilt libraries from \"${DOWNLOAD_URL}\" (try ${i}/${MAX_RETRIES})")
        file(DOWNLOAD "${DOWNLOAD_URL}" "${DOWNLOAD_FILE}" SHOW_PROGRESS TLS_VERIFY ON STATUS DOWNLOAD_STATUS_LIST)

        list(GET DOWNLOAD_STATUS_LIST 0 DOWNLOAD_STATUS)
        list(GET DOWNLOAD_STATUS_LIST 1 DOWNLOAD_ERROR)
        if (DOWNLOAD_STATUS EQUAL 0)
            break()
        endif()
        message(STATUS "Download of prebuilt binaries failed: ${DOWNLOAD_ERROR}!")
    endforeach()

    if (NOT (DOWNLOAD_STATUS EQUAL 0))
        message(FATAL_ERROR "${MAX_RETRIES} download attempts failed!")
        return()
    endif()

    if (IS_DIRECTORY "${PREBUILT_LIB_DIR}")
        # Remove previous files
        file(REMOVE_RECURSE "${PREBUILT_LIB_DIR}")
        file(MAKE_DIRECTORY "${PREBUILT_LIB_DIR}")
    else()
        # Make sure the directory exists
        file(MAKE_DIRECTORY "${PREBUILT_LIB_DIR}")
    endif()
    
    
    # Extract the downloaded file
    message(STATUS "Extracting library package...")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf "${DOWNLOAD_FILE}"
        WORKING_DIRECTORY "${PREBUILT_LIB_DIR}"
        RESULT_VARIABLE EXTRACT_RESULT
        ERROR_VARIABLE ERROR_TEXT
    )
    
    if (NOT (EXTRACT_RESULT EQUAL 0))
        message(FATAL_ERROR "Extracting prebuilt libraries failed! Error message: ${ERROR_TEXT}")
        return()
    endif()
    
    file(REMOVE "${DOWNLOAD_FILE}")
    
    # We are done now. Set the cache variables and return the result
    set(DOWNLOADED_PREBUILT_VERSION "${PREBUILT_VERSION_NAME}" CACHE INTERNAL "")
    set(${OUT_VAR} "${PREBUILT_LIB_DIR}" PARENT_SCOPE)
endfunction(get_prebuilt_path)
