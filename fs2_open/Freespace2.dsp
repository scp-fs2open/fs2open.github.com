# Microsoft Developer Studio Project File - Name="Freespace2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Freespace2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Freespace2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Freespace2.mak" CFG="Freespace2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Freespace2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Freespace2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Freespace2", RHWBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Freespace2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W2 /GX /Zi /Ot /Ow /Og /Oi /Oy /I "code" /I "c:\mssdk\include\\" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WIN32" /D "USE_OPENGL" /D "HTL" /D "MORE_SPECIES" /D "FS2_SPEECH" /U "_DEBUG" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 Quartz.lib Dxerr8.lib d3d8.lib d3dx8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib code.lib /nologo /subsystem:windows /map /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /out:"C:\games\TBP\fs2_open_r.exe" /libpath:"release" /libpath:"c:\mssdk\lib"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Copying $(InputPath) to D:\games\freespace2
InputPath=C:\games\TBP\fs2_open_r.exe
SOURCE="$(InputPath)"

"D:\games\freespace2\fs2_open_r.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) D:\games\freespace2

# End Custom Build

!ELSEIF  "$(CFG)" == "Freespace2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\Profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MTd /W4 /Gm /Gi /GX /ZI /Od /I "code" /I "c:\mssdk\include\\" /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "NO_CD_CHECK" /D "HTL" /D "MORE_SPECIES" /D "DBUGFILE_ACTIVE" /U "NDEBUG" /Fr /YX /FD /GZ /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"debug/profile/Freespace2.bsc" "./debug/profile/*.sbr"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Quartz.lib d3d8.lib d3dx8.lib DxErr8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib code.lib Dxerr8.lib d3d8.lib d3dx8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib code.lib Dxerr8.lib d3d8.lib d3dx8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib code.lib Dxerr8.lib d3d8.lib d3dx8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib code.lib /nologo /subsystem:windows /debug /debugtype:both /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /out:"C:\games\TBP\fs2_open_d.exe" /libpath:"debug" /libpath:"c:\mssdk\lib"
# SUBTRACT LINK32 /pdb:none /incremental:no /map
# Begin Custom Build - Copying $(InputPath) to D:\games\freespace2
InputPath=C:\games\TBP\fs2_open_d.exe
SOURCE="$(InputPath)"

"D:\games\freespace2\fs2_open_d.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) D:\games\freespace2

# End Custom Build

!ENDIF 

# Begin Target

# Name "Freespace2 - Win32 Release"
# Name "Freespace2 - Win32 Debug"
# Begin Source File

SOURCE=.\code\FREESPACE2\app_icon.ico
# End Source File
# Begin Source File

SOURCE=.\code\FREESPACE2\FreeSpace.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FREESPACE2\FreeSpace.h
# End Source File
# Begin Source File

SOURCE=.\code\FREESPACE2\FreeSpace.rc
# End Source File
# Begin Source File

SOURCE=.\code\FREESPACE2\FreespaceResource.h
# End Source File
# Begin Source File

SOURCE=.\code\freespace2\goal_com.bmp
# End Source File
# Begin Source File

SOURCE=.\code\freespace2\goal_fail.bmp
# End Source File
# Begin Source File

SOURCE=.\code\freespace2\goal_inc.bmp
# End Source File
# Begin Source File

SOURCE=.\code\freespace2\goal_none.bmp
# End Source File
# Begin Source File

SOURCE=.\code\freespace2\goal_ord.bmp
# End Source File
# Begin Source File

SOURCE=.\code\FREESPACE2\LevelPaging.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FREESPACE2\LevelPaging.h
# End Source File
# End Target
# End Project
