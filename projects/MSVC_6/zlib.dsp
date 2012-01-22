# Microsoft Developer Studio Project File - Name="zlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=zlib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak" CFG="zlib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 Release Inferno" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 Debug Inferno" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Profile\zlib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G5 /MT /W3 /O2 /D "WIN32" /D "NDEBUG" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\Profile\zlib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G5 /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\zlib.lib"

!ELSEIF  "$(CFG)" == "zlib - Win32 Release Inferno"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release Inferno"
# PROP BASE Intermediate_Dir "Release Inferno"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Inferno"
# PROP Intermediate_Dir "Release_Inferno\Profile\zlib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G5 /MT /W3 /O2 /D "WIN32" /D "NDEBUG" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Inferno"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug Inferno"
# PROP BASE Intermediate_Dir "Debug Inferno"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Inferno"
# PROP Intermediate_Dir "Debug_Inferno\Profile\zlib"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G5 /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug_Inferno\zlib.lib"

!ENDIF 

# Begin Target

# Name "zlib - Win32 Release"
# Name "zlib - Win32 Debug"
# Name "zlib - Win32 Release Inferno"
# Name "zlib - Win32 Debug Inferno"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=..\..\zlib\infback.c
# End Source File
# Begin Source File

SOURCE=..\..\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=..\..\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\zlib\zutil.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\zlib\crc32.h
# End Source File
# Begin Source File

SOURCE=..\..\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=..\..\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=..\..\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\..\zlib\inflate.h
# End Source File
# Begin Source File

SOURCE=..\..\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\..\zlib\trees.h
# End Source File
# Begin Source File

SOURCE=..\..\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=..\..\zlib\zutil.h
# End Source File
# End Group
# End Target
# End Project
