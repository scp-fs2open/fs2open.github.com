## code/Makefile.am for fs2_open
##
## $Id: Makefile.am,v 1.9 2005-03-27 06:07:00 taylor Exp $

noinst_LIBRARIES = libcode.a
noinst_PROGRAMS = @FS2_BINARY@

EXTRA_PROGRAMS = fs2_open_r fs2_open_d

###############################################################################
##
## Begin CORE library
##

FS2_SOURCES =	\
	ai/ai.cpp	\
	ai/aibig.cpp	\
	ai/aicode.cpp	\
	ai/aigoals.cpp	\
	ai/aiturret.cpp	\
	anim/animplay.cpp	\
	anim/packunpack.cpp	\
	asteroid/asteroid.cpp	\
	autopilot/autopilot.cpp	\
	bmpman/bmpman.cpp	\
	camera/camera.cpp	\
	cfile/cfile.cpp	\
	cfile/cfilearchive.cpp	\
	cfile/cfilelist.cpp	\
	cfile/cfilesystem.cpp	\
	cmdline/cmdline.cpp	\
	cmeasure/cmeasure.cpp	\
	controlconfig/controlsconfig.cpp	\
	controlconfig/controlsconfigcommon.cpp	\
	cutscene/cutscenes.cpp	\
	cutscene/movie.cpp	\
	ddsutils/ddsutils.cpp	\
	debris/debris.cpp	\
	debugconsole/console.cpp	\
	debugconsole/timerbar.cpp	\
	decals/decals.cpp	\
	demo/demo.cpp	\
	exceptionhandler/exceptionhandler.cpp	\
	fireball/fireballs.cpp	\
	fireball/warpineffect.cpp	\
	fs2open_pxo/Client.cpp	\
	fs2open_pxo/TCP_Client.cpp	\
	fs2open_pxo/TCP_Socket.cpp	\
	fs2open_pxo/udpsocket.cpp	\
	gamehelp/contexthelp.cpp	\
	gamehelp/gameplayhelp.cpp	\
	gamesequence/gamesequence.cpp	\
	globalincs/alphacolors.cpp	\
	globalincs/crypt.cpp	\
	globalincs/systemvars.cpp	\
	globalincs/version.cpp	\
	graphics/2d.cpp	\
	graphics/font.cpp	\
	graphics/grbatch.cpp	\
	graphics/gropengl.cpp	\
	graphics/gropenglbmpman.cpp	\
	graphics/gropenglextension.cpp	\
	graphics/gropengllight.cpp	\
	graphics/gropengltexture.cpp	\
	graphics/gropengltnl.cpp	\
	graphics/grzbuffer.cpp	\
	hud/hud.cpp	\
	hud/hudartillery.cpp	\
	hud/hudbrackets.cpp	\
	hud/hudconfig.cpp	\
	hud/hudescort.cpp	\
	hud/hudets.cpp	\
	hud/hudlock.cpp	\
	hud/hudmessage.cpp	\
	hud/hudnavigation.cpp	\
	hud/hudobserver.cpp	\
	hud/hudparse.cpp	\
	hud/hudreticle.cpp	\
	hud/hudshield.cpp	\
	hud/hudsquadmsg.cpp	\
	hud/hudtarget.cpp	\
	hud/hudtargetbox.cpp	\
	hud/hudwingmanstatus.cpp	\
	io/key.cpp	\
	io/keycontrol.cpp	\
	io/mouse.cpp	\
	io/timer.cpp	\
	irc/irc.cpp	\
	jpgutils/jpgutils.cpp	\
	jumpnode/jumpnode.cpp	\
	lab/lab.cpp	\
	lab/wmcgui.cpp	\
	lighting/lighting.cpp	\
	localization/fhash.cpp	\
	localization/localize.cpp	\
	math/fix.cpp	\
	math/floating.cpp	\
	math/fvi.cpp	\
	math/spline.cpp	\
	math/staticrand.cpp	\
	math/vecmat.cpp	\
	menuui/barracks.cpp	\
	menuui/credits.cpp	\
	menuui/fishtank.cpp	\
	menuui/mainhallmenu.cpp	\
	menuui/mainhalltemp.cpp	\
	menuui/optionsmenu.cpp	\
	menuui/optionsmenumulti.cpp	\
	menuui/playermenu.cpp	\
	menuui/readyroom.cpp	\
	menuui/snazzyui.cpp	\
	menuui/storybook.cpp	\
	menuui/techmenu.cpp	\
	menuui/trainingmenu.cpp	\
	mission/missionbriefcommon.cpp	\
	mission/missioncampaign.cpp	\
	mission/missiongoals.cpp	\
	mission/missiongrid.cpp	\
	mission/missionhotkey.cpp	\
	mission/missionload.cpp	\
	mission/missionlog.cpp	\
	mission/missionmessage.cpp	\
	mission/missionparse.cpp	\
	mission/missiontraining.cpp	\
	missionui/chatbox.cpp	\
	missionui/missionbrief.cpp	\
	missionui/missioncmdbrief.cpp	\
	missionui/missiondebrief.cpp	\
	missionui/missionloopbrief.cpp	\
	missionui/missionpause.cpp	\
	missionui/missionrecommend.cpp	\
	missionui/missionscreencommon.cpp	\
	missionui/missionshipchoice.cpp	\
	missionui/missionstats.cpp	\
	missionui/missionweaponchoice.cpp	\
	missionui/redalert.cpp	\
	model/modelcollide.cpp	\
	model/modelinterp.cpp	\
	model/modeloctant.cpp	\
	model/modelread.cpp	\
	nebula/neb.cpp	\
	nebula/neblightning.cpp	\
	network/fs2ox.cpp	\
	network/multi.cpp	\
	network/multimsgs.cpp	\
	network/multiteamselect.cpp	\
	network/multiui.cpp	\
	network/multiutil.cpp	\
	network/multi_campaign.cpp	\
	network/multi_data.cpp	\
	network/multi_dogfight.cpp	\
	network/multi_endgame.cpp	\
	network/multi_ingame.cpp	\
	network/multi_kick.cpp	\
	network/multi_log.cpp	\
	network/multi_obj.cpp	\
	network/multi_observer.cpp	\
	network/multi_options.cpp	\
	network/multi_pause.cpp	\
	network/multi_pinfo.cpp	\
	network/multi_ping.cpp	\
	network/multi_pmsg.cpp	\
	network/multi_rate.cpp	\
	network/multi_respawn.cpp	\
	network/multi_team.cpp	\
	network/multi_voice.cpp	\
	network/multi_xfer.cpp	\
	network/psnet.cpp	\
	network/psnet2.cpp	\
	object/collidedebrisship.cpp	\
	object/collidedebrisweapon.cpp	\
	object/collideshipship.cpp	\
	object/collideshipweapon.cpp	\
	object/collideweaponweapon.cpp	\
	object/objcollide.cpp	\
	object/objectdock.cpp	\
	object/object.cpp	\
	object/objectsort.cpp	\
	observer/observer.cpp	\
	palman/palman.cpp	\
	parse/encrypt.cpp	\
	parse/parselo.cpp	\
	parse/sexp.cpp	\
	particle/particle.cpp	\
	pcxutils/pcxutils.cpp	\
	physics/physics.cpp	\
	playerman/managepilot.cpp	\
	playerman/playercontrol.cpp	\
	popup/popup.cpp	\
	popup/popupdead.cpp	\
	radar/radar.cpp	\
	radar/radarorb.cpp \
	radar/radarsetup.cpp \
	render/3dclipper.cpp	\
	render/3ddraw.cpp	\
	render/3dlaser.cpp	\
	render/3dmath.cpp	\
	render/3dsetup.cpp	\
	ship/afterburner.cpp	\
	ship/awacs.cpp	\
	ship/shield.cpp	\
	ship/ship.cpp	\
	ship/shipcontrails.cpp	\
	ship/shipfx.cpp	\
	ship/shiphit.cpp	\
	starfield/nebula.cpp	\
	starfield/starfield.cpp	\
	starfield/supernova.cpp	\
	stats/medals.cpp	\
	stats/scoring.cpp	\
	stats/stats.cpp	\
	tgautils/tgautils.cpp	\
	ui/button.cpp	\
	ui/checkbox.cpp	\
	ui/gadget.cpp	\
	ui/icon.cpp	\
	ui/inputbox.cpp	\
	ui/keytrap.cpp	\
	ui/listbox.cpp	\
	ui/radio.cpp	\
	ui/scroll.cpp	\
	ui/slider.cpp	\
	ui/slider2.cpp	\
	ui/uidraw.cpp	\
	ui/uimouse.cpp	\
	ui/window.cpp	\
	weapon/beam.cpp	\
	weapon/corkscrew.cpp	\
	weapon/emp.cpp	\
	weapon/flak.cpp	\
	weapon/muzzleflash.cpp	\
	weapon/shockwave.cpp	\
	weapon/swarm.cpp	\
	weapon/trails.cpp	\
	weapon/weapons.cpp	\
	species_defs/species_defs.cpp 

## conditional includes follow:

## joystic support sources
if FS2_JOYSTICK

if FS2_OS_WIN32
JOY_SOURCES =	\
	io/joy.cpp	\
	io/joy_ff.cpp	\
	io/swff_lib.cpp
endif

if FS2_OS_UNIX
JOY_SOURCES =	\
	io/joy-unix.cpp
endif

endif

## networking support sources
if FS2_OS_WIN32
FS2_SOURCES +=	\
	network/stand_gui.cpp
endif

if FS2_OS_UNIX
FS2_SOURCES +=	\
	network/stand_gui-unix.cpp
endif

## sound support sources
if FS2_SOUND
SOUND_SOURCES =	\
	gamesnd/eventmusic.cpp	\
	gamesnd/gamesnd.cpp	\
	object/objectsnd.cpp	\
	sound/ogg/ogg.cpp	\
	sound/ds.cpp	\
	sound/midifile.cpp	\
	sound/rbaudio.cpp	\
	sound/rtvoice.cpp	\
	sound/sound.cpp	\
	sound/winmidi.cpp	\
	sound/winmidi_base.cpp	\
	vcodec/codec1.cpp	\
	sound/fsspeech.cpp	\
	sound/speech.cpp

if FS2_OS_WIN32
SOUND_SOURCES +=	\
	sound/dscap.cpp	\
	sound/ds3d.cpp	\
	sound/audiostr.cpp	\
	sound/acm.cpp
endif

if FS2_OS_UNIX
SOUND_SOURCES +=	\
	sound/audiostr-openal.cpp	\
	sound/acm-unix.cpp
endif

endif

## Win32-specific sources
if FS2_OS_WIN32
WIN32_SOURCES =	\
	directx/dx8show.cpp	\
	globalincs/windebug.cpp	\
	glide/glide.cpp	\
	graphics/grd3d.cpp	\
	graphics/grd3dbatch.cpp	\
	graphics/grd3drender.cpp	\
	graphics/grd3dtexture.cpp	\
	graphics/grdirectdraw.cpp	\
	graphics/grglide.cpp	\
	graphics/grglidetexture.cpp	\
	osapi/osapi.cpp	\
	osapi/osregistry.cpp	\
	osapi/outwnd.cpp
endif

## Unix-specific sources
if FS2_OS_UNIX
UNIX_SOURCES =	\
	io/xmouse.cpp	\
	osapi/osapi_unix.cpp	\
	osapi/osregistry_unix.cpp	\
	osapi/outwnd_unix.cpp	\
	windows_stub/stubs.cpp
endif


## libcode.a
libcode_a_SOURCES = ${FS2_SOURCES} ${JOY_SOURCES} ${SOUND_SOURCES}	\
	${WIN32_SOURCES} ${UNIX_SOURCES}

##
## End CODE library
##
###############################################################################

###############################################################################
##
## Begin MAIN fs2 binary
##

if FS2_OS_OSX
PLATFORM_SOURCES =	\
	mac_osx/SDLMain.m
endif

fs2_open_d_SOURCES =	\
	freespace2/freespace.cpp freespace2/levelpaging.cpp	\
	${PLATFORM_SOURCES}

fs2_open_r_SOURCES =	\
	freespace2/freespace.cpp freespace2/levelpaging.cpp	\
	${PLATFORM_SOURCES}

fs2_open_d_LDADD = libcode.a ../libjpeg/libjpeg.a
fs2_open_r_LDADD = libcode.a ../libjpeg/libjpeg.a

##
## End MAIN library
##
###############################################################################
