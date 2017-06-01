
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
    src/util.cpp
    src/util.h
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
    ui/FredView.ui
)

add_file_folder(resources "Resources"
    resources/resources.qrc
)

add_file_folder(resources_images "Resources/Images"
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
    resources/images/variable.png
    resources/images/V_fred.png
    resources/images/wingdisband.png
    resources/images/wingform.png
    resources/images/zoomext.png
    resources/images/zoomsel.png
)
