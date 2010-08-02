# Microsoft Developer Studio Project File - Name="wxFRED2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=wxFRED2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wxFRED2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wxFRED2.mak" CFG="wxFRED2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wxFRED2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "wxFRED2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "wxFRED2 - Win32 Release Inferno" (based on "Win32 (x86) Application")
!MESSAGE "wxFRED2 - Win32 Debug Inferno" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wxFRED2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "wxFRED2___Win32_Release"
# PROP BASE Intermediate_Dir "wxFRED2___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Profile\wxfred2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(WXWIN)/include" /I "$(WXWIN)/contrib/include" /I "$(WXWIN)/lib/msw" /I "$(WXWIN)/lib/vc_lib/msw" /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /U "_DEBUG" /YX /FD /c
# SUBTRACT CPP /Z<none> /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../code/wxfred2/res" /i "$(WXWIN)/include" /i "$(WXWIN)/contrib/include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Release/Profile/wxFRED2.bsc" "Release\Profile\*.sbr"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib wxmsw28_xrc.lib wxmsw28_html.lib wxmsw28_adv.lib wxmsw28_core.lib wxbase28_xml.lib wxbase28.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libci.lib" /out:"Release/wxfred2_open_trunk_r.exe" /libpath:"$(WXWIN)/lib" /libpath:"$(WXWIN)/lib/vc_lib"
# SUBTRACT LINK32 /map /debug /nodefaultlib
# Begin Custom Build - Copying build...
ProjDir=.
InputPath=.\Release\wxfred2_open_trunk_r.exe
SOURCE="$(InputPath)"

BuildCmds= \
	copy $(InputPath) "$(FS2PATH)\wxfred2_open_trunk_r.exe" \
	copy "$(ProjDir)\..\..\code\wxfred2\wxfred.xrc" "$(FS2PATH)\wxfred.xrc" \
	

"$(FS2PATH)\wxfred2_open_trunk_r.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(FS2PATH)\wxfred.xrc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "wxFRED2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "wxFRED2___Win32_Debug"
# PROP BASE Intermediate_Dir "wxFRED2___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\Profile\wxfred2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)/include" /I "$(WXWIN)/contrib/include" /I "$(WXWIN)/lib/mswd" /I "$(WXWIN)/lib/vc_lib/mswd" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /U "NDEBUG" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../code/wxfred2/res" /i "$(WXWIN)/include" /i "$(WXWIN)/contrib/include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug/Profile/wxFRED2.bsc" "Debug\Profile\*.sbr"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib wxmsw28d_xrc.lib wxmsw28d_html.lib wxmsw28d_adv.lib wxmsw28d_core.lib wxbase28d_xml.lib wxbase28d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxexpatd.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libcid.lib" /out:"Debug/wxfred2_open_trunk_d.exe" /pdbtype:sept /libpath:"$(WXWIN)/lib" /libpath:"$(WXWIN)/lib/vc_lib"
# SUBTRACT LINK32 /nodefaultlib
# Begin Custom Build - Copying build...
ProjDir=.
InputPath=.\Debug\wxfred2_open_trunk_d.exe
SOURCE="$(InputPath)"

BuildCmds= \
	copy $(InputPath) "$(FS2PATH)/wxfred2_open_trunk_d.exe" \
	copy "$(ProjDir)\..\..\code\wxfred2\wxfred.xrc" "$(FS2PATH)\wxfred.xrc" \
	

"$(FS2PATH)/wxfred2_open_trunk_d.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(FS2PATH)\wxfred.xrc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "wxFRED2 - Win32 Release Inferno"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "wxFRED2___Win32_Release_Inferno"
# PROP BASE Intermediate_Dir "wxFRED2___Win32_Release_Inferno"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Inferno"
# PROP Intermediate_Dir "Release_Inferno\Profile\wxfred2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O2 /I "$(WXWIN)/include" /I "$(WXWIN)/contrib/include" /I "$(WXWIN)/lib/msw" /I "$(WXWIN)/lib/vc_lib/msw" /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /U "_DEBUG" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(WXWIN)/include" /I "$(WXWIN)/contrib/include" /I "$(WXWIN)/lib/msw" /I "$(WXWIN)/lib/vc_lib/msw" /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "INF_BUILD" /U "_DEBUG" /YX /FD /c
# SUBTRACT CPP /Z<none> /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "../../code/wxfred2/res" /i "$(WXWIN)/include" /i "$(WXWIN)/contrib/include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../code/wxfred2/res" /i "$(WXWIN)/include" /i "$(WXWIN)/contrib/include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Release/Profile/wxFRED2.bsc" "Release\Profile\*.sbr"
# ADD BSC32 /nologo /o"Release_Inferno/Profile/wxFRED2.bsc" "Release\Profile\*.sbr"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib wxmsw28_xrc.lib wxmsw28_html.lib wxmsw28_adv.lib wxmsw28_core.lib wxbase28_xml.lib wxbase28.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libci.lib" /out:"Release/wxfred2_open_trunk_r.exe" /libpath:"$(WXWIN)/lib" /libpath:"$(WXWIN)/lib/vc_lib"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib wxmsw28_xrc.lib wxmsw28_html.lib wxmsw28_adv.lib wxmsw28_core.lib wxbase28_xml.lib wxbase28.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libci.lib" /out:"Release_Inferno/wxfred2_open_trunk_INF_r.exe" /libpath:"$(WXWIN)/lib" /libpath:"$(WXWIN)/lib/vc_lib"
# SUBTRACT LINK32 /map /debug /nodefaultlib
# Begin Custom Build - Copying build...
ProjDir=.
InputPath=.\Release_Inferno\wxfred2_open_trunk_INF_r.exe
SOURCE="$(InputPath)"

BuildCmds= \
	copy $(InputPath) "$(FS2PATH)\wxfred2_open_trunk_INF_r.exe" \
	copy "$(ProjDir)\..\..\code\wxfred2\wxfred.xrc" "$(FS2PATH)\wxfred.xrc" \
	

"$(FS2PATH)\wxfred2_open_trunk_INF_r.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(FS2PATH)\wxfred.xrc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "wxFRED2 - Win32 Debug Inferno"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "wxFRED2___Win32_Debug_Inferno"
# PROP BASE Intermediate_Dir "wxFRED2___Win32_Debug_Inferno"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Inferno"
# PROP Intermediate_Dir "Debug_Inferno\Profile\wxfred2"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)/include" /I "$(WXWIN)/contrib/include" /I "$(WXWIN)/lib/mswd" /I "$(WXWIN)/lib/vc_lib/mswd" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /U "NDEBUG" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)/include" /I "$(WXWIN)/contrib/include" /I "$(WXWIN)/lib/mswd" /I "$(WXWIN)/lib/vc_lib/mswd" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "INF_BUILD" /U "NDEBUG" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "../../code/wxfred2/res" /i "$(WXWIN)/include" /i "$(WXWIN)/contrib/include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../code/wxfred2/res" /i "$(WXWIN)/include" /i "$(WXWIN)/contrib/include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Debug/Profile/wxFRED2.bsc" "Debug\Profile\*.sbr"
# ADD BSC32 /nologo /o"Debug_Inferno/Profile/wxFRED2.bsc" "Debug\Profile\*.sbr"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib wxmsw28d_xrc.lib wxmsw28d_html.lib wxmsw28d_adv.lib wxmsw28d_core.lib wxbase28d_xml.lib wxbase28d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxexpatd.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libcid.lib" /out:"Debug/wxfred2_open_trunk_d.exe" /pdbtype:sept /libpath:"$(WXWIN)/lib" /libpath:"$(WXWIN)/lib/vc_lib"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib wxmsw28d_xrc.lib wxmsw28d_html.lib wxmsw28d_adv.lib wxmsw28d_core.lib wxbase28d_xml.lib wxbase28d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxexpatd.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libcid.lib" /out:"Debug_Inferno/wxfred2_open_trunk_INF_d.exe" /pdbtype:sept /libpath:"$(WXWIN)/lib" /libpath:"$(WXWIN)/lib/vc_lib"
# SUBTRACT LINK32 /nodefaultlib
# Begin Custom Build - Copying build...
ProjDir=.
InputPath=.\Debug_Inferno\wxfred2_open_trunk_INF_d.exe
SOURCE="$(InputPath)"

BuildCmds= \
	copy $(InputPath) "$(FS2PATH)/wxfred2_open_trunk_INF_d.exe" \
	copy "$(ProjDir)\..\..\code\wxfred2\wxfred.xrc" "$(FS2PATH)\wxfred.xrc" \
	

"$(FS2PATH)/wxfred2_open_trunk_INF_d.exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(FS2PATH)\wxfred.xrc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# Begin Target

# Name "wxFRED2 - Win32 Release"
# Name "wxFRED2 - Win32 Debug"
# Name "wxFRED2 - Win32 Release Inferno"
# Name "wxFRED2 - Win32 Debug Inferno"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\code\wxfred2\aboutbox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\asteroidfieldeditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\campaigneditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\commandbriefingeditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\debriefingeditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\fredframe.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\missionspecseditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\setglobalshipflagseditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\shieldsystemeditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\voiceactingmanagereditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\voicefilemanager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wxfred2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wxfred_xrc.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\code\wxfred2\aboutbox.h
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\asteroidfieldeditor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\campaigneditor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\commandbriefingeditor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\debriefingeditor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\fredframe.h
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\missionspecseditor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\setglobalshipflagseditor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\shieldsystemeditor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\voiceactingmanagereditor.h
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\voicefilemanager.h
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wxfred2.h
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wxfred_xrc.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\blank.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\bullseye.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\cdrom.ico
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\computer.ico
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\drive.ico
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\file1.ico
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\floppy.ico
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\folder1.ico
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\folder2.ico
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\FRED.ico
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\hand.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\magnif1.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\noentry.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\pbrush.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\pencil.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\pntleft.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\pntright.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\query.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\removble.ico
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\rightarr.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\roller.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\size.cur
# End Source File
# Begin Source File

SOURCE=..\..\code\wxfred2\wx\msw\watch1.cur
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\code\wxfred2\wxfred.xrc

!IF  "$(CFG)" == "wxFRED2 - Win32 Release"

# Begin Custom Build - Compiling XRC resources...
InputDir=..\..\code\wxfred2
InputPath=..\..\code\wxfred2\wxfred.xrc
InputName=wxfred

"$(InputDir)/$(InputName)_xrc.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"$(WXRC)\wxrc.exe" /c /o"$(InputDir)/$(InputName)_xrc.inl" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "wxFRED2 - Win32 Debug"

# Begin Custom Build - Compiling XRC resources...
InputDir=..\..\code\wxfred2
InputPath=..\..\code\wxfred2\wxfred.xrc
InputName=wxfred

"$(InputDir)/$(InputName)_xrc.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"$(WXRC)\wxrc.exe" /c /o"$(InputDir)/$(InputName)_xrc.inl" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "wxFRED2 - Win32 Release Inferno"

# Begin Custom Build - Compiling XRC resources...
InputDir=..\..\code\wxfred2
InputPath=..\..\code\wxfred2\wxfred.xrc
InputName=wxfred

"$(InputDir)/$(InputName)_xrc.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"$(WXRC)\wxrc.exe" /c /o"$(InputDir)/$(InputName)_xrc.inl" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "wxFRED2 - Win32 Debug Inferno"

# Begin Custom Build - Compiling XRC resources...
InputDir=..\..\code\wxfred2
InputPath=..\..\code\wxfred2\wxfred.xrc
InputName=wxfred

"$(InputDir)/$(InputName)_xrc.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"$(WXRC)\wxrc.exe" /c /o"$(InputDir)/$(InputName)_xrc.inl" "$(InputPath)"

# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
