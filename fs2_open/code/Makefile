TARGET  = fs2

ifeq ($(USE_SOUND), 1)
SOUND_DEF=
SOUND_OBJS= \
 gamesnd/eventmusic.o \
 gamesnd/gamesnd.o \
 object/objectsnd.o \
 sound/acm.o \
 sound/audiostr.o \
 sound/ds.o \
 sound/ds3d.o \
 sound/dscap.o \
 sound/midifile.o \
 sound/rbaudio.o \
 sound/rtvoice.o \
 sound/sound.o \
 sound/winmidi.o \
 sound/winmidi_base.o \
 vcodec/codec1.o
else
SOUND_DEF=-DNO_SOUND
SOUND_OBJS=
endif # USE_SOUND


ifeq ($(USE_JOYSTICK), 1)
JOYSTICK_DEF=
JOYSTICK_OBJS= \
 io/joy.o \
 io/joy_ff.o \
 io/swff_lib.o
else
JOYSTICK_DEF=-DNO_JOYSTICK
JOYSTICK_OBJS=
endif # USE_JOYSTICK


ifeq ($(USE_NETWORK), 1)
# no support for standalone server yet... (UI is Win32 specific)
NETWORK_DEF=-DNO_STANDALONE
NETWORK_OBJS= \
 hud/hudobserver.o \
 network/multi.o \
 network/multimsgs.o \
 network/multiteamselect.o \
 network/multiui.o \
 network/multiutil.o \
 network/multi_campaign.o \
 network/multi_data.o \
 network/multi_dogfight.o \
 network/multi_endgame.o \
 network/multi_ingame.o \
 network/multi_kick.o \
 network/multi_log.o \
 network/multi_obj.o \
 network/multi_observer.o \
 network/multi_options.o \
 network/multi_pause.o \
 network/multi_pinfo.o \
 network/multi_ping.o \
 network/multi_pmsg.o \
 network/multi_rate.o \
 network/multi_respawn.o \
 network/multi_team.o \
 network/multi_voice.o \
 network/multi_xfer.o \
 network/psnet.o \
 network/psnet2.o \
 menuui/optionsmenumulti.o \
 missionui/chatbox.o
#
# Network/stand_gui.o
#
else
NETWORK_DEF=-DNO_NETWORK -DNO_STANDALONE
NETWORK_OBJS=
endif # USE_NETWORK


ifeq ($(USE_OPENGL), 0)
OPENGL_DEF=
else
OPENGL_DEF=-DUSE_OPENGL
endif # USE_OPENGL


LDFLAGS = -g
LDLIBS = -L/usr/X11R6/lib -lSDL -lGL -lGLU -lpthread

CXXFLAGS = \
 -g \
 -I/usr/local/include/SDL \
 -I. \
 -fno-exceptions \
 -D_DEBUG -DNO_CD_CHECK -UNDEBUG \
 -DNO_DIRECT3D $(SOUND_DEF) $(JOYSTICK_DEF) $(NETWORK_DEF) $(OPENGL_DEF)

ANIM_OBJFILES = \
 anim/animplay.o \
 anim/packunpack.o

ASTEROID_OBJFILES = \
 asteroid/asteroid.o

BMPMAN_OBJFILES = \
 bmpman/bmpman.o

CFILE_OBJFILES = \
 cfile/cfile.o \
 cfile/cfilearchive.o \
 cfile/cfilelist.o \
 cfile/cfilesystem.o

CMDLINE_OBJFILES = \
 cmdline/cmdline.o

CMEASURE_OBJFILES = \
 cmeasure/cmeasure.o

CONTROLCONFIG_OBJFILES = \
 controlconfig/controlsconfig.o \
 controlconfig/controlsconfigcommon.o

CUTSCENE_OBJFILES = \
 cutscene/cutscenes.o

DEBRIS_OBJFILES = \
 debris/debris.o

DEBUGCONSOLE_OBJFILES = \
 debugconsole/console.o

DEMO_OBJFILES = \
 demo/demo.o

EXCEPTIONHANDLER_OBJFILES = \
 exceptionhandler/exceptionhandler.o

FIREBALL_OBJFILES = \
 fireball/fireballs.o \
 fireball/warpineffect.o

GAMEHELP_OBJFILES = \
 gamehelp/contexthelp.o \
 gamehelp/gameplayhelp.o

GAMESEQUENCE_OBJFILES = \
 gamesequence/gamesequence.o

# gamesnd/eventmusic.o
# gamesnd/gamesnd.o
GAMESND_OBJFILES =

# glide/glide.o
GLIDE_OBJFILES = 

# globalincs/windebug.o
GLOBALINCS_OBJFILES = \
 globalincs/alphacolors.o \
 globalincs/crypt.o \
 globalincs/systemvars.o \
 globalincs/version.o

# graphics/grd3d.o
# graphics/grd3drender.o
# graphics/grd3dtexture.o
# graphics/grdirectdraw.o
# graphics/grglide.o
# graphics/grglidetexture.o
# graphics/grsoft.o 
# graphics/scaler.o 
# graphics/tmapgenericscans.o 
# graphics/tmapscantiled128x128.o 
# graphics/tmapscantiled16x16.o 
# graphics/tmapscantiled256x256.o 
# graphics/tmapscantiled32x32.o 
# graphics/tmapscantiled64x64.o
# graphics/tmapscanline.o
# graphics/tmapper.o
GRAPHICS_OBJFILES = \
 graphics/2d.o \
 graphics/aaline.o \
 graphics/bitblt.o \
 graphics/circle.o \
 graphics/colors.o \
 graphics/font.o \
 graphics/gradient.o \
 graphics/gropengl.o \
 graphics/grzbuffer.o \
 graphics/line.o \
 graphics/pixel.o \
 graphics/rect.o \
 graphics/shade.o

# hud/hudobserver.o 
HUD_OBJFILES = \
 hud/hud.o \
 hud/hudartillery.o \
 hud/hudbrackets.o \
 hud/hudconfig.o \
 hud/hudescort.o \
 hud/hudets.o \
 hud/hudlock.o \
 hud/hudmessage.o \
 hud/hudreticle.o \
 hud/hudshield.o \
 hud/hudsquadmsg.o \
 hud/hudtarget.o \
 hud/hudtargetbox.o \
 hud/hudwingmanstatus.o

INETFILE_OBJFILES = \
 inetfile/cftp.o \
 inetfile/chttpget.o \
 inetfile/inetgetfile.o

# io/joy.o
# io/joy_ff.o
# io/swff_lib.o
IO_OBJFILES = \
 io/key.o \
 io/keycontrol.o \
 io/mouse.o \
 io/timer.o \
 io/xmouse.o

JUMPNODE_OBJFILES = \
 jumpnode/jumpnode.o

LIGHTING_OBJFILES = \
 lighting/lighting.o

LOCALIZATION_OBJFILES = \
 localization/fhash.o \
 localization/localize.o

MATH_OBJFILES = \
 math/fix.o \
 math/floating.o \
 math/fvi.o \
 math/spline.o \
 math/staticrand.o \
 math/vecmat.o

# menuui/optionsmenumulti.o 
MENUUI_OBJFILES = \
 menuui/barracks.o \
 menuui/credits.o \
 menuui/fishtank.o \
 menuui/mainhallmenu.o \
 menuui/mainhalltemp.o \
 menuui/optionsmenu.o \
 menuui/playermenu.o \
 menuui/readyroom.o \
 menuui/snazzyui.o \
 menuui/techmenu.o \
 menuui/trainingmenu.o

MISSION_OBJFILES = \
 mission/missionbriefcommon.o \
 mission/missioncampaign.o \
 mission/missiongoals.o \
 mission/missiongrid.o \
 mission/missionhotkey.o \
 mission/missionload.o \
 mission/missionlog.o \
 mission/missionmessage.o \
 mission/missionparse.o \
 mission/missiontraining.o

# missionui/chatbox.o
MISSIONUI_OBJFILES = \
 missionui/missionbrief.o \
 missionui/missioncmdbrief.o \
 missionui/missiondebrief.o \
 missionui/missionloopbrief.o \
 missionui/missionpause.o \
 missionui/missionrecommend.o \
 missionui/missionscreencommon.o \
 missionui/missionshipchoice.o \
 missionui/missionstats.o \
 missionui/missionweaponchoice.o \
 missionui/redalert.o

MODEL_OBJFILES = \
 model/modelcollide.o \
 model/modelinterp.o \
 model/modeloctant.o \
 model/modelread.o

NEBULA_OBJFILES = \
 nebula/neb.o \
 nebula/neblightning.o

# network/multi.o
# network/multi_campaign.o
# network/multi_data.o
# network/multi_dogfight.o
# network/multi_endgame.o
# network/multi_ingame.o
# network/multi_kick.o
# network/multi_log.o
# network/multimsgs.o
# network/multi_obj.o
# network/multi_observer.o
# network/multi_oo.o
# network/multi_options.o
# network/multi_pause.o
# network/multi_pinfo.o
# network/multi_ping.o
# network/multi_pmsg.o
# network/multi_rate.o
# network/multi_respawn.o
# network/multi_team.o
# network/multi_update.o
# network/multi_voice.o
# network/multi_xfer.o
# network/multilag.o
# network/multiteamselect.o
# network/multiui.o
# network/psnet.o
# network/psnet2.o
# network/stand_gui.o
NETWORK_OBJFILES = \
 network/multiutil.o

# object/objectsnd.o 
OBJECT_OBJFILES = \
 object/collidedebrisship.o \
 object/collidedebrisweapon.o \
 object/collideshipship.o \
 object/collideshipweapon.o \
 object/collideweaponweapon.o \
 object/objcollide.o \
 object/object.o \
 object/objectsort.o

OBSERVER_OBJFILES = \
 observer/observer.o

OSAPI_OBJFILES = \
 osapi/osapi_unix.o \
 osapi/osregistry_unix.o \
 osapi/outwnd_unix.o

PALMAN_OBJFILES = \
 palman/palman.o

PARSE_OBJFILES = \
 parse/encrypt.o \
 parse/parselo.o \
 parse/sexp.o

PARTICLE_OBJFILES = \
 particle/particle.o

PCXUTILS_OBJFILES = \
 pcxutils/pcxutils.o

PHYSICS_OBJFILES = \
 physics/physics.o

PLAYERMAN_OBJFILES = \
 playerman/managepilot.o \
 playerman/playercontrol.o

POPUP_OBJFILES = \
 popup/popup.o \
 popup/popupdead.o

RADAR_OBJFILES = \
 radar/radar.o

RENDER_OBJFILES = \
 render/3dclipper.o \
 render/3ddraw.o \
 render/3dlaser.o \
 render/3dmath.o \
 render/3dsetup.o

SHIP_OBJFILES = \
 ship/afterburner.o \
 ship/ai.o \
 ship/aibig.o \
 ship/aicode.o \
 ship/aigoals.o \
 ship/awacs.o \
 ship/shield.o \
 ship/ship.o \
 ship/shipcontrails.o \
 ship/shipfx.o \
 ship/shiphit.o

SOUND_OBJFILES = \
 sound/acm.o \
 sound/audiostr.o \
 sound/ds.o \
 sound/ds3d.o \
 sound/dscap.o \
 sound/midifile.o \
 sound/rbaudio.o \
 sound/rtvoice.o \
 sound/sound.o \
 sound/winmidi.o \
 sound/winmidi_base.o

STARFIELD_OBJFILES = \
 starfield/nebula.o \
 starfield/starfield.o \
 starfield/supernova.o

STATS_OBJFILES = \
 stats/medals.o \
 stats/scoring.o \
 stats/stats.o

TGAUTILS_OBJFILES = \
 tgautils/tgautils.o

UI_OBJFILES = \
 ui/button.o \
 ui/checkbox.o \
 ui/gadget.o \
 ui/icon.o \
 ui/inputbox.o \
 ui/keytrap.o \
 ui/listbox.o \
 ui/radio.o \
 ui/scroll.o \
 ui/slider.o \
 ui/slider2.o \
 ui/uidraw.o \
 ui/uimouse.o \
 ui/window.o

VCODEC_OBJFILES = \
 vcodec/codec1.o

WEAPON_OBJFILES = \
 weapon/beam.o \
 weapon/corkscrew.o \
 weapon/emp.o \
 weapon/flak.o \
 weapon/muzzleflash.o \
 weapon/shockwave.o \
 weapon/swarm.o \
 weapon/trails.o \
 weapon/weapons.o

# SOUND_OBJFILES  
# INETFILE_OBJFILES  
# EXCEPTIONHANDLER_OBJFILES 
OBJFILES = \
 ANIM_OBJFILES  \
 ASTEROID_OBJFILES  \
 BMPMAN_OBJFILES  \
 CFILE_OBJFILES \
 CMDLINE_OBJFILES  \
 CMEASURE_OBJFILES  \
 CONTROLCONFIG_OBJFILES  \
 CUTSCENE_OBJFILES  \
 DEBRIS_OBJFILES  \
 DEBUGCONSOLE_OBJFILES  \
 FIREBALL_OBJFILES  \
 GAMEHELP_OBJFILES  \
 GAMESEQUENCE_OBJFILES  \
 GAMESND_OBJFILES  \
 GLIDE_OBJFILES  \
 GLOBALINCS_OBJFILES  \
 GRAPHICS_OBJFILES  \
 HUD_OBJFILES  \
 IO_OBJFILES \
 JUMPNODE_OBJFILES  \
 LIGHTING_OBJFILES  \
 MATH_OBJFILES  \
 MENUUI_OBJFILES  \
 MISSION_OBJFILES  \
 MISSIONUI_OBJFILES  \
 MODEL_OBJFILES  \
 NETWORK_OBJFILES \
 OBJECT_OBJFILES  \
 OBSERVER_OBJFILES  \
 OSAPI_OBJFILES \
 PALMAN_OBJFILES  \
 PARSE_OBJFILES  \
 PARTICLE_OBJFILES  \
 PCXUTILS_OBJFILES  \
 PHYSICS_OBJFILES  \
 PLAYERMAN_OBJFILES  \
 POPUP_OBJFILES  \
 RADAR_OBJFILES  \
 RENDER_OBJFILES  \
 SHIP_OBJFILES  \
 STARFIELD_OBJFILES  \
 STATS_OBJFILES  \
 UI_OBJFILES  \
 VCODEC_OBJFILES  \
 WEAPON_OBJFILES  \
 NEBULA_OBJFILES  \
 LOCALIZATION_OBJFILES  \
 TGAUTILS_OBJFILES  \
 DEMO_OBJFILES

# SOUND_OBJFILES  
# INETFILE_OBJFILES  
# EXCEPTIONHANDLER_OBJFILES 
LIB_OBJFILES = \
 $(ANIM_OBJFILES)  \
 $(ASTEROID_OBJFILES)  \
 $(BMPMAN_OBJFILES)  \
 $(CFILE_OBJFILES) \
 $(CMDLINE_OBJFILES)  \
 $(CMEASURE_OBJFILES)  \
 $(CONTROLCONFIG_OBJFILES)  \
 $(CUTSCENE_OBJFILES)  \
 $(DEBRIS_OBJFILES)  \
 $(DEBUGCONSOLE_OBJFILES)  \
 $(FIREBALL_OBJFILES)  \
 $(GAMEHELP_OBJFILES)  \
 $(GAMESEQUENCE_OBJFILES)  \
 $(GAMESND_OBJFILES)  \
 $(GLIDE_OBJFILES)  \
 $(GLOBALINCS_OBJFILES)  \
 $(GRAPHICS_OBJFILES)  \
 $(HUD_OBJFILES)  \
 $(IO_OBJFILES) \
 $(JUMPNODE_OBJFILES)  \
 $(LIGHTING_OBJFILES)  \
 $(MATH_OBJFILES)  \
 $(MENUUI_OBJFILES)  \
 $(MISSION_OBJFILES)  \
 $(MISSIONUI_OBJFILES)  \
 $(MODEL_OBJFILES)  \
 $(NETWORK_OBJFILES) \
 $(OBJECT_OBJFILES)  \
 $(OBSERVER_OBJFILES)  \
 $(OSAPI_OBJFILES) \
 $(PALMAN_OBJFILES)  \
 $(PARSE_OBJFILES)  \
 $(PARTICLE_OBJFILES)  \
 $(PCXUTILS_OBJFILES)  \
 $(PHYSICS_OBJFILES)  \
 $(PLAYERMAN_OBJFILES)  \
 $(POPUP_OBJFILES)  \
 $(RADAR_OBJFILES)  \
 $(RENDER_OBJFILES)  \
 $(SHIP_OBJFILES)  \
 $(STARFIELD_OBJFILES)  \
 $(STATS_OBJFILES)  \
 $(UI_OBJFILES)  \
 $(VCODEC_OBJFILES)  \
 $(WEAPON_OBJFILES)  \
 $(NEBULA_OBJFILES)  \
 $(LOCALIZATION_OBJFILES)  \
 $(TGAUTILS_OBJFILES)  \
 $(DEMO_OBJFILES) \
 $(SOUND_OBJS) \
 $(JOYSTICK_OBJS) \
 $(NETWORK_OBJS)

FREESPACE2_OBJFILES = \
 freespace2/freespace.o \
 freespace2/levelpaging.o

STUB_OBJFILES = \
 windows_stub/stubs.o

# ==============================
.PHONY: all clean code \
 ANIM_OBJFILES  \
 ASTEROID_OBJFILES  \
 BMPMAN_OBJFILES  \
 CMDLINE_OBJFILES  \
 CMEASURE_OBJFILES  \
 CONTROLCONFIG_OBJFILES  \
 CUTSCENE_OBJFILES  \
 DEBRIS_OBJFILES  \
 DEBUGCONSOLE_OBJFILES  \
 FIREBALL_OBJFILES  \
 GAMEHELP_OBJFILES  \
 GAMESEQUENCE_OBJFILES  \
 GAMESND_OBJFILES  \
 GLIDE_OBJFILES  \
 GLOBALINCS_OBJFILES  \
 GRAPHICS_OBJFILES  \
 HUD_OBJFILES  \
 IO_OBJFILES  \
 JUMPNODE_OBJFILES  \
 LIGHTING_OBJFILES  \
 MATH_OBJFILES  \
 MENUUI_OBJFILES  \
 MISSION_OBJFILES  \
 MISSIONUI_OBJFILES  \
 MODEL_OBJFILES  \
 OBJECT_OBJFILES  \
 OBSERVER_OBJFILES  \
 OSAPI_OBJFILES  \
 PALMAN_OBJFILES  \
 PARSE_OBJFILES  \
 PARTICLE_OBJFILES  \
 PCXUTILS_OBJFILES  \
 PHYSICS_OBJFILES  \
 PLAYERMAN_OBJFILES  \
 POPUP_OBJFILES  \
 RADAR_OBJFILES  \
 RENDER_OBJFILES  \
 SHIP_OBJFILES  \
 SOUND_OBJFILES  \
 STARFIELD_OBJFILES  \
 STATS_OBJFILES  \
 UI_OBJFILES  \
 VCODEC_OBJFILES  \
 WEAPON_OBJFILES  \
 NEBULA_OBJFILES  \
 LOCALIZATION_OBJFILES  \
 TGAUTILS_OBJFILES  \
 DEMO_OBJFILES  \
 INETFILE_OBJFILES  \
 EXCEPTIONHANDLER_OBJFILES  \
 NETWORK_OBJFILES

.cpp.o:	
	$(CXX) $(CXXFLAGS) -c -o $@ $<


all:	$(TARGET)

clean:
	rm -f $(TARGET) libcode.a core *.o *~
	find -name *.o -exec rm {} \;
	find -name *~ -exec rm {} \;

code:	libcode.a


# ==============================
libcode.a:	$(LIB_OBJFILES)
	$(AR) -r $@ $(LIB_OBJFILES)
#	$(CXX) -o $@ $(LIB_OBJFILES)

fs2:	libcode.a $(FREESPACE2_OBJFILES) $(STUB_OBJFILES)
	$(CXX) $(LDFLAGS) -o $@ $(FREESPACE2_OBJFILES) $(STUB_OBJFILES) $(LDLIBS) -L. -lcode


# exceptions to the rule...
freespace2/freespace.o:	freespace2/freespace.cpp
	$(CXX) $(CXXFLAGS) -fexceptions -c -o $@ $<

ANIM_OBJFILES:  $(ANIM_OBJFILES)
ASTEROID_OBJFILES:  $(ASTEROID_OBJFILES)
BMPMAN_OBJFILES:  $(BMPMAN_OBJFILES)
CMDLINE_OBJFILES:  $(CMDLINE_OBJFILES)
CMEASURE_OBJFILES:  $(CMEASURE_OBJFILES)
CONTROLCONFIG_OBJFILES:  $(CONTROLCONFIG_OBJFILES)
CUTSCENE_OBJFILES:  $(CUTSCENE_OBJFILES)
DEBRIS_OBJFILES:  $(DEBRIS_OBJFILES)
DEBUGCONSOLE_OBJFILES:  $(DEBUGCONSOLE_OBJFILES)
FIREBALL_OBJFILES:  $(FIREBALL_OBJFILES)
GAMEHELP_OBJFILES:  $(GAMEHELP_OBJFILES)
GAMESEQUENCE_OBJFILES:  $(GAMESEQUENCE_OBJFILES)
GAMESND_OBJFILES:  $(GAMESND_OBJFILES)
GLIDE_OBJFILES:  $(GLIDE_OBJFILES)
GLOBALINCS_OBJFILES:  $(GLOBALINCS_OBJFILES)
GRAPHICS_OBJFILES:  $(GRAPHICS_OBJFILES)
HUD_OBJFILES:  $(HUD_OBJFILES)
IO_OBJFILES:  $(IO_OBJFILES)
JUMPNODE_OBJFILES:  $(JUMPNODE_OBJFILES)
LIGHTING_OBJFILES:  $(LIGHTING_OBJFILES)
MATH_OBJFILES:  $(MATH_OBJFILES)
MENUUI_OBJFILES:  $(MENUUI_OBJFILES)
MISSION_OBJFILES:  $(MISSION_OBJFILES)
MISSIONUI_OBJFILES:  $(MISSIONUI_OBJFILES)
MODEL_OBJFILES:  $(MODEL_OBJFILES)
OBJECT_OBJFILES:  $(OBJECT_OBJFILES)
OBSERVER_OBJFILES:  $(OBSERVER_OBJFILES)
OSAPI_OBJFILES:  $(OSAPI_OBJFILES)
PALMAN_OBJFILES:  $(PALMAN_OBJFILES)
PARSE_OBJFILES:  $(PARSE_OBJFILES)
PARTICLE_OBJFILES:  $(PARTICLE_OBJFILES)
PCXUTILS_OBJFILES:  $(PCXUTILS_OBJFILES)
PHYSICS_OBJFILES:  $(PHYSICS_OBJFILES)
PLAYERMAN_OBJFILES:  $(PLAYERMAN_OBJFILES)
POPUP_OBJFILES:  $(POPUP_OBJFILES)
RADAR_OBJFILES:  $(RADAR_OBJFILES)
RENDER_OBJFILES:  $(RENDER_OBJFILES)
SHIP_OBJFILES:  $(SHIP_OBJFILES)
SOUND_OBJFILES:  $(SOUND_OBJFILES)
STARFIELD_OBJFILES:  $(STARFIELD_OBJFILES)
STATS_OBJFILES:  $(STATS_OBJFILES)
UI_OBJFILES:  $(UI_OBJFILES)
VCODEC_OBJFILES:  $(VCODEC_OBJFILES)
WEAPON_OBJFILES:  $(WEAPON_OBJFILES)
NEBULA_OBJFILES:  $(NEBULA_OBJFILES)
LOCALIZATION_OBJFILES:  $(LOCALIZATION_OBJFILES)
TGAUTILS_OBJFILES:  $(TGAUTILS_OBJFILES)
DEMO_OBJFILES:  $(DEMO_OBJFILES)
INETFILE_OBJFILES:  $(INETFILE_OBJFILES)
EXCEPTIONHANDLER_OBJFILES:  $(EXCEPTIONHANDLER_OBJFILES)
NETWORK_OBJFILES:  $(NETWORK_OBJFILES)
