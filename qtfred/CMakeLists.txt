cmake_minimum_required(VERSION 3.16)

project(qtfred VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(QT6_INSTALL_ROOT "" CACHE PATH
"The path to the Qt6 installation root. May be necessary on windows if the standard find_package fails to find the Qt installation.")
list(APPEND CMAKE_PREFIX_PATH "${QT6_INSTALL_ROOT}")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL Help) 
include(source_groups.cmake)
qt_standard_project_setup()
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)
INCLUDE(InstallRequiredSystemLibraries)
qt6_wrap_ui(QTFRED_UI_GENERATED ${files_UI})
source_group("UI\\Generated"            FILES ${QTFRED_UI_GENERATED})
qt_add_executable(qtfred
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

set(CMAKE_MAP_IMPORTED_CONFIG_FASTDEBUG Release Debug)

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
    COMMAND "${_QTFRED_QMAKE}" -query QT_INSTALL_LIBS
    OUTPUT_VARIABLE _QTFRED_QT_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE)
find_program(QHELPGENERATOR_EXECUTABLE
    NAMES qhelpgenerator
    HINTS "${_QTFRED_QT_BINS}"
    REQUIRED)

set(QTFRED_HELP_QHP "${CMAKE_CURRENT_SOURCE_DIR}/help-src/doc/qtfred.qhp")
set(QTFRED_HELP_QCH "${CMAKE_CURRENT_BINARY_DIR}/qtfred_help.qch")

file(GLOB_RECURSE QTFRED_HELP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/help-src/doc/*.html"
    "${CMAKE_CURRENT_SOURCE_DIR}/help-src/doc/*.css"
    "${CMAKE_CURRENT_SOURCE_DIR}/help-src/doc/*.qhp")

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
    COMMENT "Compiling QtFRED help (qtfred_help.qch)"
    VERBATIM)

add_custom_target(qtfred_help
    DEPENDS "${QTFRED_HELP_QCH}"
    SOURCES ${QTFRED_HELP_SOURCES})

add_dependencies(qtfred qtfred_help)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/help-src"
    PREFIX "help-src"
    FILES ${QTFRED_HELP_SOURCES})

add_custom_command(TARGET qtfred_help POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:qtfred>/help"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QTFRED_HELP_QCH}"
        "$<TARGET_FILE_DIR:qtfred>/help/qtfred_help.qch"
    VERBATIM)

install(FILES "${QTFRED_HELP_QCH}"
    DESTINATION ${BINARY_DESTINATION}/help
    COMPONENT "qtFRED")
# --- end QtFRED built-in help ---

enable_clang_tidy(qtfred)
COPY_FILES_TO_TARGET(qtfred)
if(WIN32)
# Retrieve the absolute path to qmake and then use that path to find
# the windeployqt executable
get_target_property(QMAKE_EXE Qt6::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR ${QMAKE_EXE} DIRECTORY)

find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

# Run windeployqt immediately after build
add_custom_command(TARGET qtfred
    POST_BUILD
    COMMAND Qt6::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt" "$<TARGET_FILE_DIR:qtfred>/$<TARGET_FILE_NAME:qtfred>"
    )
install(
        DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
        DESTINATION ${BINARY_DESTINATION}
        COMPONENT "qtFRED"
    )

    # Windows requires that the qwindows DLL is copied as well
    execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS OUTPUT_VARIABLE QT_INSTALL_PLUGINS OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(qwindows_path "${QT_INSTALL_PLUGINS}/platforms/qwindows$<$<OR:$<CONFIG:Debug>,$<CONFIG:FastDebug>>:d>.dll")

    add_custom_command(TARGET qtfred
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${qwindows_path}" "$<TARGET_FILE_DIR:qtfred>/platforms/qwindows$<$<OR:$<CONFIG:Debug>,$<CONFIG:FastDebug>>:d>.dll"
    VERBATIM)

    install(FILES ${qwindows_path}
        DESTINATION ${BINARY_DESTINATION}/platforms
        COMPONENT "qtFRED"
    )

    # Qt Help requires the SQLite SQL driver
    set(qsqlite_path "${QT_INSTALL_PLUGINS}/sqldrivers/qsqlite$<$<OR:$<CONFIG:Debug>,$<CONFIG:FastDebug>>:d>.dll")

    add_custom_command(TARGET qtfred
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${qsqlite_path}" "$<TARGET_FILE_DIR:qtfred>/sqldrivers/qsqlite$<$<OR:$<CONFIG:Debug>,$<CONFIG:FastDebug>>:d>.dll"
    VERBATIM)

    install(FILES ${qsqlite_path}
        DESTINATION ${BINARY_DESTINATION}/sqldrivers
        COMPONENT "qtFRED"
    )
elseif(FSO_BUILD_APPIMAGE)
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
endif()

if (FSO_INSTALL_DEBUG_FILES)
    if (MSVC)
        install(FILES "$<TARGET_PDB_FILE:qtfred>"
                DESTINATION ${BINARY_DESTINATION}
                OPTIONAL)
    endif()
endif()