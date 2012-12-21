# Microsoft Developer Studio Project File - Name="libpng" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libpng - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libpng.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libpng.mak" CFG="libpng - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libpng - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libpng - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "libpng - Win32 Release Inferno" (based on "Win32 (x86) Static Library")
!MESSAGE "libpng - Win32 Debug Inferno" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libpng - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libpng___Win32_Release"
# PROP BASE Intermediate_Dir "libpng___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Profile\libpng"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G5 /MT /W3 /O2 /I "..\.." /I "..\..\zlib" /D "PNG_NO_MMX_CODE" /D "WIN32" /D "NDEBUG" /D "_CRT_SECURE_NO_WARNINGS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\.." /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libpng - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libpng___Win32_Debug"
# PROP BASE Intermediate_Dir "libpng___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\Profile\libpng"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G5 /MTd /W3 /Gm /ZI /Od /I "..\.." /I "..\..\zlib" /D "WIN32" /D "_DEBUG" /D "DEBUG" /D "PNG_NO_MMX_CODE" /D PNG_DEBUG=1 /D "_CRT_SECURE_NO_WARNINGS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\libpng.lib"

!ELSEIF  "$(CFG)" == "libpng - Win32 Release Inferno"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libpng___Win32_Release_Inferno"
# PROP BASE Intermediate_Dir "libpng___Win32_Release_Inferno"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Inferno"
# PROP Intermediate_Dir "Release_Inferno\Profile\libpng"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G5 /MT /W3 /O2 /I "..\.." /I "..\..\zlib" /D "PNG_NO_MMX_CODE" /D "WIN32" /D "NDEBUG" /D "_CRT_SECURE_NO_WARNINGS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\.." /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libpng - Win32 Debug Inferno"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libpng___Win32_Debug_Inferno"
# PROP BASE Intermediate_Dir "libpng___Win32_Debug_Inferno"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Inferno"
# PROP Intermediate_Dir "Debug_Inferno\Profile\libpng"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G5 /MTd /W3 /Gm /ZI /Od /I "..\.." /I "..\..\zlib" /D "WIN32" /D "_DEBUG" /D "DEBUG" /D "PNG_NO_MMX_CODE" /D PNG_DEBUG=1 /D "_CRT_SECURE_NO_WARNINGS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug_Inferno\libpng.lib"

!ENDIF 

# Begin Target

# Name "libpng - Win32 Release"
# Name "libpng - Win32 Debug"
# Name "libpng - Win32 Release Inferno"
# Name "libpng - Win32 Debug Inferno"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\libpng\png.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngerror.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngget.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngmem.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngpread.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngread.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngrio.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngrtran.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngrutil.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngset.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngtrans.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngwio.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngwrite.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngwtran.c
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngwutil.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\libpng\png.h
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngconf.h
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngdebug.h
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pnginfo.h
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pnglibconf.h
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngpriv.h
# End Source File
# Begin Source File

SOURCE=..\..\libpng\pngstruct.h
# End Source File
# End Group
# End Target
# End Project
