# Microsoft Developer Studio Generated NMAKE File, Based on fs2_open.dsp
# heavily modified by penguin
# Test comment by Inquisitor

!IF "$(CFG)" != "Release" && "$(CFG)" != "Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fs2_open.w32.mak" CFG=Debug options...
!MESSAGE 
!MESSAGE Possible choices for CFG are:
!MESSAGE   Release
!MESSAGE   Debug
!MESSAGE 
!MESSAGE Other options:
!MESSAGE   USE_SOUND=0      disable sound support (default is 1)
!MESSAGE   USE_JOYSTICK=0   disable joystick support (default is 1)
!MESSAGE   USE_SOUND=0      disable network support (default is 1)
!MESSAGE   USE_OPENGL=1     enable OpenGL support (default is 0)
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF "$(USE_SOUND)" == "0"
SOUND_DEF=/D "NO_SOUND"
SOUND_OBJS=
!ELSE
SOUND_DEF=
SOUND_OBJS= \
	"$(INTDIR)\eventmusic.obj" \
	"$(INTDIR)\gamesnd.obj" \
	"$(INTDIR)\objectsnd.obj" \
	"$(INTDIR)\acm.obj" \
	"$(INTDIR)\audiostr.obj" \
	"$(INTDIR)\ds.obj" \
	"$(INTDIR)\ds3d.obj" \
	"$(INTDIR)\dscap.obj" \
	"$(INTDIR)\midifile.obj" \
	"$(INTDIR)\rbaudio.obj" \
	"$(INTDIR)\rtvoice.obj" \
	"$(INTDIR)\sound.obj" \
	"$(INTDIR)\winmidi.obj" \
	"$(INTDIR)\winmidi_base.obj" \
	"$(INTDIR)\codec1.obj"
!ENDIF # USE_SOUND

!IF "$(USE_JOYSTICK)" == "0"
JOYSTICK_DEF=/D "NO_JOYSTICK"
JOYSTICK_OBJS=
!ELSE
JOYSTICK_DEF=
JOYSTICK_OBJS= \
	"$(INTDIR)\joy.obj" \
	"$(INTDIR)\joy_ff.obj" \
	"$(INTDIR)\swff_lib.obj"
!ENDIF # USE_JOYSTICK

!IF "$(USE_NETWORK)" == "0"
NETWORK_DEF=/D "NO_NETWORK"
NETWORK_OBJS=
!ELSE
NETWORK_DEF=
NETWORK_OBJS= \
	"$(INTDIR)\hudobserver.obj" \
	"$(INTDIR)\multi.obj" \
	"$(INTDIR)\multimsgs.obj" \
	"$(INTDIR)\multiteamselect.obj" \
	"$(INTDIR)\multiui.obj" \
	"$(INTDIR)\multi_campaign.obj" \
	"$(INTDIR)\multi_data.obj" \
	"$(INTDIR)\multi_dogfight.obj" \
	"$(INTDIR)\multi_endgame.obj" \
	"$(INTDIR)\multi_ingame.obj" \
	"$(INTDIR)\multi_kick.obj" \
	"$(INTDIR)\multi_obj.obj" \
	"$(INTDIR)\multi_observer.obj" \
	"$(INTDIR)\multi_options.obj" \
	"$(INTDIR)\multi_pause.obj" \
	"$(INTDIR)\multi_pinfo.obj" \
	"$(INTDIR)\multi_ping.obj" \
	"$(INTDIR)\multi_pmsg.obj" \
	"$(INTDIR)\multi_rate.obj" \
	"$(INTDIR)\multi_respawn.obj" \
	"$(INTDIR)\multi_team.obj" \
	"$(INTDIR)\multi_voice.obj" \
	"$(INTDIR)\multi_xfer.obj" \
	"$(INTDIR)\psnet2.obj" \
	"$(INTDIR)\psnet.obj" \
	"$(INTDIR)\stand_gui.obj" \
	"$(INTDIR)\optionsmenumulti.obj" \
	"$(INTDIR)\chatbox.obj" \
	"$(INTDIR)\fs2ox.obj"
!ENDIF # USE_NETWORK


!IF "$(USE_OPENGL)" == "1"
!ELSE
!ENDIF # USE_OPENGL


!IF  "$(CFG)" == "Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\fs2_open.exe"


CLEAN :
	-@erase "$(INTDIR)\2d.obj"
	-@erase "$(INTDIR)\3dclipper.obj"
	-@erase "$(INTDIR)\3ddraw.obj"
	-@erase "$(INTDIR)\3dlaser.obj"
	-@erase "$(INTDIR)\3dmath.obj"
	-@erase "$(INTDIR)\3dsetup.obj"
	-@erase "$(INTDIR)\aaline.obj"
	-@erase "$(INTDIR)\afterburner.obj"
	-@erase "$(INTDIR)\ai.obj"
	-@erase "$(INTDIR)\aibig.obj"
	-@erase "$(INTDIR)\aicode.obj"
	-@erase "$(INTDIR)\aigoals.obj"
	-@erase "$(INTDIR)\alphacolors.obj"
	-@erase "$(INTDIR)\animplay.obj"
	-@erase "$(INTDIR)\asteroid.obj"
	-@erase "$(INTDIR)\awacs.obj"
	-@erase "$(INTDIR)\barracks.obj"
	-@erase "$(INTDIR)\beam.obj"
	-@erase "$(INTDIR)\bitblt.obj"
	-@erase "$(INTDIR)\bmpman.obj"
	-@erase "$(INTDIR)\button.obj"
	-@erase "$(INTDIR)\cfile.obj"
	-@erase "$(INTDIR)\cfilearchive.obj"
	-@erase "$(INTDIR)\cfilelist.obj"
	-@erase "$(INTDIR)\cfilesystem.obj"
	-@erase "$(INTDIR)\cftp.obj"
	-@erase "$(INTDIR)\checkbox.obj"
	-@erase "$(INTDIR)\chttpget.obj"
	-@erase "$(INTDIR)\circle.obj"
	-@erase "$(INTDIR)\cmdline.obj"
	-@erase "$(INTDIR)\cmeasure.obj"
	-@erase "$(INTDIR)\collidedebrisship.obj"
	-@erase "$(INTDIR)\collidedebrisweapon.obj"
	-@erase "$(INTDIR)\collideshipship.obj"
	-@erase "$(INTDIR)\collideshipweapon.obj"
	-@erase "$(INTDIR)\collideweaponweapon.obj"
	-@erase "$(INTDIR)\colors.obj"
	-@erase "$(INTDIR)\console.obj"
	-@erase "$(INTDIR)\contexthelp.obj"
	-@erase "$(INTDIR)\controlsconfig.obj"
	-@erase "$(INTDIR)\controlsconfigcommon.obj"
	-@erase "$(INTDIR)\corkscrew.obj"
	-@erase "$(INTDIR)\credits.obj"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\cutscenes.obj"
	-@erase "$(INTDIR)\debris.obj"
	-@erase "$(INTDIR)\demo.obj"
	-@erase "$(INTDIR)\emp.obj"
	-@erase "$(INTDIR)\encrypt.obj"
	-@erase "$(INTDIR)\exceptionhandler.obj"
	-@erase "$(INTDIR)\fhash.obj"
	-@erase "$(INTDIR)\fireballs.obj"
	-@erase "$(INTDIR)\fishtank.obj"
	-@erase "$(INTDIR)\fix.obj"
	-@erase "$(INTDIR)\flak.obj"
	-@erase "$(INTDIR)\floating.obj"
	-@erase "$(INTDIR)\font.obj"
	-@erase "$(INTDIR)\freespace.obj"
	-@erase "$(INTDIR)\fvi.obj"
	-@erase "$(INTDIR)\gadget.obj"
	-@erase "$(INTDIR)\gameplayhelp.obj"
	-@erase "$(INTDIR)\gamesequence.obj"
	-@erase "$(INTDIR)\glide.obj"
	-@erase "$(INTDIR)\gradient.obj"
	-@erase "$(INTDIR)\grd3d.obj"
	-@erase "$(INTDIR)\grd3drender.obj"
	-@erase "$(INTDIR)\grd3dtexture.obj"
	-@erase "$(INTDIR)\grdirectdraw.obj"
	-@erase "$(INTDIR)\grglide.obj"
	-@erase "$(INTDIR)\grglidetexture.obj"
	-@erase "$(INTDIR)\grsoft.obj"
	-@erase "$(INTDIR)\grzbuffer.obj"
	-@erase "$(INTDIR)\hud.obj"
	-@erase "$(INTDIR)\hudartillery.obj"
	-@erase "$(INTDIR)\hudbrackets.obj"
	-@erase "$(INTDIR)\hudconfig.obj"
	-@erase "$(INTDIR)\hudescort.obj"
	-@erase "$(INTDIR)\hudets.obj"
	-@erase "$(INTDIR)\hudlock.obj"
	-@erase "$(INTDIR)\hudmessage.obj"
	-@erase "$(INTDIR)\hudreticle.obj"
	-@erase "$(INTDIR)\hudshield.obj"
	-@erase "$(INTDIR)\hudsquadmsg.obj"
	-@erase "$(INTDIR)\hudtarget.obj"
	-@erase "$(INTDIR)\hudtargetbox.obj"
	-@erase "$(INTDIR)\hudwingmanstatus.obj"
	-@erase "$(INTDIR)\icon.obj"
	-@erase "$(INTDIR)\inputbox.obj"
	-@erase "$(INTDIR)\jumpnode.obj"
	-@erase "$(INTDIR)\key.obj"
	-@erase "$(INTDIR)\keycontrol.obj"
	-@erase "$(INTDIR)\keytrap.obj"
	-@erase "$(INTDIR)\levelpaging.obj"
	-@erase "$(INTDIR)\lighting.obj"
	-@erase "$(INTDIR)\line.obj"
	-@erase "$(INTDIR)\listbox.obj"
	-@erase "$(INTDIR)\localize.obj"
	-@erase "$(INTDIR)\mainhallmenu.obj"
	-@erase "$(INTDIR)\managepilot.obj"
	-@erase "$(INTDIR)\medals.obj"
	-@erase "$(INTDIR)\missionbrief.obj"
	-@erase "$(INTDIR)\missionbriefcommon.obj"
	-@erase "$(INTDIR)\missioncampaign.obj"
	-@erase "$(INTDIR)\missioncmdbrief.obj"
	-@erase "$(INTDIR)\missiondebrief.obj"
	-@erase "$(INTDIR)\missiongoals.obj"
	-@erase "$(INTDIR)\missiongrid.obj"
	-@erase "$(INTDIR)\missionhotkey.obj"
	-@erase "$(INTDIR)\missionload.obj"
	-@erase "$(INTDIR)\missionlog.obj"
	-@erase "$(INTDIR)\missionloopbrief.obj"
	-@erase "$(INTDIR)\missionmessage.obj"
	-@erase "$(INTDIR)\missionparse.obj"
	-@erase "$(INTDIR)\missionpause.obj"
	-@erase "$(INTDIR)\missionrecommend.obj"
	-@erase "$(INTDIR)\missionscreencommon.obj"
	-@erase "$(INTDIR)\missionshipchoice.obj"
	-@erase "$(INTDIR)\missionstats.obj"
	-@erase "$(INTDIR)\missiontraining.obj"
	-@erase "$(INTDIR)\missionweaponchoice.obj"
	-@erase "$(INTDIR)\modelcollide.obj"
	-@erase "$(INTDIR)\modelinterp.obj"
	-@erase "$(INTDIR)\modeloctant.obj"
	-@erase "$(INTDIR)\modelread.obj"
	-@erase "$(INTDIR)\mouse.obj"
	-@erase "$(INTDIR)\multi_log.obj"
	-@erase "$(INTDIR)\multiutil.obj"
	-@erase "$(INTDIR)\muzzleflash.obj"
	-@erase "$(INTDIR)\neb.obj"
	-@erase "$(INTDIR)\neblightning.obj"
	-@erase "$(INTDIR)\nebula.obj"
	-@erase "$(INTDIR)\objcollide.obj"
	-@erase "$(INTDIR)\object.obj"
	-@erase "$(INTDIR)\objectsort.obj"
	-@erase "$(INTDIR)\observer.obj"
	-@erase "$(INTDIR)\optionsmenu.obj"
	-@erase "$(INTDIR)\osapi.obj"
	-@erase "$(INTDIR)\osregistry.obj"
	-@erase "$(INTDIR)\outwnd.obj"
	-@erase "$(INTDIR)\packunpack.obj"
	-@erase "$(INTDIR)\palman.obj"
	-@erase "$(INTDIR)\parselo.obj"
	-@erase "$(INTDIR)\particle.obj"
	-@erase "$(INTDIR)\pcxutils.obj"
	-@erase "$(INTDIR)\physics.obj"
	-@erase "$(INTDIR)\pixel.obj"
	-@erase "$(INTDIR)\playercontrol.obj"
	-@erase "$(INTDIR)\playermenu.obj"
	-@erase "$(INTDIR)\popup.obj"
	-@erase "$(INTDIR)\popupdead.obj"
	-@erase "$(INTDIR)\radar.obj"
	-@erase "$(INTDIR)\radio.obj"
	-@erase "$(INTDIR)\readyroom.obj"
	-@erase "$(INTDIR)\rect.obj"
	-@erase "$(INTDIR)\redalert.obj"
	-@erase "$(INTDIR)\scaler.obj"
	-@erase "$(INTDIR)\scoring.obj"
	-@erase "$(INTDIR)\scroll.obj"
	-@erase "$(INTDIR)\sexp.obj"
	-@erase "$(INTDIR)\shade.obj"
	-@erase "$(INTDIR)\shield.obj"
	-@erase "$(INTDIR)\ship.obj"
	-@erase "$(INTDIR)\shipcontrails.obj"
	-@erase "$(INTDIR)\shipfx.obj"
	-@erase "$(INTDIR)\shiphit.obj"
	-@erase "$(INTDIR)\shockwave.obj"
	-@erase "$(INTDIR)\slider.obj"
	-@erase "$(INTDIR)\slider2.obj"
	-@erase "$(INTDIR)\snazzyui.obj"
	-@erase "$(INTDIR)\spline.obj"
	-@erase "$(INTDIR)\starfield.obj"
	-@erase "$(INTDIR)\staticrand.obj"
	-@erase "$(INTDIR)\stats.obj"
	-@erase "$(INTDIR)\supernova.obj"
	-@erase "$(INTDIR)\swarm.obj"
	-@erase "$(INTDIR)\systemvars.obj"
	-@erase "$(INTDIR)\techmenu.obj"
	-@erase "$(INTDIR)\tgautils.obj"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\tmapper.obj"
	-@erase "$(INTDIR)\tmapscanline.obj"
	-@erase "$(INTDIR)\tmapscantiled128x128.obj"
	-@erase "$(INTDIR)\tmapscantiled16x16.obj"
	-@erase "$(INTDIR)\tmapscantiled256x256.obj"
	-@erase "$(INTDIR)\tmapscantiled32x32.obj"
	-@erase "$(INTDIR)\tmapscantiled64x64.obj"
	-@erase "$(INTDIR)\trails.obj"
	-@erase "$(INTDIR)\trainingmenu.obj"
	-@erase "$(INTDIR)\uidraw.obj"
	-@erase "$(INTDIR)\uimouse.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vecmat.obj"
	-@erase "$(INTDIR)\version.obj"
	-@erase "$(INTDIR)\warpineffect.obj"
	-@erase "$(INTDIR)\weapons.obj"
	-@erase "$(INTDIR)\windebug.obj"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(INTDIR)\eventmusic.obj"
	-@erase "$(INTDIR)\gamesnd.obj"
	-@erase "$(INTDIR)\objectsnd.obj"
	-@erase "$(INTDIR)\acm.obj"
	-@erase "$(INTDIR)\audiostr.obj"
	-@erase "$(INTDIR)\ds.obj"
	-@erase "$(INTDIR)\ds3d.obj"
	-@erase "$(INTDIR)\dscap.obj"
	-@erase "$(INTDIR)\midifile.obj"
	-@erase "$(INTDIR)\rbaudio.obj"
	-@erase "$(INTDIR)\rtvoice.obj"
	-@erase "$(INTDIR)\sound.obj"
	-@erase "$(INTDIR)\winmidi.obj"
	-@erase "$(INTDIR)\winmidi_base.obj"
	-@erase "$(INTDIR)\codec1.obj"
	-@erase "$(INTDIR)\joy.obj"
	-@erase "$(INTDIR)\joy_ff.obj"
	-@erase "$(INTDIR)\swff_lib.obj"
	-@erase "$(INTDIR)\multi.obj"
	-@erase "$(INTDIR)\multimsgs.obj"
	-@erase "$(INTDIR)\multiteamselect.obj"
	-@erase "$(INTDIR)\multiui"
	-@erase "$(INTDIR)\multi_campaign.obj"
	-@erase "$(INTDIR)\multi_data.obj"
	-@erase "$(INTDIR)\multi_dogfight.obj"
	-@erase "$(INTDIR)\multi_endgame"
	-@erase "$(INTDIR)\multi_ingame"
	-@erase "$(INTDIR)\multi_kick"
	-@erase "$(INTDIR)\multi_obj.obj"
	-@erase "$(INTDIR)\multi_observer.obj"
	-@erase "$(INTDIR)\multi_options.obj"
	-@erase "$(INTDIR)\multi_pause.obj"
	-@erase "$(INTDIR)\multi_pinfo.obj"
	-@erase "$(INTDIR)\multi_ping.obj"
	-@erase "$(INTDIR)\multi_pmsg.obj"
	-@erase "$(INTDIR)\multi_rate.obj"
	-@erase "$(INTDIR)\multi_respawn.obj"
	-@erase "$(INTDIR)\multi_team.obj"
	-@erase "$(INTDIR)\multi_xfer.obj"
	-@erase "$(INTDIR)\psnet2.obj"
	-@erase "$(INTDIR)\psnet.obj"
	-@erase "$(INTDIR)\stand_gui.obj"
	-@erase "$(INTDIR)\optionsmenumulti.obj"
	-@erase "$(INTDIR)\chatbox.obj"
	-@erase "$(INTDIR)\fs2ox.obj"
	-@erase "$(OUTDIR)\fs2_open.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "code" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" $(NETWORK_DEF) $(JOYSTICK_DEF) $(SOUND_DEF) /Fp"$(INTDIR)\fs2_open.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\fs2_open.bsc" 
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib advapi32.lib wsock32.lib dxguid.lib ddraw.lib dinput.lib winmm.lib msacm32.lib ole32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\fs2_open.pdb" /machine:I386 /out:"$(OUTDIR)\fs2_open.exe" 
LINK32_OBJS= \
	"$(INTDIR)\2d.obj" \
	"$(INTDIR)\3dclipper.obj" \
	"$(INTDIR)\3ddraw.obj" \
	"$(INTDIR)\3dlaser.obj" \
	"$(INTDIR)\3dmath.obj" \
	"$(INTDIR)\3dsetup.obj" \
	"$(INTDIR)\aaline.obj" \
	"$(INTDIR)\afterburner.obj" \
	"$(INTDIR)\ai.obj" \
	"$(INTDIR)\aibig.obj" \
	"$(INTDIR)\aicode.obj" \
	"$(INTDIR)\aigoals.obj" \
	"$(INTDIR)\alphacolors.obj" \
	"$(INTDIR)\animplay.obj" \
	"$(INTDIR)\asteroid.obj" \
	"$(INTDIR)\awacs.obj" \
	"$(INTDIR)\barracks.obj" \
	"$(INTDIR)\beam.obj" \
	"$(INTDIR)\bitblt.obj" \
	"$(INTDIR)\bmpman.obj" \
	"$(INTDIR)\button.obj" \
	"$(INTDIR)\cfile.obj" \
	"$(INTDIR)\cfilearchive.obj" \
	"$(INTDIR)\cfilelist.obj" \
	"$(INTDIR)\cfilesystem.obj" \
	"$(INTDIR)\cftp.obj" \
	"$(INTDIR)\checkbox.obj" \
	"$(INTDIR)\chttpget.obj" \
	"$(INTDIR)\circle.obj" \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\cmeasure.obj" \
	"$(INTDIR)\collidedebrisship.obj" \
	"$(INTDIR)\collidedebrisweapon.obj" \
	"$(INTDIR)\collideshipship.obj" \
	"$(INTDIR)\collideshipweapon.obj" \
	"$(INTDIR)\collideweaponweapon.obj" \
	"$(INTDIR)\colors.obj" \
	"$(INTDIR)\console.obj" \
	"$(INTDIR)\contexthelp.obj" \
	"$(INTDIR)\controlsconfig.obj" \
	"$(INTDIR)\controlsconfigcommon.obj" \
	"$(INTDIR)\corkscrew.obj" \
	"$(INTDIR)\credits.obj" \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\cutscenes.obj" \
	"$(INTDIR)\debris.obj" \
	"$(INTDIR)\demo.obj" \
	"$(INTDIR)\emp.obj" \
	"$(INTDIR)\encrypt.obj" \
	"$(INTDIR)\exceptionhandler.obj" \
	"$(INTDIR)\fhash.obj" \
	"$(INTDIR)\fireballs.obj" \
	"$(INTDIR)\fishtank.obj" \
	"$(INTDIR)\fix.obj" \
	"$(INTDIR)\flak.obj" \
	"$(INTDIR)\floating.obj" \
	"$(INTDIR)\font.obj" \
	"$(INTDIR)\fvi.obj" \
	"$(INTDIR)\gadget.obj" \
	"$(INTDIR)\gameplayhelp.obj" \
	"$(INTDIR)\gamesequence.obj" \
	"$(INTDIR)\gradient.obj" \
	"$(INTDIR)\grzbuffer.obj" \
	"$(INTDIR)\hud.obj" \
	"$(INTDIR)\hudartillery.obj" \
	"$(INTDIR)\hudbrackets.obj" \
	"$(INTDIR)\hudconfig.obj" \
	"$(INTDIR)\hudescort.obj" \
	"$(INTDIR)\hudets.obj" \
	"$(INTDIR)\hudlock.obj" \
	"$(INTDIR)\hudmessage.obj" \
	"$(INTDIR)\hudreticle.obj" \
	"$(INTDIR)\hudshield.obj" \
	"$(INTDIR)\hudsquadmsg.obj" \
	"$(INTDIR)\hudtarget.obj" \
	"$(INTDIR)\hudtargetbox.obj" \
	"$(INTDIR)\hudwingmanstatus.obj" \
	"$(INTDIR)\icon.obj" \
	"$(INTDIR)\inputbox.obj" \
	"$(INTDIR)\jumpnode.obj" \
	"$(INTDIR)\key.obj" \
	"$(INTDIR)\keycontrol.obj" \
	"$(INTDIR)\keytrap.obj" \
	"$(INTDIR)\lighting.obj" \
	"$(INTDIR)\line.obj" \
	"$(INTDIR)\listbox.obj" \
	"$(INTDIR)\localize.obj" \
	"$(INTDIR)\mainhallmenu.obj" \
	"$(INTDIR)\managepilot.obj" \
	"$(INTDIR)\medals.obj" \
	"$(INTDIR)\missionbrief.obj" \
	"$(INTDIR)\missionbriefcommon.obj" \
	"$(INTDIR)\missioncampaign.obj" \
	"$(INTDIR)\missioncmdbrief.obj" \
	"$(INTDIR)\missiondebrief.obj" \
	"$(INTDIR)\missiongoals.obj" \
	"$(INTDIR)\missiongrid.obj" \
	"$(INTDIR)\missionhotkey.obj" \
	"$(INTDIR)\missionload.obj" \
	"$(INTDIR)\missionlog.obj" \
	"$(INTDIR)\missionloopbrief.obj" \
	"$(INTDIR)\missionmessage.obj" \
	"$(INTDIR)\missionparse.obj" \
	"$(INTDIR)\missionpause.obj" \
	"$(INTDIR)\missionrecommend.obj" \
	"$(INTDIR)\missionscreencommon.obj" \
	"$(INTDIR)\missionshipchoice.obj" \
	"$(INTDIR)\missionstats.obj" \
	"$(INTDIR)\missiontraining.obj" \
	"$(INTDIR)\missionweaponchoice.obj" \
	"$(INTDIR)\modelcollide.obj" \
	"$(INTDIR)\modelinterp.obj" \
	"$(INTDIR)\modeloctant.obj" \
	"$(INTDIR)\modelread.obj" \
	"$(INTDIR)\mouse.obj" \
	"$(INTDIR)\multiutil.obj" \
	"$(INTDIR)\muzzleflash.obj" \
	"$(INTDIR)\neb.obj" \
	"$(INTDIR)\neblightning.obj" \
	"$(INTDIR)\nebula.obj" \
	"$(INTDIR)\objcollide.obj" \
	"$(INTDIR)\object.obj" \
	"$(INTDIR)\objectsort.obj" \
	"$(INTDIR)\observer.obj" \
	"$(INTDIR)\optionsmenu.obj" \
	"$(INTDIR)\osapi.obj" \
	"$(INTDIR)\osregistry.obj" \
	"$(INTDIR)\outwnd.obj" \
	"$(INTDIR)\packunpack.obj" \
	"$(INTDIR)\palman.obj" \
	"$(INTDIR)\parselo.obj" \
	"$(INTDIR)\particle.obj" \
	"$(INTDIR)\pcxutils.obj" \
	"$(INTDIR)\physics.obj" \
	"$(INTDIR)\pixel.obj" \
	"$(INTDIR)\playercontrol.obj" \
	"$(INTDIR)\playermenu.obj" \
	"$(INTDIR)\popup.obj" \
	"$(INTDIR)\popupdead.obj" \
	"$(INTDIR)\radar.obj" \
	"$(INTDIR)\radio.obj" \
	"$(INTDIR)\readyroom.obj" \
	"$(INTDIR)\rect.obj" \
	"$(INTDIR)\redalert.obj" \
	"$(INTDIR)\scoring.obj" \
	"$(INTDIR)\scroll.obj" \
	"$(INTDIR)\sexp.obj" \
	"$(INTDIR)\shade.obj" \
	"$(INTDIR)\shield.obj" \
	"$(INTDIR)\ship.obj" \
	"$(INTDIR)\shipcontrails.obj" \
	"$(INTDIR)\shipfx.obj" \
	"$(INTDIR)\shiphit.obj" \
	"$(INTDIR)\shockwave.obj" \
	"$(INTDIR)\slider.obj" \
	"$(INTDIR)\slider2.obj" \
	"$(INTDIR)\snazzyui.obj" \
	"$(INTDIR)\spline.obj" \
	"$(INTDIR)\starfield.obj" \
	"$(INTDIR)\staticrand.obj" \
	"$(INTDIR)\stats.obj" \
	"$(INTDIR)\supernova.obj" \
	"$(INTDIR)\swarm.obj" \
	"$(INTDIR)\systemvars.obj" \
	"$(INTDIR)\techmenu.obj" \
	"$(INTDIR)\tgautils.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\trails.obj" \
	"$(INTDIR)\trainingmenu.obj" \
	"$(INTDIR)\uidraw.obj" \
	"$(INTDIR)\uimouse.obj" \
	"$(INTDIR)\vecmat.obj" \
	"$(INTDIR)\version.obj" \
	"$(INTDIR)\warpineffect.obj" \
	"$(INTDIR)\weapons.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\windebug.obj" \
	"$(INTDIR)\levelpaging.obj" \
	"$(INTDIR)\freespace.obj" \
	"$(INTDIR)\grglidetexture.obj" \
	"$(INTDIR)\grd3drender.obj" \
	"$(INTDIR)\grd3dtexture.obj" \
	"$(INTDIR)\grdirectdraw.obj" \
	"$(INTDIR)\grglide.obj" \
	"$(INTDIR)\grd3d.obj" \
	"$(INTDIR)\glide.obj" \
	"$(INTDIR)\grsoft.obj" \
	"$(INTDIR)\multi_log.obj" \
	"$(INTDIR)\scaler.obj" \
	"$(INTDIR)\tmapper.obj" \
	"$(INTDIR)\tmapscantiled64x64.obj" \
	"$(INTDIR)\tmapscantiled16x16.obj" \
	"$(INTDIR)\tmapscantiled256x256.obj" \
	"$(INTDIR)\tmapscantiled32x32.obj" \
	"$(INTDIR)\tmapscantiled128x128.obj" \
	"$(INTDIR)\tmapscanline.obj" \
	$(SOUND_OBJS) \
	$(JOYSTICK_OBJS) \
	$(NETWORK_OBJS)

"$(OUTDIR)\fs2_open.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\fs2_open.exe"


CLEAN :
	-@erase "$(INTDIR)\2d.obj"
	-@erase "$(INTDIR)\3dclipper.obj"
	-@erase "$(INTDIR)\3ddraw.obj"
	-@erase "$(INTDIR)\3dlaser.obj"
	-@erase "$(INTDIR)\3dmath.obj"
	-@erase "$(INTDIR)\3dsetup.obj"
	-@erase "$(INTDIR)\aaline.obj"
	-@erase "$(INTDIR)\afterburner.obj"
	-@erase "$(INTDIR)\ai.obj"
	-@erase "$(INTDIR)\aibig.obj"
	-@erase "$(INTDIR)\aicode.obj"
	-@erase "$(INTDIR)\aigoals.obj"
	-@erase "$(INTDIR)\alphacolors.obj"
	-@erase "$(INTDIR)\animplay.obj"
	-@erase "$(INTDIR)\asteroid.obj"
	-@erase "$(INTDIR)\awacs.obj"
	-@erase "$(INTDIR)\barracks.obj"
	-@erase "$(INTDIR)\beam.obj"
	-@erase "$(INTDIR)\bitblt.obj"
	-@erase "$(INTDIR)\bmpman.obj"
	-@erase "$(INTDIR)\button.obj"
	-@erase "$(INTDIR)\cfile.obj"
	-@erase "$(INTDIR)\cfilearchive.obj"
	-@erase "$(INTDIR)\cfilelist.obj"
	-@erase "$(INTDIR)\cfilesystem.obj"
	-@erase "$(INTDIR)\cftp.obj"
	-@erase "$(INTDIR)\checkbox.obj"
	-@erase "$(INTDIR)\chttpget.obj"
	-@erase "$(INTDIR)\circle.obj"
	-@erase "$(INTDIR)\cmdline.obj"
	-@erase "$(INTDIR)\cmeasure.obj"
	-@erase "$(INTDIR)\collidedebrisship.obj"
	-@erase "$(INTDIR)\collidedebrisweapon.obj"
	-@erase "$(INTDIR)\collideshipship.obj"
	-@erase "$(INTDIR)\collideshipweapon.obj"
	-@erase "$(INTDIR)\collideweaponweapon.obj"
	-@erase "$(INTDIR)\colors.obj"
	-@erase "$(INTDIR)\console.obj"
	-@erase "$(INTDIR)\contexthelp.obj"
	-@erase "$(INTDIR)\controlsconfig.obj"
	-@erase "$(INTDIR)\controlsconfigcommon.obj"
	-@erase "$(INTDIR)\corkscrew.obj"
	-@erase "$(INTDIR)\credits.obj"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\cutscenes.obj"
	-@erase "$(INTDIR)\debris.obj"
	-@erase "$(INTDIR)\demo.obj"
	-@erase "$(INTDIR)\emp.obj"
	-@erase "$(INTDIR)\encrypt.obj"
	-@erase "$(INTDIR)\exceptionhandler.obj"
	-@erase "$(INTDIR)\fhash.obj"
	-@erase "$(INTDIR)\fireballs.obj"
	-@erase "$(INTDIR)\fishtank.obj"
	-@erase "$(INTDIR)\fix.obj"
	-@erase "$(INTDIR)\flak.obj"
	-@erase "$(INTDIR)\floating.obj"
	-@erase "$(INTDIR)\font.obj"
	-@erase "$(INTDIR)\freespace.obj"
	-@erase "$(INTDIR)\fvi.obj"
	-@erase "$(INTDIR)\gadget.obj"
	-@erase "$(INTDIR)\gameplayhelp.obj"
	-@erase "$(INTDIR)\gamesequence.obj"
	-@erase "$(INTDIR)\glide.obj"
	-@erase "$(INTDIR)\gradient.obj"
	-@erase "$(INTDIR)\grd3d.obj"
	-@erase "$(INTDIR)\grd3drender.obj"
	-@erase "$(INTDIR)\grd3dtexture.obj"
	-@erase "$(INTDIR)\grdirectdraw.obj"
	-@erase "$(INTDIR)\grglide.obj"
	-@erase "$(INTDIR)\grglidetexture.obj"
	-@erase "$(INTDIR)\grsoft.obj"
	-@erase "$(INTDIR)\grzbuffer.obj"
	-@erase "$(INTDIR)\hud.obj"
	-@erase "$(INTDIR)\hudartillery.obj"
	-@erase "$(INTDIR)\hudbrackets.obj"
	-@erase "$(INTDIR)\hudconfig.obj"
	-@erase "$(INTDIR)\hudescort.obj"
	-@erase "$(INTDIR)\hudets.obj"
	-@erase "$(INTDIR)\hudlock.obj"
	-@erase "$(INTDIR)\hudmessage.obj"
	-@erase "$(INTDIR)\hudreticle.obj"
	-@erase "$(INTDIR)\hudshield.obj"
	-@erase "$(INTDIR)\hudsquadmsg.obj"
	-@erase "$(INTDIR)\hudtarget.obj"
	-@erase "$(INTDIR)\hudtargetbox.obj"
	-@erase "$(INTDIR)\hudwingmanstatus.obj"
	-@erase "$(INTDIR)\icon.obj"
	-@erase "$(INTDIR)\inputbox.obj"
	-@erase "$(INTDIR)\jumpnode.obj"
	-@erase "$(INTDIR)\key.obj"
	-@erase "$(INTDIR)\keycontrol.obj"
	-@erase "$(INTDIR)\keytrap.obj"
	-@erase "$(INTDIR)\levelpaging.obj"
	-@erase "$(INTDIR)\lighting.obj"
	-@erase "$(INTDIR)\line.obj"
	-@erase "$(INTDIR)\listbox.obj"
	-@erase "$(INTDIR)\localize.obj"
	-@erase "$(INTDIR)\mainhallmenu.obj"
	-@erase "$(INTDIR)\managepilot.obj"
	-@erase "$(INTDIR)\medals.obj"
	-@erase "$(INTDIR)\missionbrief.obj"
	-@erase "$(INTDIR)\missionbriefcommon.obj"
	-@erase "$(INTDIR)\missioncampaign.obj"
	-@erase "$(INTDIR)\missioncmdbrief.obj"
	-@erase "$(INTDIR)\missiondebrief.obj"
	-@erase "$(INTDIR)\missiongoals.obj"
	-@erase "$(INTDIR)\missiongrid.obj"
	-@erase "$(INTDIR)\missionhotkey.obj"
	-@erase "$(INTDIR)\missionload.obj"
	-@erase "$(INTDIR)\missionlog.obj"
	-@erase "$(INTDIR)\missionloopbrief.obj"
	-@erase "$(INTDIR)\missionmessage.obj"
	-@erase "$(INTDIR)\missionparse.obj"
	-@erase "$(INTDIR)\missionpause.obj"
	-@erase "$(INTDIR)\missionrecommend.obj"
	-@erase "$(INTDIR)\missionscreencommon.obj"
	-@erase "$(INTDIR)\missionshipchoice.obj"
	-@erase "$(INTDIR)\missionstats.obj"
	-@erase "$(INTDIR)\missiontraining.obj"
	-@erase "$(INTDIR)\missionweaponchoice.obj"
	-@erase "$(INTDIR)\modelcollide.obj"
	-@erase "$(INTDIR)\modelinterp.obj"
	-@erase "$(INTDIR)\modeloctant.obj"
	-@erase "$(INTDIR)\modelread.obj"
	-@erase "$(INTDIR)\mouse.obj"
	-@erase "$(INTDIR)\multi_log.obj"
	-@erase "$(INTDIR)\multiutil.obj"
	-@erase "$(INTDIR)\muzzleflash.obj"
	-@erase "$(INTDIR)\neb.obj"
	-@erase "$(INTDIR)\neblightning.obj"
	-@erase "$(INTDIR)\nebula.obj"
	-@erase "$(INTDIR)\objcollide.obj"
	-@erase "$(INTDIR)\object.obj"
	-@erase "$(INTDIR)\objectsort.obj"
	-@erase "$(INTDIR)\observer.obj"
	-@erase "$(INTDIR)\optionsmenu.obj"
	-@erase "$(INTDIR)\osapi.obj"
	-@erase "$(INTDIR)\osregistry.obj"
	-@erase "$(INTDIR)\outwnd.obj"
	-@erase "$(INTDIR)\packunpack.obj"
	-@erase "$(INTDIR)\palman.obj"
	-@erase "$(INTDIR)\parselo.obj"
	-@erase "$(INTDIR)\particle.obj"
	-@erase "$(INTDIR)\pcxutils.obj"
	-@erase "$(INTDIR)\physics.obj"
	-@erase "$(INTDIR)\pixel.obj"
	-@erase "$(INTDIR)\playercontrol.obj"
	-@erase "$(INTDIR)\playermenu.obj"
	-@erase "$(INTDIR)\popup.obj"
	-@erase "$(INTDIR)\popupdead.obj"
	-@erase "$(INTDIR)\radar.obj"
	-@erase "$(INTDIR)\radio.obj"
	-@erase "$(INTDIR)\readyroom.obj"
	-@erase "$(INTDIR)\rect.obj"
	-@erase "$(INTDIR)\redalert.obj"
	-@erase "$(INTDIR)\scaler.obj"
	-@erase "$(INTDIR)\scoring.obj"
	-@erase "$(INTDIR)\scroll.obj"
	-@erase "$(INTDIR)\sexp.obj"
	-@erase "$(INTDIR)\shade.obj"
	-@erase "$(INTDIR)\shield.obj"
	-@erase "$(INTDIR)\ship.obj"
	-@erase "$(INTDIR)\shipcontrails.obj"
	-@erase "$(INTDIR)\shipfx.obj"
	-@erase "$(INTDIR)\shiphit.obj"
	-@erase "$(INTDIR)\shockwave.obj"
	-@erase "$(INTDIR)\slider.obj"
	-@erase "$(INTDIR)\slider2.obj"
	-@erase "$(INTDIR)\snazzyui.obj"
	-@erase "$(INTDIR)\spline.obj"
	-@erase "$(INTDIR)\starfield.obj"
	-@erase "$(INTDIR)\staticrand.obj"
	-@erase "$(INTDIR)\stats.obj"
	-@erase "$(INTDIR)\supernova.obj"
	-@erase "$(INTDIR)\swarm.obj"
	-@erase "$(INTDIR)\systemvars.obj"
	-@erase "$(INTDIR)\techmenu.obj"
	-@erase "$(INTDIR)\tgautils.obj"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\tmapper.obj"
	-@erase "$(INTDIR)\tmapscanline.obj"
	-@erase "$(INTDIR)\tmapscantiled128x128.obj"
	-@erase "$(INTDIR)\tmapscantiled16x16.obj"
	-@erase "$(INTDIR)\tmapscantiled256x256.obj"
	-@erase "$(INTDIR)\tmapscantiled32x32.obj"
	-@erase "$(INTDIR)\tmapscantiled64x64.obj"
	-@erase "$(INTDIR)\trails.obj"
	-@erase "$(INTDIR)\trainingmenu.obj"
	-@erase "$(INTDIR)\uidraw.obj"
	-@erase "$(INTDIR)\uimouse.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vecmat.obj"
	-@erase "$(INTDIR)\version.obj"
	-@erase "$(INTDIR)\warpineffect.obj"
	-@erase "$(INTDIR)\weapons.obj"
	-@erase "$(INTDIR)\windebug.obj"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(INTDIR)\eventmusic.obj"
	-@erase "$(INTDIR)\gamesnd.obj"
	-@erase "$(INTDIR)\objectsnd.obj"
	-@erase "$(INTDIR)\acm.obj"
	-@erase "$(INTDIR)\audiostr.obj"
	-@erase "$(INTDIR)\ds.obj"
	-@erase "$(INTDIR)\ds3d.obj"
	-@erase "$(INTDIR)\dscap.obj"
	-@erase "$(INTDIR)\midifile.obj"
	-@erase "$(INTDIR)\rbaudio.obj"
	-@erase "$(INTDIR)\rtvoice.obj"
	-@erase "$(INTDIR)\sound.obj"
	-@erase "$(INTDIR)\winmidi.obj"
	-@erase "$(INTDIR)\winmidi_base.obj"
	-@erase "$(INTDIR)\codec1.obj"
	-@erase "$(INTDIR)\joy.obj"
	-@erase "$(INTDIR)\joy_ff.obj"
	-@erase "$(INTDIR)\swff_lib.obj"
	-@erase "$(INTDIR)\multi.obj"
	-@erase "$(INTDIR)\multimsgs.obj"
	-@erase "$(INTDIR)\multiteamselect.obj"
	-@erase "$(INTDIR)\multiui"
	-@erase "$(INTDIR)\multi_campaign.obj"
	-@erase "$(INTDIR)\multi_data.obj"
	-@erase "$(INTDIR)\multi_dogfight.obj"
	-@erase "$(INTDIR)\multi_endgame"
	-@erase "$(INTDIR)\multi_ingame"
	-@erase "$(INTDIR)\multi_kick"
	-@erase "$(INTDIR)\multi_obj.obj"
	-@erase "$(INTDIR)\multi_observer.obj"
	-@erase "$(INTDIR)\multi_options.obj"
	-@erase "$(INTDIR)\multi_pause.obj"
	-@erase "$(INTDIR)\multi_pinfo.obj"
	-@erase "$(INTDIR)\multi_ping.obj"
	-@erase "$(INTDIR)\multi_pmsg.obj"
	-@erase "$(INTDIR)\multi_rate.obj"
	-@erase "$(INTDIR)\multi_respawn.obj"
	-@erase "$(INTDIR)\multi_team.obj"
	-@erase "$(INTDIR)\multi_xfer.obj"
	-@erase "$(INTDIR)\psnet2.obj"
	-@erase "$(INTDIR)\psnet.obj"
	-@erase "$(INTDIR)\stand_gui.obj"
	-@erase "$(INTDIR)\optionsmenumulti.obj"
	-@erase "$(INTDIR)\chatbox.obj"
	-@erase "$(INTDIR)\fs2ox.obj"
	-@erase "$(OUTDIR)\fs2_open.exe"
	-@erase "$(OUTDIR)\fs2_open.ilk"
	-@erase "$(OUTDIR)\fs2_open.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "code" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" $(NETWORK_DEF) $(JOYSTICK_DEF) $(SOUND_DEF) /Fp"$(INTDIR)\fs2_open.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"\
 /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\fs2_open.bsc" 
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib advapi32.lib wsock32.lib dxguid.lib ddraw.lib dinput.lib winmm.lib msacm32.lib ole32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\fs2_open.pdb" /debug /machine:I386 /out:"$(OUTDIR)\fs2_open.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\2d.obj" \
	"$(INTDIR)\3dclipper.obj" \
	"$(INTDIR)\3ddraw.obj" \
	"$(INTDIR)\3dlaser.obj" \
	"$(INTDIR)\3dmath.obj" \
	"$(INTDIR)\3dsetup.obj" \
	"$(INTDIR)\aaline.obj" \
	"$(INTDIR)\afterburner.obj" \
	"$(INTDIR)\ai.obj" \
	"$(INTDIR)\aibig.obj" \
	"$(INTDIR)\aicode.obj" \
	"$(INTDIR)\aigoals.obj" \
	"$(INTDIR)\alphacolors.obj" \
	"$(INTDIR)\animplay.obj" \
	"$(INTDIR)\asteroid.obj" \
	"$(INTDIR)\awacs.obj" \
	"$(INTDIR)\barracks.obj" \
	"$(INTDIR)\beam.obj" \
	"$(INTDIR)\bitblt.obj" \
	"$(INTDIR)\bmpman.obj" \
	"$(INTDIR)\button.obj" \
	"$(INTDIR)\cfile.obj" \
	"$(INTDIR)\cfilearchive.obj" \
	"$(INTDIR)\cfilelist.obj" \
	"$(INTDIR)\cfilesystem.obj" \
	"$(INTDIR)\cftp.obj" \
	"$(INTDIR)\checkbox.obj" \
	"$(INTDIR)\chttpget.obj" \
	"$(INTDIR)\circle.obj" \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\cmeasure.obj" \
	"$(INTDIR)\collidedebrisship.obj" \
	"$(INTDIR)\collidedebrisweapon.obj" \
	"$(INTDIR)\collideshipship.obj" \
	"$(INTDIR)\collideshipweapon.obj" \
	"$(INTDIR)\collideweaponweapon.obj" \
	"$(INTDIR)\colors.obj" \
	"$(INTDIR)\console.obj" \
	"$(INTDIR)\contexthelp.obj" \
	"$(INTDIR)\controlsconfig.obj" \
	"$(INTDIR)\controlsconfigcommon.obj" \
	"$(INTDIR)\corkscrew.obj" \
	"$(INTDIR)\credits.obj" \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\cutscenes.obj" \
	"$(INTDIR)\debris.obj" \
	"$(INTDIR)\demo.obj" \
	"$(INTDIR)\emp.obj" \
	"$(INTDIR)\encrypt.obj" \
	"$(INTDIR)\exceptionhandler.obj" \
	"$(INTDIR)\fhash.obj" \
	"$(INTDIR)\fireballs.obj" \
	"$(INTDIR)\fishtank.obj" \
	"$(INTDIR)\fix.obj" \
	"$(INTDIR)\flak.obj" \
	"$(INTDIR)\floating.obj" \
	"$(INTDIR)\font.obj" \
	"$(INTDIR)\fvi.obj" \
	"$(INTDIR)\gadget.obj" \
	"$(INTDIR)\gameplayhelp.obj" \
	"$(INTDIR)\gamesequence.obj" \
	"$(INTDIR)\gradient.obj" \
	"$(INTDIR)\grzbuffer.obj" \
	"$(INTDIR)\hud.obj" \
	"$(INTDIR)\hudartillery.obj" \
	"$(INTDIR)\hudbrackets.obj" \
	"$(INTDIR)\hudconfig.obj" \
	"$(INTDIR)\hudescort.obj" \
	"$(INTDIR)\hudets.obj" \
	"$(INTDIR)\hudlock.obj" \
	"$(INTDIR)\hudmessage.obj" \
	"$(INTDIR)\hudreticle.obj" \
	"$(INTDIR)\hudshield.obj" \
	"$(INTDIR)\hudsquadmsg.obj" \
	"$(INTDIR)\hudtarget.obj" \
	"$(INTDIR)\hudtargetbox.obj" \
	"$(INTDIR)\hudwingmanstatus.obj" \
	"$(INTDIR)\icon.obj" \
	"$(INTDIR)\inputbox.obj" \
	"$(INTDIR)\jumpnode.obj" \
	"$(INTDIR)\key.obj" \
	"$(INTDIR)\keycontrol.obj" \
	"$(INTDIR)\keytrap.obj" \
	"$(INTDIR)\lighting.obj" \
	"$(INTDIR)\line.obj" \
	"$(INTDIR)\listbox.obj" \
	"$(INTDIR)\localize.obj" \
	"$(INTDIR)\mainhallmenu.obj" \
	"$(INTDIR)\managepilot.obj" \
	"$(INTDIR)\medals.obj" \
	"$(INTDIR)\missionbrief.obj" \
	"$(INTDIR)\missionbriefcommon.obj" \
	"$(INTDIR)\missioncampaign.obj" \
	"$(INTDIR)\missioncmdbrief.obj" \
	"$(INTDIR)\missiondebrief.obj" \
	"$(INTDIR)\missiongoals.obj" \
	"$(INTDIR)\missiongrid.obj" \
	"$(INTDIR)\missionhotkey.obj" \
	"$(INTDIR)\missionload.obj" \
	"$(INTDIR)\missionlog.obj" \
	"$(INTDIR)\missionloopbrief.obj" \
	"$(INTDIR)\missionmessage.obj" \
	"$(INTDIR)\missionparse.obj" \
	"$(INTDIR)\missionpause.obj" \
	"$(INTDIR)\missionrecommend.obj" \
	"$(INTDIR)\missionscreencommon.obj" \
	"$(INTDIR)\missionshipchoice.obj" \
	"$(INTDIR)\missionstats.obj" \
	"$(INTDIR)\missiontraining.obj" \
	"$(INTDIR)\missionweaponchoice.obj" \
	"$(INTDIR)\modelcollide.obj" \
	"$(INTDIR)\modelinterp.obj" \
	"$(INTDIR)\modeloctant.obj" \
	"$(INTDIR)\modelread.obj" \
	"$(INTDIR)\mouse.obj" \
	"$(INTDIR)\multiutil.obj" \
	"$(INTDIR)\muzzleflash.obj" \
	"$(INTDIR)\neb.obj" \
	"$(INTDIR)\neblightning.obj" \
	"$(INTDIR)\nebula.obj" \
	"$(INTDIR)\objcollide.obj" \
	"$(INTDIR)\object.obj" \
	"$(INTDIR)\objectsort.obj" \
	"$(INTDIR)\observer.obj" \
	"$(INTDIR)\optionsmenu.obj" \
	"$(INTDIR)\osapi.obj" \
	"$(INTDIR)\osregistry.obj" \
	"$(INTDIR)\outwnd.obj" \
	"$(INTDIR)\packunpack.obj" \
	"$(INTDIR)\palman.obj" \
	"$(INTDIR)\parselo.obj" \
	"$(INTDIR)\particle.obj" \
	"$(INTDIR)\pcxutils.obj" \
	"$(INTDIR)\physics.obj" \
	"$(INTDIR)\pixel.obj" \
	"$(INTDIR)\playercontrol.obj" \
	"$(INTDIR)\playermenu.obj" \
	"$(INTDIR)\popup.obj" \
	"$(INTDIR)\popupdead.obj" \
	"$(INTDIR)\radar.obj" \
	"$(INTDIR)\radio.obj" \
	"$(INTDIR)\readyroom.obj" \
	"$(INTDIR)\rect.obj" \
	"$(INTDIR)\redalert.obj" \
	"$(INTDIR)\scoring.obj" \
	"$(INTDIR)\scroll.obj" \
	"$(INTDIR)\sexp.obj" \
	"$(INTDIR)\shade.obj" \
	"$(INTDIR)\shield.obj" \
	"$(INTDIR)\ship.obj" \
	"$(INTDIR)\shipcontrails.obj" \
	"$(INTDIR)\shipfx.obj" \
	"$(INTDIR)\shiphit.obj" \
	"$(INTDIR)\shockwave.obj" \
	"$(INTDIR)\slider.obj" \
	"$(INTDIR)\slider2.obj" \
	"$(INTDIR)\snazzyui.obj" \
	"$(INTDIR)\spline.obj" \
	"$(INTDIR)\starfield.obj" \
	"$(INTDIR)\staticrand.obj" \
	"$(INTDIR)\stats.obj" \
	"$(INTDIR)\supernova.obj" \
	"$(INTDIR)\swarm.obj" \
	"$(INTDIR)\systemvars.obj" \
	"$(INTDIR)\techmenu.obj" \
	"$(INTDIR)\tgautils.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\trails.obj" \
	"$(INTDIR)\trainingmenu.obj" \
	"$(INTDIR)\uidraw.obj" \
	"$(INTDIR)\uimouse.obj" \
	"$(INTDIR)\vecmat.obj" \
	"$(INTDIR)\version.obj" \
	"$(INTDIR)\warpineffect.obj" \
	"$(INTDIR)\weapons.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\windebug.obj" \
	"$(INTDIR)\levelpaging.obj" \
	"$(INTDIR)\freespace.obj" \
	"$(INTDIR)\grglidetexture.obj" \
	"$(INTDIR)\grd3drender.obj" \
	"$(INTDIR)\grd3dtexture.obj" \
	"$(INTDIR)\grdirectdraw.obj" \
	"$(INTDIR)\grglide.obj" \
	"$(INTDIR)\grd3d.obj" \
	"$(INTDIR)\glide.obj" \
	"$(INTDIR)\grsoft.obj" \
	"$(INTDIR)\multi_log.obj" \
	"$(INTDIR)\scaler.obj" \
	"$(INTDIR)\tmapper.obj" \
	"$(INTDIR)\tmapscantiled64x64.obj" \
	"$(INTDIR)\tmapscantiled16x16.obj" \
	"$(INTDIR)\tmapscantiled256x256.obj" \
	"$(INTDIR)\tmapscantiled32x32.obj" \
	"$(INTDIR)\tmapscantiled128x128.obj" \
	"$(INTDIR)\tmapscanline.obj" \
	$(SOUND_OBJS) \
	$(JOYSTICK_OBJS) \
	$(NETWORK_OBJS)

"$(OUTDIR)\fs2_open.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


# !IF "$(NO_EXTERNAL_DEPS)" != "1"
# !IF EXISTS("fs2_open.dep")
# !INCLUDE "fs2_open.dep"
# !ELSE 
# !MESSAGE Warning: cannot find "fs2_open.dep"
# !ENDIF 
# !ENDIF 


!IF "$(CFG)" == "Release" || "$(CFG)" == "Debug"
SOURCE=.\code\graphics\2d.cpp

"$(INTDIR)\2d.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\render\3dclipper.cpp

"$(INTDIR)\3dclipper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\render\3ddraw.cpp

"$(INTDIR)\3ddraw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\render\3dlaser.cpp

"$(INTDIR)\3dlaser.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\render\3dmath.cpp

"$(INTDIR)\3dmath.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\render\3dsetup.cpp

"$(INTDIR)\3dsetup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\aaline.cpp

"$(INTDIR)\aaline.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ship\afterburner.cpp

"$(INTDIR)\afterburner.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ship\ai.cpp

"$(INTDIR)\ai.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ship\aibig.cpp

"$(INTDIR)\aibig.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ship\aicode.cpp

"$(INTDIR)\aicode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ship\aigoals.cpp

"$(INTDIR)\aigoals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\globalincs\alphacolors.cpp

"$(INTDIR)\alphacolors.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\anim\animplay.cpp

"$(INTDIR)\animplay.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\asteroid\asteroid.cpp

"$(INTDIR)\asteroid.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ship\awacs.cpp

"$(INTDIR)\awacs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\menuui\barracks.cpp

"$(INTDIR)\barracks.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\weapon\beam.cpp

"$(INTDIR)\beam.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\bitblt.cpp

"$(INTDIR)\bitblt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\bmpman\bmpman.cpp

"$(INTDIR)\bmpman.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\button.cpp

"$(INTDIR)\button.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\cfile\cfile.cpp

"$(INTDIR)\cfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\cfile\cfilearchive.cpp

"$(INTDIR)\cfilearchive.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\cfile\cfilelist.cpp

"$(INTDIR)\cfilelist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\cfile\cfilesystem.cpp

"$(INTDIR)\cfilesystem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\inetfile\cftp.cpp

"$(INTDIR)\cftp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\checkbox.cpp

"$(INTDIR)\checkbox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\inetfile\chttpget.cpp

"$(INTDIR)\chttpget.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\circle.cpp

"$(INTDIR)\circle.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\cmdline\cmdline.cpp

"$(INTDIR)\cmdline.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\cmeasure\cmeasure.cpp

"$(INTDIR)\cmeasure.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\object\collidedebrisship.cpp

"$(INTDIR)\collidedebrisship.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\object\collidedebrisweapon.cpp

"$(INTDIR)\collidedebrisweapon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\object\collideshipship.cpp

"$(INTDIR)\collideshipship.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\object\collideshipweapon.cpp

"$(INTDIR)\collideshipweapon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\object\collideweaponweapon.cpp

"$(INTDIR)\collideweaponweapon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\colors.cpp

"$(INTDIR)\colors.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\debugconsole\console.cpp

"$(INTDIR)\console.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\gamehelp\contexthelp.cpp

"$(INTDIR)\contexthelp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\controlconfig\controlsconfig.cpp

"$(INTDIR)\controlsconfig.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\controlconfig\controlsconfigcommon.cpp

"$(INTDIR)\controlsconfigcommon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\weapon\corkscrew.cpp

"$(INTDIR)\corkscrew.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\menuui\credits.cpp

"$(INTDIR)\credits.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\globalincs\crypt.cpp

"$(INTDIR)\crypt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\cutscene\cutscenes.cpp

"$(INTDIR)\cutscenes.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\debris\debris.cpp

"$(INTDIR)\debris.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\demo\demo.cpp

"$(INTDIR)\demo.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\weapon\emp.cpp

"$(INTDIR)\emp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\parse\encrypt.cpp

"$(INTDIR)\encrypt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\exceptionhandler\exceptionhandler.cpp

"$(INTDIR)\exceptionhandler.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\localization\fhash.cpp

"$(INTDIR)\fhash.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\fireball\fireballs.cpp

"$(INTDIR)\fireballs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\menuui\fishtank.cpp

"$(INTDIR)\fishtank.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\math\fix.cpp

"$(INTDIR)\fix.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\weapon\flak.cpp

"$(INTDIR)\flak.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\math\floating.cpp

"$(INTDIR)\floating.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\font.cpp

"$(INTDIR)\font.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\freespace2\freespace.cpp

"$(INTDIR)\freespace.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\math\fvi.cpp

"$(INTDIR)\fvi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\gadget.cpp

"$(INTDIR)\gadget.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\gamehelp\gameplayhelp.cpp

"$(INTDIR)\gameplayhelp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\gamesequence\gamesequence.cpp

"$(INTDIR)\gamesequence.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\glide\glide.cpp

"$(INTDIR)\glide.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\gradient.cpp

"$(INTDIR)\gradient.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\grd3d.cpp

"$(INTDIR)\grd3d.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\grd3drender.cpp

"$(INTDIR)\grd3drender.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\grd3dtexture.cpp

"$(INTDIR)\grd3dtexture.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\grdirectdraw.cpp

"$(INTDIR)\grdirectdraw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\grglide.cpp

"$(INTDIR)\grglide.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\grglidetexture.cpp

"$(INTDIR)\grglidetexture.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\grsoft.cpp

"$(INTDIR)\grsoft.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\grzbuffer.cpp

"$(INTDIR)\grzbuffer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hud.cpp

"$(INTDIR)\hud.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudartillery.cpp

"$(INTDIR)\hudartillery.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudbrackets.cpp

"$(INTDIR)\hudbrackets.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudconfig.cpp

"$(INTDIR)\hudconfig.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudescort.cpp

"$(INTDIR)\hudescort.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudets.cpp

"$(INTDIR)\hudets.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudlock.cpp

"$(INTDIR)\hudlock.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudmessage.cpp

"$(INTDIR)\hudmessage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudreticle.cpp

"$(INTDIR)\hudreticle.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudshield.cpp

"$(INTDIR)\hudshield.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudsquadmsg.cpp

"$(INTDIR)\hudsquadmsg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudtarget.cpp

"$(INTDIR)\hudtarget.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudtargetbox.cpp

"$(INTDIR)\hudtargetbox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudwingmanstatus.cpp

"$(INTDIR)\hudwingmanstatus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\icon.cpp

"$(INTDIR)\icon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\inputbox.cpp

"$(INTDIR)\inputbox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\jumpnode\jumpnode.cpp

"$(INTDIR)\jumpnode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\io\key.cpp

"$(INTDIR)\key.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\io\keycontrol.cpp

"$(INTDIR)\keycontrol.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\keytrap.cpp

"$(INTDIR)\keytrap.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\freespace2\levelpaging.cpp

"$(INTDIR)\levelpaging.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\lighting\lighting.cpp

"$(INTDIR)\lighting.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\line.cpp

"$(INTDIR)\line.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\listbox.cpp

"$(INTDIR)\listbox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\localization\localize.cpp

"$(INTDIR)\localize.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\menuui\mainhallmenu.cpp

"$(INTDIR)\mainhallmenu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\playerman\managepilot.cpp

"$(INTDIR)\managepilot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\stats\medals.cpp

"$(INTDIR)\medals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\missionui\missionbrief.cpp

"$(INTDIR)\missionbrief.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\mission\missionbriefcommon.cpp

"$(INTDIR)\missionbriefcommon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\mission\missioncampaign.cpp

"$(INTDIR)\missioncampaign.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\missionui\missioncmdbrief.cpp

"$(INTDIR)\missioncmdbrief.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\missionui\missiondebrief.cpp

"$(INTDIR)\missiondebrief.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\mission\missiongoals.cpp

"$(INTDIR)\missiongoals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\mission\missiongrid.cpp

"$(INTDIR)\missiongrid.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\mission\missionhotkey.cpp

"$(INTDIR)\missionhotkey.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\mission\missionload.cpp

"$(INTDIR)\missionload.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\mission\missionlog.cpp

"$(INTDIR)\missionlog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\missionui\missionloopbrief.cpp

"$(INTDIR)\missionloopbrief.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\mission\missionmessage.cpp

"$(INTDIR)\missionmessage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\mission\missionparse.cpp

"$(INTDIR)\missionparse.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\missionui\missionpause.cpp

"$(INTDIR)\missionpause.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\missionui\missionrecommend.cpp

"$(INTDIR)\missionrecommend.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\missionui\missionscreencommon.cpp

"$(INTDIR)\missionscreencommon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\missionui\missionshipchoice.cpp

"$(INTDIR)\missionshipchoice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\missionui\missionstats.cpp

"$(INTDIR)\missionstats.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\mission\missiontraining.cpp

"$(INTDIR)\missiontraining.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\missionui\missionweaponchoice.cpp

"$(INTDIR)\missionweaponchoice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\model\modelcollide.cpp

"$(INTDIR)\modelcollide.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\model\modelinterp.cpp

"$(INTDIR)\modelinterp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\model\modeloctant.cpp

"$(INTDIR)\modeloctant.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\model\modelread.cpp

"$(INTDIR)\modelread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\io\mouse.cpp

"$(INTDIR)\mouse.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_log.cpp

"$(INTDIR)\multi_log.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multiutil.cpp

"$(INTDIR)\multiutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\weapon\muzzleflash.cpp

"$(INTDIR)\muzzleflash.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\nebula\neb.cpp

"$(INTDIR)\neb.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\nebula\neblightning.cpp

"$(INTDIR)\neblightning.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\starfield\nebula.cpp

"$(INTDIR)\nebula.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\object\objcollide.cpp

"$(INTDIR)\objcollide.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\object\object.cpp

"$(INTDIR)\object.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\object\objectsort.cpp

"$(INTDIR)\objectsort.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\observer\observer.cpp

"$(INTDIR)\observer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\menuui\optionsmenu.cpp

"$(INTDIR)\optionsmenu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\osapi\osapi.cpp

"$(INTDIR)\osapi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\osapi\osregistry.cpp

"$(INTDIR)\osregistry.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\osapi\outwnd.cpp

"$(INTDIR)\outwnd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\anim\packunpack.cpp

"$(INTDIR)\packunpack.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\palman\palman.cpp

"$(INTDIR)\palman.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\parse\parselo.cpp

"$(INTDIR)\parselo.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\particle\particle.cpp

"$(INTDIR)\particle.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\pcxutils\pcxutils.cpp

"$(INTDIR)\pcxutils.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\physics\physics.cpp

"$(INTDIR)\physics.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\pixel.cpp

"$(INTDIR)\pixel.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\playerman\playercontrol.cpp

"$(INTDIR)\playercontrol.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\menuui\playermenu.cpp

"$(INTDIR)\playermenu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\popup\popup.cpp

"$(INTDIR)\popup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\popup\popupdead.cpp

"$(INTDIR)\popupdead.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\radar\radar.cpp

"$(INTDIR)\radar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\radio.cpp

"$(INTDIR)\radio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\menuui\readyroom.cpp

"$(INTDIR)\readyroom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\rect.cpp

"$(INTDIR)\rect.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\missionui\redalert.cpp

"$(INTDIR)\redalert.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\scaler.cpp

"$(INTDIR)\scaler.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\stats\scoring.cpp

"$(INTDIR)\scoring.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\scroll.cpp

"$(INTDIR)\scroll.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\parse\sexp.cpp

"$(INTDIR)\sexp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\shade.cpp

"$(INTDIR)\shade.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ship\shield.cpp

"$(INTDIR)\shield.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ship\ship.cpp

"$(INTDIR)\ship.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ship\shipcontrails.cpp

"$(INTDIR)\shipcontrails.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ship\shipfx.cpp

"$(INTDIR)\shipfx.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ship\shiphit.cpp

"$(INTDIR)\shiphit.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\weapon\shockwave.cpp

"$(INTDIR)\shockwave.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\slider.cpp

"$(INTDIR)\slider.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\slider2.cpp

"$(INTDIR)\slider2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\menuui\snazzyui.cpp

"$(INTDIR)\snazzyui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\math\spline.cpp

"$(INTDIR)\spline.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\starfield\starfield.cpp

"$(INTDIR)\starfield.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\math\staticrand.cpp

"$(INTDIR)\staticrand.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\stats\stats.cpp

"$(INTDIR)\stats.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\starfield\supernova.cpp

"$(INTDIR)\supernova.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\weapon\swarm.cpp

"$(INTDIR)\swarm.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\globalincs\systemvars.cpp

"$(INTDIR)\systemvars.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\menuui\techmenu.cpp

"$(INTDIR)\techmenu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\tgautils\tgautils.cpp

"$(INTDIR)\tgautils.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\io\timer.cpp

"$(INTDIR)\timer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\tmapper.cpp

"$(INTDIR)\tmapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\tmapscanline.cpp

"$(INTDIR)\tmapscanline.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\tmapscantiled128x128.cpp

"$(INTDIR)\tmapscantiled128x128.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\tmapscantiled16x16.cpp

"$(INTDIR)\tmapscantiled16x16.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\tmapscantiled256x256.cpp

"$(INTDIR)\tmapscantiled256x256.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\tmapscantiled32x32.cpp

"$(INTDIR)\tmapscantiled32x32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\graphics\tmapscantiled64x64.cpp

"$(INTDIR)\tmapscantiled64x64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\weapon\trails.cpp

"$(INTDIR)\trails.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\menuui\trainingmenu.cpp

"$(INTDIR)\trainingmenu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\uidraw.cpp

"$(INTDIR)\uidraw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\uimouse.cpp

"$(INTDIR)\uimouse.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\math\vecmat.cpp

"$(INTDIR)\vecmat.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\globalincs\version.cpp

"$(INTDIR)\version.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\fireball\warpineffect.cpp

"$(INTDIR)\warpineffect.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\weapon\weapons.cpp

"$(INTDIR)\weapons.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\globalincs\windebug.cpp

"$(INTDIR)\windebug.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\ui\window.cpp

"$(INTDIR)\window.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\gamesnd\eventmusic.cpp

"$(INTDIR)\eventmusic.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\gamesnd\gamesnd.cpp

"$(INTDIR)\gamesnd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\object\objectsnd.cpp

"$(INTDIR)\objectsnd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\sound\acm.cpp

"$(INTDIR)\acm.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\sound\audiostr.cpp

"$(INTDIR)\audiostr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\sound\ds.cpp

"$(INTDIR)\ds.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\sound\ds3d.cpp

"$(INTDIR)\ds3d.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\sound\dscap.cpp

"$(INTDIR)\dscap.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\sound\midifile.cpp

"$(INTDIR)\midifile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\sound\rbaudio.cpp

"$(INTDIR)\rbaudio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\sound\rtvoice.cpp

"$(INTDIR)\rtvoice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\sound\sound.cpp

"$(INTDIR)\sound.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\sound\winmidi.cpp

"$(INTDIR)\winmidi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\sound\winmidi_base.cpp

"$(INTDIR)\winmidi_base.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\vcodec\codec1.cpp

"$(INTDIR)\codec1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\io\joy.cpp

"$(INTDIR)\joy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\io\joy_ff.cpp

"$(INTDIR)\joy_ff.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\io\swff_lib.cpp

"$(INTDIR)\swff_lib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\hud\hudobserver.cpp

"$(INTDIR)\hudobserver.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi.cpp

"$(INTDIR)\multi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multimsgs.cpp

"$(INTDIR)\multimsgs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multiteamselect.cpp

"$(INTDIR)\multiteamselect.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multiui.cpp

"$(INTDIR)\multiui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_campaign.cpp

"$(INTDIR)\multi_campaign.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_data.cpp

"$(INTDIR)\multi_data.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_dogfight.cpp

"$(INTDIR)\multi_dogfight.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_endgame.cpp

"$(INTDIR)\multi_endgame.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_ingame.cpp

"$(INTDIR)\multi_ingame.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_kick.cpp

"$(INTDIR)\multi_kick.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_obj.cpp

"$(INTDIR)\multi_obj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_observer.cpp

"$(INTDIR)\multi_observer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_options.cpp

"$(INTDIR)\multi_options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_pause.cpp

"$(INTDIR)\multi_pause.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_pinfo.cpp

"$(INTDIR)\multi_pinfo.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_ping.cpp

"$(INTDIR)\multi_ping.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_pmsg.cpp

"$(INTDIR)\multi_pmsg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_rate.cpp

"$(INTDIR)\multi_rate.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_respawn.cpp

"$(INTDIR)\multi_respawn.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_team.cpp

"$(INTDIR)\multi_team.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_voice.cpp

"$(INTDIR)\multi_voice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\multi_xfer.cpp

"$(INTDIR)\multi_xfer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\psnet.cpp

"$(INTDIR)\psnet.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\psnet2.cpp

"$(INTDIR)\psnet2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\stand_gui.cpp

"$(INTDIR)\stand_gui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\menuui\optionsmenumulti.cpp

"$(INTDIR)\optionsmenumulti.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\missionui\chatbox.cpp

"$(INTDIR)\chatbox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\code\network\fs2ox.cpp

"$(INTDIR)\fs2ox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 
