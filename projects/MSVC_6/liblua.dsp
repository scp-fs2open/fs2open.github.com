# Microsoft Developer Studio Project File - Name="liblua" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=liblua - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "liblua.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "liblua.mak" CFG="liblua - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "liblua - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "liblua - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "liblua - Win32 Release Inferno" (based on "Win32 (x86) Static Library")
!MESSAGE "liblua - Win32 Debug Inferno" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "liblua - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Profile\liblua"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Release\Profile\liblua.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "liblua - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "liblua___Win32_Debug"
# PROP BASE Intermediate_Dir "liblua___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\Profile\liblua"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug/liblua_d.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\liblua_d.lib"

!ELSEIF  "$(CFG)" == "liblua - Win32 Release Inferno"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "liblua___Win32_Release_Inferno"
# PROP BASE Intermediate_Dir "liblua___Win32_Release_Inferno"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Inferno"
# PROP Intermediate_Dir "Release_Inferno\Profile\liblua"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Release\Profile\liblua.bsc"
# ADD BSC32 /nologo /o"Release_Inferno\Profile\liblua.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "liblua - Win32 Debug Inferno"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "liblua___Win32_Debug_Inferno"
# PROP BASE Intermediate_Dir "liblua___Win32_Debug_Inferno"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Inferno"
# PROP Intermediate_Dir "Debug_Inferno\Profile\liblua"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Debug/liblua_d.bsc"
# ADD BSC32 /nologo /o"Debug_Inferno/liblua_d.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug\liblua_d.lib"
# ADD LIB32 /nologo /out:"Debug_Inferno\liblua_d.lib"

!ENDIF 

# Begin Target

# Name "liblua - Win32 Release"
# Name "liblua - Win32 Debug"
# Name "liblua - Win32 Release Inferno"
# Name "liblua - Win32 Debug Inferno"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\lua\lapi.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lauxlib.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lbaselib.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lcode.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\ldblib.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\ldebug.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\ldo.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\ldump.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lfunc.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lgc.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\linit.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\liolib.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\llex.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lmathlib.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lmem.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\loadlib.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lobject.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lopcodes.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\loslib.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lparser.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lstate.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lstring.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lstrlib.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\ltable.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\ltablib.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\ltests.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\ltm.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lundump.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lvm.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\lzio.c
# End Source File
# Begin Source File

SOURCE=..\..\lua\print.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\lua\lapi.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lauxlib.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lcode.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\ldebug.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\ldo.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lfunc.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lgc.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\llex.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\llimits.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lmem.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lobject.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lopcodes.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lparser.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lstate.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lstring.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\ltable.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\ltm.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lua.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\luaconf.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lualib.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lundump.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lvm.h
# End Source File
# Begin Source File

SOURCE=..\..\lua\lzio.h
# End Source File
# End Group
# End Target
# End Project
