
set(source_files)

macro(add_file_folder VAR_NAME FOLDER_NAME)
    set(files_${VAR_NAME} ${ARGN})
    string(REPLACE "/" "\\" FIXED_NAME "${FOLDER_NAME}")
    source_group("${FIXED_NAME}" FILES ${files_${VAR_NAME}})
    set(source_files ${source_files} ${files_${VAR_NAME}})
endmacro(add_file_folder)

add_file_folder(src "Source"
    src/fredstubs.cpp
    src/main.cpp
)

add_file_folder(src_mission "Source/Mission"
    src/mission/editor.cpp
    src/mission/editor.h
    src/mission/FredRenderer.cpp
    src/mission/FredRenderer.h
    src/mission/iterators.h
    src/mission/object.cpp
    src/mission/object.h
    src/mission/wing.h
)

add_file_folder(src_ui "Source/UI"
    src/ui/fredGlobals.cpp
    src/ui/fredGlobals.h
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/QtGraphicsOperations.cpp
    src/ui/QtGraphicsOperations.h
    src/ui/renderwidget.cpp
    src/ui/renderwidget.h
)

add_file_folder(ui "UI"
    ui/mainwindow.ui
)

add_file_folder(resources "Resources"
    resources/resources.qrc
    resources/splash.png
)
