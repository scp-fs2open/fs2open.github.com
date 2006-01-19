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
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /Z7 /O2 /I "../../code" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "FRED" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 DxErr8.lib d3d8.lib d3dx8.lib wsock32.lib winmm.lib msacm32.lib vfw32.lib comctl32.lib code.lib libjpeg.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /out:"Release/fred2_open_r.exe" /libpath:"Release" /libpath:"../../oggvorbis/lib" /libpath:"../../dx8sdk/lib"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Copying build...
InputPath=.\Release\fred2_open_r.exe
SOURCE="$(InputPath)"

"$(FS2PATH)/fred2_open_r.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) "$(FS2PATH)/fred2_open_r.exe"

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
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /I "../../code" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "FRED" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Quartz.lib DxErr8.lib d3d8.lib d3dx8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib code.lib libjpeg_d.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /out:"Debug/fred2_open_d.exe" /pdbtype:sept /libpath:"Debug" /libpath:"../../oggvorbis/lib" /libpath:"../../dx8sdk/lib"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Copying build...
InputPath=.\Debug\fred2_open_d.exe
SOURCE="$(InputPath)"

"$(FS2PATH)/fred2_open_d.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) "$(FS2PATH)/fred2_open_d.exe"

# End Custom Build

!ENDIF 

# Begin Target

# Name "Fred2 - Win32 Release"
# Name "Fred2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AddVariableDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AdjustGridDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AsteroidEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BgBitmapDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BriefingEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignFilelistBox.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignTreeView.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignTreeWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdBrief.cpp
# End Source File
# Begin Source File

SOURCE=.\CreateWingDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CustomWingNames.cpp
# End Source File
# Begin Source File

SOURCE=.\DebriefingEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialog1.cpp
# End Source File
# Begin Source File

SOURCE=.\DumpStats.cpp
# End Source File
# Begin Source File

SOURCE=.\EventEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\FRED.cpp
# End Source File
# Begin Source File

SOURCE=.\FRED.rc
# End Source File
# Begin Source File

SOURCE=.\FREDDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\FredRender.cpp
# End Source File
# Begin Source File

SOURCE=.\FredStubs.cpp
# End Source File
# Begin Source File

SOURCE=.\FREDView.cpp
# End Source File
# Begin Source File

SOURCE=.\Grid.cpp
# End Source File
# Begin Source File

SOURCE=.\IgnoreOrdersDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\InitialShips.cpp
# End Source File
# Begin Source File

SOURCE=.\InitialStatus.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Management.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MissionGoalsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MissionNotesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MissionSave.cpp
# End Source File
# Begin Source File

SOURCE=.\ModifyVariableDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OperatorArgTypeSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\OrientEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerStartEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\PrefsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ReinforcementEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SetGlobalShipFlags.cpp
# End Source File
# Begin Source File

SOURCE=.\Sexp_tree.cpp
# End Source File
# Begin Source File

SOURCE=.\ShieldSysDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ship_select.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipCheckListBox.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipClassEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipFlagsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipGoalsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipSpecialDamage.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipSpecialHitpoints.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipTexturesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StarfieldEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\TextViewDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\VoiceActingManager.cpp
# End Source File
# Begin Source File

SOURCE=.\VoiceFileManager.cpp
# End Source File
# Begin Source File

SOURCE=.\WaypointPathDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\WeaponEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\wing.cpp
# End Source File
# Begin Source File

SOURCE=.\wing_editor.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AddVariableDlg.h
# End Source File
# Begin Source File

SOURCE=.\AdjustGridDlg.h
# End Source File
# Begin Source File

SOURCE=.\AsteroidEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\BgBitmapDlg.h
# End Source File
# Begin Source File

SOURCE=.\BriefingEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\CampaignEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\CampaignFilelistBox.h
# End Source File
# Begin Source File

SOURCE=.\CampaignTreeView.h
# End Source File
# Begin Source File

SOURCE=.\CampaignTreeWnd.h
# End Source File
# Begin Source File

SOURCE=.\CmdBrief.h
# End Source File
# Begin Source File

SOURCE=.\CreateWingDlg.h
# End Source File
# Begin Source File

SOURCE=.\CustomWingNames.h
# End Source File
# Begin Source File

SOURCE=.\DebriefingEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\dialog1.h
# End Source File
# Begin Source File

SOURCE=.\DumpStats.h
# End Source File
# Begin Source File

SOURCE=.\editor.h
# End Source File
# Begin Source File

SOURCE=.\EventEditor.h
# End Source File
# Begin Source File

SOURCE=.\FRED.h
# End Source File
# Begin Source File

SOURCE=.\FREDDoc.h
# End Source File
# Begin Source File

SOURCE=.\FredRender.h
# End Source File
# Begin Source File

SOURCE=.\FREDView.h
# End Source File
# Begin Source File

SOURCE=.\Grid.h
# End Source File
# Begin Source File

SOURCE=.\IgnoreOrdersDlg.h
# End Source File
# Begin Source File

SOURCE=.\InitialShips.h
# End Source File
# Begin Source File

SOURCE=.\InitialStatus.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Management.h
# End Source File
# Begin Source File

SOURCE=.\MessageEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\MissionGoalsDlg.h
# End Source File
# Begin Source File

SOURCE=.\MissionNotesDlg.h
# End Source File
# Begin Source File

SOURCE=.\MissionSave.h
# End Source File
# Begin Source File

SOURCE=.\ModifyVariableDlg.h
# End Source File
# Begin Source File

SOURCE=.\OperatorArgTypeSelect.h
# End Source File
# Begin Source File

SOURCE=.\OrientEditor.h
# End Source File
# Begin Source File

SOURCE=.\PlayerStartEditor.h
# End Source File
# Begin Source File

SOURCE=.\PrefsDlg.h
# End Source File
# Begin Source File

SOURCE=.\ReinforcementEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SetGlobalShipFlags.h
# End Source File
# Begin Source File

SOURCE=.\Sexp_tree.h
# End Source File
# Begin Source File

SOURCE=.\ShieldSysDlg.h
# End Source File
# Begin Source File

SOURCE=.\ship_select.h
# End Source File
# Begin Source File

SOURCE=.\ShipCheckListBox.h
# End Source File
# Begin Source File

SOURCE=.\ShipClassEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\ShipEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\ShipFlagsDlg.h
# End Source File
# Begin Source File

SOURCE=.\ShipGoalsDlg.h
# End Source File
# Begin Source File

SOURCE=.\ShipSpecialDamage.h
# End Source File
# Begin Source File

SOURCE=.\ShipSpecialHitpoints.h
# End Source File
# Begin Source File

SOURCE=.\ShipTexturesDlg.h
# End Source File
# Begin Source File

SOURCE=.\StarfieldEditor.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\TextViewDlg.h
# End Source File
# Begin Source File

SOURCE=.\VoiceActingManager.h
# End Source File
# Begin Source File

SOURCE=.\VoiceFileManager.h
# End Source File
# Begin Source File

SOURCE=.\WaypointPathDlg.h
# End Source File
# Begin Source File

SOURCE=.\WeaponEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\wing.h
# End Source File
# Begin Source File

SOURCE=.\wing_editor.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\black_do.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\chained.bmp
# End Source File
# Begin Source File

SOURCE=.\res\chained_directive.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor2.cur
# End Source File
# Begin Source File

SOURCE=.\res\data.bmp
# End Source File
# Begin Source File

SOURCE=.\res\FRED.ico
# End Source File
# Begin Source File

SOURCE=.\res\FREDDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\green_do.bmp
# End Source File
# Begin Source File

SOURCE=.\res\root.bmp
# End Source File
# Begin Source File

SOURCE=.\res\root_directive.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\variable.bmp
# End Source File
# End Group
# End Target
# End Project
