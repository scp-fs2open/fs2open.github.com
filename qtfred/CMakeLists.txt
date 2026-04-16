# largely inspired from http://doc.qt.io/qt-5/cmake-manual.html
# Find includes in corresponding build directories

# Find the QtWidgets library

SET(QT5_INSTALL_ROOT "" CACHE PATH
"The path to the Qt5 installation root. May be necessary on windows if the standard find_package fails to find the Qt installation.")

list(APPEND CMAKE_PREFIX_PATH "${QT5_INSTALL_ROOT}")

find_package(Qt5 COMPONENTS Widgets OpenGL Help REQUIRED)

include(source_groups.cmake)

# When building qtFRED we need to install the runtime libraries since the Qt DLLs need them
INCLUDE(InstallRequiredSystemLibraries)

qt5_wrap_ui(QTFRED_UI_GENERATED ${files_UI})

source_group("UI\\Generated"            FILES ${QTFRED_UI_GENERATED})

add_executable(qtfred ${EXE_GUI_TYPE}
    ${source_files}
    ${QTFRED_UI_GENERATED}
)
set_target_properties(qtfred PROPERTIES
        AUTORCC TRUE
        AUTOMOC TRUE)

set_target_properties(qtfred PROPERTIES OUTPUT_NAME "qtfred_${FSO_BINARY_SUFFIX}")

target_compile_features(qtfred PUBLIC cxx_std_17)

target_compile_definitions(qtfred PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:PDB_DEBUGGING=1>")

# Undefine emit since it conflicts with our code
target_compile_definitions(qtfred PRIVATE QT_NO_EMIT)

# Define the deprecated attribute so that usage of such functions causes a warning
target_compile_definitions(qtfred PRIVATE QT_DEPRECATED_WARNINGS)

target_compile_definitions(qtfred PUBLIC USING_THIRD_PARTY_LIBS FRED)

target_include_directories(qtfred PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(WIN32)
    # We handle the main method ourself since the prebuilt version conflicts with our setup
    set_property(TARGET qtfred PROPERTY QT5_NO_LINK_QTMAIN ON)
endif()

set(CMAKE_MAP_IMPORTED_CONFIG_FASTDEBUG Release Debug)

target_link_libraries(qtfred
    PUBLIC
    code
    Qt5::Widgets Qt5::OpenGL Qt5::Help)

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
get_target_property(_QTFRED_QMAKE Qt5::qmake IMPORTED_LOCATION)
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

if (WIN32)
    set(additional_dlls
    )

    include(util)
    list_target_dependencies(qtfred qtfred_deps)

    get_target_property (QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_BINS OUTPUT_VARIABLE QT_INSTALL_BINS OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(file_paths)
    foreach(dep ${qtfred_deps})
        if ("${dep}" MATCHES "(^|;)Qt5::[A-Za-z0-9_]")
            set(file_paths ${file_paths} "$<TARGET_FILE:${dep}>")
        endif ("${dep}" MATCHES "(^|;)Qt5::[A-Za-z0-9_]")
    endforeach(dep)

    foreach(dll ${additional_dlls})
        set(file_paths ${file_paths} "${QT_INSTALL_BINS}/${dll}")
    endforeach(dll)

    foreach(path ${file_paths})
        add_custom_command(TARGET qtfred
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${path}" "$<TARGET_FILE_DIR:qtfred>"
        VERBATIM)
    endforeach(path)

    install(FILES ${file_paths}
        DESTINATION ${BINARY_DESTINATION}
        COMPONENT "qtFRED"
    )

    # Windows requires that the qwindows DLL is copied as well
    execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS OUTPUT_VARIABLE QT_INSTALL_PLUGINS OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(qwindows_path "${QT_INSTALL_PLUGINS}/platforms/qwindows$<$<CONFIG:Debug>:d>.dll")

    add_custom_command(TARGET qtfred
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${qwindows_path}" "$<TARGET_FILE_DIR:qtfred>/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
    VERBATIM)

    install(FILES ${qwindows_path}
        DESTINATION ${BINARY_DESTINATION}/platforms
        COMPONENT "qtFRED"
    )

    # Qt Help requires the SQLite SQL driver
    set(qsqlite_path "${QT_INSTALL_PLUGINS}/sqldrivers/qsqlite$<$<CONFIG:Debug>:d>.dll")

    add_custom_command(TARGET qtfred
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${qsqlite_path}" "$<TARGET_FILE_DIR:qtfred>/sqldrivers/qsqlite$<$<CONFIG:Debug>:d>.dll"
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
