
set(source_files)

# top-level files
add_file_folder(""
	prefix_header.h
)

add_file_folder("Actions"
	actions/Action.cpp
	actions/Action.h
	actions/ActionDefinition.cpp
	actions/ActionDefinition.h
	actions/ActionDefinitionManager.cpp
	actions/ActionDefinitionManager.h
	actions/BuiltinActionDefinition.h
	actions/common.cpp
	actions/common.h
	actions/Program.cpp
	actions/Program.h
)

add_file_folder("Actions\\\\Expression"
	actions/expression/ActionExpression.cpp
	actions/expression/ActionExpression.h
	actions/expression/ExpressionParser.cpp
	actions/expression/ExpressionParser.h
	actions/expression/FunctionManager.cpp
	actions/expression/FunctionManager.h
	actions/expression/ParseContext.h
	actions/expression/ProgramVariables.cpp
	actions/expression/ProgramVariables.h
	actions/expression/TypeDefinition.cpp
	actions/expression/TypeDefinition.h
	actions/expression/Value.cpp
	actions/expression/Value.h
)

add_file_folder("Actions\\\\Expression\\\\Nodes"
	actions/expression/nodes/AbstractExpression.cpp
	actions/expression/nodes/AbstractExpression.h
	actions/expression/nodes/FunctionCallExpression.cpp
	actions/expression/nodes/FunctionCallExpression.h
	actions/expression/nodes/LiteralExpression.cpp
	actions/expression/nodes/LiteralExpression.h
	actions/expression/nodes/RandomRangeExpression.cpp
	actions/expression/nodes/RandomRangeExpression.h
	actions/expression/nodes/VariableReferenceExpression.cpp
	actions/expression/nodes/VariableReferenceExpression.h
	actions/expression/nodes/VectorConstructorExpression.cpp
	actions/expression/nodes/VectorConstructorExpression.h
)

add_file_folder("Actions\\\\Types"
	actions/types/MoveToSubmodel.cpp
	actions/types/MoveToSubmodel.h
	actions/types/ParticleEffectAction.cpp
	actions/types/ParticleEffectAction.h
	actions/types/PlaySoundAction.cpp
	actions/types/PlaySoundAction.h
	actions/types/SetDirectionAction.cpp
	actions/types/SetDirectionAction.h
	actions/types/SetPositionAction.cpp
	actions/types/SetPositionAction.h
	actions/types/WaitAction.cpp
	actions/types/WaitAction.h
)

# AI files
add_file_folder("AI"
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
add_file_folder("Anim"
	anim/animplay.cpp
	anim/animplay.h
	anim/packunpack.cpp
	anim/packunpack.h
)

# Asteroid files
add_file_folder("Asteroid"
	asteroid/asteroid.cpp
	asteroid/asteroid.h
)

# Autopilot files
add_file_folder("Autopilot"
	autopilot/autopilot.cpp
	autopilot/autopilot.h
)

# Bmpman files
add_file_folder("Bmpman"
	bmpman/bm_internal.h
	bmpman/bmpman.cpp
	bmpman/bmpman.h
)

# Camera files
add_file_folder("Camera"
	camera/camera.cpp
	camera/camera.h
)

# CFile files
add_file_folder("CFile"
	cfile/cfile.cpp
	cfile/cfile.h
	cfile/cfilearchive.cpp
	cfile/cfilearchive.h
	cfile/cfilelist.cpp
	cfile/cfilesystem.cpp
	cfile/cfilesystem.h
	cfile/cfilecompression.cpp
	cfile/cfilecompression.h
)

# Cmdline files
add_file_folder("Cmdline"
	cmdline/cmdline.cpp
	cmdline/cmdline.h
)

# CMeasure files
add_file_folder("CMeasure"
	cmeasure/cmeasure.cpp
	cmeasure/cmeasure.h
)

# ControlConfig files
add_file_folder("ControlConfig"
	controlconfig/controlsconfig.cpp
	controlconfig/controlsconfig.h
	controlconfig/controlsconfigcommon.cpp
	controlconfig/presets.cpp
	controlconfig/presets.h
)

# Cutscene files
add_file_folder("Cutscene"
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

if (FSO_BUILD_WITH_FFMPEG)
	# Cutscene\ffmpeg files
	add_file_folder("Cutscene\\\\ffmpeg"
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
endif()

# ddsutils files
add_file_folder("ddsutils"
	ddsutils/ddsutils.cpp
	ddsutils/ddsutils.h
)

# Debris files
add_file_folder("Debris"
	debris/debris.cpp
	debris/debris.h
)

# DebugConsole files
add_file_folder("DebugConsole"
	debugconsole/console.cpp
	debugconsole/console.h
	debugconsole/consolecmds.cpp
	debugconsole/consoleparse.cpp
	debugconsole/consoleparse.h
)

add_file_folder("Decals"
	decals/decals.cpp
	decals/decals.h
)

add_file_folder("Default files"
	def_files/def_files.h
	def_files/def_files.cpp
	${file_root_def_files}
)

add_file_folder("Default files\\\\data"
)

add_file_folder("Default files\\\\data\\\\effects"
	def_files/data/effects/batched-f.sdr
	def_files/data/effects/batched-v.sdr
	def_files/data/effects/bloom-comp-f.sdr
	def_files/data/effects/blur-f.sdr
	def_files/data/effects/brightpass-f.sdr
	def_files/data/effects/decal-f.sdr
	def_files/data/effects/decal-v.sdr
	def_files/data/effects/deferred-clear-f.sdr
	def_files/data/effects/deferred-clear-v.sdr
	def_files/data/effects/deferred-f.sdr
	def_files/data/effects/deferred-v.sdr
	def_files/data/effects/effect-distort-f.sdr
	def_files/data/effects/effect-distort-v.sdr
	def_files/data/effects/effect-f.sdr
	def_files/data/effects/effect-g.sdr
	def_files/data/effects/effect-v.sdr
	def_files/data/effects/fog-f.sdr
	def_files/data/effects/fxaa-f.sdr
	def_files/data/effects/fxaa-v.sdr
	def_files/data/effects/fxaapre-f.sdr
	def_files/data/effects/gamma.sdr
	def_files/data/effects/lighting.sdr
	def_files/data/effects/ls-f.sdr
	def_files/data/effects/main-f.sdr
	def_files/data/effects/main-g.sdr
	def_files/data/effects/main-v.sdr
	def_files/data/effects/nanovg-f.sdr
	def_files/data/effects/nanovg-v.sdr
	def_files/data/effects/normals.sdr
	def_files/data/effects/passthrough-f.sdr
	def_files/data/effects/passthrough-v.sdr
	def_files/data/effects/post-f.sdr
	def_files/data/effects/post-v.sdr
	def_files/data/effects/rocketui-f.sdr
	def_files/data/effects/rocketui-v.sdr
	def_files/data/effects/shadows.sdr
	def_files/data/effects/shield-impact-v.sdr
	def_files/data/effects/shield-impact-f.sdr
	def_files/data/effects/SMAA.sdr
	def_files/data/effects/smaa-blend-v.sdr
	def_files/data/effects/smaa-blend-f.sdr
	def_files/data/effects/smaa-edge-v.sdr
	def_files/data/effects/smaa-edge-f.sdr
	def_files/data/effects/smaa-neighbour-v.sdr
	def_files/data/effects/smaa-neighbour-f.sdr
	def_files/data/effects/tonemapping-f.sdr
	def_files/data/effects/video-f.sdr
	def_files/data/effects/video-v.sdr
)

add_file_folder("Default files\\\\data\\\\maps"
	def_files/data/maps/app_icon.png
	def_files/data/maps/app_icon_d.png
	def_files/data/maps/app_icon_sse.png
	def_files/data/maps/app_icon_sse_d.png
)

add_file_folder("Default files\\\\data\\\\scripts"
	def_files/data/scripts/cfile_require.lua
)

add_file_folder("Default files\\\\data\\\\tables"
	def_files/data/tables/autopilot.tbl
	def_files/data/tables/controlconfigdefaults.tbl
	def_files/data/tables/fonts.tbl
	def_files/data/tables/game_settings.tbl
	def_files/data/tables/iff_defs.tbl
	def_files/data/tables/objecttypes.tbl
	def_files/data/tables/post_processing.tbl
	def_files/data/tables/species_defs.tbl
)

# These files will be included in the executable but not in CFile
add_file_folder("Default files\\\\builtin"
	def_files/ai_profiles.tbl
)

# Variable for all embedded files
set(default_files_files
	${files_Default_files_data}
	${files_Default_files_data_effects}
	${files_Default_files_data_maps}
	${files_Default_files_data_scripts}
	${files_Default_files_data_tables}
	${files_Default_files_builtin}
)

add_file_folder("Events"
	events/events.cpp
	events/events.h
)

# ExceptionHandler files
add_file_folder("ExceptionHandler"
	exceptionhandler/exceptionhandler.cpp
	exceptionhandler/exceptionhandler.h
)

# Executor files
add_file_folder("Executor"
	executor/CombinedExecutionContext.cpp
	executor/CombinedExecutionContext.h
	executor/Executor.cpp
	executor/Executor.h
	executor/GameStateExecutionContext.cpp
	executor/GameStateExecutionContext.h
	executor/global_executors.cpp
	executor/global_executors.h
	executor/IExecutionContext.cpp
	executor/IExecutionContext.h
)

# ExternalDLL files
add_file_folder("ExternalDLL"
	external_dll/externalcode.h
)

# Fireball files
add_file_folder("Fireball"
	fireball/fireballs.cpp
	fireball/fireballs.h
	fireball/warpineffect.cpp
)

# GameHelp files
add_file_folder("GameHelp"
	gamehelp/contexthelp.cpp
	gamehelp/contexthelp.h
	gamehelp/gameplayhelp.cpp
	gamehelp/gameplayhelp.h
)

# GameSequence files
add_file_folder("GameSequence"
	gamesequence/gamesequence.cpp
	gamesequence/gamesequence.h
)

# GameSnd files
add_file_folder("GameSnd"
	gamesnd/eventmusic.cpp
	gamesnd/eventmusic.h
	gamesnd/gamesnd.cpp
	gamesnd/gamesnd.h
)

add_file_folder("Generated Files"
	${GENERATED_SOURCE_DIR}/project.h
	${PLATFORM_CHECK_HEADER}
)

# GlobalIncs files
add_file_folder("GlobalIncs"
	globalincs/alphacolors.cpp
	globalincs/alphacolors.h
	globalincs/crashdump.cpp
	globalincs/crashdump.h
	globalincs/fsmemory.h
	globalincs/globals.h
	globalincs/linklist.h
	globalincs/pstypes.h
	globalincs/safe_strings.cpp
	globalincs/safe_strings.h
	globalincs/systemvars.cpp
	globalincs/systemvars.h
	globalincs/toolchain.h
	globalincs/undosys.cpp
	globalincs/undosys.h
	globalincs/version.cpp
	globalincs/version.h
	globalincs/vmallocator.h
	globalincs/scp_defines.h
	globalincs/flagset.h
)

IF (WIN32)
	add_file_folder("GlobalIncs"
		${file_root_globalincs}
		globalincs/mspdb_callstack.cpp
		globalincs/mspdb_callstack.h
		globalincs/windebug.cpp
	)
ENDIF(WIN32)

add_file_folder("GlobalIncs\\\\Memory"
	globalincs/memory/memory.h
	globalincs/memory/memory.cpp
	globalincs/memory/utils.h
)

add_file_folder("GlobalIncs\\\\Toolchain"
	globalincs/toolchain/clang.h
	globalincs/toolchain/doxygen.h
	globalincs/toolchain/gcc.h
	globalincs/toolchain/mingw.h
	globalincs/toolchain/msvc.h
)

# Graphics files
add_file_folder("Graphics"
	graphics/2d.cpp
	graphics/2d.h
	graphics/decal_draw_list.cpp
	graphics/decal_draw_list.h
	graphics/grbatch.cpp
	graphics/grbatch.h
	graphics/grinternal.cpp
	graphics/grinternal.h
	graphics/light.cpp
	graphics/light.h
	graphics/line_draw_list.cpp
	graphics/line_draw_list.h
	graphics/material.cpp
	graphics/material.h
	graphics/matrix.cpp
	graphics/matrix.h
	graphics/post_processing.cpp
	graphics/post_processing.h
	graphics/render.cpp
	graphics/render.h
	graphics/shadows.cpp
	graphics/shadows.h
	graphics/tmapper.h
	graphics/uniforms.cpp
	graphics/uniforms.h
)

if (FSO_BUILD_WITH_OPENGL)
	# Graphics -> OpenGLGr files
	add_file_folder("Graphics\\\\OpenGLGr"
	)

	# Graphics -> OpenGLGr -> OpenGL CPPs files
	add_file_folder("Graphics\\\\OpenGLGr\\\\OpenGL CPPs"
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
	add_file_folder("Graphics\\\\OpenGLGr\\\\OpenGL Headers"
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
		graphics/opengl/SmaaAreaTex.h
		graphics/opengl/SmaaSearchTex.h
	)
endif()

# Graphics -> Paths
add_file_folder("Graphics\\\\Paths"
	graphics/paths/NanoVGRenderer.cpp
	graphics/paths/NanoVGRenderer.h
	graphics/paths/PathRenderer.cpp
	graphics/paths/PathRenderer.h
)

# Graphics -> Paths
add_file_folder("Graphics\\\\Paths\\\\nanovg"
	graphics/paths/nanovg/fontstash.h
	graphics/paths/nanovg/nanovg.c
	graphics/paths/nanovg/nanovg.h
	graphics/paths/nanovg/stb_image.h
	graphics/paths/nanovg/stb_truetype.h
)

# Graphics -> SoftwareGr files
add_file_folder("Graphics\\\\SoftwareGr"
	graphics/generic.cpp
	graphics/generic.h
	graphics/grstub.cpp
	graphics/grstub.h
	graphics/line.h
	graphics/font.h
)


add_file_folder("Graphics\\\\SoftwareGr\\\\Font"
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

add_file_folder("Graphics\\\\Util"
	graphics/util/GPUMemoryHeap.cpp
	graphics/util/GPUMemoryHeap.h
	graphics/util/uniform_structs.h
	graphics/util/UniformAligner.h
	graphics/util/UniformAligner.cpp
	graphics/util/UniformBuffer.h
	graphics/util/UniformBuffer.cpp
	graphics/util/UniformBufferManager.h
	graphics/util/UniformBufferManager.cpp
)

if (FSO_BUILD_WITH_VULKAN)
	add_file_folder("Graphics\\\\Vulkan"
		graphics/vulkan/gr_vulkan.cpp
		graphics/vulkan/gr_vulkan.h
		graphics/vulkan/RenderFrame.cpp
		graphics/vulkan/RenderFrame.h
		graphics/vulkan/vulkan_stubs.cpp
		graphics/vulkan/vulkan_stubs.h
		graphics/vulkan/VulkanRenderer.cpp
		graphics/vulkan/VulkanRenderer.h
	)
endif()

# HeadTracking files
add_file_folder("HeadTracking"
	headtracking/headtracking_internal.h
	headtracking/headtracking.h
	headtracking/headtracking.cpp
)
if(WIN32)
	add_file_folder("HeadTracking"
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
add_file_folder("Hud"
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
	hud/hudscripting.cpp
	hud/hudscripting.h
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
add_file_folder("iff_defs"
	iff_defs/iff_defs.cpp
	iff_defs/iff_defs.h
)

# InetFile files
add_file_folder("InetFile"
	inetfile/cftp.cpp
	inetfile/cftp.h
	inetfile/chttpget.cpp
	inetfile/chttpget.h
	inetfile/inetgetfile.cpp
	inetfile/inetgetfile.h
)

# Io files
add_file_folder("Io"
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
add_file_folder("jpgutils"
	jpgutils/jpgutils.cpp
	jpgutils/jpgutils.h
)

# JumpNode files
add_file_folder("JumpNode"
	jumpnode/jumpnode.cpp
	jumpnode/jumpnode.h
)

# Lab files
add_file_folder("Lab"
	lab/wmcgui.cpp
	lab/wmcgui.h
	lab/labv2.h
	lab/labv2_internal.h
	lab/labv2.cpp
)

add_file_folder("Lab\\\\Dialogs"
	lab/dialogs/lab_dialog.h
	lab/dialogs/ship_classes.h
	lab/dialogs/ship_classes.cpp
	lab/dialogs/weapon_classes.h
	lab/dialogs/weapon_classes.cpp
	lab/dialogs/class_descriptions.h
	lab/dialogs/class_descriptions.cpp
	lab/dialogs/class_options.h
	lab/dialogs/class_options.cpp
	lab/dialogs/class_variables.h
	lab/dialogs/class_variables.cpp
	lab/dialogs/render_options.h
	lab/dialogs/render_options.cpp
	lab/dialogs/backgrounds.h
	lab/dialogs/backgrounds.cpp
	lab/dialogs/actions.h
	lab/dialogs/actions.cpp
)

add_file_folder("Lab\\\\Manager"
	lab/manager/lab_manager.h
	lab/manager/lab_manager.cpp
)

add_file_folder("Lab\\\\Renderer"
	lab/renderer/lab_renderer.h
	lab/renderer/lab_renderer.cpp
	lab/renderer/lab_cameras.h
	lab/renderer/lab_cameras.cpp
)

add_file_folder("Libs"
	libs/jansson.cpp
	libs/jansson.h
)

add_file_folder("Libs\\\\AntLR"
	libs/antlr/ErrorListener.cpp
	libs/antlr/ErrorListener.h
	)

add_file_folder("Libs\\\\Discord"
	libs/discord/discord.cpp
	libs/discord/discord.h
)

if (FSO_BUILD_WITH_FFMPEG)
	add_file_folder("Libs\\\\FFmpeg"
		libs/ffmpeg/FFmpeg.cpp
		libs/ffmpeg/FFmpeg.h
		libs/ffmpeg/FFmpegContext.cpp
		libs/ffmpeg/FFmpegContext.h
		libs/ffmpeg/FFmpegHeaders.h
	)
endif()

add_file_folder("Libs\\\\RenderDoc"
	libs/renderdoc/renderdoc.cpp
	libs/renderdoc/renderdoc.h
	libs/renderdoc/renderdoc_app.h
)

# Lighting files
add_file_folder("Lighting"
	lighting/lighting.cpp
	lighting/lighting.h
	lighting/lighting_profiles.cpp
	lighting/lighting_profiles.h
)

# Localization files
add_file_folder("Localization"
	localization/fhash.cpp
	localization/fhash.h
	localization/localize.cpp
	localization/localize.h
)

# Math files
add_file_folder("Math"
	math/bitarray.h
	math/fix.cpp
	math/fix.h
	math/floating.cpp
	math/floating.h
	math/fvi.cpp
	math/fvi.h
        math/ik_solver.cpp
        math/ik_solver.h
	math/spline.cpp
	math/spline.h
	math/staticrand.cpp
	math/staticrand.h
	math/vecmat.cpp
	math/vecmat.h
)

# MenuUI files
add_file_folder("MenuUI"
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
add_file_folder("Mission"
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
add_file_folder("MissionUI"
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
add_file_folder("mod_table"
	mod_table/mod_table.cpp
	mod_table/mod_table.h
)

# Model files
add_file_folder("Model"
	model/model.h
	model/modelanimation.cpp
	model/modelanimation.h
	model/modelanimation_moveables.cpp
	model/modelanimation_moveables.h
	model/modelanimation_segments.cpp
	model/modelanimation_segments.h
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
add_file_folder("Nebula"
	nebula/neb.cpp
	nebula/neb.h
	nebula/neblightning.cpp
	nebula/neblightning.h
)

# Network files
add_file_folder("Network"
	network/chat_api.cpp
	network/chat_api.h
	network/gtrack.cpp
	network/gtrack.h
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
	network/multi_fstracker.cpp
	network/multi_fstracker.h
	network/multi_ingame.cpp
	network/multi_ingame.h
	network/multi_kick.cpp
	network/multi_kick.h
	network/multi_log.cpp
	network/multi_log.h
	network/multi_mdns.cpp
	network/multi_mdns.h
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
	network/multi_portfwd.cpp
	network/multi_portfwd.h
	network/multi_pxo.cpp
	network/multi_pxo.h
	network/multi_rate.cpp
	network/multi_rate.h
	network/multi_respawn.cpp
	network/multi_respawn.h
	network/multi_sexp.cpp
	network/multi_sexp.h
	network/multi_sw.cpp
	network/multi_sw.h
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
	network/ptrack.cpp
	network/ptrack.h
	network/stand_gui.h
	network/valid.cpp
	network/valid.h
)

IF(WIN32)
add_file_folder("Network"
	${file_root_network}
	network/stand_gui.cpp
)
ELSE(WIN32)
add_file_folder("Network"
	${file_root_network}
	network/stand_gui-unix.cpp
)
ENDIF(WIN32)

# Object files
add_file_folder("Object"
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
add_file_folder("Observer"
	observer/observer.cpp
	observer/observer.h
)

add_file_folder("Options"
	options/Option.cpp
	options/Option.h
	options/OptionsManager.cpp
	options/OptionsManager.h
)

# OsApi files
add_file_folder("OsApi"
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
add_file_folder("Parse"
	parse/encrypt.cpp
	parse/encrypt.h
	parse/generic_log.cpp
	parse/generic_log.h
	parse/parselo.cpp
	parse/parselo.h
	parse/sexp.cpp
	parse/sexp.h
	parse/sexp_container.cpp
	parse/sexp_container.h
)

add_file_folder("Parse\\\\SEXP"
	parse/sexp/DynamicSEXP.cpp
	parse/sexp/DynamicSEXP.h
	parse/sexp/EngineSEXP.cpp
	parse/sexp/EngineSEXP.h
	parse/sexp/LuaSEXP.cpp
	parse/sexp/LuaSEXP.h
	parse/sexp/sexp_lookup.cpp
	parse/sexp/sexp_lookup.h
	parse/sexp/SEXPParameterExtractor.cpp
	parse/sexp/SEXPParameterExtractor.h
)

# Particle files
add_file_folder("Particle"
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

add_file_folder("Particle\\\\Effects"
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
	particle/effects/VolumeEffect.cpp
	particle/effects/VolumeEffect.h
)

add_file_folder("Particle\\\\Util"
	particle/util/EffectTiming.cpp
	particle/util/EffectTiming.h
	particle/util/ParticleProperties.cpp
	particle/util/ParticleProperties.h
)

# PcxUtils files
add_file_folder("PcxUtils"
	pcxutils/pcxutils.cpp
	pcxutils/pcxutils.h
)

# Physics files
add_file_folder("Physics"
	physics/physics.cpp
	physics/physics.h
)

# PilotFile files
add_file_folder("PilotFile"
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
add_file_folder("Playerman"
	playerman/managepilot.cpp
	playerman/managepilot.h
	playerman/player.h
	playerman/playercontrol.cpp
)

# pngutils files
add_file_folder("pngutils"
	pngutils/pngutils.cpp
	pngutils/pngutils.h
)

# Popup files
add_file_folder("Popup"
	popup/popup.cpp
	popup/popup.h
	popup/popupdead.cpp
	popup/popupdead.h
)

# Radar files
add_file_folder("Radar"
	radar/radar.cpp
	radar/radar.h
	radar/radardradis.cpp
	radar/radardradis.h
	radar/radarngon.cpp
	radar/radarngon.h
	radar/radarorb.cpp
	radar/radarorb.h
	radar/radarsetup.cpp
	radar/radarsetup.h
)

# Render files
add_file_folder("Render"
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

add_file_folder("ScpUi"
	scpui/IncludeNodeHandler.cpp
	scpui/IncludeNodeHandler.h
	scpui/rocket_ui.cpp
	scpui/rocket_ui.h
	scpui/RocketFileInterface.cpp
	scpui/RocketFileInterface.h
	scpui/RocketLuaSystemInterface.cpp
	scpui/RocketLuaSystemInterface.h
	scpui/RocketRenderingInterface.cpp
	scpui/RocketRenderingInterface.h
	scpui/RocketSystemInterface.cpp
	scpui/RocketSystemInterface.h
	scpui/SoundPlugin.cpp
	scpui/SoundPlugin.h
)

add_file_folder("ScpUi\\\\Elements"
	scpui/elements/AnimationElement.cpp
	scpui/elements/AnimationElement.h
	scpui/elements/ScrollingTextElement.cpp
	scpui/elements/ScrollingTextElement.h
)

add_file_folder("Scripting"
	scripting/ade.cpp
	scripting/ade.h
	scripting/ade_api.h
	scripting/ade_args.cpp
	scripting/ade_args.h
	scripting/ade_doc.cpp
	scripting/ade_doc.h
	scripting/doc_html.cpp
	scripting/doc_html.h
	scripting/doc_json.cpp
	scripting/doc_json.h
	scripting/doc_parser.cpp
	scripting/doc_parser.h
	scripting/hook_api.cpp
	scripting/hook_api.h
	scripting/lua.cpp
	scripting/scripting.cpp
	scripting/scripting.h
	scripting/scripting_doc.h
)

add_file_folder("Scripting\\\\Util"
	scripting/util/LuaValueDeserializer.cpp
	scripting/util/LuaValueDeserializer.h
	scripting/util/LuaValueSerializer.cpp
	scripting/util/LuaValueSerializer.h
)

add_file_folder("Scripting\\\\Api"
	scripting/api/LuaCoroutineRunner.cpp
	scripting/api/LuaCoroutineRunner.h
	scripting/api/LuaEventCallback.cpp
	scripting/api/LuaEventCallback.h
	scripting/api/LuaExecutionContext.cpp
	scripting/api/LuaExecutionContext.h
	scripting/api/LuaPromise.cpp
	scripting/api/LuaPromise.h
)

add_file_folder("Scripting\\\\Api\\\\Libs"
	scripting/api/libs/async.cpp
	scripting/api/libs/async.h
	scripting/api/libs/audio.cpp
	scripting/api/libs/audio.h
	scripting/api/libs/base.cpp
	scripting/api/libs/base.h
	scripting/api/libs/bitops.cpp
	scripting/api/libs/bitops.h
	scripting/api/libs/cfile.cpp
	scripting/api/libs/cfile.h
	scripting/api/libs/engine.cpp
	scripting/api/libs/engine.h
	scripting/api/libs/graphics.cpp
	scripting/api/libs/graphics.h
	scripting/api/libs/hookvars.cpp
	scripting/api/libs/hookvars.h
	scripting/api/libs/hud.cpp
	scripting/api/libs/hud.h
	scripting/api/libs/mission.cpp
	scripting/api/libs/mission.h
	scripting/api/libs/options.cpp
	scripting/api/libs/options.h
	scripting/api/libs/parse.cpp
	scripting/api/libs/parse.h
	scripting/api/libs/tables.cpp
	scripting/api/libs/tables.h
	scripting/api/libs/testing.cpp
	scripting/api/libs/testing.h
    scripting/api/libs/time_lib.cpp
    scripting/api/libs/time_lib.h
	scripting/api/libs/utf8.cpp
	scripting/api/libs/utf8.h
	scripting/api/libs/ui.cpp
	scripting/api/libs/ui.h
)

add_file_folder("Scripting\\\\Api\\\\Objs"
	scripting/api/objs/asteroid.cpp
	scripting/api/objs/asteroid.h
	scripting/api/objs/audio_stream.cpp
	scripting/api/objs/audio_stream.h
	scripting/api/objs/background_element.cpp
	scripting/api/objs/background_element.h
	scripting/api/objs/beam.cpp
	scripting/api/objs/beam.h
	scripting/api/objs/bytearray.cpp
	scripting/api/objs/bytearray.h
	scripting/api/objs/camera.cpp
	scripting/api/objs/camera.h
	scripting/api/objs/cmd_brief.cpp
	scripting/api/objs/cmd_brief.h
	scripting/api/objs/cockpit_display.cpp
	scripting/api/objs/cockpit_display.h
	scripting/api/objs/color.cpp
	scripting/api/objs/color.h
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
	scripting/api/objs/execution_context.cpp
	scripting/api/objs/execution_context.h
	scripting/api/objs/executor.cpp
	scripting/api/objs/executor.h
	scripting/api/objs/eye.cpp
	scripting/api/objs/eye.h
	scripting/api/objs/file.cpp
	scripting/api/objs/file.h
	scripting/api/objs/fireballclass.cpp
	scripting/api/objs/fireballclass.h
	scripting/api/objs/fireball.cpp
	scripting/api/objs/fireball.h
	scripting/api/objs/font.cpp
	scripting/api/objs/font.h
	scripting/api/objs/gameevent.cpp
	scripting/api/objs/gameevent.h
	scripting/api/objs/gamestate.cpp
	scripting/api/objs/gamestate.h
	scripting/api/objs/hudgauge.cpp
	scripting/api/objs/hudgauge.h
	scripting/api/objs/intelentry.cpp
	scripting/api/objs/intelentry.h
	scripting/api/objs/LuaSEXP.cpp
	scripting/api/objs/LuaSEXP.h
	scripting/api/objs/mc_info.cpp
	scripting/api/objs/mc_info.h
	scripting/api/objs/message.cpp
	scripting/api/objs/message.h
	scripting/api/objs/model.cpp
	scripting/api/objs/model.h
	scripting/api/objs/model_path.cpp
	scripting/api/objs/model_path.h
	scripting/api/objs/movie_player.cpp
	scripting/api/objs/movie_player.h
	scripting/api/objs/object.cpp
	scripting/api/objs/object.h
	scripting/api/objs/option.cpp
	scripting/api/objs/option.h
	scripting/api/objs/order.cpp
	scripting/api/objs/order.h
	scripting/api/objs/oswpt.cpp
	scripting/api/objs/oswpt.h
	scripting/api/objs/parse_object.cpp
	scripting/api/objs/parse_object.h
	scripting/api/objs/particle.cpp
	scripting/api/objs/particle.h
	scripting/api/objs/physics_info.cpp
	scripting/api/objs/physics_info.h
	scripting/api/objs/player.cpp
	scripting/api/objs/player.h
	scripting/api/objs/promise.cpp
	scripting/api/objs/promise.h
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
    scripting/api/objs/time_obj.cpp
    scripting/api/objs/time_obj.h
	scripting/api/objs/tracing_category.cpp
	scripting/api/objs/tracing_category.h
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

add_file_folder("Scripting\\\\Lua"
	scripting/lua/LuaArgs.cpp
	scripting/lua/LuaArgs.h
	scripting/lua/LuaConvert.cpp
	scripting/lua/LuaConvert.h
	scripting/lua/LuaException.h
	scripting/lua/LuaFunction.cpp
	scripting/lua/LuaFunction.h
	scripting/lua/LuaHeaders.h
	scripting/lua/LuaReference.cpp
	scripting/lua/LuaReference.h
	scripting/lua/LuaTable.cpp
	scripting/lua/LuaTable.h
	scripting/lua/LuaThread.cpp
	scripting/lua/LuaThread.h
	scripting/lua/LuaTypes.h
	scripting/lua/LuaUtil.cpp
	scripting/lua/LuaUtil.h
	scripting/lua/LuaValue.cpp
	scripting/lua/LuaValue.h
)

# Ship files
add_file_folder("Ship"
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
add_file_folder("Sound"
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
	sound/IAudioFile.h
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

if (FSO_BUILD_WITH_FFMPEG)
	# Sound -> ffmpeg files
	add_file_folder("Sound\\\\FFmpeg"
		sound/ffmpeg/FFmpegAudioReader.cpp
		sound/ffmpeg/FFmpegAudioReader.h
		sound/ffmpeg/FFmpegWaveFile.cpp
		sound/ffmpeg/FFmpegWaveFile.h
	)
endif()

# Species_Defs files
add_file_folder("Species_Defs"
	species_defs/species_defs.cpp
	species_defs/species_defs.h
)

# Starfield files
add_file_folder("Starfield"
	starfield/nebula.cpp
	starfield/nebula.h
	starfield/starfield.cpp
	starfield/starfield.h
	starfield/supernova.cpp
	starfield/supernova.h
)

# Stats files
add_file_folder("Stats"
	stats/medals.cpp
	stats/medals.h
	stats/scoring.cpp
	stats/scoring.h
	stats/stats.cpp
	stats/stats.h
)

# TgaUtils files
add_file_folder("TgaUtils"
	tgautils/tgautils.cpp
	tgautils/tgautils.h
)

# Tracing files
add_file_folder("Tracing"
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
add_file_folder("Ui"
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

add_file_folder("Utils"
	utils/encoding.cpp
	utils/encoding.h
	utils/event.h
	utils/finally.h
	utils/HeapAllocator.cpp
	utils/HeapAllocator.h
	utils/id.h
	utils/join_string.h
	utils/Random.cpp
	utils/Random.h
	utils/RandomRange.h
	utils/string_utils.cpp
	utils/string_utils.h
	utils/strings.h
	utils/tuples.h
	utils/unicode.cpp
	utils/unicode.h
)

# Utils files
add_file_folder("Utils\\\\boost"
	utils/boost/hash_combine.h
	utils/boost/syncboundedqueue.h
)

# Weapon files
add_file_folder("Weapon"
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
add_file_folder("Windows Stubs"
	windows_stub/config.h
	windows_stub/stubs.cpp
)
