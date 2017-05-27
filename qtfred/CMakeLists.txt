# largely inspired from http://doc.qt.io/qt-5/cmake-manual.html
# Find includes in corresponding build directories

# Find the QtWidgets library
find_package(Qt5 COMPONENTS Widgets OpenGL REQUIRED)

set(QTFRED_SRC
    src/editor.h
    src/editor.cpp
    src/fredGlobals.h
    src/fredGlobals.cpp
    src/FredRenderer.h
    src/FredRenderer.cpp
    src/fredstubs.cpp
    src/iterators.h
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/object.h
    src/object.cpp
    src/QtGraphicsOperations.h
    src/QtGraphicsOperations.cpp
    src/renderwidget.h
    src/renderwidget.cpp
    src/wing.h
)

include(source_groups.cmake)

set(QTFRED_RESOURCES
    resources/resources.qrc
)

set(QTFRED_UI
    ui/mainwindow.ui
)

qt5_wrap_ui(QTFRED_UI_GENERATED ${files_ui})

source_group("UI\\Generated"            FILES ${QTFRED_UI_GENERATED})

add_executable(qtfred ${EXE_GUI_TYPE}
    ${source_files}
    ${QTFRED_UI_GENERATED}
)
set_target_properties(qtfred PROPERTIES
        AUTORCC TRUE
        AUTOMOC TRUE)

set_target_properties(qtfred PROPERTIES OUTPUT_NAME "qtfred_${FSO_BINARY_SUFFIX}")

target_compile_definitions(qtfred PRIVATE "$<$<CONFIG:RELEASE>:NDEBUG>;$<$<CONFIG:DEBUG>:_DEBUG;$<$<CXX_COMPILER_ID:MSVC>:PDB_DEBUGGING=1>>")

target_compile_definitions(qtfred PUBLIC USING_THIRD_PARTY_LIBS FRED)

target_include_directories(qtfred PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(qtfred
    PUBLIC
    code
    Qt5::Widgets Qt5::OpenGL)

include(CreateLaunchers)
create_target_launcher(qtfred
    WORKING_DIRECTORY ${FSO_FREESPACE_PATH}/data
    ARGS ${FSO_RUN_ARGUMENTS})

INSTALL(
    TARGETS qtfred
    RUNTIME DESTINATION ${BINARY_DESTINATION}
    BUNDLE DESTINATION ${BINARY_DESTINATION}
    CONFIGURATIONS Debug Release
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
        CONFIGURATIONS Debug Release
    )
endif(WIN32)
