
# NOTE: These variables must be cached to allow QtFRED to use Qt6 prebuilt libs
#       without setting Qt6 up project wide

set(PREBUILT_VERSION_NAME "b561f6e8" CACHE INTERNAL "")
set(ANDROID_PREBUILT_VERSION_NAME "android-prebuilts-v1" CACHE INTERNAL "")

set(FSO_PREBUILT_OVERRIDE "" CACHE PATH "Path to the prebuilt binaries, if empty the binaries will be downloaded.")
set(PREBUILT_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/prebuilt" CACHE INTERNAL "")
set(CURRENT_ROOT "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "")

function(get_prebuilt_path OUT_VAR)
    if (NOT "${FSO_PREBUILT_OVERRIDE}" STREQUAL "" AND IS_DIRECTORY "${FSO_PREBUILT_OVERRIDE}")
        set(${OUT_VAR} "${FSO_PREBUILT_OVERRIDE}" PARENT_SCOPE)
        return()
    endif()

    set(PREBUILT_PATH)
    set(PREBUILT_CACHE_KEY "${PREBUILT_VERSION_NAME}")
    set(DOWNLOAD_HASH_ARGS)
    set(TAG_NAME "bin-${PREBUILT_VERSION_NAME}")
    if(PLATFORM_WINDOWS)
        if (IS_ARM64)
            set(FILENAME "bin-winarm64.zip")
        elseif (IS_64BIT)
            set(FILENAME "bin-win64.zip")
        else()
            set(FILENAME "bin-win32.zip")
        endif()
    elseif(PLATFORM_MAC)
        set(FILENAME "bin-mac.tar.gz")
    elseif(ANDROID)
        if(ANDROID_ABI STREQUAL "arm64-v8a")
            set(FILENAME "bin-android-arm64.tar.gz")
            set(PREBUILT_SHA256 "d56464fba097c16c6034ae483215045353078c0a339bb1566f0f7e20d063ca52")
        elseif(ANDROID_ABI STREQUAL "x86_64")
            set(FILENAME "bin-android-x64.tar.gz")
            set(PREBUILT_SHA256 "ab030d21200fb3ef799bb08d7a27ae973274a239dd244f51d68304e90a4021d5")
        elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
            set(FILENAME "bin-android-arm32.tar.gz")
            set(PREBUILT_SHA256 "8c040685cb4e91cf1e95d106200aa8d852f4f32859e3d1fe6d8e995168b122b6")
        elseif(ANDROID_ABI STREQUAL "x86")
            set(FILENAME "bin-android-x86.tar.gz")
            set(PREBUILT_SHA256 "020bf41dc8c1d7414c12c050b2e3abc0a701795073ed48b18706b2a89c30e473")
        else()
            message(FATAL_ERROR "Unsupported Android ABI for prebuilt libraries: ${ANDROID_ABI}")
        endif()
    else()
        # Use Linux binaries...
        if (IS_ARM64)
            set(FILENAME "bin-linux_arm64.tar.gz")
        else()
            set(FILENAME "bin-linux.tar.gz")
        endif()
    endif()
    if(ANDROID) # TODO: REMOVE THIS ONCE ANDROID PREBUILTS ARE IN THE SCP-PREBUILT REPO
        set(TAG_NAME "${ANDROID_PREBUILT_VERSION_NAME}")
        set(DOWNLOAD_URL "https://github.com/Shivansps/Fso-Android-Prebuilts/releases/download/${TAG_NAME}/${FILENAME}")
        set(DOWNLOAD_HASH_ARGS EXPECTED_HASH "SHA256=${PREBUILT_SHA256}")
        # Re-download when the Android release, ABI, or expected archive changes.
        set(PREBUILT_CACHE_KEY "${TAG_NAME}/${ANDROID_ABI}/${PREBUILT_SHA256}")
    else()
        set(DOWNLOAD_URL "https://github.com/scp-fs2open/scp-prebuilt/releases/download/${TAG_NAME}/${FILENAME}")
    endif()

    if (IS_DIRECTORY "${PREBUILT_LIB_DIR}")
        if ("${DOWNLOADED_PREBUILT_VERSION}" STREQUAL "${PREBUILT_CACHE_KEY}")
            # Libraries already downloaded and up-to-date
            set(${OUT_VAR} "${PREBUILT_LIB_DIR}" PARENT_SCOPE)
            return()
        endif()
    endif()
    set(DOWNLOAD_FILE "${CURRENT_ROOT}/${FILENAME}")

    set(MAX_RETRIES 5)
    foreach(i RANGE 1 ${MAX_RETRIES})
        if (NOT (i EQUAL 1))
            message(STATUS "Retry after 5 seconds (attempt #${i}) ...")
            execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep "5")
        endif()

        message(STATUS "Downloading prebuilt libraries from \"${DOWNLOAD_URL}\" (try ${i}/${MAX_RETRIES})")
        file(DOWNLOAD "${DOWNLOAD_URL}" "${DOWNLOAD_FILE}" SHOW_PROGRESS TLS_VERIFY ON
            ${DOWNLOAD_HASH_ARGS} STATUS DOWNLOAD_STATUS_LIST)

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
    set(DOWNLOADED_PREBUILT_VERSION "${PREBUILT_CACHE_KEY}" CACHE INTERNAL "")
    set(${OUT_VAR} "${PREBUILT_LIB_DIR}" PARENT_SCOPE)
endfunction(get_prebuilt_path)
