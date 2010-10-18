# Microsoft Developer Studio Project File - Name="code" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=code - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "code.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "code.mak" CFG="code - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "code - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "code - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "code - Win32 Release Inferno" (based on "Win32 (x86) Static Library")
!MESSAGE "code - Win32 Debug Inferno" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Freespace2/code", UCACAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "code - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Profile\code"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /Ob2 /I "../../code" /I "../../oggvorbis/include" /I "../../lua" /I "../../openal/include" /I "../../speech/include" /I "../../libpng" /I "../../zlib" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "FS2_SPEECH" /D "FS2_VOICER" /U "_DEBUG" /FR /YX /FD /c
# SUBTRACT CPP /Z<none>
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Release\Profile\code.bsc" "Release\Profile\*.sbr"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\Profile\code"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../../code" /I "../../oggvorbis/include" /I "../../lua" /I "../../openal/include" /I "../../speech/include" /I "../../libpng" /I "../../zlib" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "FS2_SPEECH" /D "FS2_VOICER" /U "NDEBUG" /FR /YX /FD /GZ /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug\Profile\code.bsc" "Debug\Profile\*.sbr"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "code - Win32 Release Inferno"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "code___Win32_Release_Inferno"
# PROP BASE Intermediate_Dir "code___Win32_Release_Inferno"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Inferno"
# PROP Intermediate_Dir "Release_Inferno"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /Zi /O2 /Ob2 /I "../../code" /I "../../oggvorbis/include" /I "../../lua" /I "../../openal/include" /I "../../speech/include" /I "../../libpng" /I "../../zlib" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "NO_DIRECT3D" /D "FS2_SPEECH" /D "FS2_VOICEREC" /U "_DEBUG" /FR /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /Ob2 /I "../../code" /I "../../oggvorbis/include" /I "../../lua" /I "../../openal/include" /I "../../speech/include" /I "../../libpng" /I "../../zlib" /D "NDEBUG" /D "INF_BUILD" /D "_WINDOWS" /D "WIN32" /D "NO_DIRECT3D" /D "FS2_SPEECH" /D "FS2_VOICER" /U "_DEBUG" /FR /YX /FD /c
# SUBTRACT CPP /Z<none>
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Release\Profile\code.bsc" "Release\Profile\*.sbr"
# ADD BSC32 /nologo /o"Release_Inferno\Profile\code.bsc" "Release\Profile\*.sbr"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "code - Win32 Debug Inferno"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "code___Win32_Debug_Inferno"
# PROP BASE Intermediate_Dir "code___Win32_Debug_Inferno"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Inferno"
# PROP Intermediate_Dir "Debug_Inferno"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../../code" /I "../../oggvorbis/include" /I "../../lua" /I "../../openal/include" /I "../../speech/include" /I "../../libpng" /I "../../zlib" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "NO_DIRECT3D" /D "FS2_SPEECH" /D "FS2_VOICEREC" /U "NDEBUG" /FR /YX /FD /GZ /GZ /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../../code" /I "../../oggvorbis/include" /I "../../lua" /I "../../openal/include" /I "../../speech/include" /I "../../libpng" /I "../../zlib" /D "_DEBUG" /D "INF_BUILD" /D "_WINDOWS" /D "WIN32" /D "NO_DIRECT3D" /D "FS2_SPEECH" /D "FS2_VOICER" /U "NDEBUG" /FR /YX /FD /GZ /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Debug\Profile\code.bsc" "Debug\Profile\*.sbr"
# ADD BSC32 /nologo /o"Debug_Inferno\Profile\code.bsc" "Debug\Profile\*.sbr"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "code - Win32 Release"
# Name "code - Win32 Debug"
# Name "code - Win32 Release Inferno"
# Name "code - Win32 Debug Inferno"
# Begin Group "AI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\ai\ai.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ai\ai.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ai\ai_profiles.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ai\ai_profiles.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ai\AiBig.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ai\AiBig.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ai\AiCode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ai\AiGoals.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ai\AiGoals.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ai\AiInternal.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ai\AiLocal.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ai\AiTurret.cpp
# End Source File
# End Group
# Begin Group "Anim"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Anim\AnimPlay.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Anim\AnimPlay.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Anim\PackUnpack.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Anim\PackUnpack.h
# End Source File
# End Group
# Begin Group "Asteroid"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Asteroid\Asteroid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Asteroid\Asteroid.h
# End Source File
# End Group
# Begin Group "Autopilot"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Autopilot\Autopilot.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Autopilot\Autopilot.h
# End Source File
# End Group
# Begin Group "Bmpman"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\bmpman\bm_internal.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Bmpman\BmpMan.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Bmpman\BmpMan.h
# End Source File
# End Group
# Begin Group "Camera"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\camera\camera.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\camera\camera.h
# End Source File
# End Group
# Begin Group "CFile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\CFile\cfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\CFile\cfile.h
# End Source File
# Begin Source File

SOURCE=..\..\code\CFile\CfileArchive.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\CFile\CfileArchive.h
# End Source File
# Begin Source File

SOURCE=..\..\code\CFile\CfileList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\CFile\CfileSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\CFile\CfileSystem.h
# End Source File
# End Group
# Begin Group "Cmdline"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Cmdline\cmdline.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Cmdline\cmdline.h
# End Source File
# End Group
# Begin Group "CMeasure"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\CMeasure\CMeasure.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\CMeasure\CMeasure.h
# End Source File
# End Group
# Begin Group "ControlConfig"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\ControlConfig\ControlsConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ControlConfig\ControlsConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ControlConfig\ControlsConfigCommon.cpp
# End Source File
# End Group
# Begin Group "Cutscene"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Cutscene\Cutscenes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Cutscene\Cutscenes.h
# End Source File
# Begin Source File

SOURCE=..\..\code\cutscene\decoder16.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\cutscene\decoder8.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\cutscene\movie.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\cutscene\movie.h
# End Source File
# Begin Source File

SOURCE=..\..\code\cutscene\mve_audio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\cutscene\mvelib.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\cutscene\mvelib.h
# End Source File
# Begin Source File

SOURCE=..\..\code\cutscene\mveplayer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\cutscene\oggplayer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\cutscene\oggplayer.h
# End Source File
# End Group
# Begin Group "ddsutils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\ddsutils\ddsutils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ddsutils\ddsutils.h
# End Source File
# End Group
# Begin Group "Debris"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Debris\Debris.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Debris\Debris.h
# End Source File
# End Group
# Begin Group "DebugConsole"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\DebugConsole\Console.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\debugconsole\dbugfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\debugconsole\dbugfile.h
# End Source File
# Begin Source File

SOURCE=..\..\code\debugconsole\timerbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\debugconsole\timerbar.h
# End Source File
# End Group
# Begin Group "Demo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Demo\Demo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Demo\Demo.h
# End Source File
# End Group
# Begin Group "DirectX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\DirectX\vasync.h
# End Source File
# Begin Source File

SOURCE=..\..\code\DirectX\vDinput.h
# End Source File
# Begin Source File

SOURCE=..\..\code\DirectX\vdplay.h
# End Source File
# Begin Source File

SOURCE=..\..\code\DirectX\vdplobby.h
# End Source File
# Begin Source File

SOURCE=..\..\code\DirectX\vdsetup.h
# End Source File
# Begin Source File

SOURCE=..\..\code\DirectX\vdvp.h
# End Source File
# Begin Source File

SOURCE=..\..\code\DirectX\vDinput.lib
# End Source File
# Begin Source File

SOURCE=..\..\code\directx\strmiids.lib
# End Source File
# Begin Source File

SOURCE=..\..\code\directx\dxguid.lib
# End Source File
# End Group
# Begin Group "ExceptionHandler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\ExceptionHandler\ExceptionHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ExceptionHandler\ExceptionHandler.h
# End Source File
# End Group
# Begin Group "external_dll"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\external_dll\externalcode.h
# End Source File
# Begin Source File

SOURCE=..\..\code\external_dll\trackirglobal.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\external_dll\trackirpublic.h
# End Source File
# End Group
# Begin Group "Fireball"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Fireball\FireBalls.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Fireball\FireBalls.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Fireball\WarpInEffect.cpp
# End Source File
# End Group
# Begin Group "fs2netd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\fs2netd\fs2netd_client.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fs2netd\fs2netd_client.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fs2netd\protocol.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fs2netd\tcp_client.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fs2netd\tcp_client.h
# End Source File
# Begin Source File

SOURCE=..\..\code\fs2netd\tcp_socket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\fs2netd\tcp_socket.h
# End Source File
# End Group
# Begin Group "GameHelp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\GameHelp\ContextHelp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\GameHelp\ContextHelp.h
# End Source File
# Begin Source File

SOURCE=..\..\code\GameHelp\GameplayHelp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\GameHelp\GameplayHelp.h
# End Source File
# End Group
# Begin Group "GameSequence"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\GameSequence\GameSequence.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\GameSequence\GameSequence.h
# End Source File
# End Group
# Begin Group "GameSnd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Gamesnd\EventMusic.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Gamesnd\EventMusic.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Gamesnd\GameSnd.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Gamesnd\GameSnd.h
# End Source File
# End Group
# Begin Group "GlobalIncs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\GlobalIncs\AlphaColors.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\GlobalIncs\AlphaColors.h
# End Source File
# Begin Source File

SOURCE=..\..\code\GlobalIncs\crypt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\GlobalIncs\crypt.h
# End Source File
# Begin Source File

SOURCE=..\..\code\globalincs\def_files.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\globalincs\def_files.h
# End Source File
# Begin Source File

SOURCE=..\..\code\globalincs\fsmemory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\globalincs\fsmemory.h
# End Source File
# Begin Source File

SOURCE=..\..\code\globalincs\globals.h
# End Source File
# Begin Source File

SOURCE=..\..\code\GlobalIncs\LinkList.h
# End Source File
# Begin Source File

SOURCE=..\..\code\globalincs\mspdb_callstack.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\globalincs\mspdb_callstack.h
# End Source File
# Begin Source File

SOURCE=..\..\code\GlobalIncs\PsTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\code\globalincs\safe_strings.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\globalincs\safe_strings.h
# End Source File
# Begin Source File

SOURCE=..\..\code\globalincs\safe_strings_test.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\GlobalIncs\SystemVars.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\GlobalIncs\SystemVars.h
# End Source File
# Begin Source File

SOURCE=..\..\code\GlobalIncs\version.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\GlobalIncs\version.h
# End Source File
# Begin Source File

SOURCE=..\..\code\globalincs\vmallocator.h
# End Source File
# Begin Source File

SOURCE=..\..\code\GlobalIncs\WinDebug.cpp
# End Source File
# End Group
# Begin Group "Graphics"

# PROP Default_Filter ""
# Begin Group "SoftwareGr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Graphics\Font.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Graphics\Font.h
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\generic.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\generic.h
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\grstub.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\grstub.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Graphics\Line.h
# End Source File
# End Group
# Begin Group "OpenGLGr"

# PROP Default_Filter ""
# Begin Group "OpenGL Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\graphics\gropengl.h
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropenglbmpman.h
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropengldraw.h
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropenglextension.h
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropengllight.h
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropenglpostprocessing.h
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropenglshader.h
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropenglstate.h
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropengltexture.h
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropengltnl.h
# End Source File
# End Group
# Begin Group "OpenGL CPPs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\graphics\gropengl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropenglbmpman.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropengldraw.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropenglextension.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropengllight.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropenglpostprocessing.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropenglshader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropenglstate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropengltexture.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\gropengltnl.cpp
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=..\..\code\Graphics\2d.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Graphics\2d.h
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\grbatch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\graphics\grbatch.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Graphics\GrInternal.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Graphics\TMAPPER.H
# End Source File
# End Group
# Begin Group "Hud"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Hud\HUD.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUD.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HudArtillery.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HudArtillery.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDbrackets.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDbrackets.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDconfig.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDconfig.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDescort.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDescort.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDets.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDets.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDgauges.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDlock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDlock.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDmessage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDmessage.h
# End Source File
# Begin Source File

SOURCE=..\..\code\hud\HUDNavigation.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\hud\HUDNavigation.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDObserver.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDObserver.h
# End Source File
# Begin Source File

SOURCE=..\..\code\hud\hudparse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\hud\hudparse.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDreticle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDreticle.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDshield.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDshield.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDsquadmsg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDsquadmsg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDtarget.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDtarget.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDtargetbox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDtargetbox.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDWingmanStatus.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Hud\HUDWingmanStatus.h
# End Source File
# End Group
# Begin Group "iff_defs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\iff_defs\iff_defs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\iff_defs\iff_defs.h
# End Source File
# End Group
# Begin Group "InetFile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Inetfile\CFtp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Inetfile\CFtp.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Inetfile\Chttpget.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Inetfile\Chttpget.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Inetfile\inetgetfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Inetfile\inetgetfile.h
# End Source File
# End Group
# Begin Group "Io"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Io\Joy.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\Joy.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\Joy_ff.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\Joy_ff.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\Key.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\Key.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\KeyControl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\KeyControl.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\Mouse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\Mouse.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\sw_error.hpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\sw_force.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\sw_guid.hpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\swff_lib.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\Timer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Io\Timer.h
# End Source File
# End Group
# Begin Group "jpgutils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\jpgutils\jpgutils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\jpgutils\jpgutils.h
# End Source File
# End Group
# Begin Group "JumpNode"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\JumpNode\JumpNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\JumpNode\JumpNode.h
# End Source File
# End Group
# Begin Group "Lab"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\lab\lab.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\lab\lab.h
# End Source File
# Begin Source File

SOURCE=..\..\code\lab\wmcgui.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\lab\wmcgui.h
# End Source File
# End Group
# Begin Group "Lighting"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Lighting\Lighting.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Lighting\Lighting.h
# End Source File
# End Group
# Begin Group "Localization"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Localization\fhash.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Localization\fhash.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Localization\localize.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Localization\localize.h
# End Source File
# End Group
# Begin Group "Math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\math\bitarray.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Math\Fix.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Math\fix.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Math\Floating.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Math\Floating.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Math\Fvi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Math\Fvi.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Math\spline.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Math\spline.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Math\StaticRand.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Math\StaticRand.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Math\VecMat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Math\VecMat.h
# End Source File
# End Group
# Begin Group "MenuUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\MenuUI\Barracks.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\Barracks.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\Credits.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\Credits.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\fishtank.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\fishtank.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\MainHallMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\MainHallMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\MainHallTemp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\MainHallTemp.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\OptionsMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\OptionsMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\OptionsMenuMulti.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\OptionsMenuMulti.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\PlayerMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\PlayerMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\ReadyRoom.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\ReadyRoom.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\SnazzyUI.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\SnazzyUI.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\TechMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\TechMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\TrainingMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MenuUI\TrainingMenu.h
# End Source File
# End Group
# Begin Group "Mission"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Mission\MissionBriefCommon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionBriefCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionCampaign.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionCampaign.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionGoals.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionGoals.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionGrid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionGrid.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionHotKey.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionHotKey.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionLoad.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionLoad.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionLog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionLog.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionMessage.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionParse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionParse.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionTraining.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Mission\MissionTraining.h
# End Source File
# End Group
# Begin Group "MissionUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\MissionUI\Chatbox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\Chatbox.h
# End Source File
# Begin Source File

SOURCE=..\..\code\missionui\fictionviewer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\missionui\fictionviewer.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionBrief.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionBrief.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionCmdBrief.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionCmdBrief.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionDebrief.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionDebrief.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionLoopBrief.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionLoopBrief.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionPause.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionPause.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionRecommend.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionRecommend.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionScreenCommon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionScreenCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionShipChoice.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionShipChoice.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionStats.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionStats.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionWeaponChoice.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\MissionWeaponChoice.h
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\RedAlert.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\MissionUI\RedAlert.h
# End Source File
# End Group
# Begin Group "Model"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Model\MODEL.H
# End Source File
# Begin Source File

SOURCE=..\..\code\model\ModelAnim.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\model\modelanim.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Model\ModelCollide.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Model\ModelInterp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Model\ModelOctant.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Model\ModelRead.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Model\ModelsInc.h
# End Source File
# End Group
# Begin Group "Nebula"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Nebula\Neb.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Nebula\Neb.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Nebula\NebLightning.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Nebula\NebLightning.h
# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\network\chat_api.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\network\chat_api.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\Multi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\Multi.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_campaign.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_campaign.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_data.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_data.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_dogfight.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_dogfight.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_endgame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_endgame.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_ingame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_ingame.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_kick.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_kick.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_log.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_obj.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_obj.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_observer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_observer.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_oo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_oo.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_options.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_options.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_pause.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_pause.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_pinfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_pinfo.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_ping.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_ping.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_pmsg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_pmsg.h
# End Source File
# Begin Source File

SOURCE=..\..\code\network\multi_pxo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\network\multi_pxo.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_rate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_rate.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_respawn.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_respawn.h
# End Source File
# Begin Source File

SOURCE=..\..\code\network\multi_sexp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\network\multi_sexp.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_team.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_team.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_update.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_update.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_voice.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_voice.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_xfer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multi_xfer.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multilag.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multilag.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\MultiMsgs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\multimsgs.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\MultiTeamSelect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\MultiTeamSelect.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\MultiUI.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\MultiUI.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\MultiUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\MultiUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\PsNet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\PsNet.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\Psnet2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\Psnet2.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\stand_gui.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Network\stand_gui.h
# End Source File
# End Group
# Begin Group "Object"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Object\CollideDebrisShip.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Object\CollideDebrisWeapon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Object\CollideShipShip.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Object\CollideShipWeapon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Object\CollideWeaponWeapon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\object\deadobjectdock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\object\deadobjectdock.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Object\ObjCollide.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Object\ObjCollide.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Object\Object.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Object\Object.h
# End Source File
# Begin Source File

SOURCE=..\..\code\object\objectdock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\object\objectdock.h
# End Source File
# Begin Source File

SOURCE=..\..\code\object\objectshield.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\object\objectshield.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Object\ObjectSnd.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Object\ObjectSnd.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Object\ObjectSort.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\object\parseobjectdock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\object\parseobjectdock.h
# End Source File
# Begin Source File

SOURCE=..\..\code\object\waypoint.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\object\waypoint.h
# End Source File
# End Group
# Begin Group "Observer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Observer\Observer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Observer\Observer.h
# End Source File
# End Group
# Begin Group "OsApi"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\OsApi\MONOPUB.H
# End Source File
# Begin Source File

SOURCE=..\..\code\OsApi\OsApi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\OsApi\OsApi.h
# End Source File
# Begin Source File

SOURCE=..\..\code\OsApi\OsRegistry.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\OsApi\OsRegistry.h
# End Source File
# Begin Source File

SOURCE=..\..\code\OsApi\OutWnd.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\OsApi\OutWnd.h
# End Source File
# End Group
# Begin Group "Palman"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Palman\PalMan.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Palman\PalMan.h
# End Source File
# End Group
# Begin Group "Parse"

# PROP Default_Filter ""
# Begin Group "Scripting"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\parse\lua.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\parse\lua.h
# End Source File
# Begin Source File

SOURCE=..\..\code\parse\scripting.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\parse\scripting.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\code\Parse\Encrypt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Parse\Encrypt.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Parse\PARSELO.CPP
# End Source File
# Begin Source File

SOURCE=..\..\code\Parse\parselo.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Parse\SEXP.CPP
# End Source File
# Begin Source File

SOURCE=..\..\code\Parse\SEXP.H
# End Source File
# End Group
# Begin Group "Particle"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Particle\Particle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Particle\Particle.h
# End Source File
# End Group
# Begin Group "PcxUtils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\PcxUtils\pcxutils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\PcxUtils\pcxutils.h
# End Source File
# End Group
# Begin Group "Physics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Physics\Physics.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Physics\Physics.h
# End Source File
# End Group
# Begin Group "Playerman"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Playerman\ManagePilot.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Playerman\ManagePilot.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Playerman\Player.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Playerman\PlayerControl.cpp
# End Source File
# End Group
# Begin Group "pngutils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\pngutils\pngutils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\pngutils\pngutils.h
# End Source File
# End Group
# Begin Group "Popup"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Popup\Popup.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Popup\Popup.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Popup\PopupDead.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Popup\PopupDead.h
# End Source File
# End Group
# Begin Group "Radar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Radar\Radar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Radar\Radar.h
# End Source File
# Begin Source File

SOURCE=..\..\code\radar\radarorb.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\radar\radarorb.h
# End Source File
# Begin Source File

SOURCE=..\..\code\radar\radarsetup.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\radar\radarsetup.h
# End Source File
# End Group
# Begin Group "Render"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Render\3D.H
# End Source File
# Begin Source File

SOURCE=..\..\code\Render\3dClipper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Render\3ddraw.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Render\3dInternal.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Render\3dLaser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Render\3dMath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Render\3dSetup.cpp
# End Source File
# End Group
# Begin Group "Ship"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\ship\Afterburner.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\Afterburner.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\AWACS.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\AWACS.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\Shield.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\Ship.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\Ship.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\ShipContrails.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\ShipContrails.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\ShipFX.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\ShipFX.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\ShipHit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\ShipHit.h
# End Source File
# Begin Source File

SOURCE=..\..\code\ship\SubsysDamage.h
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Group "ogg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\sound\ogg\ogg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\sound\ogg\ogg.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\code\sound\acm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\acm.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\AudioStr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\AudioStr.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\channel.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\ds.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\ds.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\ds3d.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\ds3d.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\dscap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\dscap.h
# End Source File
# Begin Source File

SOURCE=..\..\code\sound\fsspeech.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\sound\fsspeech.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\midiseq.h
# End Source File
# Begin Source File

SOURCE=..\..\code\sound\openal.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\sound\openal.h
# End Source File
# Begin Source File

SOURCE=..\..\code\sound\phrases.xml

!IF  "$(CFG)" == "code - Win32 Release"

# Begin Custom Build
InputDir=..\..\code\sound
InputPath=..\..\code\sound\phrases.xml
InputName=phrases

BuildCmds= \
	"$(InputDir)\gc" "$(InputDir)\$(InputName)" \
	"$(InputDir)\gc" /h "$(InputDir)\grammar.h" "$(InputDir)\$(InputName)" \
	

"$(InputDir)\phrases.cfg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\grammar.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

# Begin Custom Build
InputDir=..\..\code\sound
InputPath=..\..\code\sound\phrases.xml
InputName=phrases

BuildCmds= \
	"$(InputDir)\gc" "$(InputDir)\$(InputName)" \
	"$(InputDir)\gc" /h "$(InputDir)\grammar.h" "$(InputDir)\$(InputName)" \
	

"$(InputDir)\phrases.cfg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\grammar.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "code - Win32 Release Inferno"

# Begin Custom Build
InputDir=..\..\code\sound
InputPath=..\..\code\sound\phrases.xml
InputName=phrases

BuildCmds= \
	"$(InputDir)\gc" "$(InputDir)\$(InputName)" \
	"$(InputDir)\gc" /h "$(InputDir)\grammar.h" "$(InputDir)\$(InputName)" \
	

"$(InputDir)\phrases.cfg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\grammar.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "code - Win32 Debug Inferno"

# Begin Custom Build
InputDir=..\..\code\sound
InputPath=..\..\code\sound\phrases.xml
InputName=phrases

BuildCmds= \
	"$(InputDir)\gc" "$(InputDir)\$(InputName)" \
	"$(InputDir)\gc" /h "$(InputDir)\grammar.h" "$(InputDir)\$(InputName)" \
	

"$(InputDir)\phrases.cfg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\grammar.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\rtvoice.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\rtvoice.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\Sound.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Sound\Sound.h
# End Source File
# Begin Source File

SOURCE=..\..\code\sound\speech.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\sound\speech.h
# End Source File
# Begin Source File

SOURCE=..\..\code\sound\voicerec.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\sound\voicerec.h
# End Source File
# End Group
# Begin Group "Species_Defs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\species_defs\species_defs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\species_defs\species_defs.h
# End Source File
# End Group
# Begin Group "Starfield"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Starfield\Nebula.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Starfield\Nebula.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Starfield\StarField.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Starfield\StarField.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Starfield\Supernova.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Starfield\Supernova.h
# End Source File
# End Group
# Begin Group "Stats"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Stats\Medals.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Stats\Medals.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Stats\Scoring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Stats\Scoring.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Stats\Stats.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Stats\Stats.h
# End Source File
# End Group
# Begin Group "TgaUtils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\TgaUtils\TgaUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\TgaUtils\TgaUtils.h
# End Source File
# End Group
# Begin Group "Ui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\UI\BUTTON.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\CHECKBOX.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\GADGET.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\icon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\INPUTBOX.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\KEYTRAP.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\LISTBOX.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\RADIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\SCROLL.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\slider.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Ui\SLIDER2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\UI.H
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\UiDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\UIDRAW.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\UIMOUSE.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\UI\WINDOW.cpp
# End Source File
# End Group
# Begin Group "Weapon"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\code\Weapon\Beam.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Beam.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Corkscrew.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Corkscrew.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Emp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Emp.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Flak.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Flak.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\MuzzleFlash.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\MuzzleFlash.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Shockwave.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Shockwave.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Swarm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Swarm.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Trails.cpp
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Trails.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Weapon.h
# End Source File
# Begin Source File

SOURCE=..\..\code\Weapon\Weapons.cpp
# End Source File
# End Group
# End Target
# End Project
