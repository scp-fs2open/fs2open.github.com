# top-level files
set (file_root
	frmFRED2.cpp
	frmFRED2.h
	glcViewport.h
	wxfred2.cpp
	wxfred2.h
)

# Base files
set (file_root_base
	base/wxFRED_base.cpp
	base/wxFRED_base.h
	base/wxFRED_base.xrc
)

# Editors files
set (file_root_editors
	editors/dlgAsteroidFieldEditor.cpp
	editors/dlgAsteroidFieldEditor.h
	editors/dlgBackgroundEditor.cpp
	editors/dlgBackgroundEditor.h
	editors/dlgEventsEditor.cpp
	editors/dlgEventsEditor.h
	editors/dlgFictionViewer.cpp
	editors/dlgFictionViewer.h
	editors/dlgMissionObjectivesEditor.cpp
	editors/dlgMissionObjectivesEditor.h
	editors/dlgMissionSpecsEditor.cpp
	editors/dlgMissionSpecsEditor.h
	editors/dlgObjectEditor.cpp
	editors/dlgObjectEditor.h
	editors/dlgReinforcementsEditor.cpp
	editors/dlgReinforcementsEditor.h
	editors/dlgReinforcementsPicker.cpp
	editors/dlgReinforcementsPicker.h
	editors/dlgSetGlobalShipFlagsEditor.cpp
	editors/dlgSetGlobalShipFlagsEditor.h
	editors/dlgShieldSystemEditor.cpp
	editors/dlgShieldSystemEditor.h
	editors/dlgSoundEnvironment.cpp
	editors/dlgSoundEnvironment.h
	editors/dlgVoiceActingManager.cpp
	editors/dlgVoiceActingManager.h
	editors/frmBriefingEditor.cpp
	editors/frmBriefingEditor.h
	editors/frmCampaignEditor.cpp
	editors/frmCampaignEditor.h
	editors/frmCommandBriefingEditor.cpp
	editors/frmCommandBriefingEditor.h
	editors/frmDebriefingEditor.cpp
	editors/frmDebriefingEditor.h
	editors/frmShipsEditor.cpp
	editors/frmShipsEditor.h
	editors/frmTeamLoadoutEditor.cpp
	editors/frmTeamLoadoutEditor.h
	editors/frmWaypointEditor.cpp
	editors/frmWaypointEditor.h
	editors/frmWingEditor.cpp
	editors/frmWingEditor.h
)

# Help files
set (file_root_help
	help/dlgAboutBox.cpp
	help/dlgAboutBox.h
	help/dlgSexpHelp.cpp
	help/dlgSexpHelp.h
)

# Misc files
set (file_root_misc
	misc/dlgMissionStats.cpp
	misc/dlgMissionStats.h
)

set(file_root_res_pngs
	${CMAKE_CURRENT_SOURCE_DIR}/res/constx.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/constxy.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/constxz.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/consty.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/constyz.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/constz.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/fredknows.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/orbitsel.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/rotlocal.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/select.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/selectlist.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/selectlock.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/selectmove.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/selectrot.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/showdist.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/wingdisband.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/wingform.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/zoomext.png
	${CMAKE_CURRENT_SOURCE_DIR}/res/zoomsel.png
)

set(file_root_res
	${file_root_res_pngs}
	res/bitmap1.xpm
	res/black_do.xpm
	res/bmp00001.xpm
	res/chained.xpm
	res/chained_directive.xpm
	res/data.xpm
	res/data00.xpm
	res/data05.xpm
	res/data10.xpm
	res/data15.xpm
	res/data20.xpm
	res/data25.xpm
	res/data30.xpm
	res/data35.xpm
	res/data40.xpm
	res/data45.xpm
	res/data50.xpm
	res/data55.xpm
	res/data60.xpm
	res/data65.xpm
	res/data70.xpm
	res/data75.xpm
	res/data80.xpm
	res/data85.xpm
	res/data90.xpm
	res/data95.xpm
	res/fred.ico
	res/freddoc.ico
	res/fred_app.xpm
	res/fred_debug.xpm
	res/fred_splash.xpm
	res/green_do.xpm
	res/play.xpm
	res/root.xpm
	res/root_directive.xpm
	res/toolbar.xpm
	res/toolbar1.xpm
	res/toolbar1_x2.xpm
	res/variable.xpm
	res/V_fred.ico
	res/V_fred.xpm
	res/wxFREDicon.rc
	res/wxFREDIcon.xpm
	res/wxfred_icon-d.ico
	res/wxfred_icon.ico
)

# the source groups
source_group(""                                   FILES ${file_root})
source_group("Base"                               FILES ${file_root_base})
source_group("Editors"                            FILES ${file_root_editors})
source_group("Help"                               FILES ${file_root_help})
source_group("Misc"                               FILES ${file_root_misc})
source_group("Resources"                          FILES ${file_root_res})

# append all files to the file_root
set (file_root
	${file_root}
	${file_root_base}
	${file_root_editors}
	${file_root_help}
	${file_root_misc}
	${file_root_res}
)
