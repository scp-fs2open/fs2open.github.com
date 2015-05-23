# Microsoft Developer Studio Project File - Name="Fred2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Fred2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Fred2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Fred2.mak" CFG="Fred2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Fred2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Fred2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Fred2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Profile\fred2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "../../STLport-5.2.1/stlport" /I "../../code" /I "../../oggvorbis/include" /I "../../openal/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "FRED" /Fr /YX /FD /c
# SUBTRACT CPP /Z<none>
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wsock32.lib winmm.lib msacm32.lib vfw32.lib comctl32.lib openal32.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib theora_static.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /out:"Release/fred2_open_3_7_3.exe" /libpath:"Release" /libpath:"../../STLport-5.2.1/lib" /libpath:"../../oggvorbis/lib" /libpath:"../../openal/libs/win32" /libpath:"../../openal/libs/win64" /MAPINFO:EXPORTS /MAPINFO:LINES /FORCE:MULTIPLE
# SUBTRACT LINK32 /map
# Begin Custom Build - Copying build...
InputPath=.\Release\fred2_open_3_7_3.exe
SOURCE="$(InputPath)"

"$(FS2PATH)/fred2_open_3_7_3.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy /y $(InputPath) "$(FS2PATH)/fred2_open_3_7_3.exe"

# End Custom Build

!ELSEIF  "$(CFG)" == "Fred2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\Profile\fred2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /I "../../STLport-5.2.1/stlport" /I "../../code" /I "../../oggvorbis/include" /I "../../openal/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "FRED" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Quartz.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib openal32.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib theora_static.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /out:"Debug/fred2_open_3_7_3-debug.exe" /pdbtype:sept /libpath:"Debug" /libpath:"../../STLport-5.2.1/lib" /libpath:"../../oggvorbis/lib" /libpath:"../../openal/libs/win32" /libpath:"../../openal/libs/win64" /MAPINFO:EXPORTS /MAPINFO:LINES /FORCE:MULTIPLE
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Copying build...
InputPath=.\Debug\fred2_open_3_7_3-debug.exe
SOURCE="$(InputPath)"

"$(FS2PATH)/fred2_open_3_7_3-debug.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy /y $(InputPath) "$(FS2PATH)/fred2_open_3_7_3-debug.exe"

# End Custom Build

!ENDIF 

# Begin Target

# Name "Fred2 - Win32 Release"
# Name "Fred2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\code\fred2\AddVariableDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\AdjustGridDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\AltShipClassDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\AsteroidEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\BackgroundChooser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\BgBitmapDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\BriefingEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CampaignEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CampaignFilelistBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CampaignTreeView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CampaignTreeWnd.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CmdBrief.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CreateWingDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CustomWingNames.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\DebriefingEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\dialog1.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\DumpStats.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\EventEditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\FictionViewerDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\folderdlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\FRED.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\FRED.rc
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\FREDDoc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\FredRender.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\FredStubs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\FREDView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\Grid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\IgnoreOrdersDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\InitialShips.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\InitialStatus.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\Management.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\MessageEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\MissionGoalsDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\MissionNotesDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\MissionSave.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ModifyVariableDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\OperatorArgTypeSelect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\OrientEditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\PlayerStartEditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\PrefsDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ReinforcementEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\restrictpaths.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\SetGlobalShipFlags.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\Sexp_tree.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShieldSysDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ship_select.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipCheckListBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipClassEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipFlagsDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipGoalsDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipSpecialDamage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipSpecialHitpoints.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipTexturesDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\soundenvironmentdlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\StarfieldEditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\TextViewDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\VoiceActingManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\WaypointPathDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\WeaponEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\wing.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\wing_editor.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\code\fred2\AddVariableDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\AdjustGridDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\AltShipClassDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\AsteroidEditorDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\BackgroundChooser.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\BgBitmapDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\BriefingEditorDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CampaignEditorDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CampaignFilelistBox.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CampaignTreeView.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CampaignTreeWnd.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CmdBrief.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CreateWingDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\CustomWingNames.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\DebriefingEditorDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\dialog1.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\DumpStats.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\editor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\EventEditor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\FictionViewerDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\folderdlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\FRED.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\FREDDoc.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\FredRender.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\FREDView.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\Grid.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\IgnoreOrdersDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\InitialShips.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\InitialStatus.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\MainFrm.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\Management.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\MessageEditorDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\MissionGoalsDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\MissionNotesDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\MissionSave.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ModifyVariableDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\OperatorArgTypeSelect.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\OrientEditor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\PlayerStartEditor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\PrefsDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ReinforcementEditorDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\restrictpaths.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\SetGlobalShipFlags.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\Sexp_tree.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShieldSysDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ship_select.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipCheckListBox.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipClassEditorDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipEditorDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipFlagsDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipGoalsDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipSpecialDamage.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipSpecialHitpoints.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\ShipTexturesDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\soundenvironmentdlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\StarfieldEditor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\stdafx.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\TextViewDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\VoiceActingManager.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\WaypointPathDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\WeaponEditorDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\wing.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\wing_editor.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\code\fred2\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\black_do.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\chained.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\chained_directive.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\cursor2.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\data.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\FRED.ico
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\FREDDoc.ico
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\green_do.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\root.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\root_directive.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\fred2\res\variable.bmp
# End Source File
# End Group
# End Target
# End Project
