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
!MESSAGE "Freespace2 - Win32 Release Inferno" (based on "Win32 (x86) Application")
!MESSAGE "Freespace2 - Win32 Debug Inferno" (based on "Win32 (x86) Application")
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
# PROP Intermediate_Dir "Release\Profile\freespace2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W2 /GX /O2 /I "../../code" /I "../../oggvorbis/include" /I "../../lua" /I "../../openal/include" /I "../../speech/include" /D "_MBCS" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "FS2_SPEECH" /D "FS2_VOICER" /U "_DEBUG" /FR /YX /FD /c
# SUBTRACT CPP /Z<none>
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "FS2_SPEECH" /d "FS2_VOICER"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Release\Profile\Freespace2.bsc" "Release\Profile\*.sbr"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 Quartz.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib openal32.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib theora_static.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /out:"Release/fs2_open_trunk_r.exe" /libpath:"Release" /libpath:"../../oggvorbis/lib" /libpath:"../../openal/libs/win32" /libpath:"../../openal/libs/win64" /libpath:"../../speech/lib/i386" /MAPINFO:EXPORTS /MAPINFO:LINES
# SUBTRACT LINK32 /map /debug
# Begin Custom Build - Copying build...
InputPath=.\Release\fs2_open_trunk_r.exe
SOURCE="$(InputPath)"

"$(FS2PATH)/fs2_open_trunk_r.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy /y $(InputPath) "$(FS2PATH)/fs2_open_trunk_r.exe"

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
# PROP Intermediate_Dir "Debug\Profile\freespace2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MTd /W4 /Gm /Gi /GX /ZI /Od /I "../../code" /I "../../oggvorbis/include" /I "../../lua" /I "../../openal/include" /I "../../speech/include" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "FS2_SPEECH" /D "FS2_VOICER" /U "NDEBUG" /FR /YX /FD /GZ /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "FS2_SPEECH" /d "FS2_VOICER"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug\Profile\Freespace2.bsc" "Debug\Profile\*.sbr"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Quartz.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib openal32.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib theora_static.lib /nologo /subsystem:windows /map /debug /debugtype:both /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /out:"Debug\fs2_open_trunk_d.exe" /libpath:"Debug" /libpath:"../../oggvorbis/lib" /libpath:"../../openal/libs/win32" /libpath:"../../openal/libs/win64" /libpath:"../../speech/lib/i386" /MAPINFO:EXPORTS /MAPINFO:LINES
# SUBTRACT LINK32 /pdb:none /incremental:no
# Begin Custom Build - Copying build...
InputPath=.\Debug\fs2_open_trunk_d.exe
SOURCE="$(InputPath)"

"$(FS2PATH)/fs2_open_trunk_d.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy /y $(InputPath) "$(FS2PATH)/fs2_open_trunk_d.exe"

# End Custom Build

!ELSEIF  "$(CFG)" == "Freespace2 - Win32 Release Inferno"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Freespace2___Win32_Release_Inferno"
# PROP BASE Intermediate_Dir "Freespace2___Win32_Release_Inferno"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Inferno"
# PROP Intermediate_Dir "Release_Inferno\Profile\freespace2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W2 /GX /Zi /O2 /I "../../code" /I "../../oggvorbis/include" /I "../../lua" /I "../../openal/include" /I "../../speech/include" /D "_MBCS" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "FS2_SPEECH" /D "FS2_VOICEREC" /U "_DEBUG" /FR /YX /FD /c
# ADD CPP /nologo /G5 /MT /W2 /GX /O2 /I "../../code" /I "../../oggvorbis/include" /I "../../lua" /I "../../openal/include" /I "../../speech/include" /D "_MBCS" /D "NDEBUG" /D "INF_BUILD" /D "_WINDOWS" /D "WIN32" /D "FS2_SPEECH" /D "FS2_VOICER" /U "_DEBUG" /FR /YX /FD /c
# SUBTRACT CPP /Z<none>
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "FS2_SPEECH" /d "FS2_VOICER"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Release\Profile\Freespace2.bsc" "Release\Profile\*.sbr"
# ADD BSC32 /nologo /o"Release_Inferno\Profile\Freespace2.bsc" "Release\Profile\*.sbr"
LINK32=link.exe
# ADD BASE LINK32 Quartz.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib openal32.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib theora_static.lib /nologo /subsystem:windows /map /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /out:"Release/fs2_open_trunk_r.exe" /libpath:"Release" /libpath:"../../oggvorbis/lib" /libpath:"../../openal/libs/win32" /libpath:"../../openal/libs/win64" /libpath:"../../speech/lib/i386" /MAPINFO:EXPORTS /MAPINFO:LINES
# SUBTRACT BASE LINK32 /pdb:none /debug
# ADD LINK32 Quartz.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib openal32.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib theora_static.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /out:"Release_Inferno/fs2_open_trunk_INF_r.exe" /libpath:"Release" /libpath:"../../oggvorbis/lib" /libpath:"../../openal/libs/win32" /libpath:"../../openal/libs/win64" /libpath:"../../speech/lib/i386" /MAPINFO:EXPORTS /MAPINFO:LINES
# SUBTRACT LINK32 /map /debug
# Begin Custom Build - Copying build...
InputPath=.\Release_Inferno\fs2_open_trunk_INF_r.exe
SOURCE="$(InputPath)"

"$(FS2PATH)/fs2_open_trunk_INF_r.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy /y $(InputPath) "$(FS2PATH)/fs2_open_trunk_INF_r.exe"

# End Custom Build

!ELSEIF  "$(CFG)" == "Freespace2 - Win32 Debug Inferno"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Freespace2___Win32_Debug_Inferno"
# PROP BASE Intermediate_Dir "Freespace2___Win32_Debug_Inferno"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Inferno"
# PROP Intermediate_Dir "Debug_Inferno\Profile\freespace2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W4 /Gm /Gi /GX /ZI /Od /I "../../code" /I "../../oggvorbis/include" /I "../../lua" /I "../../openal/include" /I "../../speech/include" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "FS2_SPEECH" /D "FS2_VOICEREC" /U "NDEBUG" /FR /YX /FD /GZ /GZ /c
# ADD CPP /nologo /G5 /MTd /W4 /Gm /Gi /GX /ZI /Od /I "../../code" /I "../../oggvorbis/include" /I "../../lua" /I "../../openal/include" /I "../../speech/include" /D "_DEBUG" /D "INF_BUILD" /D "_WINDOWS" /D "WIN32" /D "FS2_SPEECH" /D "FS2_VOICER" /U "NDEBUG" /FR /YX /FD /GZ /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "FS2_SPEECH" /d "FS2_VOICER"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Debug\Profile\Freespace2.bsc" "Debug\Profile\*.sbr"
# ADD BSC32 /nologo /o"Debug_Inferno\Profile\Freespace2.bsc" "Debug\Profile\*.sbr"
LINK32=link.exe
# ADD BASE LINK32 Quartz.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib openal32.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib theora_static.lib /nologo /subsystem:windows /map /debug /debugtype:both /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /out:"Debug\fs2_open_trunk_d.exe" /libpath:"Debug" /libpath:"../../oggvorbis/lib" /libpath:"../../openal/libs/win32" /libpath:"../../openal/libs/win64" /libpath:"../../speech/lib/i386" /MAPINFO:EXPORTS /MAPINFO:LINES
# SUBTRACT BASE LINK32 /pdb:none /incremental:no
# ADD LINK32 Quartz.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vfw32.lib msacm32.lib comctl32.lib openal32.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib theora_static.lib /nologo /subsystem:windows /map /debug /debugtype:both /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /out:"Debug_Inferno\fs2_open_trunk_INF_d.exe" /libpath:"Debug" /libpath:"../../oggvorbis/lib" /libpath:"../../openal/libs/win32" /libpath:"../../openal/libs/win64" /libpath:"../../speech/lib/i386" /MAPINFO:EXPORTS /MAPINFO:LINES
# SUBTRACT LINK32 /pdb:none /incremental:no
# Begin Custom Build - Copying build...
InputPath=.\Debug_Inferno\fs2_open_trunk_INF_d.exe
SOURCE="$(InputPath)"

"$(FS2PATH)/fs2_open_trunk_INF_d.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy /y $(InputPath) "$(FS2PATH)/fs2_open_trunk_INF_d.exe"

# End Custom Build

!ENDIF 

# Begin Target

# Name "Freespace2 - Win32 Release"
# Name "Freespace2 - Win32 Debug"
# Name "Freespace2 - Win32 Release Inferno"
# Name "Freespace2 - Win32 Debug Inferno"
# Begin Source File

SOURCE=..\..\code\FREESPACE2\app_icon.ico
# End Source File
# Begin Source File

SOURCE=..\..\code\FREESPACE2\FreeSpace.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\FREESPACE2\FreeSpace.h
# End Source File
# Begin Source File

SOURCE=..\..\code\FREESPACE2\FreeSpace.rc
# End Source File
# Begin Source File

SOURCE=..\..\code\FREESPACE2\FreespaceResource.h
# End Source File
# Begin Source File

SOURCE=..\..\code\freespace2\goal_com.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\freespace2\goal_fail.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\freespace2\goal_inc.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\freespace2\goal_none.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\freespace2\goal_ord.bmp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\grstub.h
# End Source File
# Begin Source File

SOURCE=..\..\code\FREESPACE2\LevelPaging.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\FREESPACE2\LevelPaging.h
# End Source File
# Begin Source File

SOURCE=..\..\code\freespace2\phrases.cfg
# End Source File
# Begin Source File

SOURCE=..\..\code\sound\phrases.cfg
# End Source File
# End Target
# End Project
