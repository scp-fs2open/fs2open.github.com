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
# PROP Output_Dir "..\Release"
# PROP Intermediate_Dir "..\Release\Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W4 /GX /Zi /Ot /Ow /Og /Oi /Oy /Ob2 /I "." /I "c:\mssdk\include\\" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "USE_OPENGL" /D "MORE_SPECIES" /U "_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
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
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir "..\Debug\Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Gi /GX /ZI /Od /I "." /I "c:\mssdk\include\\" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "DBUGFILE_ACTIVE" /D "USE_OPENGL" /D "MORE_SPECIES" /U "NDEBUG" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o".\debug/profile/code.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "code - Win32 Release"
# Name "code - Win32 Debug"
# Begin Group "Anim"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Anim\AnimPlay.cpp
# End Source File
# Begin Source File

SOURCE=.\Anim\AnimPlay.h
# End Source File
# Begin Source File

SOURCE=.\Anim\PackUnpack.cpp
# End Source File
# Begin Source File

SOURCE=.\Anim\PackUnpack.h
# End Source File
# End Group
# Begin Group "Asteroid"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Asteroid\Asteroid.cpp
# End Source File
# Begin Source File

SOURCE=.\Asteroid\Asteroid.h
# End Source File
# End Group
# Begin Group "Bmpman"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Bmpman\BmpMan.cpp
# End Source File
# Begin Source File

SOURCE=.\Bmpman\BmpMan.h
# End Source File
# End Group
# Begin Group "CFile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CFile\cfile.cpp
# End Source File
# Begin Source File

SOURCE=.\CFile\cfile.h
# End Source File
# Begin Source File

SOURCE=.\CFile\CfileArchive.cpp
# End Source File
# Begin Source File

SOURCE=.\CFile\CfileArchive.h
# End Source File
# Begin Source File

SOURCE=.\CFile\CfileList.cpp
# End Source File
# Begin Source File

SOURCE=.\CFile\CfileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\CFile\CfileSystem.h
# End Source File
# End Group
# Begin Group "CMeasure"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CMeasure\CMeasure.cpp
# End Source File
# Begin Source File

SOURCE=.\CMeasure\CMeasure.h
# End Source File
# End Group
# Begin Group "ControlConfig"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ControlConfig\ControlsConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\ControlConfig\ControlsConfig.h
# End Source File
# Begin Source File

SOURCE=.\ControlConfig\ControlsConfigCommon.cpp
# End Source File
# End Group
# Begin Group "Cutscene"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Cutscene\Cutscenes.cpp
# End Source File
# Begin Source File

SOURCE=.\Cutscene\Cutscenes.h
# End Source File
# Begin Source File

SOURCE=.\cutscene\movie.cpp
# End Source File
# Begin Source File

SOURCE=.\cutscene\movie.h
# End Source File
# End Group
# Begin Group "Debris"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Debris\Debris.cpp
# End Source File
# Begin Source File

SOURCE=.\Debris\Debris.h
# End Source File
# End Group
# Begin Group "DebugConsole"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DebugConsole\Console.cpp
# End Source File
# Begin Source File

SOURCE=.\debugconsole\dbugfile.cpp
# End Source File
# Begin Source File

SOURCE=.\debugconsole\dbugfile.h
# End Source File
# Begin Source File

SOURCE=.\debugconsole\timerbar.cpp
# End Source File
# Begin Source File

SOURCE=.\debugconsole\timerbar.h
# End Source File
# End Group
# Begin Group "DirectX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\directx\dx8show.cpp
# End Source File
# Begin Source File

SOURCE=.\directx\dx8show.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vasync.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vD3dcaps.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vd3drmdef.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vd3drmobj.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vd3drmwin.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vd3dvec.inl
# End Source File
# Begin Source File

SOURCE=.\DirectX\vDdraw.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vDinput.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vdplay.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vdplobby.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vdsetup.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vDsound.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vdvp.h
# End Source File
# Begin Source File

SOURCE=.\DirectX\vDinput.lib
# End Source File
# Begin Source File

SOURCE=.\DirectX\vDsound.lib
# End Source File
# Begin Source File

SOURCE=.\directx\ddraw.lib
# End Source File
# Begin Source File

SOURCE=.\directx\strmiids.lib
# End Source File
# Begin Source File

SOURCE=.\directx\dxguid.lib
# End Source File
# End Group
# Begin Group "Fireball"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Fireball\FireBalls.cpp
# End Source File
# Begin Source File

SOURCE=.\Fireball\FireBalls.h
# End Source File
# Begin Source File

SOURCE=.\Fireball\WarpInEffect.cpp
# End Source File
# End Group
# Begin Group "GameHelp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GameHelp\ContextHelp.cpp
# End Source File
# Begin Source File

SOURCE=.\GameHelp\ContextHelp.h
# End Source File
# Begin Source File

SOURCE=.\GameHelp\GameplayHelp.cpp
# End Source File
# Begin Source File

SOURCE=.\GameHelp\GameplayHelp.h
# End Source File
# End Group
# Begin Group "GameSequence"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GameSequence\GameSequence.cpp
# End Source File
# Begin Source File

SOURCE=.\GameSequence\GameSequence.h
# End Source File
# End Group
# Begin Group "GameSnd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Gamesnd\EventMusic.cpp
# End Source File
# Begin Source File

SOURCE=.\Gamesnd\EventMusic.h
# End Source File
# Begin Source File

SOURCE=.\Gamesnd\GameSnd.cpp
# End Source File
# Begin Source File

SOURCE=.\Gamesnd\GameSnd.h
# End Source File
# End Group
# Begin Group "Glide"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Glide\3DFX.H
# End Source File
# Begin Source File

SOURCE=.\Glide\FXDLL.H
# End Source File
# Begin Source File

SOURCE=.\Glide\FXGLOB.H
# End Source File
# Begin Source File

SOURCE=.\Glide\FXOS.H
# End Source File
# Begin Source File

SOURCE=.\Glide\Glide.cpp
# End Source File
# Begin Source File

SOURCE=.\Glide\glide.h
# End Source File
# Begin Source File

SOURCE=.\Glide\glidesys.h
# End Source File
# Begin Source File

SOURCE=.\Glide\glideutl.h
# End Source File
# Begin Source File

SOURCE=.\Glide\SST1VID.H
# End Source File
# End Group
# Begin Group "GlobalIncs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GlobalIncs\AlphaColors.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalIncs\AlphaColors.h
# End Source File
# Begin Source File

SOURCE=.\GlobalIncs\crypt.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalIncs\crypt.h
# End Source File
# Begin Source File

SOURCE=.\GlobalIncs\LinkList.h
# End Source File
# Begin Source File

SOURCE=.\GlobalIncs\PsTypes.h
# End Source File
# Begin Source File

SOURCE=.\GlobalIncs\SystemVars.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalIncs\SystemVars.h
# End Source File
# Begin Source File

SOURCE=.\GlobalIncs\version.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalIncs\version.h
# End Source File
# Begin Source File

SOURCE=.\GlobalIncs\WinDebug.cpp
# End Source File
# End Group
# Begin Group "Graphics"

# PROP Default_Filter ""
# Begin Group "SoftwareGr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Graphics\Bitblt.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\Bitblt.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\Circle.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\Circle.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\Colors.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\Colors.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\Font.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\Font.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\Gradient.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\Gradient.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrSoft.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrSoft.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrZbuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrZbuffer.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\Line.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\Line.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\Pixel.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\Pixel.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\Rect.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\Rect.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\Scaler.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\Scaler.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\Shade.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\Shade.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\Tmapper.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\TmapScanline.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\TmapScanline.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\TmapScanTiled128x128.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\TmapScanTiled16x16.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\TmapScanTiled256x256.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\TmapScanTiled32x32.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\TmapScanTiled64x64.cpp
# End Source File
# End Group
# Begin Group "GlideGr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Graphics\GrGlide.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrGlide.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrGlideInternal.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrGlideTexture.cpp
# End Source File
# End Group
# Begin Group "D3D8Gr"

# PROP Default_Filter ""
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Graphics\GrD3D.h
# End Source File
# Begin Source File

SOURCE=.\graphics\grd3dbatch.h
# End Source File
# Begin Source File

SOURCE=.\graphics\GrD3DBmpman.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrD3DInternal.h
# End Source File
# Begin Source File

SOURCE=.\graphics\grd3dlight.h
# End Source File
# Begin Source File

SOURCE=.\graphics\grd3dsetup.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Graphics\GrD3D.cpp
# End Source File
# Begin Source File

SOURCE=.\graphics\grd3dbatch.cpp
# End Source File
# Begin Source File

SOURCE=.\graphics\GrD3DBmpman.cpp
# End Source File
# Begin Source File

SOURCE=.\graphics\GrD3DCalls.cpp
# End Source File
# Begin Source File

SOURCE=.\graphics\grd3dlight.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrD3DRender.cpp
# End Source File
# Begin Source File

SOURCE=.\graphics\GrD3Dsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrD3DTexture.cpp
# End Source File
# End Group
# Begin Group "DDrawGr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Graphics\GrDirectDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrDirectDraw.h
# End Source File
# End Group
# Begin Group "OpenGLGr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Graphics\GrOpenGL.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrOpenGL.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Graphics\2d.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\2d.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\aaline.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\GrInternal.h
# End Source File
# Begin Source File

SOURCE=.\Graphics\TmapGenericScans.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphics\TMAPPER.H
# End Source File
# End Group
# Begin Group "Hud"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Hud\HUD.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUD.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HudArtillery.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HudArtillery.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDbrackets.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDbrackets.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDconfig.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDescort.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDescort.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDets.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDets.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDgauges.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDlock.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDlock.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDmessage.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDmessage.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDObserver.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDObserver.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDreticle.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDreticle.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDshield.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDshield.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDsquadmsg.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDsquadmsg.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDtarget.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDtarget.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDtargetbox.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDtargetbox.h
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDWingmanStatus.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud\HUDWingmanStatus.h
# End Source File
# End Group
# Begin Group "Io"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Io\Joy.cpp
# End Source File
# Begin Source File

SOURCE=.\Io\Joy.h
# End Source File
# Begin Source File

SOURCE=.\Io\Joy_ff.cpp
# End Source File
# Begin Source File

SOURCE=.\Io\Joy_ff.h
# End Source File
# Begin Source File

SOURCE=.\Io\Key.cpp
# End Source File
# Begin Source File

SOURCE=.\Io\Key.h
# End Source File
# Begin Source File

SOURCE=.\Io\KeyControl.cpp
# End Source File
# Begin Source File

SOURCE=.\Io\KeyControl.h
# End Source File
# Begin Source File

SOURCE=.\Io\Mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\Io\Mouse.h
# End Source File
# Begin Source File

SOURCE=.\Io\sw_error.hpp
# End Source File
# Begin Source File

SOURCE=.\Io\sw_force.h
# End Source File
# Begin Source File

SOURCE=.\Io\sw_guid.hpp
# End Source File
# Begin Source File

SOURCE=.\Io\swff_lib.cpp
# End Source File
# Begin Source File

SOURCE=.\Io\Timer.cpp
# End Source File
# Begin Source File

SOURCE=.\Io\Timer.h
# End Source File
# End Group
# Begin Group "JumpNode"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\JumpNode\JumpNode.cpp
# End Source File
# Begin Source File

SOURCE=.\JumpNode\JumpNode.h
# End Source File
# End Group
# Begin Group "Lighting"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Lighting\Lighting.cpp
# End Source File
# Begin Source File

SOURCE=.\Lighting\Lighting.h
# End Source File
# End Group
# Begin Group "Math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Math\Fix.cpp
# End Source File
# Begin Source File

SOURCE=.\Math\fix.h
# End Source File
# Begin Source File

SOURCE=.\Math\Floating.cpp
# End Source File
# Begin Source File

SOURCE=.\Math\Floating.h
# End Source File
# Begin Source File

SOURCE=.\Math\Fvi.cpp
# End Source File
# Begin Source File

SOURCE=.\Math\Fvi.h
# End Source File
# Begin Source File

SOURCE=.\Math\spline.cpp
# End Source File
# Begin Source File

SOURCE=.\Math\spline.h
# End Source File
# Begin Source File

SOURCE=.\Math\StaticRand.cpp
# End Source File
# Begin Source File

SOURCE=.\Math\StaticRand.h
# End Source File
# Begin Source File

SOURCE=.\Math\VecMat.cpp
# End Source File
# Begin Source File

SOURCE=.\Math\VecMat.h
# End Source File
# End Group
# Begin Group "MenuUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MenuUI\Barracks.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuUI\Barracks.h
# End Source File
# Begin Source File

SOURCE=.\MenuUI\Credits.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuUI\Credits.h
# End Source File
# Begin Source File

SOURCE=.\MenuUI\fishtank.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuUI\fishtank.h
# End Source File
# Begin Source File

SOURCE=.\MenuUI\MainHallMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuUI\MainHallMenu.h
# End Source File
# Begin Source File

SOURCE=.\MenuUI\MainHallTemp.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuUI\MainHallTemp.h
# End Source File
# Begin Source File

SOURCE=.\MenuUI\OptionsMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuUI\OptionsMenu.h
# End Source File
# Begin Source File

SOURCE=.\MenuUI\OptionsMenuMulti.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuUI\OptionsMenuMulti.h
# End Source File
# Begin Source File

SOURCE=.\MenuUI\PlayerMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuUI\PlayerMenu.h
# End Source File
# Begin Source File

SOURCE=.\MenuUI\ReadyRoom.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuUI\ReadyRoom.h
# End Source File
# Begin Source File

SOURCE=.\MenuUI\SnazzyUI.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuUI\SnazzyUI.h
# End Source File
# Begin Source File

SOURCE=.\MenuUI\TechMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuUI\TechMenu.h
# End Source File
# Begin Source File

SOURCE=.\MenuUI\TrainingMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuUI\TrainingMenu.h
# End Source File
# End Group
# Begin Group "Mission"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Mission\MissionBriefCommon.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Mission\MissionBriefCommon.h
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionCampaign.cpp
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionCampaign.h
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionGoals.cpp
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionGoals.h
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionGrid.h
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionHotKey.cpp
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionHotKey.h
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionLoad.h
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionLog.cpp
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionLog.h
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionMessage.h
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionParse.cpp
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionParse.h
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionTraining.cpp
# End Source File
# Begin Source File

SOURCE=.\Mission\MissionTraining.h
# End Source File
# End Group
# Begin Group "MissionUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MissionUI\Chatbox.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MissionUI\Chatbox.h
# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionBrief.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionBrief.h
# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionCmdBrief.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionCmdBrief.h
# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionDebrief.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionDebrief.h
# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionLoopBrief.cpp
# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionLoopBrief.h
# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionPause.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionPause.h
# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionRecommend.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionRecommend.h
# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionScreenCommon.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionScreenCommon.h
# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionShipChoice.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionShipChoice.h
# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionStats.cpp
# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionStats.h
# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionWeaponChoice.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MissionUI\MissionWeaponChoice.h
# End Source File
# Begin Source File

SOURCE=.\MissionUI\RedAlert.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MissionUI\RedAlert.h
# End Source File
# End Group
# Begin Group "Model"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Model\MODEL.H
# End Source File
# Begin Source File

SOURCE=.\Model\ModelCollide.cpp
# End Source File
# Begin Source File

SOURCE=.\Model\ModelInterp.cpp
# End Source File
# Begin Source File

SOURCE=.\Model\ModelOctant.cpp
# End Source File
# Begin Source File

SOURCE=.\Model\ModelRead.cpp
# End Source File
# Begin Source File

SOURCE=.\Model\ModelsInc.h
# End Source File
# End Group
# Begin Group "Object"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Object\CollideDebrisShip.cpp
# End Source File
# Begin Source File

SOURCE=.\Object\CollideDebrisWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\Object\CollideShipShip.cpp
# End Source File
# Begin Source File

SOURCE=.\Object\CollideShipWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\Object\CollideWeaponWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\Object\ObjCollide.cpp
# End Source File
# Begin Source File

SOURCE=.\Object\ObjCollide.h
# End Source File
# Begin Source File

SOURCE=.\Object\Object.cpp
# End Source File
# Begin Source File

SOURCE=.\Object\Object.h
# End Source File
# Begin Source File

SOURCE=.\Object\ObjectSnd.cpp
# End Source File
# Begin Source File

SOURCE=.\Object\ObjectSnd.h
# End Source File
# Begin Source File

SOURCE=.\Object\ObjectSort.cpp
# End Source File
# End Group
# Begin Group "Observer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Observer\Observer.cpp
# End Source File
# Begin Source File

SOURCE=.\Observer\Observer.h
# End Source File
# End Group
# Begin Group "OsApi"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\OsApi\MONOPUB.H
# End Source File
# Begin Source File

SOURCE=.\OsApi\OsApi.cpp
# End Source File
# Begin Source File

SOURCE=.\OsApi\OsApi.h
# End Source File
# Begin Source File

SOURCE=.\OsApi\OsRegistry.cpp
# End Source File
# Begin Source File

SOURCE=.\OsApi\OsRegistry.h
# End Source File
# Begin Source File

SOURCE=.\OsApi\OutWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\OsApi\OutWnd.h
# End Source File
# End Group
# Begin Group "Palman"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Palman\PalMan.cpp
# End Source File
# Begin Source File

SOURCE=.\Palman\PalMan.h
# End Source File
# End Group
# Begin Group "Parse"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Parse\Encrypt.cpp
# End Source File
# Begin Source File

SOURCE=.\Parse\Encrypt.h
# End Source File
# Begin Source File

SOURCE=.\Parse\PARSELO.CPP
# End Source File
# Begin Source File

SOURCE=.\Parse\parselo.h
# End Source File
# Begin Source File

SOURCE=.\Parse\SEXP.CPP
# End Source File
# Begin Source File

SOURCE=.\Parse\SEXP.H
# End Source File
# End Group
# Begin Group "Particle"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Particle\Particle.cpp
# End Source File
# Begin Source File

SOURCE=.\Particle\Particle.h
# End Source File
# End Group
# Begin Group "PcxUtils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PcxUtils\pcxutils.cpp
# End Source File
# Begin Source File

SOURCE=.\PcxUtils\pcxutils.h
# End Source File
# End Group
# Begin Group "Physics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Physics\Physics.cpp
# End Source File
# Begin Source File

SOURCE=.\Physics\Physics.h
# End Source File
# End Group
# Begin Group "Playerman"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Playerman\ManagePilot.cpp
# End Source File
# Begin Source File

SOURCE=.\Playerman\ManagePilot.h
# End Source File
# Begin Source File

SOURCE=.\Playerman\Player.h
# End Source File
# Begin Source File

SOURCE=.\Playerman\PlayerControl.cpp
# End Source File
# End Group
# Begin Group "Popup"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Popup\Popup.cpp
# End Source File
# Begin Source File

SOURCE=.\Popup\Popup.h
# End Source File
# Begin Source File

SOURCE=.\Popup\PopupDead.cpp
# End Source File
# Begin Source File

SOURCE=.\Popup\PopupDead.h
# End Source File
# End Group
# Begin Group "Radar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Radar\Radar.cpp
# End Source File
# Begin Source File

SOURCE=.\Radar\Radar.h
# End Source File
# End Group
# Begin Group "Render"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Render\3D.H
# End Source File
# Begin Source File

SOURCE=.\Render\3dClipper.cpp
# End Source File
# Begin Source File

SOURCE=.\Render\3ddraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Render\3dInternal.h
# End Source File
# Begin Source File

SOURCE=.\Render\3dLaser.cpp
# End Source File
# Begin Source File

SOURCE=.\Render\3dMath.cpp
# End Source File
# Begin Source File

SOURCE=.\Render\3dSetup.cpp
# End Source File
# End Group
# Begin Group "Ship"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Ship\Afterburner.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ship\Afterburner.h
# End Source File
# Begin Source File

SOURCE=.\Ship\ai.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ship\ai.h
# End Source File
# Begin Source File

SOURCE=.\Ship\AiBig.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ship\AiBig.h
# End Source File
# Begin Source File

SOURCE=.\Ship\AiCode.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ship\AiGoals.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ship\AiGoals.h
# End Source File
# Begin Source File

SOURCE=.\Ship\AiLocal.h
# End Source File
# Begin Source File

SOURCE=.\Ship\AWACS.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ship\AWACS.h
# End Source File
# Begin Source File

SOURCE=.\Ship\Shield.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ship\Ship.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ship\Ship.h
# End Source File
# Begin Source File

SOURCE=.\Ship\ShipContrails.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ship\ShipContrails.h
# End Source File
# Begin Source File

SOURCE=.\Ship\ShipFX.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ship\ShipFX.h
# End Source File
# Begin Source File

SOURCE=.\Ship\ShipHit.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ship\ShipHit.h
# End Source File
# Begin Source File

SOURCE=.\Ship\SubsysDamage.h
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Sound\acm.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\acm.h
# End Source File
# Begin Source File

SOURCE=.\Sound\AudioStr.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\AudioStr.h
# End Source File
# Begin Source File

SOURCE=.\Sound\channel.h
# End Source File
# Begin Source File

SOURCE=.\Sound\ds.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\ds.h
# End Source File
# Begin Source File

SOURCE=.\Sound\ds3d.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\ds3d.h
# End Source File
# Begin Source File

SOURCE=.\Sound\dscap.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\dscap.h
# End Source File
# Begin Source File

SOURCE=.\sound\fsspeech.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\fsspeech.h
# End Source File
# Begin Source File

SOURCE=.\Sound\midifile.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\midifile.h
# End Source File
# Begin Source File

SOURCE=.\Sound\midiseq.h
# End Source File
# Begin Source File

SOURCE=.\Sound\RBAudio.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\RBAudio.h
# End Source File
# Begin Source File

SOURCE=.\Sound\rtvoice.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\rtvoice.h
# End Source File
# Begin Source File

SOURCE=.\Sound\Sound.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\Sound.h
# End Source File
# Begin Source File

SOURCE=.\sound\speech.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\speech.h
# End Source File
# Begin Source File

SOURCE=.\Sound\WinMIDI.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\WinMIDI.h
# End Source File
# Begin Source File

SOURCE=.\Sound\winmidi_base.cpp
# End Source File
# End Group
# Begin Group "Starfield"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Starfield\Nebula.cpp
# End Source File
# Begin Source File

SOURCE=.\Starfield\Nebula.h
# End Source File
# Begin Source File

SOURCE=.\Starfield\StarField.cpp
# End Source File
# Begin Source File

SOURCE=.\Starfield\StarField.h
# End Source File
# Begin Source File

SOURCE=.\Starfield\Supernova.cpp
# End Source File
# Begin Source File

SOURCE=.\Starfield\Supernova.h
# End Source File
# End Group
# Begin Group "Stats"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Stats\Medals.cpp
# End Source File
# Begin Source File

SOURCE=.\Stats\Medals.h
# End Source File
# Begin Source File

SOURCE=.\Stats\Scoring.cpp
# End Source File
# Begin Source File

SOURCE=.\Stats\Scoring.h
# End Source File
# Begin Source File

SOURCE=.\Stats\Stats.cpp
# End Source File
# Begin Source File

SOURCE=.\Stats\Stats.h
# End Source File
# End Group
# Begin Group "Ui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\UI\BUTTON.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\CHECKBOX.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\GADGET.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\icon.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\INPUTBOX.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\KEYTRAP.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\LISTBOX.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\RADIO.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\SCROLL.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\slider.cpp
# End Source File
# Begin Source File

SOURCE=.\Ui\SLIDER2.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\UI.H
# End Source File
# Begin Source File

SOURCE=.\UI\UiDefs.h
# End Source File
# Begin Source File

SOURCE=.\UI\UIDRAW.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\UIMOUSE.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\WINDOW.cpp
# End Source File
# End Group
# Begin Group "VCodec"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\VCodec\CODEC1.CPP
# End Source File
# Begin Source File

SOURCE=.\VCodec\CODEC1.H
# End Source File
# End Group
# Begin Group "Weapon"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Weapon\Beam.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapon\Beam.h
# End Source File
# Begin Source File

SOURCE=.\Weapon\Corkscrew.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapon\Corkscrew.h
# End Source File
# Begin Source File

SOURCE=.\Weapon\Emp.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapon\Emp.h
# End Source File
# Begin Source File

SOURCE=.\Weapon\Flak.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapon\Flak.h
# End Source File
# Begin Source File

SOURCE=.\Weapon\MuzzleFlash.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapon\MuzzleFlash.h
# End Source File
# Begin Source File

SOURCE=.\Weapon\Shockwave.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapon\Shockwave.h
# End Source File
# Begin Source File

SOURCE=.\Weapon\Swarm.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapon\Swarm.h
# End Source File
# Begin Source File

SOURCE=.\Weapon\Trails.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapon\Trails.h
# End Source File
# Begin Source File

SOURCE=.\Weapon\Weapon.h
# End Source File
# Begin Source File

SOURCE=.\Weapon\Weapons.cpp
# End Source File
# End Group
# Begin Group "Nebula"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Nebula\Neb.cpp
# End Source File
# Begin Source File

SOURCE=.\Nebula\Neb.h
# End Source File
# Begin Source File

SOURCE=.\Nebula\NebLightning.cpp

!IF  "$(CFG)" == "code - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "code - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Nebula\NebLightning.h
# End Source File
# End Group
# Begin Group "Localization"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Localization\fhash.cpp
# End Source File
# Begin Source File

SOURCE=.\Localization\fhash.h
# End Source File
# Begin Source File

SOURCE=.\Localization\localize.cpp
# End Source File
# Begin Source File

SOURCE=.\Localization\localize.h
# End Source File
# End Group
# Begin Group "TgaUtils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\TgaUtils\TgaUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\TgaUtils\TgaUtils.h
# End Source File
# End Group
# Begin Group "Demo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Demo\Demo.cpp
# End Source File
# Begin Source File

SOURCE=.\Demo\Demo.h
# End Source File
# End Group
# Begin Group "InetFile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Inetfile\CFtp.cpp
# End Source File
# Begin Source File

SOURCE=.\Inetfile\CFtp.h
# End Source File
# Begin Source File

SOURCE=.\Inetfile\Chttpget.cpp
# End Source File
# Begin Source File

SOURCE=.\Inetfile\Chttpget.h
# End Source File
# Begin Source File

SOURCE=.\Inetfile\inetgetfile.cpp
# End Source File
# Begin Source File

SOURCE=.\Inetfile\inetgetfile.h
# End Source File
# End Group
# Begin Group "ExceptionHandler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ExceptionHandler\ExceptionHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\ExceptionHandler\ExceptionHandler.h
# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Network\Multi.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\Multi.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_campaign.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_campaign.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_data.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_data.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_dogfight.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_dogfight.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_endgame.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_endgame.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_ingame.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_ingame.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_kick.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_kick.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_log.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_log.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_obj.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_obj.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_observer.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_observer.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_oo.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_oo.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_options.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_options.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_pause.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_pause.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_pinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_pinfo.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_ping.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_ping.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_pmsg.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_pmsg.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_rate.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_rate.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_respawn.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_respawn.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_team.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_team.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_update.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_update.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_voice.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_voice.h
# End Source File
# Begin Source File

SOURCE=.\Network\multi_xfer.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multi_xfer.h
# End Source File
# Begin Source File

SOURCE=.\Network\multilag.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multilag.h
# End Source File
# Begin Source File

SOURCE=.\Network\MultiMsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\multimsgs.h
# End Source File
# Begin Source File

SOURCE=.\Network\MultiTeamSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\MultiTeamSelect.h
# End Source File
# Begin Source File

SOURCE=.\Network\MultiUI.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\MultiUI.h
# End Source File
# Begin Source File

SOURCE=.\Network\MultiUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\MultiUtil.h
# End Source File
# Begin Source File

SOURCE=.\Network\PsNet.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\PsNet.h
# End Source File
# Begin Source File

SOURCE=.\Network\Psnet2.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\Psnet2.h
# End Source File
# Begin Source File

SOURCE=.\Network\stand_gui.cpp
# End Source File
# Begin Source File

SOURCE=.\Network\stand_gui.h
# End Source File
# End Group
# Begin Group "Decals"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\decals\decals.cpp
# End Source File
# Begin Source File

SOURCE=.\decals\decals.h
# End Source File
# End Group
# Begin Group "ddsutils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ddsutils\ddsutils.cpp
# End Source File
# Begin Source File

SOURCE=.\ddsutils\ddsutils.h
# End Source File
# End Group
# Begin Group "Cmdline"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Cmdline\cmdline.cpp
# End Source File
# Begin Source File

SOURCE=.\Cmdline\cmdline.h
# End Source File
# End Group
# Begin Group "fs2open_pxo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fs2open_pxo\Client.cpp
# End Source File
# Begin Source File

SOURCE=.\fs2open_pxo\Client.h
# End Source File
# Begin Source File

SOURCE=.\fs2open_pxo\protocol.h
# End Source File
# Begin Source File

SOURCE=.\fs2open_pxo\TCP_Client.cpp
# End Source File
# Begin Source File

SOURCE=.\fs2open_pxo\TCP_Socket.cpp
# End Source File
# Begin Source File

SOURCE=.\fs2open_pxo\TCP_Socket.h
# End Source File
# Begin Source File

SOURCE=.\fs2open_pxo\udpsocket.cpp
# End Source File
# Begin Source File

SOURCE=.\fs2open_pxo\udpsocket.h
# End Source File
# End Group
# Begin Group "Species_Defs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\species_defs\species_defs.cpp
# End Source File
# Begin Source File

SOURCE=.\species_defs\species_defs.h
# End Source File
# End Group
# End Target
# End Project
