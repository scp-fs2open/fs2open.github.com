cmake_minimum_required(VERSION 3.21)

project(qtfred VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_policy(CMP0177 NEW)	# normalize install() destination paths

option(QT_USE_PRECOMPILED "Use precompiled version of Qt. If disabled the system libraries will be used." OFF)

if(QT_USE_PRECOMPILED)
    get_prebuilt_path(PREBUILT_PATH)
    set(QT6_INSTALL_ROOT "${PREBUILT_PATH}/Qt6")
else()
    SET(QT6_INSTALL_ROOT "" CACHE PATH
        "The path to the Qt6 installation root. May be necessary on windows if the standard find_package fails to find the Qt installation.")
endif()

if(QT6_INSTALL_ROOT)
    list(PREPEND CMAKE_PREFIX_PATH "${QT6_INSTALL_ROOT}")
endif()

# Must be set before find_package -- CMake snapshots this into each imported target's
# MAP_IMPORTED_CONFIG_FASTDEBUG property at target-creation time, not at link time.
# The list entries are tried in order:
#   "Debug"   -- matches the per-config IMPORTED_LOCATION_DEBUG on Qt's libraries.
#   "Release" -- per-config fallback if any target ships only a Release variant.
#   ""        -- the empty entry means "fall back to the suffix-less IMPORTED_LOCATION".
#                Required because Qt's host tools (uic/moc/rcc/qhelpgenerator) set
#                only IMPORTED_LOCATION (no IMPORTED_CONFIGURATIONS), so without this
#                CMake errors with "IMPORTED_LOCATION not set for imported target
#                Qt5::uic configuration FastDebug".
set(CMAKE_MAP_IMPORTED_CONFIG_FASTDEBUG Debug Release "")

find_package(Qt6 COMPONENTS Core Widgets OpenGL Help)

# fall back to precompiled version if system install not found
if(Qt6_FOUND AND NOT QT_USE_PRECOMPILED)
    message(STATUS "Using system Qt libraries. Don't distribute these FSO binaries if these are the GPL libraries/plugins!")
elseif(NOT Qt6_FOUND)
    if(NOT QT_USE_PRECOMPILED)
        message(STATUS "Qt6 libraries could not be found. Using prebuilt version...")

        set(QT_USE_PRECOMPILED ON CACHE BOOL "Precompiled Qt usage forced ON" FORCE)
        get_prebuilt_path(PREBUILT_PATH)
        list(PREPEND CMAKE_PREFIX_PATH "${PREBUILT_PATH}/Qt6")

        find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL Help)
    else()
        message(FATAL_ERROR "Required Qt6 libraries were not found!")
    endif()
endif()

if(CLANG_USE_LIBCXX AND NOT APPLE)
    if(QT_USE_PRECOMPILED)
        message(FATAL_ERROR "Prebuilt Qt6 libraries are not compatible with clang's libc++!")
    else()
        message(WARNING "Clang's libc++ enabled, QtFRED may not link properly!")
    endif()
endif()

include(source_groups.cmake)
qt_standard_project_setup()
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)
qt6_wrap_ui(QTFRED_UI_GENERATED ${files_UI})
source_group("UI\\Generated"            FILES ${QTFRED_UI_GENERATED})
qt_add_executable(qtfred
    ${EXE_GUI_TYPE}
    ${source_files}
    ${QTFRED_UI_GENERATED}
)
set_target_properties(qtfred PROPERTIES
        AUTORCC TRUE
        AUTOUIC FALSE)
set_target_properties(qtfred PROPERTIES OUTPUT_NAME "qtfred_${FSO_BINARY_SUFFIX}")
get_target_property(QT_PLUGINS_DIR Qt6::Core QT_PLUGINS_DIR)
if(MSVC AND QT_PLUGINS_DIR)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        VS_DEBUGGER_ENVIRONMENT "QT_QPA_PLATFORM_PLUGIN_PATH=${QT_PLUGINS_DIR}"
    )
endif()
target_compile_definitions(qtfred PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:PDB_DEBUGGING=1>")

# Undefine emit since it conflicts with our code
target_compile_definitions(qtfred PRIVATE QT_NO_EMIT)

target_compile_definitions(qtfred PUBLIC USING_THIRD_PARTY_LIBS FRED)

target_include_directories(qtfred PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(qtfred PUBLIC code Qt6::Widgets Qt6::OpenGL Qt6::Help)

include(CreateLaunchers)
create_target_launcher(qtfred
    WORKING_DIRECTORY ${FSO_FREESPACE_PATH}
    ARGS ${FSO_RUN_ARGUMENTS})

INSTALL(
    TARGETS qtfred
    RUNTIME DESTINATION ${BINARY_DESTINATION}
    COMPONENT "qtFRED"
    BUNDLE DESTINATION ${BINARY_DESTINATION}
    COMPONENT "qtFRED"
)
COPY_FILES_TO_TARGET(qtfred)

# --- QtFRED built-in help ---
get_target_property(_QTFRED_QMAKE Qt6::qmake IMPORTED_LOCATION)
execute_process(
    COMMAND "${_QTFRED_QMAKE}" -query QT_INSTALL_BINS
    OUTPUT_VARIABLE _QTFRED_QT_BINS
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND "${_QTFRED_QMAKE}" -query QT_INSTALL_LIBEXECS
    OUTPUT_VARIABLE _QTFRED_QT_LIBEXECS
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND "${_QTFRED_QMAKE}" -query QT_INSTALL_LIBS
    OUTPUT_VARIABLE _QTFRED_QT_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE)
find_program(QHELPGENERATOR_EXECUTABLE
    NAMES qhelpgenerator
    HINTS "${_QTFRED_QT_BINS}" "${_QTFRED_QT_LIBEXECS}"
    REQUIRED)

set(QTFRED_HELP_QHP  "${CMAKE_CURRENT_SOURCE_DIR}/help-src/doc/qtfred.qhp")
set(QTFRED_HELP_QCH  "${CMAKE_CURRENT_BINARY_DIR}/qtfred_help.qch")
set(QTFRED_HELP_QHCP "${CMAKE_CURRENT_BINARY_DIR}/qtfred_help.qhcp")
set(QTFRED_HELP_QHC  "${CMAKE_CURRENT_BINARY_DIR}/qtfred_help.qhc")

file(GLOB_RECURSE QTFRED_HELP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/help-src/doc/*.html"
    "${CMAKE_CURRENT_SOURCE_DIR}/help-src/doc/*.css"
    "${CMAKE_CURRENT_SOURCE_DIR}/help-src/doc/*.qhp")

# Collection project file pointing at the generated .qch (relative path is
# resolved relative to the .qhcp's directory, so both live in the build dir).
file(GENERATE OUTPUT "${QTFRED_HELP_QHCP}" CONTENT
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<QHelpCollectionProject version=\"1.0\">
    <docFiles>
        <register>
            <file>qtfred_help.qch</file>
        </register>
    </docFiles>
</QHelpCollectionProject>
")

# qhelpgenerator is a Qt application and needs a platform plugin at build time.
# On headless Linux CI the Qt platform plugins fail to dlopen because the Qt
# shared libs are not in LD_LIBRARY_PATH for dlopen-loaded plugins (DT_RUNPATH
# in the main binary does not propagate to dlopen). Pass the Qt lib dir and
# force the offscreen (no-display) platform explicitly.
add_custom_command(
    OUTPUT "${QTFRED_HELP_QCH}"
    COMMAND ${CMAKE_COMMAND} -E env
        "QT_QPA_PLATFORM=offscreen"
        "LD_LIBRARY_PATH=${_QTFRED_QT_LIBS}"
        "${QHELPGENERATOR_EXECUTABLE}" "${QTFRED_HELP_QHP}" -o "${QTFRED_HELP_QCH}"
    DEPENDS ${QTFRED_HELP_SOURCES}
    COMMENT "Compiling QtFRED help content (qtfred_help.qch)"
    VERBATIM)

# Pre-build the collection (.qhc) with the .qch already registered.  At runtime
# we copy this to AppDataLocation (writable on all platforms including macOS) and
# open it from there so Qt can write the search index alongside it.  We avoid
# creating a fresh .qhc at runtime because QHelpEngine::registerDocumentation
# bootstrapping on a new empty collection is unreliable under Qt6.
add_custom_command(
    OUTPUT "${QTFRED_HELP_QHC}"
    COMMAND ${CMAKE_COMMAND} -E env
        "QT_QPA_PLATFORM=offscreen"
        "LD_LIBRARY_PATH=${_QTFRED_QT_LIBS}"
        "${QHELPGENERATOR_EXECUTABLE}" "${QTFRED_HELP_QHCP}" -o "${QTFRED_HELP_QHC}"
    DEPENDS "${QTFRED_HELP_QCH}" "${QTFRED_HELP_QHCP}"
    COMMENT "Compiling QtFRED help collection (qtfred_help.qhc)"
    VERBATIM)

add_custom_target(qtfred_help
    DEPENDS "${QTFRED_HELP_QCH}" "${QTFRED_HELP_QHC}"
    SOURCES ${QTFRED_HELP_SOURCES})

add_dependencies(qtfred qtfred_help)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/help-src"
    PREFIX "help-src"
    FILES ${QTFRED_HELP_SOURCES})

add_custom_command(TARGET qtfred_help POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:qtfred>/$<$<BOOL:${PLATFORM_MAC}>:../Resources/>help"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QTFRED_HELP_QCH}"
        "$<TARGET_FILE_DIR:qtfred>/$<$<BOOL:${PLATFORM_MAC}>:../Resources/>help/qtfred_help.qch"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QTFRED_HELP_QHC}"
        "$<TARGET_FILE_DIR:qtfred>/$<$<BOOL:${PLATFORM_MAC}>:../Resources/>help/qtfred_help.qhc"
    VERBATIM)

install(FILES "${QTFRED_HELP_QCH}" "${QTFRED_HELP_QHC}"
    DESTINATION ${BINARY_DESTINATION}/help
    COMPONENT "qtFRED")
# --- end QtFRED built-in help ---

enable_clang_tidy(qtfred)
COPY_FILES_TO_TARGET(qtfred)

set(QTFRED_DEPLOY_PREFIX "$<TARGET_FILE_DIR:qtfred>")
set(QTFRED_DEPLOY_OPTIONS "")

if(PLATFORM_WINDOWS)
    set(QTFRED_DEPLOY_OPTIONS
        NO_COMPILER_RUNTIME
        DEPLOY_TOOL_OPTIONS
            BIN_DIR ${BINARY_DESTINATION}
            LIB_DIR ${LIBRAY_DESTINATION}
            NO_OVERWRITE
    )
elseif(PLATFORM_MAC)
    # Handling of mac resources
    set(subpath resources/mac)

    set_target_properties(qtfred PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/${subpath}/Info.plist.in")
    set_target_properties(qtfred PROPERTIES MACOSX_BUNDLE_ICON_FILE "fred_icon")
    set_target_properties(qtfred PROPERTIES MACOSX_BUNDLE_LONG_VERSION_STRING "${FSO_FULL_VERSION_STRING}")
    set_target_properties(qtfred PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING "${FSO_PRODUCT_VERSION_STRING}")
    set_target_properties(qtfred PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "qtFRED")
    set_target_properties(qtfred PROPERTIES MACOSX_DEPLOYMENT_TARGET "${CMAKE_OSX_DEPLOYMENT_TARGET}")
    set_target_properties(qtfred PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "us.indiegames.scp.qtFRED")
    set_target_properties(qtfred PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual")
    set_target_properties(qtfred PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")

    # Copy everything from the Resources directory
    add_custom_command(TARGET qtfred POST_BUILD
        COMMAND cp -a "${CMAKE_CURRENT_SOURCE_DIR}/${subpath}/Resources/*" "$<TARGET_FILE_DIR:qtfred>/../Resources"
        COMMENT "Copying resources into bundle..."
    )

    set(QTFRED_DEPLOY_PREFIX "$<TARGET_BUNDLE_DIR:qtfred>/..")
    set(QTFRED_DEPLOY_OPTIONS
        DEPLOY_TOOL_OPTIONS
            -codesign=-
            NO_APP_STORE_COMPLIANCE
            NO_OVERWRITE
    )
elseif(PLATFORM_LINUX)
    if (FSO_BUILD_APPIMAGE)
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/AppRun.in" "${CMAKE_CURRENT_BINARY_DIR}/AppRun.gen" @ONLY)
        file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AppRun-$<CONFIG>"
                INPUT "${CMAKE_CURRENT_BINARY_DIR}/AppRun.gen")

        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/AppRun-$<CONFIG>" DESTINATION "." RENAME "AppRun"
                COMPONENT "qtFRED")

        configure_file("${CMAKE_CURRENT_LIST_DIR}/cmake/AppImage.desktop.in" "${CMAKE_CURRENT_BINARY_DIR}/AppImage.desktop.gen" @ONLY)
        file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AppImage-$<CONFIG>.desktop"
                INPUT "${CMAKE_CURRENT_BINARY_DIR}/AppImage.desktop.gen")

        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/AppImage-$<CONFIG>.desktop" DESTINATION "." RENAME "qtfred.desktop"
                COMPONENT "qtFRED")

        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/fred_icon.png" DESTINATION "."
                COMPONENT "qtFRED")

        install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts" DESTINATION ${LIBRAY_DESTINATION}
                COMPONENT "qtFRED")
    endif()

    add_custom_command(
        TARGET qtfred POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts"
            "$<TARGET_FILE_DIR:qtfred>/${LIBRAY_DESTINATION}/fonts"
        COMMENT "Copying QtFRED fonts..."
        VERBATIM
    )

    set(QTFRED_DEPLOY_OPTIONS
        INCLUDE_PLUGINS
            eglfs linuxfb minimalegl minimal
            offscreen vkkhrdisplay vnc
            wayland xcb
    )
endif()

# create Qt deploy script to fixup Qt libs/plugins
qt_generate_deploy_app_script(
    TARGET qtfred
    OUTPUT_SCRIPT QTFRED_DEPLOY_SCRIPT
    NO_UNSUPPORTED_PLATFORM_ERROR
    ${QTFRED_DEPLOY_OPTIONS}
)

if(PLATFORM_LINUX AND QT_USE_PRECOMPILED)
    # Using prebuilt Qt libs on Linux poses a problem since the deploy script
    # needs to be able to find the Qt libs, which aren't in a normal location.
    # So to address this we need to modify the deploy script to change the RPATH
    # of the QtFRED binary to include this alternate location, run the deployment
    # and then reset the RPATH to it's original value.
    #
    # WARNING: We do this using undocumented internal functionality of CMake's
    #          file() command, and requires CMake 3.21 or newer to work.

    # save current RPATH to reset it later
    get_target_property(install_rpath_old qtfred INSTALL_RPATH)
    # fix delimiters
    string(REPLACE ";" ":" QTFRED_RPATH_orig "${install_rpath_old}")

    # set new RPATH to be written to binary
    # Note: file(RPATH_SET ...) cannot extend RPATH length so we have to do it
    # this way to reserve enough space for the modified script to work properly
    set_target_properties(qtfred PROPERTIES
        INSTALL_RPATH "${QTFRED_RPATH_orig}:${_QTFRED_QT_LIBS}"
    )

    # swap script name so no other code needs to be changed to use it
    set(QTFRED_DEPLOY_SCRIPT_orig ${QTFRED_DEPLOY_SCRIPT})
    set(QTFRED_DEPLOY_SCRIPT "${QTFRED_DEPLOY_SCRIPT_orig}-libfix")

    # create modified deploy script with our RPATH fix/hack
    file(GENERATE OUTPUT ${QTFRED_DEPLOY_SCRIPT} CONTENT
"file(RPATH_SET FILE $<TARGET_FILE:qtfred> NEW_RPATH \"${QTFRED_RPATH_orig}:${_QTFRED_QT_LIBS}\")
include(${QTFRED_DEPLOY_SCRIPT_orig})
file(RPATH_SET FILE $<TARGET_FILE:qtfred> NEW_RPATH \"${QTFRED_RPATH_orig}\")\n"
    )
endif()

# deploy Qt libs/plugins
add_custom_command(TARGET qtfred POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -D CMAKE_INSTALL_PREFIX=${QTFRED_DEPLOY_PREFIX}
        -D QT_DEPLOY_PREFIX=${QTFRED_DEPLOY_PREFIX}
        -D QT_DEPLOY_BIN_DIR=${BINARY_DESTINATION}
        -D QT_DEPLOY_LIB_DIR=${LIBRAY_DESTINATION}
        -P "${QTFRED_DEPLOY_SCRIPT}"
    COMMENT "Running Qt deploy script..."
    VERBATIM
)

install(CODE "
    set(QT_DEPLOY_BIN_DIR ${BINARY_DESTINATION})
    set(QT_DEPLOY_LIB_DIR ${LIBRAY_DESTINATION})
" COMPONENT "qtFRED")

install(SCRIPT "${QTFRED_DEPLOY_SCRIPT}" COMPONENT "qtFRED")

if (FSO_INSTALL_DEBUG_FILES AND MSVC)
    install(FILES "$<TARGET_PDB_FILE:qtfred>"
            DESTINATION ${BINARY_DESTINATION}
            OPTIONAL)
endif()
