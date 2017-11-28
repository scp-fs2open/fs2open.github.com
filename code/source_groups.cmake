# top-level files
set (file_root
	prefix_header.h
)

# AI files
set (file_root_ai
	ai/ai.cpp
	ai/ai.h
	ai/ai_flags.h
	ai/ai_profiles.cpp
	ai/ai_profiles.h
	ai/aibig.cpp
	ai/aibig.h
	ai/aicode.cpp
	ai/aigoals.cpp
	ai/aigoals.h
	ai/aiinternal.h
	ai/aiturret.cpp
)

# Anim files
set (file_root_anim
	anim/animplay.cpp
	anim/animplay.h
	anim/packunpack.cpp
	anim/packunpack.h
)

# Asteroid files
set (file_root_asteroid
	asteroid/asteroid.cpp
	asteroid/asteroid.h
)

# Autopilot files
set (file_root_autopilot
	autopilot/autopilot.cpp
	autopilot/autopilot.h
)

# Bmpman files
set (file_root_bmpman
	bmpman/bm_internal.h
	bmpman/bmpman.cpp
	bmpman/bmpman.h
)

# Camera files
set (file_root_camera
	camera/camera.cpp
	camera/camera.h
)

# CFile files
set (file_root_cfile
	cfile/cfile.cpp
	cfile/cfile.h
	cfile/cfilearchive.cpp
	cfile/cfilearchive.h
	cfile/cfilelist.cpp
	cfile/cfilesystem.cpp
	cfile/cfilesystem.h
)

# Cmdline files
set (file_root_cmdline
	cmdline/cmdline.cpp
	cmdline/cmdline.h
)

# CMeasure files
set (file_root_cmeasure
	cmeasure/cmeasure.cpp
	cmeasure/cmeasure.h
)

# ControlConfig files
set (file_root_controlconfig
	controlconfig/controlsconfig.cpp
	controlconfig/controlsconfig.h
	controlconfig/controlsconfigcommon.cpp
)

# Cutscene files
set (file_root_cutscene
	cutscene/Decoder.cpp
	cutscene/Decoder.h
	cutscene/cutscenes.cpp
	cutscene/cutscenes.h
	cutscene/movie.cpp
	cutscene/movie.h
	cutscene/player.cpp
	cutscene/player.h
	cutscene/VideoPresenter.cpp
	cutscene/VideoPresenter.h
)

# Cutscene\ffmpeg files
set (file_root_cutscene_ffmpeg
	cutscene/ffmpeg/AudioDecoder.cpp
	cutscene/ffmpeg/AudioDecoder.h
	cutscene/ffmpeg/FFMPEGDecoder.cpp
	cutscene/ffmpeg/FFMPEGDecoder.h
	cutscene/ffmpeg/internal.cpp
	cutscene/ffmpeg/internal.h
	cutscene/ffmpeg/SubtitleDecoder.cpp
	cutscene/ffmpeg/SubtitleDecoder.h
	cutscene/ffmpeg/VideoDecoder.cpp
	cutscene/ffmpeg/VideoDecoder.h
)

# ddsutils files
set (file_root_ddsutils
	ddsutils/ddsutils.cpp
	ddsutils/ddsutils.h
)

# Debris files
set (file_root_debris
	debris/debris.cpp
	debris/debris.h
)

# DebugConsole files
set (file_root_debugconsole
	debugconsole/console.cpp
	debugconsole/console.h
	debugconsole/consolecmds.cpp
	debugconsole/consoleparse.cpp
	debugconsole/consoleparse.h
)


SET(file_root_def_files
	def_files/def_files.h
)
if(MSVC)
	SET(file_root_def_files
		${file_root_def_files}
		def_files/def_files-win32.cpp
	)
else()
	SET(file_root_def_files
		${file_root_def_files}
		def_files/def_files-generic.cpp
	)
endif()

SET(file_root_def_files_files
	def_files/ai_profiles.tbl
	def_files/autopilot.tbl
	def_files/batched-f.sdr
	def_files/batched-v.sdr
	def_files/bloom-comp-f.sdr
	def_files/blur-f.sdr
	def_files/brightpass-f.sdr
	def_files/controlconfigdefaults.tbl
	def_files/default-material-f.sdr
	def_files/deferred-clear-f.sdr
	def_files/deferred-clear-v.sdr
	def_files/deferred-f.sdr
	def_files/deferred-v.sdr
	def_files/effect-distort-f.sdr
	def_files/effect-distort-v.sdr
	def_files/effect-particle-f.sdr
	def_files/effect-screen-g.sdr
	def_files/effect-v.sdr
	def_files/fonts.tbl
	def_files/fxaa-f.sdr
	def_files/fxaa-v.sdr
	def_files/fxaapre-f.sdr
	def_files/game_settings.tbl
	def_files/iff_defs.tbl
	def_files/ls-f.sdr
	def_files/main-f.sdr
	def_files/main-g.sdr
	def_files/main-v.sdr
	def_files/nanovg-f.sdr
	def_files/nanovg-v.sdr
	def_files/objecttypes.tbl
	def_files/passthrough-f.sdr
	def_files/passthrough-v.sdr
	def_files/post-f.sdr
	def_files/post-v.sdr
	def_files/post_processing.tbl
	def_files/shadowdebug-f.sdr
	def_files/shadowdebug-v.sdr
	def_files/species_defs.tbl
	def_files/tonemapping-f.sdr
	def_files/video-f.sdr
	def_files/video-v.sdr
	def_files/shield-impact-v.sdr
	def_files/shield-impact-f.sdr
)

# ExceptionHandler files
set (file_root_exceptionhandler
	exceptionhandler/exceptionhandler.cpp
	exceptionhandler/exceptionhandler.h
)

# ExternalDLL files
set (file_root_externaldll
	external_dll/externalcode.h
)

# Fireball files
set (file_root_fireball
	fireball/fireballs.cpp
	fireball/fireballs.h
	fireball/warpineffect.cpp
)

# GameHelp files
set (file_root_gamehelp
	gamehelp/contexthelp.cpp
	gamehelp/contexthelp.h
	gamehelp/gameplayhelp.cpp
	gamehelp/gameplayhelp.h
)

# GameHelp -> fs2netd files
set (file_root_gamehelp_fs2netd
	fs2netd/fs2netd_client.cpp
	fs2netd/fs2netd_client.h
	fs2netd/protocol.h
	fs2netd/tcp_client.cpp
	fs2netd/tcp_client.h
	fs2netd/tcp_socket.cpp
	fs2netd/tcp_socket.h
)

# GameSequence files
set (file_root_gamesequence
	gamesequence/gamesequence.cpp
	gamesequence/gamesequence.h
)

# GameSnd files
set (file_root_gamesnd
	gamesnd/eventmusic.cpp
	gamesnd/eventmusic.h
	gamesnd/gamesnd.cpp
	gamesnd/gamesnd.h
)

set(file_root_generated
	${GENERATED_SOURCE_DIR}/project.h
	${GENERATED_SOURCE_DIR}/scp_compiler_detection.h
	${PLATFORM_CHECK_HEADER}
)

# GlobalIncs files
set (file_root_globalincs
	globalincs/alphacolors.cpp
	globalincs/alphacolors.h
	globalincs/fsmemory.h
	globalincs/globals.h
	globalincs/linklist.h
	globalincs/pstypes.h
	globalincs/safe_strings.cpp
	globalincs/safe_strings.h
	globalincs/systemvars.cpp
	globalincs/systemvars.h
	globalincs/toolchain.h
	globalincs/version.cpp
	globalincs/version.h
	globalincs/vmallocator.h
	globalincs/scp_defines.h
	globalincs/flagset.h
)

IF (WIN32)
	set (file_root_globalincs
		${file_root_globalincs}
		globalincs/mspdb_callstack.cpp
		globalincs/mspdb_callstack.h
		globalincs/windebug.cpp
	)
ENDIF(WIN32)

set(file_root_globalincs_memory
	globalincs/memory/memory.h
	globalincs/memory/memory.cpp
	globalincs/memory/utils.h
)

set(file_root_globalincs_toolchain
	globalincs/toolchain/clang.h
	globalincs/toolchain/doxygen.h
	globalincs/toolchain/gcc.h
	globalincs/toolchain/mingw.h
	globalincs/toolchain/msvc.h
)

# Graphics files
set (file_root_graphics
	graphics/2d.cpp
	graphics/2d.h
	graphics/grbatch.cpp
	graphics/grbatch.h
	graphics/grinternal.h
	graphics/light.cpp
	graphics/light.h
	graphics/material.cpp
	graphics/material.h
	graphics/matrix.cpp
	graphics/matrix.h
	graphics/render.cpp
	graphics/render.h
	graphics/shadows.cpp
	graphics/shadows.h
	graphics/tmapper.h
	graphics/uniforms.cpp
	graphics/uniforms.h
)

# Graphics -> OpenGLGr files
set (file_root_graphics_openglgr
)

# Graphics -> OpenGLGr -> OpenGL CPPs files
set (file_root_graphics_openglgr_opengl_cpps
	graphics/opengl/gropengl.cpp
	graphics/opengl/gropenglbmpman.cpp
	graphics/opengl/gropengldeferred.cpp
	graphics/opengl/gropengldraw.cpp
	graphics/opengl/gropenglpostprocessing.cpp
	graphics/opengl/gropenglquery.cpp
	graphics/opengl/gropenglshader.cpp
	graphics/opengl/gropenglstate.cpp
	graphics/opengl/gropenglsync.cpp
	graphics/opengl/gropengltexture.cpp
	graphics/opengl/gropengltnl.cpp
	graphics/opengl/ShaderProgram.cpp
)

# Graphics -> OpenGLGr -> OpenGL Headers files
set (file_root_graphics_openglgr_opengl_headers
	graphics/opengl/gropengl.h
	graphics/opengl/gropenglbmpman.h
	graphics/opengl/gropengldeferred.h
	graphics/opengl/gropengldraw.h
	graphics/opengl/gropenglpostprocessing.h
	graphics/opengl/gropenglquery.h
	graphics/opengl/gropenglshader.h
	graphics/opengl/gropenglstate.h
	graphics/opengl/gropenglsync.h
	graphics/opengl/gropengltexture.h
	graphics/opengl/gropengltnl.h
	graphics/opengl/ShaderProgram.h
)

# Graphics -> Paths
set (file_root_graphics_paths
	graphics/paths/NanoVGRenderer.cpp
	graphics/paths/NanoVGRenderer.h
	graphics/paths/PathRenderer.cpp
	graphics/paths/PathRenderer.h
)

# Graphics -> Paths
set (file_root_graphics_paths_nanovg
	graphics/paths/nanovg/fontstash.h
	graphics/paths/nanovg/nanovg.c
	graphics/paths/nanovg/nanovg.h
	graphics/paths/nanovg/stb_image.h
	graphics/paths/nanovg/stb_truetype.h
)

# Graphics -> SoftwareGr files
set (file_root_graphics_softwaregr
	graphics/generic.cpp
	graphics/generic.h
	graphics/grstub.cpp
	graphics/grstub.h
	graphics/line.h
	graphics/font.h
)


set (file_root_graphics_softwaregr_font
	graphics/software/font.h
	graphics/software/font.cpp
	graphics/software/font_internal.h
	graphics/software/FontManager.h
	graphics/software/FontManager.cpp
	graphics/software/FSFont.h
	graphics/software/FSFont.cpp
	graphics/software/NVGFont.h
	graphics/software/NVGFont.cpp
	graphics/software/VFNTFont.h
	graphics/software/VFNTFont.cpp
)

set(file_root_graphics_util
	graphics/util/uniform_structs.h
	graphics/util/UniformAligner.h
	graphics/util/UniformAligner.cpp
	graphics/util/UniformBuffer.h
	graphics/util/UniformBuffer.cpp
	graphics/util/UniformBufferManager.h
	graphics/util/UniformBufferManager.cpp
)

# HeadTracking files
set (file_root_headtracking
	headtracking/headtracking_internal.h
	headtracking/headtracking.h
	headtracking/headtracking.cpp
)
if(WIN32)
	set(file_root_headtracking
		${file_root_headtracking}

		headtracking/freetrack.h
		headtracking/freetrack.cpp

		headtracking/trackir.h
		headtracking/trackir.cpp
		headtracking/trackirpublic.h
		headtracking/trackirpublic.cpp
	)
endif()

# Hud files
set (file_root_hud
	hud/hud.cpp
	hud/hud.h
	hud/hudartillery.cpp
	hud/hudartillery.h
	hud/hudbrackets.cpp
	hud/hudbrackets.h
	hud/hudconfig.cpp
	hud/hudconfig.h
	hud/hudescort.cpp
	hud/hudescort.h
	hud/hudets.cpp
	hud/hudets.h
	hud/hudgauges.h
	hud/hudlock.cpp
	hud/hudlock.h
	hud/hudmessage.cpp
	hud/hudmessage.h
	hud/hudnavigation.cpp
	hud/hudnavigation.h
	hud/hudobserver.cpp
	hud/hudobserver.h
	hud/hudparse.cpp
	hud/hudparse.h
	hud/hudreticle.cpp
	hud/hudreticle.h
	hud/hudshield.cpp
	hud/hudshield.h
	hud/hudsquadmsg.cpp
	hud/hudsquadmsg.h
	hud/hudtarget.cpp
	hud/hudtarget.h
	hud/hudtargetbox.cpp
	hud/hudtargetbox.h
	hud/hudwingmanstatus.cpp
	hud/hudwingmanstatus.h
)

# iff_defs files
set (file_root_iff_defs
	iff_defs/iff_defs.cpp
	iff_defs/iff_defs.h
)

# InetFile files
set (file_root_inetfile
	inetfile/cftp.cpp
	inetfile/cftp.h
	inetfile/chttpget.cpp
	inetfile/chttpget.h
	inetfile/inetgetfile.cpp
	inetfile/inetgetfile.h
)

# Io files
set (file_root_io
	io/cursor.cpp
	io/cursor.h
	io/key.cpp
	io/key.h
	io/keycontrol.cpp
	io/keycontrol.h
	io/mouse.cpp
	io/mouse.h
	io/timer.cpp
	io/timer.h
	io/joy.h
	io/joy-sdl.cpp
	io/joy_ff.h
	io/joy_ff-sdl.cpp
)

# jpgutils files
set (file_root_jpgutils
	jpgutils/jpgutils.cpp
	jpgutils/jpgutils.h
)

# JumpNode files
set (file_root_jumpnode
	jumpnode/jumpnode.cpp
	jumpnode/jumpnode.h
)

# Lab files
set (file_root_lab
	lab/lab.cpp
	lab/lab.h
	lab/wmcgui.cpp
	lab/wmcgui.h
)

set(file_root_libs
)

set(file_root_libs_ffmpeg
	libs/ffmpeg/FFmpeg.cpp
	libs/ffmpeg/FFmpeg.h
	libs/ffmpeg/FFmpegContext.cpp
	libs/ffmpeg/FFmpegContext.h
	libs/ffmpeg/FFmpegHeaders.h
)

# Lighting files
set (file_root_lighting
	lighting/lighting.cpp
	lighting/lighting.h
)

# Localization files
set (file_root_localization
	localization/fhash.cpp
	localization/fhash.h
	localization/localize.cpp
	localization/localize.h
)

# Math files
set (file_root_math
	math/bitarray.h
	math/fix.cpp
	math/fix.h
	math/floating.cpp
	math/floating.h
	math/fvi.cpp
	math/fvi.h
	math/spline.cpp
	math/spline.h
	math/staticrand.cpp
	math/staticrand.h
	math/vecmat.cpp
	math/vecmat.h
)

# MenuUI files
set (file_root_menuui
	menuui/barracks.cpp
	menuui/barracks.h
	menuui/credits.cpp
	menuui/credits.h
	menuui/fishtank.cpp
	menuui/fishtank.h
	menuui/mainhallmenu.cpp
	menuui/mainhallmenu.h
	menuui/mainhalltemp.cpp
	menuui/mainhalltemp.h
	menuui/optionsmenu.cpp
	menuui/optionsmenu.h
	menuui/optionsmenumulti.cpp
	menuui/optionsmenumulti.h
	menuui/playermenu.cpp
	menuui/playermenu.h
	menuui/readyroom.cpp
	menuui/readyroom.h
	menuui/snazzyui.cpp
	menuui/snazzyui.h
	menuui/techmenu.cpp
	menuui/techmenu.h
	menuui/trainingmenu.cpp
	menuui/trainingmenu.h
)

# Mission files
set (file_root_mission
	mission/missionbriefcommon.cpp
	mission/missionbriefcommon.h
	mission/missioncampaign.cpp
	mission/missioncampaign.h
	mission/missiongoals.cpp
	mission/missiongoals.h
	mission/missiongrid.cpp
	mission/missiongrid.h
	mission/missionhotkey.cpp
	mission/missionhotkey.h
	mission/missionload.cpp
	mission/missionload.h
	mission/missionlog.cpp
	mission/missionlog.h
	mission/missionmessage.cpp
	mission/missionmessage.h
	mission/missionparse.cpp
	mission/missionparse.h
	mission/missiontraining.cpp
	mission/missiontraining.h
	mission/mission_flags.h
)

# MissionUI files
set (file_root_missionui
	missionui/chatbox.cpp
	missionui/chatbox.h
	missionui/fictionviewer.cpp
	missionui/fictionviewer.h
	missionui/missionbrief.cpp
	missionui/missionbrief.h
	missionui/missioncmdbrief.cpp
	missionui/missioncmdbrief.h
	missionui/missiondebrief.cpp
	missionui/missiondebrief.h
	missionui/missionloopbrief.cpp
	missionui/missionloopbrief.h
	missionui/missionpause.cpp
	missionui/missionpause.h
	missionui/missionscreencommon.cpp
	missionui/missionscreencommon.h
	missionui/missionshipchoice.cpp
	missionui/missionshipchoice.h
	missionui/missionweaponchoice.cpp
	missionui/missionweaponchoice.h
	missionui/redalert.cpp
	missionui/redalert.h
)

# mod_table files
set (file_root_mod_table
	mod_table/mod_table.cpp
	mod_table/mod_table.h
)

# Model files
set (file_root_model
	model/model.h
	model/modelanim.cpp
	model/modelanim.h
	model/modelcollide.cpp
	model/modelinterp.cpp
	model/modeloctant.cpp
	model/modelread.cpp
	model/modelrender.h
	model/modelrender.cpp
	model/modelsinc.h
	model/model_flags.h
)

# Nebula files
set (file_root_nebula
	nebula/neb.cpp
	nebula/neb.h
	nebula/neblightning.cpp
	nebula/neblightning.h
)

# Network files
set (file_root_network
	network/chat_api.cpp
	network/chat_api.h
	network/multi.cpp
	network/multi.h
	network/multi_campaign.cpp
	network/multi_campaign.h
	network/multi_data.cpp
	network/multi_data.h
	network/multi_dogfight.cpp
	network/multi_dogfight.h
	network/multi_endgame.cpp
	network/multi_endgame.h
	network/multi_ingame.cpp
	network/multi_ingame.h
	network/multi_kick.cpp
	network/multi_kick.h
	network/multi_log.cpp
	network/multi_log.h
	network/multi_obj.cpp
	network/multi_obj.h
	network/multi_observer.cpp
	network/multi_observer.h
	network/multi_options.cpp
	network/multi_options.h
	network/multi_pause.cpp
	network/multi_pause.h
	network/multi_pinfo.cpp
	network/multi_pinfo.h
	network/multi_ping.cpp
	network/multi_ping.h
	network/multi_pmsg.cpp
	network/multi_pmsg.h
	network/multi_pxo.cpp
	network/multi_pxo.h
	network/multi_rate.cpp
	network/multi_rate.h
	network/multi_respawn.cpp
	network/multi_respawn.h
	network/multi_sexp.cpp
	network/multi_sexp.h
	network/multi_team.cpp
	network/multi_team.h
	network/multi_update.cpp
	network/multi_update.h
	network/multi_voice.cpp
	network/multi_voice.h
	network/multi_xfer.cpp
	network/multi_xfer.h
	network/multilag.cpp
	network/multilag.h
	network/multimsgs.cpp
	network/multimsgs.h
	network/multiteamselect.cpp
	network/multiteamselect.h
	network/multiui.cpp
	network/multiui.h
	network/multiutil.cpp
	network/multiutil.h
	network/psnet2.cpp
	network/psnet2.h
	network/stand_gui.cpp
	network/stand_gui.h
)

IF(WIN32)
set (file_root_network
	${file_root_network}
	network/stand_gui.cpp
)
ELSE(WIN32)
set (file_root_network
	${file_root_network}
	network/stand_gui-unix.cpp
)
ENDIF(WIN32)

# Object files
set (file_root_object
	object/collidedebrisship.cpp
	object/collidedebrisweapon.cpp
	object/collideshipship.cpp
	object/collideshipweapon.cpp
	object/collideweaponweapon.cpp
	object/deadobjectdock.cpp
	object/deadobjectdock.h
	object/objcollide.cpp
	object/objcollide.h
	object/object.cpp
	object/object.h
	object/objectdock.cpp
	object/objectdock.h
	object/objectshield.cpp
	object/objectshield.h
	object/objectsnd.cpp
	object/objectsnd.h
	object/objectsort.cpp
	object/parseobjectdock.cpp
	object/parseobjectdock.h
	object/waypoint.cpp
	object/waypoint.h
	object/object_flags.h
)

# Observer files
set (file_root_observer
	observer/observer.cpp
	observer/observer.h
)

# OsApi files
set (file_root_osapi
	osapi/DebugWindow.h
	osapi/DebugWindow.cpp
	osapi/osapi.h
	osapi/osapi.cpp
	osapi/dialogs.h
	osapi/dialogs.cpp
	osapi/osregistry.h
	osapi/osregistry.cpp
	osapi/outwnd.h
	osapi/outwnd.cpp
)

# Parse files
set (file_root_parse
	parse/encrypt.cpp
	parse/encrypt.h
	parse/generic_log.cpp
	parse/generic_log.h
	parse/parselo.cpp
	parse/parselo.h
	parse/sexp.cpp
	parse/sexp.h
)

# Particle files
set (file_root_particle
	particle/particle.cpp
	particle/particle.h
	particle/ParticleEffect.h
	particle/ParticleManager.cpp
	particle/ParticleManager.h
	particle/ParticleSource.cpp
	particle/ParticleSource.h
	particle/ParticleSourceWrapper.cpp
	particle/ParticleSourceWrapper.h
)

set(file_root_particle_effects
	particle/effects/BeamPiercingEffect.cpp
	particle/effects/BeamPiercingEffect.h
	particle/effects/CompositeEffect.cpp
	particle/effects/CompositeEffect.h
	particle/effects/ConeShape.h
	particle/effects/GenericShapeEffect.h
	particle/effects/ParticleEmitterEffect.cpp
	particle/effects/ParticleEmitterEffect.h
	particle/effects/SingleParticleEffect.cpp
	particle/effects/SingleParticleEffect.h
	particle/effects/SphereShape.h
)

set(file_root_particle_util
	particle/util/EffectTiming.cpp
	particle/util/EffectTiming.h
	particle/util/ParticleProperties.cpp
	particle/util/ParticleProperties.h
)

# PcxUtils files
set (file_root_pcxutils
	pcxutils/pcxutils.cpp
	pcxutils/pcxutils.h
)

# Physics files
set (file_root_physics
	physics/physics.cpp
	physics/physics.h
)

# PilotFile files
set (file_root_pilotfile
	pilotfile/BinaryFileHandler.cpp
	pilotfile/BinaryFileHandler.h
	pilotfile/csg.cpp
	pilotfile/FileHandler.h
	pilotfile/JSONFileHandler.cpp
	pilotfile/JSONFileHandler.h
	pilotfile/csg_convert.cpp
	pilotfile/pilotfile.cpp
	pilotfile/pilotfile.h
	pilotfile/pilotfile_convert.cpp
	pilotfile/pilotfile_convert.h
	pilotfile/plr.cpp
	pilotfile/plr_convert.cpp
)

# Playerman files
set (file_root_playerman
	playerman/managepilot.cpp
	playerman/managepilot.h
	playerman/player.h
	playerman/playercontrol.cpp
)

# pngutils files
set (file_root_pngutils
	pngutils/pngutils.cpp
	pngutils/pngutils.h
)

# Popup files
set (file_root_popup
	popup/popup.cpp
	popup/popup.h
	popup/popupdead.cpp
	popup/popupdead.h
)

# Radar files
set (file_root_radar
	radar/radar.cpp
	radar/radar.h
	radar/radardradis.cpp
	radar/radardradis.h
	radar/radarorb.cpp
	radar/radarorb.h
	radar/radarsetup.cpp
	radar/radarsetup.h
)

# Render files
set (file_root_render
	render/3d.h
	render/3dclipper.cpp
	render/3ddraw.cpp
	render/3dinternal.h
	render/3dlaser.cpp
	render/3dmath.cpp
	render/3dsetup.cpp
	render/batching.cpp
	render/batching.h
)

set(file_root_scripting
	scripting/ade.cpp
	scripting/ade.h
	scripting/ade_api.h
	scripting/ade_args.cpp
	scripting/ade_args.h
	scripting/lua.cpp
	scripting/scripting.cpp
	scripting/scripting.h
)

set(file_root_scripting_api_libs
	scripting/api/libs/audio.cpp
	scripting/api/libs/audio.h
	scripting/api/libs/base.cpp
	scripting/api/libs/base.h
	scripting/api/libs/bitops.cpp
	scripting/api/libs/bitops.h
	scripting/api/libs/cfile.cpp
	scripting/api/libs/cfile.h
	scripting/api/libs/hookvars.cpp
	scripting/api/libs/hookvars.h
	scripting/api/libs/hud.cpp
	scripting/api/libs/hud.h
	scripting/api/libs/mission.cpp
	scripting/api/libs/mission.h
	scripting/api/libs/tables.cpp
	scripting/api/libs/tables.h
	scripting/api/libs/testing.cpp
	scripting/api/libs/testing.h
	scripting/api/libs/utf8.cpp
	scripting/api/libs/utf8.h
)

set(file_root_scripting_api_objs
	scripting/api/objs/asteroid.cpp
	scripting/api/objs/asteroid.h
	scripting/api/objs/beam.cpp
	scripting/api/objs/beam.h
	scripting/api/objs/camera.cpp
	scripting/api/objs/camera.h
	scripting/api/objs/cockpit_display.cpp
	scripting/api/objs/cockpit_display.h
	scripting/api/objs/control_info.cpp
	scripting/api/objs/control_info.h
	scripting/api/objs/controls.cpp
	scripting/api/objs/controls.h
	scripting/api/objs/debris.cpp
	scripting/api/objs/debris.h
	scripting/api/objs/enums.cpp
	scripting/api/objs/enums.h
	scripting/api/objs/event.cpp
	scripting/api/objs/event.h
	scripting/api/objs/eye.cpp
	scripting/api/objs/eye.h
	scripting/api/objs/file.cpp
	scripting/api/objs/file.h
	scripting/api/objs/font.cpp
	scripting/api/objs/font.h
	scripting/api/objs/gameevent.cpp
	scripting/api/objs/gameevent.h
	scripting/api/objs/gamestate.cpp
	scripting/api/objs/gamestate.h
	scripting/api/objs/graphics.cpp
	scripting/api/objs/graphics.h
	scripting/api/objs/hudgauge.cpp
	scripting/api/objs/hudgauge.h
	scripting/api/objs/mc_info.cpp
	scripting/api/objs/mc_info.h
	scripting/api/objs/message.cpp
	scripting/api/objs/message.h
	scripting/api/objs/model.cpp
	scripting/api/objs/model.h
	scripting/api/objs/object.cpp
	scripting/api/objs/object.h
	scripting/api/objs/order.cpp
	scripting/api/objs/order.h
	scripting/api/objs/particle.cpp
	scripting/api/objs/particle.h
	scripting/api/objs/physics_info.cpp
	scripting/api/objs/physics_info.h
	scripting/api/objs/player.cpp
	scripting/api/objs/player.h
	scripting/api/objs/sexpvar.cpp
	scripting/api/objs/sexpvar.h
	scripting/api/objs/shields.cpp
	scripting/api/objs/shields.h
	scripting/api/objs/ship_bank.cpp
	scripting/api/objs/ship_bank.h
	scripting/api/objs/shipclass.cpp
	scripting/api/objs/shipclass.h
	scripting/api/objs/ship.cpp
	scripting/api/objs/ship.h
	scripting/api/objs/shiptype.cpp
	scripting/api/objs/shiptype.h
	scripting/api/objs/sound.cpp
	scripting/api/objs/sound.h
	scripting/api/objs/species.cpp
	scripting/api/objs/species.h
	scripting/api/objs/streaminganim.cpp
	scripting/api/objs/streaminganim.h
	scripting/api/objs/subsystem.cpp
	scripting/api/objs/subsystem.h
	scripting/api/objs/team.cpp
	scripting/api/objs/team.h
	scripting/api/objs/texture.cpp
	scripting/api/objs/texture.h
	scripting/api/objs/texturemap.cpp
	scripting/api/objs/texturemap.h
	scripting/api/objs/vecmath.cpp
	scripting/api/objs/vecmath.h
	scripting/api/objs/waypoint.cpp
	scripting/api/objs/waypoint.h
	scripting/api/objs/weaponclass.cpp
	scripting/api/objs/weaponclass.h
	scripting/api/objs/weapon.cpp
	scripting/api/objs/weapon.h
	scripting/api/objs/wing.cpp
	scripting/api/objs/wing.h
)

set(file_root_scripting_lua
	scripting/lua/LuaArgs.cpp
	scripting/lua/LuaArgs.h
	scripting/lua/LuaConvert.h
	scripting/lua/LuaException.h
	scripting/lua/LuaFunction.cpp
	scripting/lua/LuaFunction.h
	scripting/lua/LuaHeaders.h
	scripting/lua/LuaReference.cpp
	scripting/lua/LuaReference.h
	scripting/lua/LuaTable.cpp
	scripting/lua/LuaTable.h
	scripting/lua/LuaUtil.cpp
	scripting/lua/LuaUtil.h
	scripting/lua/LuaValue.cpp
	scripting/lua/LuaValue.h
)

# Ship files
set (file_root_ship
	ship/afterburner.cpp
	ship/afterburner.h
	ship/awacs.cpp
	ship/awacs.h
	ship/shield.cpp
	ship/ship.cpp
	ship/ship.h
	ship/shipcontrails.cpp
	ship/shipcontrails.h
	ship/shipfx.cpp
	ship/shipfx.h
	ship/shiphit.cpp
	ship/shiphit.h
	ship/subsysdamage.h
	ship/ship_flags.h
)

# Sound files
set (file_root_sound
		sound/audiostr.cpp
	sound/audiostr.h
	sound/channel.h
	sound/ds.cpp
	sound/ds.h
	sound/ds3d.cpp
	sound/ds3d.h
	sound/dscap.cpp
	sound/dscap.h
	sound/fsspeech.cpp
	sound/fsspeech.h
	sound/openal.cpp
	sound/openal.h
	sound/phrases.xml
	sound/rtvoice.cpp
	sound/rtvoice.h
	sound/sound.cpp
	sound/sound.h
	sound/speech.cpp
	sound/speech.h
	sound/voicerec.cpp
	sound/voicerec.h
)

# Sound -> ffmpeg files
set (file_root_sound_ffmpeg
	sound/ffmpeg/FFmpegAudioReader.cpp
	sound/ffmpeg/FFmpegAudioReader.h
	sound/ffmpeg/WaveFile.cpp
	sound/ffmpeg/WaveFile.h
)

# Species_Defs files
set (file_root_species_defs
	species_defs/species_defs.cpp
	species_defs/species_defs.h
)

# Starfield files
set (file_root_starfield
	starfield/nebula.cpp
	starfield/nebula.h
	starfield/starfield.cpp
	starfield/starfield.h
	starfield/supernova.cpp
	starfield/supernova.h
)

# Stats files
set (file_root_stats
	stats/medals.cpp
	stats/medals.h
	stats/scoring.cpp
	stats/scoring.h
	stats/stats.cpp
	stats/stats.h
)

# TgaUtils files
set (file_root_tgautils
	tgautils/tgautils.cpp
	tgautils/tgautils.h
)

# Tracing files
set (file_root_tracing
	tracing/categories.cpp
	tracing/categories.h
	tracing/FrameProfiler.h
	tracing/FrameProfiler.cpp
	tracing/MainFrameTimer.h
	tracing/MainFrameTimer.cpp
	tracing/Monitor.h
	tracing/Monitor.cpp
	tracing/scopes.cpp
	tracing/scopes.h
	tracing/ThreadedEventProcessor.h
	tracing/TraceEventWriter.h
	tracing/TraceEventWriter.cpp
	tracing/tracing.h
	tracing/tracing.cpp
)

# Ui files
set (file_root_ui
	ui/button.cpp
	ui/checkbox.cpp
	ui/gadget.cpp
	ui/icon.cpp
	ui/inputbox.cpp
	ui/keytrap.cpp
	ui/listbox.cpp
	ui/radio.cpp
	ui/scroll.cpp
	ui/slider.cpp
	ui/slider2.cpp
	ui/ui.h
	ui/uidefs.h
	ui/uidraw.cpp
	ui/uimouse.cpp
	ui/window.cpp
)

set(file_root_utils
	utils/encoding.cpp
    utils/encoding.h
	utils/RandomRange.h
	utils/strings.h
    utils/unicode.cpp
    utils/unicode.h
)

# Utils files
set (file_root_utils_boost
	utils/boost/hash_combine.h
	utils/boost/syncboundedqueue.h
)

# Weapon files
set (file_root_weapon
	weapon/beam.cpp
	weapon/beam.h
	weapon/corkscrew.cpp
	weapon/corkscrew.h
	weapon/emp.cpp
	weapon/emp.h
	weapon/flak.cpp
	weapon/flak.h
	weapon/muzzleflash.cpp
	weapon/muzzleflash.h
	weapon/shockwave.cpp
	weapon/shockwave.h
	weapon/swarm.cpp
	weapon/swarm.h
	weapon/trails.cpp
	weapon/trails.h
	weapon/weapon.h
	weapon/weapons.cpp
	weapon/weapon_flags.h
)

# Windows stubs files
set(file_root_windows_stubs
	windows_stub/config.h
)

IF(UNIX)
	SET(file_root_windows_stubs
		${file_root_windows_stubs}
		windows_stub/stubs.cpp
	)
ENDIF(UNIX)

# the source groups
source_group(""                                   FILES ${file_root})
source_group("AI"                                 FILES ${file_root_ai})
source_group("Anim"                               FILES ${file_root_anim})
source_group("Asteroid"                           FILES ${file_root_asteroid})
source_group("Autopilot"                          FILES ${file_root_autopilot})
source_group("Bmpman"                             FILES ${file_root_bmpman})
source_group("Camera"                             FILES ${file_root_camera})
source_group("CFile"                              FILES ${file_root_cfile})
source_group("Cmdline"                            FILES ${file_root_cmdline})
source_group("CMeasure"                           FILES ${file_root_cmeasure})
source_group("ControlConfig"                      FILES ${file_root_controlconfig})
source_group("Cutscene"                           FILES ${file_root_cutscene})
source_group("Cutscene\\ffmpeg"                   FILES ${file_root_cutscene_ffmpeg})
source_group("Cutscene\\Player"                   FILES ${file_root_cutscene_player})
source_group("ddsutils"                           FILES ${file_root_ddsutils})
source_group("Debris"                             FILES ${file_root_debris})
source_group("DebugConsole"                       FILES ${file_root_debugconsole})
source_group("Default files"                      FILES ${file_root_def_files})
source_group("Default files\\Files"               FILES ${file_root_def_files_files})
source_group("ExceptionHandler"                   FILES ${file_root_exceptionhandler})
source_group("ExternalDLL"                        FILES ${file_root_externaldll})
source_group("Fireball"                           FILES ${file_root_fireball})
source_group("GameHelp"                           FILES ${file_root_gamehelp})
source_group("GameHelp\\fs2netd"                  FILES ${file_root_gamehelp_fs2netd})
source_group("GameSequence"                       FILES ${file_root_gamesequence})
source_group("GameSnd"                            FILES ${file_root_gamesnd})
source_group("Generated Files"                    FILES ${file_root_generated})
source_group("GlobalIncs"                         FILES ${file_root_globalincs})
source_group("GlobalIncs\\Memory"                 FILES ${file_root_globalincs_memory})
source_group("GlobalIncs\\Toolchain"              FILES ${file_root_globalincs_toolchain})
source_group("Graphics"                           FILES ${file_root_graphics})
source_group("Graphics\\OpenGLGr"                 FILES ${file_root_graphics_openglgr})
source_group("Graphics\\OpenGLGr\\OpenGL CPPs"    FILES ${file_root_graphics_openglgr_opengl_cpps})
source_group("Graphics\\OpenGLGr\\OpenGL Headers" FILES ${file_root_graphics_openglgr_opengl_headers})
source_group("Graphics\\Paths"                    FILES ${file_root_graphics_paths})
source_group("Graphics\\Paths\\nanovg"            FILES ${file_root_graphics_paths_nanovg})
source_group("Graphics\\SoftwareGr"               FILES ${file_root_graphics_softwaregr})
source_group("Graphics\\SoftwareGr\\Font"         FILES ${file_root_graphics_softwaregr_font})
source_group("Graphics\\Util"                     FILES ${file_root_graphics_util})
source_group("HeadTracking"                       FILES ${file_root_headtracking})
source_group("Hud"                                FILES ${file_root_hud})
source_group("iff_defs"                           FILES ${file_root_iff_defs})
source_group("InetFile"                           FILES ${file_root_inetfile})
source_group("Io"                                 FILES ${file_root_io})
source_group("jpgutils"                           FILES ${file_root_jpgutils})
source_group("JumpNode"                           FILES ${file_root_jumpnode})
source_group("Lab"                                FILES ${file_root_lab})
source_group("Libs"                               FILES ${file_root_libs})
source_group("Libs\\FFmpeg"                       FILES ${file_root_libs_ffmpeg})
source_group("Lighting"                           FILES ${file_root_lighting})
source_group("Localization"                       FILES ${file_root_localization})
source_group("Math"                               FILES ${file_root_math})
source_group("MenuUI"                             FILES ${file_root_menuui})
source_group("Mission"                            FILES ${file_root_mission})
source_group("MissionUI"                          FILES ${file_root_missionui})
source_group("mod_table"                          FILES ${file_root_mod_table})
source_group("Model"                              FILES ${file_root_model})
source_group("Nebula"                             FILES ${file_root_nebula})
source_group("Network"                            FILES ${file_root_network})
source_group("Object"                             FILES ${file_root_object})
source_group("Observer"                           FILES ${file_root_observer})
source_group("OsApi"                              FILES ${file_root_osapi})
source_group("Parse"                              FILES ${file_root_parse})
source_group("Particle"                           FILES ${file_root_particle})
source_group("Particle\\Effects"                  FILES ${file_root_particle_effects})
source_group("Particle\\Util"                     FILES ${file_root_particle_util})
source_group("PcxUtils"                           FILES ${file_root_pcxutils})
source_group("Physics"                            FILES ${file_root_physics})
source_group("PilotFile"                          FILES ${file_root_pilotfile})
source_group("Playerman"                          FILES ${file_root_playerman})
source_group("pngutils"                           FILES ${file_root_pngutils})
source_group("Popup"                              FILES ${file_root_popup})
source_group("Radar"                              FILES ${file_root_radar})
source_group("Render"                             FILES ${file_root_render})
source_group("Scripting"                          FILES ${file_root_scripting})
source_group("Scripting\\Api\\Libs"               FILES ${file_root_scripting_api_libs})
source_group("Scripting\\Api\\Objs"               FILES ${file_root_scripting_api_objs})
source_group("Scripting\\Lua"                     FILES ${file_root_scripting_lua})
source_group("Ship"                               FILES ${file_root_ship})
source_group("Sound"                              FILES ${file_root_sound})
source_group("Sound\\FFmpeg"                      FILES ${file_root_sound_ffmpeg})
source_group("Species_Defs"                       FILES ${file_root_species_defs})
source_group("Starfield"                          FILES ${file_root_starfield})
source_group("Stats"                              FILES ${file_root_stats})
source_group("TgaUtils"                           FILES ${file_root_tgautils})
source_group("Tracing"                            FILES ${file_root_tracing})
source_group("Ui"                                 FILES ${file_root_ui})
source_group("Utils"                              FILES ${file_root_utils})
source_group("Utils\\boost"                       FILES ${file_root_utils_boost})
source_group("Weapon"                             FILES ${file_root_weapon})
source_group("Windows Stubs"                      FILES ${file_root_windows_stubs})

# append all files to the file_root
set (file_root
	${file_root}
	${file_root_ai}
	${file_root_anim}
	${file_root_asteroid}
	${file_root_autopilot}
	${file_root_bmpman}
	${file_root_camera}
	${file_root_cfile}
	${file_root_cmdline}
	${file_root_cmeasure}
	${file_root_controlconfig}
	${file_root_cutscene}
	${file_root_cutscene_ffmpeg}
	${file_root_cutscene_player}
	${file_root_ddsutils}
	${file_root_debris}
	${file_root_debugconsole}
	${file_root_def_files}
	${file_root_def_files_files}
	${file_root_exceptionhandler}
	${file_root_externaldll}
	${file_root_fireball}
	${file_root_gamehelp}
	${file_root_gamehelp_fs2netd}
	${file_root_gamesequence}
	${file_root_gamesnd}
	${file_root_generated}
	${file_root_globalincs}
	${file_root_globalincs_memory}
	${file_root_globalincs_toolchain}
	${file_root_graphics}
	${file_root_graphics_openglgr}
	${file_root_graphics_openglgr_opengl_cpps}
	${file_root_graphics_openglgr_opengl_headers}
	${file_root_graphics_paths}
	${file_root_graphics_paths_nanovg}
	${file_root_graphics_softwaregr}
	${file_root_graphics_softwaregr_font}
	${file_root_graphics_util}
	${file_root_headtracking}
	${file_root_hud}
	${file_root_iff_defs}
	${file_root_inetfile}
	${file_root_io}
	${file_root_jpgutils}
	${file_root_jumpnode}
	${file_root_lab}
	${file_root_libs}
	${file_root_libs_ffmpeg}
	${file_root_lighting}
	${file_root_localization}
	${file_root_math}
	${file_root_menuui}
	${file_root_mission}
	${file_root_missionui}
	${file_root_mod_table}
	${file_root_model}
	${file_root_nebula}
	${file_root_network}
	${file_root_object}
	${file_root_observer}
	${file_root_osapi}
	${file_root_parse}
	${file_root_particle}
	${file_root_particle_effects}
	${file_root_particle_util}
	${file_root_pcxutils}
	${file_root_physics}
	${file_root_pilotfile}
	${file_root_playerman}
	${file_root_pngutils}
	${file_root_popup}
	${file_root_radar}
	${file_root_render}
	${file_root_scripting}
	${file_root_scripting_api_libs}
	${file_root_scripting_api_objs}
	${file_root_scripting_lua}
	${file_root_ship}
	${file_root_sound}
	${file_root_sound_ffmpeg}
	${file_root_species_defs}
	${file_root_starfield}
	${file_root_stats}
	${file_root_tgautils}
	${file_root_tracing}
	${file_root_ui}
	${file_root_utils}
	${file_root_utils_boost}
	${file_root_weapon}
	${file_root_windows_stubs}
)
