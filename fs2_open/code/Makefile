TARGET  = fs2

ifeq ($(USE_SOUND), 1)
SOUND_DEF=
SOUND_OBJS= \
 Gamesnd/eventmusic.o \
 Gamesnd/gamesnd.o \
 Object/objectsnd.o \
 Sound/acm.o \
 Sound/audiostr.o \
 Sound/ds.o \
 Sound/ds3d.o \
 Sound/dscap.o \
 Sound/midifile.o \
 Sound/rbaudio.o \
 Sound/rtvoice.o \
 Sound/sound.o \
 Sound/winmidi.o \
 Sound/winmidi_base.o \
 VCodec/codec1.o
else
SOUND_DEF=-DNO_SOUND
SOUND_OBJS=
endif # USE_SOUND


ifeq ($(USE_JOYSTICK), 1)
JOYSTICK_DEF=
JOYSTICK_OBJS= \
 Io/joy.o \
 Io/joy_ff.o \
 Io/swff_lib.o
else
JOYSTICK_DEF=-DNO_JOYSTICK
JOYSTICK_OBJS=
endif # USE_JOYSTICK


ifeq ($(USE_NETWORK), 1)
# no support for standalone server yet... (UI is Win32 specific)
NETWORK_DEF=-DNO_STANDALONE
NETWORK_OBJS= \
 Hud/hudobserver.o \
 Network/multi.o \
 Network/multimsgs.o \
 Network/multiteamselect.o \
 Network/multiui.o \
 Network/multiutil.o \
 Network/multi_campaign.o \
 Network/multi_data.o \
 Network/multi_dogfight.o \
 Network/multi_endgame.o \
 Network/multi_ingame.o \
 Network/multi_kick.o \
 Network/multi_log.o \
 Network/multi_obj.o \
 Network/multi_observer.o \
 Network/multi_options.o \
 Network/multi_pause.o \
 Network/multi_pinfo.o \
 Network/multi_ping.o \
 Network/multi_pmsg.o \
 Network/multi_rate.o \
 Network/multi_respawn.o \
 Network/multi_team.o \
 Network/multi_voice.o \
 Network/multi_xfer.o \
 Network/psnet.o \
 Network/psnet2.o \
 MenuUI/optionsmenumulti.o \
 MissionUI/chatbox.o
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
 -Iwindows_stub \
 -IAnim -IAsteroid -IBmpman -ICFile -ICmdline -ICMeasure -IControlConfig \
 -ICutscene -IDebris -IDebugConsole -IDirectX -IFireball -IGameHelp -IFREESPACE2 \
 -IGameSequence -IGamesnd -IGlide -IGlobalIncs -IGraphics -IHud -IIo -IJumpNode \
 -ILighting -IMath -IMenuUI -IMission -IMissionUI -IModel -INetwork -IObject \
 -IObserver -IOsApi -IPalman -IParse -IParticle -IPcxUtils -IPhysics -IPlayerman \
 -IPopup -IRadar -IRender -IShip -ISound -IStarfield -IStats -ITgaUtils -IUI -IVCodec \
 -fno-exceptions \
 -IWeapon -ILocalization -INebula -IDemo -IInetfile -IExceptionHandler \
 -D_DEBUG -DNO_CD_CHECK -UNDEBUG \
 -DNO_DIRECT3D $(SOUND_DEF) $(JOYSTICK_DEF) $(NETWORK_DEF) $(OPENGL_DEF)

ANIM_OBJFILES = \
 Anim/animplay.o \
 Anim/packunpack.o

ASTEROID_OBJFILES = \
 Asteroid/asteroid.o

BMPMAN_OBJFILES = \
 Bmpman/bmpman.o

CFILE_OBJFILES = \
 CFile/cfile.o \
 CFile/cfilearchive.o \
 CFile/cfilelist.o \
 CFile/cfilesystem.o

CMDLINE_OBJFILES = \
 Cmdline/cmdline.o

CMEASURE_OBJFILES = \
 CMeasure/cmeasure.o

CONTROLCONFIG_OBJFILES = \
 ControlConfig/controlsconfig.o \
 ControlConfig/controlsconfigcommon.o

CUTSCENE_OBJFILES = \
 Cutscene/cutscenes.o

DEBRIS_OBJFILES = \
 Debris/debris.o

DEBUGCONSOLE_OBJFILES = \
 DebugConsole/console.o

DEMO_OBJFILES = \
 Demo/demo.o

EXCEPTIONHANDLER_OBJFILES = \
 ExceptionHandler/exceptionhandler.o

FIREBALL_OBJFILES = \
 Fireball/fireballs.o \
 Fireball/warpineffect.o

GAMEHELP_OBJFILES = \
 GameHelp/contexthelp.o \
 GameHelp/gameplayhelp.o

GAMESEQUENCE_OBJFILES = \
 GameSequence/gamesequence.o

# Gamesnd/eventmusic.o
# Gamesnd/gamesnd.o
GAMESND_OBJFILES =

GLIDE_OBJFILES = 
#GLIDE_OBJFILES = \
# Glide/glide.o

# GlobalIncs/windebug.o
GLOBALINCS_OBJFILES = \
 GlobalIncs/alphacolors.o \
 GlobalIncs/crypt.o \
 GlobalIncs/systemvars.o \
 GlobalIncs/version.o

# Graphics/grd3d.o
# Graphics/grd3drender.o
# Graphics/grd3dtexture.o
# Graphics/grdirectdraw.o
# Graphics/grglide.o
# Graphics/grglidetexture.o
# Graphics/grsoft.o 
# Graphics/scaler.o 
# Graphics/tmapgenericscans.o 
# Graphics/tmapscantiled128x128.o 
# Graphics/tmapscantiled16x16.o 
# Graphics/tmapscantiled256x256.o 
# Graphics/tmapscantiled32x32.o 
# Graphics/tmapscantiled64x64.o
# Graphics/tmapscanline.o
# Graphics/tmapper.o
GRAPHICS_OBJFILES = \
 Graphics/2d.o \
 Graphics/aaline.o \
 Graphics/bitblt.o \
 Graphics/circle.o \
 Graphics/colors.o \
 Graphics/font.o \
 Graphics/gradient.o \
 Graphics/gropengl.o \
 Graphics/grzbuffer.o \
 Graphics/line.o \
 Graphics/pixel.o \
 Graphics/rect.o \
 Graphics/shade.o

# Hud/hudobserver.o 
HUD_OBJFILES = \
 Hud/hud.o \
 Hud/hudartillery.o \
 Hud/hudbrackets.o \
 Hud/hudconfig.o \
 Hud/hudescort.o \
 Hud/hudets.o \
 Hud/hudlock.o \
 Hud/hudmessage.o \
 Hud/hudreticle.o \
 Hud/hudshield.o \
 Hud/hudsquadmsg.o \
 Hud/hudtarget.o \
 Hud/hudtargetbox.o \
 Hud/hudwingmanstatus.o

INETFILE_OBJFILES = \
 Inetfile/cftp.o \
 Inetfile/chttpget.o \
 Inetfile/inetgetfile.o

# Io/joy.o
# Io/joy_ff.o
# Io/swff_lib.o
IO_OBJFILES = \
 Io/key.o \
 Io/keycontrol.o \
 Io/mouse.o \
 Io/timer.o \
 Io/xmouse.o

JUMPNODE_OBJFILES = \
 JumpNode/jumpnode.o

LIGHTING_OBJFILES = \
 Lighting/lighting.o

LOCALIZATION_OBJFILES = \
 Localization/fhash.o \
 Localization/localize.o

MATH_OBJFILES = \
 Math/fix.o \
 Math/floating.o \
 Math/fvi.o \
 Math/spline.o \
 Math/staticrand.o \
 Math/vecmat.o

# MenuUI/optionsmenumulti.o 
MENUUI_OBJFILES = \
 MenuUI/barracks.o \
 MenuUI/credits.o \
 MenuUI/fishtank.o \
 MenuUI/mainhallmenu.o \
 MenuUI/mainhalltemp.o \
 MenuUI/optionsmenu.o \
 MenuUI/playermenu.o \
 MenuUI/readyroom.o \
 MenuUI/snazzyui.o \
 MenuUI/techmenu.o \
 MenuUI/trainingmenu.o

MISSION_OBJFILES = \
 Mission/missionbriefcommon.o \
 Mission/missioncampaign.o \
 Mission/missiongoals.o \
 Mission/missiongrid.o \
 Mission/missionhotkey.o \
 Mission/missionload.o \
 Mission/missionlog.o \
 Mission/missionmessage.o \
 Mission/missionparse.o \
 Mission/missiontraining.o

# MissionUI/chatbox.o
MISSIONUI_OBJFILES = \
 MissionUI/missionbrief.o \
 MissionUI/missioncmdbrief.o \
 MissionUI/missiondebrief.o \
 MissionUI/missionloopbrief.o \
 MissionUI/missionpause.o \
 MissionUI/missionrecommend.o \
 MissionUI/missionscreencommon.o \
 MissionUI/missionshipchoice.o \
 MissionUI/missionstats.o \
 MissionUI/missionweaponchoice.o \
 MissionUI/redalert.o

MODEL_OBJFILES = \
 Model/modelcollide.o \
 Model/modelinterp.o \
 Model/modeloctant.o \
 Model/modelread.o

NEBULA_OBJFILES = \
 Nebula/neb.o \
 Nebula/neblightning.o

# Network/multi.o
# Network/multi_campaign.o
# Network/multi_data.o
# Network/multi_dogfight.o
# Network/multi_endgame.o
# Network/multi_ingame.o
# Network/multi_kick.o
# Network/multi_log.o
# Network/multimsgs.o
# Network/multi_obj.o
# Network/multi_observer.o
# Network/multi_oo.o
# Network/multi_options.o
# Network/multi_pause.o
# Network/multi_pinfo.o
# Network/multi_ping.o
# Network/multi_pmsg.o
# Network/multi_rate.o
# Network/multi_respawn.o
# Network/multi_team.o
# Network/multi_update.o
# Network/multi_voice.o
# Network/multi_xfer.o
# Network/multilag.o
# Network/multiteamselect.o
# Network/multiui.o
# Network/psnet.o
# Network/psnet2.o
# Network/stand_gui.o
NETWORK_OBJFILES = \
 Network/multiutil.o

# Object/objectsnd.o 
OBJECT_OBJFILES = \
 Object/collidedebrisship.o \
 Object/collidedebrisweapon.o \
 Object/collideshipship.o \
 Object/collideshipweapon.o \
 Object/collideweaponweapon.o \
 Object/objcollide.o \
 Object/object.o \
 Object/objectsort.o

OBSERVER_OBJFILES = \
 Observer/observer.o

OSAPI_OBJFILES = \
 OsApi/osapi_unix.o \
 OsApi/osregistry_unix.o \
 OsApi/outwnd_unix.o

PALMAN_OBJFILES = \
 Palman/palman.o

PARSE_OBJFILES = \
 Parse/encrypt.o \
 Parse/parselo.o \
 Parse/sexp.o

PARTICLE_OBJFILES = \
 Particle/particle.o

PCXUTILS_OBJFILES = \
 PcxUtils/pcxutils.o

PHYSICS_OBJFILES = \
 Physics/physics.o

PLAYERMAN_OBJFILES = \
 Playerman/managepilot.o \
 Playerman/playercontrol.o

POPUP_OBJFILES = \
 Popup/popup.o \
 Popup/popupdead.o

RADAR_OBJFILES = \
 Radar/radar.o

RENDER_OBJFILES = \
 Render/3dclipper.o \
 Render/3ddraw.o \
 Render/3dlaser.o \
 Render/3dmath.o \
 Render/3dsetup.o

SHIP_OBJFILES = \
 Ship/afterburner.o \
 Ship/ai.o \
 Ship/aibig.o \
 Ship/aicode.o \
 Ship/aigoals.o \
 Ship/awacs.o \
 Ship/shield.o \
 Ship/ship.o \
 Ship/shipcontrails.o \
 Ship/shipfx.o \
 Ship/shiphit.o

SOUND_OBJFILES = \
 Sound/acm.o \
 Sound/audiostr.o \
 Sound/ds.o \
 Sound/ds3d.o \
 Sound/dscap.o \
 Sound/midifile.o \
 Sound/rbaudio.o \
 Sound/rtvoice.o \
 Sound/sound.o \
 Sound/winmidi.o \
 Sound/winmidi_base.o

STARFIELD_OBJFILES = \
 Starfield/nebula.o \
 Starfield/starfield.o \
 Starfield/supernova.o

STATS_OBJFILES = \
 Stats/medals.o \
 Stats/scoring.o \
 Stats/stats.o

TGAUTILS_OBJFILES = \
 TgaUtils/tgautils.o

UI_OBJFILES = \
 UI/button.o \
 UI/checkbox.o \
 UI/gadget.o \
 UI/icon.o \
 UI/inputbox.o \
 UI/keytrap.o \
 UI/listbox.o \
 UI/radio.o \
 UI/scroll.o \
 UI/slider.o \
 UI/slider2.o \
 UI/uidraw.o \
 UI/uimouse.o \
 UI/window.o

VCODEC_OBJFILES = \
 VCodec/codec1.o

WEAPON_OBJFILES = \
 Weapon/beam.o \
 Weapon/corkscrew.o \
 Weapon/emp.o \
 Weapon/flak.o \
 Weapon/muzzleflash.o \
 Weapon/shockwave.o \
 Weapon/swarm.o \
 Weapon/trails.o \
 Weapon/weapons.o

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
 FREESPACE2/freespace.o \
 FREESPACE2/levelpaging.o

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
FREESPACE2/freespace.o:	FREESPACE2/freespace.cpp
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
