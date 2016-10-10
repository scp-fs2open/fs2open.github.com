
set(PREBUILT_VERSION_NAME "7775d50e9b162b9668160c4c0e7c8b9b4abeeb3c")

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
    set(FILENAME "bin-${PREBUILT_VERSION_NAME}")
    if(PLATFORM_WINDOWS)
        if (IS_64BIT)
            set(FILENAME "${FILENAME}-win64.zip")
        else()
            set(FILENAME "${FILENAME}-win32.zip")
        endif()
    elseif(PLATFORM_MAC)
        set(FILENAME "${FILENAME}-mac.tar.gz")
    else()
        # Use Linux binaries...
        set(FILENAME "${FILENAME}-linux.tar.gz")
    endif()
    set(DOWNLOAD_URL "https://bintray.com/scp-fs2open/FSO/download_file?file_path=${FILENAME}")
    set(DOWNLOAD_FILE "${CURRENT_ROOT}/${FILENAME}")
    
    message(STATUS "Downloading prebuilt libraries from \"${DOWNLOAD_URL}\"")
    file(DOWNLOAD "${DOWNLOAD_URL}" "${DOWNLOAD_FILE}" SHOW_PROGRESS TLS_VERIFY ON STATUS DOWNLOAD_STATUS_LIST)
    
    list(GET DOWNLOAD_STATUS_LIST 0 DOWNLOAD_STATUS)
    list(GET DOWNLOAD_STATUS_LIST 1 DOWNLOAD_ERROR)
    if (NOT (DOWNLOAD_STATUS EQUAL 0))
        message(FATAL_ERROR "Download of prebuilt binaries failed: ${DOWNLOAD_ERROR}!")
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
        message(FATAL_ERROR "Extracing prebuilt libraries failed! Error message: ${ERROR_TEXT}")
        return()
    endif()
    
    file(REMOVE "${DOWNLOAD_FILE}")
    
    # We are done now. Set the cache variables and return the result
    set(DOWNLOADED_PREBUILT_VERSION "${PREBUILT_VERSION_NAME}" CACHE INTERNAL "")
    set(${OUT_VAR} "${PREBUILT_LIB_DIR}" PARENT_SCOPE)
endfunction(get_prebuilt_path)
