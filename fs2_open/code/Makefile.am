# code/Makefile.am for fs2_open
#
# $Id: Makefile.am,v 1.1 2003-03-02 04:53:46 penguin Exp $

noinst_PROGRAMS = fs2

fs2_SOURCES = \
 anim/animplay.cpp \
 anim/packunpack.cpp \
 asteroid/asteroid.cpp \
 bmpman/bmpman.cpp \
 cfile/cfile.cpp \
 cfile/cfilearchive.cpp \
 cfile/cfilelist.cpp \
 cfile/cfilesystem.cpp \
 cmdline/cmdline.cpp \
 cmeasure/cmeasure.cpp \
 controlconfig/controlsconfig.cpp \
 controlconfig/controlsconfigcommon.cpp \
 cutscene/cutscenes.cpp \
 debris/debris.cpp \
 debugconsole/console.cpp \
 decals/decals.cpp \
 demo/demo.cpp \
 exceptionhandler/exceptionhandler.cpp \
 fireball/fireballs.cpp \
 fireball/warpineffect.cpp \
 freespace2/freespace.cpp \
 freespace2/levelpaging.cpp \
 gamehelp/contexthelp.cpp \
 gamehelp/gameplayhelp.cpp \
 gamesequence/gamesequence.cpp \
 globalincs/alphacolors.cpp \
 globalincs/crypt.cpp \
 globalincs/systemvars.cpp \
 globalincs/version.cpp \
 graphics/2d.cpp \
 graphics/aaline.cpp \
 graphics/bitblt.cpp \
 graphics/circle.cpp \
 graphics/colors.cpp \
 graphics/font.cpp \
 graphics/gradient.cpp \
 graphics/gropengl.cpp \
 graphics/grzbuffer.cpp \
 graphics/line.cpp \
 graphics/pixel.cpp \
 graphics/rect.cpp \
 graphics/scaler.cpp \
 graphics/shade.cpp \
 hud/hud.cpp \
 hud/hudartillery.cpp \
 hud/hudbrackets.cpp \
 hud/hudconfig.cpp \
 hud/hudescort.cpp \
 hud/hudets.cpp \
 hud/hudlock.cpp \
 hud/hudmessage.cpp \
 hud/hudreticle.cpp \
 hud/hudshield.cpp \
 hud/hudsquadmsg.cpp \
 hud/hudtarget.cpp \
 hud/hudtargetbox.cpp \
 hud/hudwingmanstatus.cpp \
 io/key.cpp \
 io/keycontrol.cpp \
 io/mouse.cpp \
 io/timer.cpp \
 jumpnode/jumpnode.cpp \
 lighting/lighting.cpp \
 localization/fhash.cpp \
 localization/localize.cpp \
 math/fix.cpp \
 math/floating.cpp \
 math/fvi.cpp \
 math/spline.cpp \
 math/staticrand.cpp \
 math/vecmat.cpp \
 menuui/barracks.cpp \
 menuui/credits.cpp \
 menuui/fishtank.cpp \
 menuui/mainhallmenu.cpp \
 menuui/mainhalltemp.cpp \
 menuui/optionsmenu.cpp \
 menuui/playermenu.cpp \
 menuui/readyroom.cpp \
 menuui/snazzyui.cpp \
 menuui/techmenu.cpp \
 menuui/trainingmenu.cpp \
 mission/missionbriefcommon.cpp \
 mission/missioncampaign.cpp \
 mission/missiongoals.cpp \
 mission/missiongrid.cpp \
 mission/missionhotkey.cpp \
 mission/missionload.cpp \
 mission/missionlog.cpp \
 mission/missionmessage.cpp \
 mission/missionparse.cpp \
 mission/missiontraining.cpp \
 missionui/missionbrief.cpp \
 missionui/missioncmdbrief.cpp \
 missionui/missiondebrief.cpp \
 missionui/missionloopbrief.cpp \
 missionui/missionpause.cpp \
 missionui/missionrecommend.cpp \
 missionui/missionscreencommon.cpp \
 missionui/missionshipchoice.cpp \
 missionui/missionstats.cpp \
 missionui/missionweaponchoice.cpp \
 missionui/redalert.cpp \
 model/modelcollide.cpp \
 model/modelinterp.cpp \
 model/modeloctant.cpp \
 model/modelread.cpp \
 nebula/neb.cpp \
 nebula/neblightning.cpp \
 object/collidedebrisship.cpp \
 object/collidedebrisweapon.cpp \
 object/collideshipship.cpp \
 object/collideshipweapon.cpp \
 object/collideweaponweapon.cpp \
 object/objcollide.cpp \
 object/object.cpp \
 object/objectsort.cpp \
 observer/observer.cpp \
 palman/palman.cpp \
 parse/encrypt.cpp \
 parse/parselo.cpp \
 parse/sexp.cpp \
 particle/particle.cpp \
 pcxutils/pcxutils.cpp \
 physics/physics.cpp \
 playerman/managepilot.cpp \
 playerman/playercontrol.cpp \
 popup/popup.cpp \
 popup/popupdead.cpp \
 radar/radar.cpp \
 render/3dclipper.cpp \
 render/3ddraw.cpp \
 render/3dlaser.cpp \
 render/3dmath.cpp \
 render/3dsetup.cpp \
 ship/afterburner.cpp \
 ship/ai.cpp \
 ship/aibig.cpp \
 ship/aicode.cpp \
 ship/aigoals.cpp \
 ship/awacs.cpp \
 ship/shield.cpp \
 ship/ship.cpp \
 ship/shipcontrails.cpp \
 ship/shipfx.cpp \
 ship/shiphit.cpp \
 starfield/nebula.cpp \
 starfield/starfield.cpp \
 starfield/supernova.cpp \
 stats/medals.cpp \
 stats/scoring.cpp \
 stats/stats.cpp \
 tgautils/tgautils.cpp \
 ui/button.cpp \
 ui/checkbox.cpp \
 ui/gadget.cpp \
 ui/icon.cpp \
 ui/inputbox.cpp \
 ui/keytrap.cpp \
 ui/listbox.cpp \
 ui/radio.cpp \
 ui/scroll.cpp \
 ui/slider.cpp \
 ui/slider2.cpp \
 ui/uidraw.cpp \
 ui/uimouse.cpp \
 ui/window.cpp \
 weapon/beam.cpp \
 weapon/corkscrew.cpp \
 weapon/emp.cpp \
 weapon/flak.cpp \
 weapon/muzzleflash.cpp \
 weapon/shockwave.cpp \
 weapon/swarm.cpp \
 weapon/trails.cpp \
 weapon/weapons.cpp


## conditional includes follow:

## joystic support sources
if FS2_JOYSTICK
fs2_SOURCES += \
 io/joy.cpp \
 io/joy_ff.cpp \
 io/swff_lib.cpp
endif # FS2_JOYSTICK

## networking support sources
if FS2_NETWORKING
fs2_SOURCES += \
 hud/hudobserver.cpp \
 network/multi.cpp \
 network/multimsgs.cpp \
 network/multiteamselect.cpp \
 network/multiui.cpp \
 network/multiutil.cpp \
 network/multi_campaign.cpp \
 network/multi_data.cpp \
 network/multi_dogfight.cpp \
 network/multi_endgame.cpp \
 network/multi_ingame.cpp \
 network/multi_kick.cpp \
 network/multi_log.cpp \
 network/multi_obj.cpp \
 network/multi_observer.cpp \
 network/multi_options.cpp \
 network/multi_pause.cpp \
 network/multi_pinfo.cpp \
 network/multi_ping.cpp \
 network/multi_pmsg.cpp \
 network/multi_rate.cpp \
 network/multi_respawn.cpp \
 network/multi_team.cpp \
 network/multi_voice.cpp \
 network/multi_xfer.cpp \
 network/psnet.cpp \
 network/psnet2.cpp \
 menuui/optionsmenumulti.cpp \
 missionui/chatbox.cpp
endif # FS2_NETWORKING

## sound support sources
if FS2_SOUND
fs2_SOURCES += \
 gamesnd/eventmusic.cpp \
 gamesnd/gamesnd.cpp \
 object/objectsnd.cpp \
 sound/acm.cpp \
 sound/audiostr.cpp \
 sound/ds.cpp \
 sound/ds3d.cpp \
 sound/dscap.cpp \
 sound/midifile.cpp \
 sound/rbaudio.cpp \
 sound/rtvoice.cpp \
 sound/sound.cpp \
 sound/winmidi.cpp \
 sound/winmidi_base.cpp \
 vcodec/codec1.cpp
endif # FS2_SOUND

## software-rendering support sources
if FS2_GRSOFT
fs2_SOURCES += \
 graphics/grsoft.cpp \
 graphics/tmapper.cpp
endif # FS2_GRSOFT

## Win32-specific sources
if FS2_OS_WIN32
fs2_SOURCES += \
 ddsutils/ddsutils.cpp \
 globalincs/windebug.cpp \
 glide/glide.cpp \
 graphics/grd3d.cpp \
 graphics/grd3drender.cpp \
 graphics/grd3dtexture.cpp \
 graphics/grdirectdraw.cpp \
 graphics/grglide.cpp \
 graphics/grglidetexture.cpp \
 network/stand_gui.cpp \
 osapi/osapi.cpp \
 osapi/osregistry.cpp \
 osapi/outwnd.cpp
endif # FS2_OS_WIN32

## Unix-specific sources
if FS2_OS_UNIX
fs2_SOURCES += \
 io/xmouse.cpp \
 osapi/osapi_unix.cpp \
 osapi/osregistry_unix.cpp \
 osapi/outwnd_unix.cpp
endif # FS2_OS_UNIX


## compiler flags
INCLUDES = -I.
fs2_CXXFLAGS = -fno-exceptions

fs2_LDADD = 

if FS2_OS_WIN32
## Win32 libraries needed
fs2_LDADD += \
 -lgdi32 -lole32 -lwsock32 -lwinmm -lmsacm32 -ldxguid -lopengl32 \
 -lcomctl32 -lddraw -ldinput
endif


if FS2_OS_UNIX
## Unix libraries needed
## -- TODO --
endif
