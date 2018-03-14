# largely inspired from http://doc.qt.io/qt-5/cmake-manual.html
# Find includes in corresponding build directories

# Find the QtWidgets library

SET(QT5_INSTALL_ROOT "" CACHE PATH
"The path to the Qt5 installation root. May be necessary on windows if the standard find_package fails to find the Qt installation.")

list(APPEND CMAKE_PREFIX_PATH "${QT5_INSTALL_ROOT}")

find_package(Qt5 COMPONENTS Widgets OpenGL REQUIRED)

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
    Qt5::Widgets Qt5::OpenGL)

include(CreateLaunchers)
create_target_launcher(qtfred
    WORKING_DIRECTORY ${FSO_FREESPACE_PATH}
    ARGS ${FSO_RUN_ARGUMENTS})

INSTALL(
    TARGETS qtfred
    RUNTIME DESTINATION ${BINARY_DESTINATION}
    BUNDLE DESTINATION ${BINARY_DESTINATION}
)
COPY_FILES_TO_TARGET(qtfred)

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
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${path}"  "$<TARGET_FILE_DIR:qtfred>"
        VERBATIM)
    endforeach(path)

    install(FILES ${file_paths}
        DESTINATION ${BINARY_DESTINATION}
    )
endif(WIN32)
