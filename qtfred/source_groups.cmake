
set(source_files)

macro(add_file_folder FOLDER_NAME)
    string(REPLACE "/" "_" VARIABLE_NAME "${FOLDER_NAME}")
    set(files_${VARIABLE_NAME} ${files_${VAR_NAME}} ${ARGN})

    string(REPLACE "/" "\\" FIXED_FOLDER_FILE "${FOLDER_NAME}")

    source_group("${FIXED_FOLDER_FILE}" FILES ${ARGN})
    set(source_files ${source_files} ${ARGN})
endmacro(add_file_folder)

add_file_folder("Source"
    src/FredApplication.cpp
    src/FredApplication.h
    src/fredstubs.cpp
    src/main.cpp
    src/util.cpp
    src/util.h
)

if (WIN32)
    add_file_folder("Source"
        src/qmain.cpp
    )
endif()

add_file_folder("Source/Mission"
    src/mission/Editor.cpp
    src/mission/Editor.h
    src/mission/EditorViewport.cpp
    src/mission/EditorViewport.h
    src/mission/FredRenderer.cpp
    src/mission/FredRenderer.h
    src/mission/IDialogProvider.h
    src/mission/management.cpp
    src/mission/management.h
    src/mission/object.cpp
    src/mission/object.h
)

add_file_folder("Source/Mission/Dialogs"
    src/mission/dialogs/AbstractDialogModel.cpp
    src/mission/dialogs/AbstractDialogModel.h
    src/mission/dialogs/EventEditorDialogModel.cpp
    src/mission/dialogs/EventEditorDialogModel.h
)

add_file_folder("Source/UI"
    src/ui/FredView.cpp
    src/ui/FredView.h
    src/ui/QtGraphicsOperations.cpp
    src/ui/QtGraphicsOperations.h
)

add_file_folder("Source/UI/Dialogs"
    src/ui/dialogs/BriefingEditorDialog.cpp
    src/ui/dialogs/BriefingEditorDialog.h
    src/ui/dialogs/EventEditorDialog.cpp
    src/ui/dialogs/EventEditorDialog.h
	src/ui/dialogs/MissionGoalsDialog.cpp
	src/ui/dialogs/MissionGoalsDialog.h
	src/ui/dialogs/TeamLoadoutDialog.cpp
	src/ui/dialogs/TeamLoadoutDialog.h
)

add_file_folder("Source/UI/Widgets"
    src/ui/widgets/ColorComboBox.cpp
    src/ui/widgets/ColorComboBox.h
    src/ui/widgets/renderwidget.cpp
    src/ui/widgets/renderwidget.h
)

add_file_folder("UI"
    ui/EventEditorDialog.ui
    ui/FredView.ui
    ui/BriefingEditorDialog.ui
	ui/MissionGoalsDialog.ui
	ui/TeamLoadoutDialog.ui
)

add_file_folder("Resources"
    resources/resources.qrc
)

if (WIN32)
    add_file_folder("Resources/Windows"
        resources/win/qtfred.manifest
        resources/win/qtfred.rc
    )
endif()

add_file_folder("Resources/Images"
    resources/images/bitmap1.png
    resources/images/black_do.png
    resources/images/bmp00001.png
    resources/images/chained_directive.png
    resources/images/chained.png
    resources/images/constx.png
    resources/images/constxy.png
    resources/images/constxz.png
    resources/images/consty.png
    resources/images/constyz.png
    resources/images/constz.png
    resources/images/cursor_rotate.png
    resources/images/data00.png
    resources/images/data05.png
    resources/images/data10.png
    resources/images/data15.png
    resources/images/data20.png
    resources/images/data25.png
    resources/images/data30.png
    resources/images/data35.png
    resources/images/data40.png
    resources/images/data45.png
    resources/images/data50.png
    resources/images/data55.png
    resources/images/data60.png
    resources/images/data65.png
    resources/images/data70.png
    resources/images/data75.png
    resources/images/data80.png
    resources/images/data85.png
    resources/images/data90.png
    resources/images/data95.png
    resources/images/data.png
    resources/images/fred_app.png
    resources/images/fred_debug.png
    resources/images/freddoc.ico
    resources/images/fred.ico
    resources/images/fredknows.png
    resources/images/fred_splash.png
    resources/images/green_do.png
    resources/images/orbitsel.png
    resources/images/play.png
    resources/images/root_directive.png
    resources/images/root.png
    resources/images/rotlocal.png
    resources/images/selectlist.png
    resources/images/selectlock.png
    resources/images/selectmove.png
    resources/images/select.png
    resources/images/selectrot.png
    resources/images/showdist.png
    resources/images/splash.png
    resources/images/toolbar1.png
    resources/images/toolbar.png
    resources/images/V_fred.ico
    resources/images/variable.png
    resources/images/wingdisband.png
    resources/images/wingform.png
    resources/images/zoomext.png
    resources/images/zoomsel.png
)
