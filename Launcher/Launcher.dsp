# Microsoft Developer Studio Project File - Name="Launcher" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Launcher - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Launcher.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Launcher.mak" CFG="Launcher - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Launcher - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Launcher - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Launcher - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "FS2_SPEECH" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x809 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 Dxerr8.lib d3d8.lib d3dx8.lib dinput.lib winmm.lib /nologo /subsystem:windows /incremental:yes /machine:I386 /out:"C:\games\Freespace2\Launcher.exe"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "Launcher - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "FS2_SPEECH" /D "DBUGFILE_ACTIVE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x809 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Dxerr8.lib d3d8.lib d3dx8.lib dinput.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /out:"C:\games\Freespace2\Launcher debug.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Launcher - Win32 Release"
# Name "Launcher - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "iniparser source files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\iniparser\dictionary.c
# End Source File
# Begin Source File

SOURCE=.\iniparser\iniparser.c
# End Source File
# Begin Source File

SOURCE=.\iniparser\strlib.c
# End Source File
# End Group
# Begin Group "Tabs"

# PROP Default_Filter ""
# Begin Group "Video"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3DFXDisp.cpp
# End Source File
# Begin Source File

SOURCE=.\DX5Disp.cpp
# End Source File
# Begin Source File

SOURCE=.\DX8Disp.cpp
# End Source File
# Begin Source File

SOURCE=.\DX9Disp.cpp
# End Source File
# Begin Source File

SOURCE=.\OGLDisp.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\TabCommLine.cpp
# End Source File
# Begin Source File

SOURCE=.\TabHelp.cpp
# End Source File
# Begin Source File

SOURCE=.\TabMOD.cpp
# End Source File
# Begin Source File

SOURCE=.\TabNetwork.cpp
# End Source File
# Begin Source File

SOURCE=.\TabRegOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\TabSound.cpp
# End Source File
# Begin Source File

SOURCE=.\TabSpeech.cpp
# End Source File
# Begin Source File

SOURCE=.\TabVideo.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\dbugfile.cpp
# End Source File
# Begin Source File

SOURCE=.\Launcher.cpp
# End Source File
# Begin Source File

SOURCE=.\Launcher.rc
# End Source File
# Begin Source File

SOURCE=.\LauncherDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\settings.cpp
# End Source File
# Begin Source File

SOURCE=.\speech.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Win32func.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\3DFXDisp.h
# End Source File
# Begin Source File

SOURCE=.\DevToolsDialog.h
# End Source File
# Begin Source File

SOURCE=.\DX5Disp.h
# End Source File
# Begin Source File

SOURCE=.\DX8Disp.h
# End Source File
# Begin Source File

SOURCE=.\DX9Disp.h
# End Source File
# Begin Source File

SOURCE=.\Launcher.h
# End Source File
# Begin Source File

SOURCE=.\LauncherDlg.h
# End Source File
# Begin Source File

SOURCE=.\misc.h
# End Source File
# Begin Source File

SOURCE=.\OGLDisp.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\settings.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TabCommLine.h
# End Source File
# Begin Source File

SOURCE=.\TabHelp.h
# End Source File
# Begin Source File

SOURCE=.\TabMOD.h
# End Source File
# Begin Source File

SOURCE=.\TabNetwork.h
# End Source File
# Begin Source File

SOURCE=.\TabRegOptions.h
# End Source File
# Begin Source File

SOURCE=.\TabSound.h
# End Source File
# Begin Source File

SOURCE=.\TabSpeech.h
# End Source File
# Begin Source File

SOURCE=.\TabVideo.h
# End Source File
# Begin Source File

SOURCE=.\Win32func.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\icon fs2.ico"
# End Source File
# Begin Source File

SOURCE=.\res\Launcher.rc2
# End Source File
# Begin Source File

SOURCE=.\title.bmp
# End Source File
# Begin Source File

SOURCE=.\title2.bmp
# End Source File
# Begin Source File

SOURCE=.\title3.bmp
# End Source File
# End Group
# End Target
# End Project
