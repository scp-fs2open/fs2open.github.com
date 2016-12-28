#include "ai/ai.h"
#include "ai/aigoals.h"
#include "asteroid/asteroid.h"
#include "camera/camera.h"
#include "cfile/cfilesystem.h"
#include "cmdline/cmdline.h"
#include "cutscene/movie.h"
#include "debris/debris.h"
#include "freespace.h"
#include "gamesequence/gamesequence.h"
#include "globalincs/linklist.h"
#include "graphics/2d.h"
#include "graphics/font.h"
#include "graphics/generic.h"
#include "graphics/opengl/gropenglpostprocessing.h"
#include "headtracking/headtracking.h"
#include "hud/hudbrackets.h"
#include "hud/hudconfig.h"
#include "hud/hudescort.h"
#include "hud/hudets.h"
#include "hud/hudgauges.h"
#include "hud/hudshield.h"
#include "iff_defs/iff_defs.h"
#include "io/joy.h"
#include "io/key.h"
#include "io/mouse.h"
#include "io/timer.h"
#include "jumpnode/jumpnode.h"
#include "lighting/lighting.h"
#include "menuui/credits.h"
#include "mission/missioncampaign.h"
#include "mission/missiongoals.h"
#include "mission/missionload.h"
#include "mission/missionlog.h"
#include "mission/missionmessage.h"
#include "mission/missiontraining.h"
#include "missionui/missionbrief.h"
#include "model/model.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "object/objectshield.h"
#include "object/waypoint.h"
#include "scripting/lua.h"
#include "parse/parselo.h"
#include "scripting/scripting.h"
#include "particle/particle.h"
#include "playerman/player.h"
#include "render/3d.h"
#include "render/3dinternal.h"
#include "scripting/ade_api.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "ship/shiphit.h"
#include "sound/audiostr.h"
#include "sound/ds.h"
#include "weapon/beam.h"
#include "weapon/weapon.h"

#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"

#include "scripting/api/bitops.h"
#include "scripting/api/enums.h"
#include "scripting/api/vecmath.h"
#include "scripting/api/event.h"
#include "scripting/api/file.h"
#include "scripting/api/font.h"
#include "scripting/api/gameevent.h"
#include "scripting/api/gamestate.h"
#include "scripting/api/hudgauge.h"
#include "scripting/api/eye.h"
#include "scripting/api/model.h"
#include "scripting/api/physics_info.h"
#include "scripting/api/sexpvar.h"
#include "scripting/api/shields.h"
#include "scripting/api/shiptype.h"
#include "scripting/api/species.h"
#include "scripting/api/team.h"
#include "scripting/api/streaminganim.h"
#include "scripting/api/texture.h"
#include "scripting/api/texturemap.h"
#include "scripting/api/weaponclass.h"
#include "scripting/api/mc_info.h"
#include "scripting/api/object.h"
#include "scripting/api/asteroid.h"
#include "scripting/api/cockpit_display.h"
#include "scripting/api/shipclass.h"
#include "scripting/api/debris.h"
#include "scripting/api/waypoint.h"
#include "scripting/api/ship_bank.h"
#include "scripting/api/subsystem.h"
#include "scripting/api/order.h"
#include "scripting/api/ship.h"
#include "scripting/api/sound.h"
#include "scripting/api/message.h"
#include "scripting/api/wing.h"
#include "scripting/api/beam.h"
#include "scripting/api/player.h"
#include "scripting/api/camera.h"
#include "scripting/api/control_info.h"
#include "scripting/api/particle.h"
#include "scripting/api/audio.h"

using namespace scripting;
using namespace scripting::api;

int ade_set_object_with_breed(lua_State *L, int obj_idx);

//**********Handles
/*ade_obj<int> l_Camera("camera", "Camera handle");
ade_obj<int> l_Cmission("cmission", "Campaign mission handle"); //WMC - We can get away with a pointer right now, but if it ever goes dynamic, it'd be a prob
ade_obj<enum_h> l_Enum("enumeration", "Enumeration object");
ade_obj<int> l_Event("event", "Mission event handle");
ade_obj<int> l_Font("font", "font handle");
ade_obj<matrix_h> l_Matrix("orientation", "Orientation matrix object");
ade_obj<int> l_Model("model", "3D Model (POF) handle");
ade_obj<object_h> l_Object("object", "Object handle");
ade_obj<physics_info_h> l_Physics("physics", "Physics handle");
ade_obj<int> l_Player("player", "Player handle");
ade_obj<object_h> l_Shields("shields", "Shields handle");
ade_obj<object_h> l_Ship("ship", "Ship handle", &l_Object);
ade_obj<int> l_Shipclass("shipclass", "Ship class handle");
ade_obj<object_h> l_ShipTextures("shiptextures", "Ship textures handle");
ade_obj<int> l_Shiptype("shiptype", "Ship type handle");
ade_obj<int> l_Species("species", "Species handle");
ade_obj<ship_subsys_h> l_Subsystem("subsystem", "Ship subsystem handle");
ade_obj<int> l_Team("team", "Team handle");
ade_obj<int> l_Texture("texture", "Texture handle");
ade_obj<int> l_Wing("wing", "Wing handle");
ade_obj<vec3d> l_Vector("vector", "Vector object");
ade_obj<object_h> l_Weapon("weapon", "Weapon handle", &l_Object);
ade_obj<ship_bank_h> l_WeaponBank("weaponbank", "Ship/subystem weapons bank handle");
ade_obj<ship_banktype_h> l_WeaponBankType("weaponbanktype", "Ship/subsystem weapons bank type handle");
ade_obj<int> l_Weaponclass("weaponclass", "Weapon class handle");
*/
//###########################################################
//########################<IMPORTANT>########################
//###########################################################
//If you are a coder who wants to add libraries, functions,
//or objects to Lua, then you want to be below this point.
//###########################################################
//########################</IMPORTANT>#######################
//###########################################################


//**********LIBRARY: Base
ade_lib l_Base("Base", NULL, "ba", "Base FreeSpace 2 functions");

ADE_FUNC(print, l_Base, "string Message", "Prints a string", NULL, NULL)
{
	mprintf(("%s", lua_tostring(L, -1)));

	return ADE_RETURN_NIL;
}

ADE_FUNC(warning, l_Base, "string Message", "Displays a FreeSpace warning (debug build-only) message with the string provided", NULL, NULL)
{
	Warning(LOCATION, "%s", lua_tostring(L, -1));

	return ADE_RETURN_NIL;
}

ADE_FUNC(error, l_Base, "string Message", "Displays a FreeSpace error message with the string provided", NULL, NULL)
{
	Error(LOCATION, "%s", lua_tostring(L, -1));

	return ADE_RETURN_NIL;
}

ADE_FUNC(createOrientation, l_Base, "[p/r1c1, b/r1c2, h/r1c3, r2c1, r2c2, r2c3, r3c1, r3c2, r3c3]", "Given 0, 3, or 9 arguments, creates an orientation object with that orientation.", "orientation", "New orientation object, or null orientation on failure")
{
	matrix m;
	int numargs = ade_get_args(L, "|fffffffff", &m.a1d[0], &m.a1d[1], &m.a1d[2], &m.a1d[3], &m.a1d[4], &m.a1d[5], &m.a1d[6], &m.a1d[7], &m.a1d[8]);
	if(!numargs)
	{
		return ade_set_args(L, "o", l_Matrix.Set( matrix_h(&vmd_identity_matrix) ));
	}
	else if(numargs == 3)
	{
		angles a = {m.a1d[0], m.a1d[1], m.a1d[2]};
		return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&a)));
	}
	else if(numargs == 9)
	{
		return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&m)));
	}

	return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));
}

ADE_FUNC(createVector, l_Base, "[x, y, z]", "Creates a vector object", "vector", "Vector object")
{
	vec3d v3 = vmd_zero_vector;
	ade_get_args(L, "|fff", &v3.xyz.x, &v3.xyz.y, &v3.xyz.z);

	return ade_set_args(L, "o", l_Vector.Set(v3));
}

ADE_FUNC(getFrametimeOverall, l_Base, NULL, "The overall frame time in seconds since the engine has started", "number", "Overall time (seconds)")
{
	return ade_set_args(L, "x", game_get_overall_frametime());
}

ADE_FUNC(getFrametime, l_Base, "[Do not adjust for time compression (Boolean)]", "Gets how long this frame is calculated to take. Use it to for animations, physics, etc to make incremental changes.", "number", "Frame time (seconds)")
{
	bool b=false;
	ade_get_args(L, "|b", &b);

	return ade_set_args(L, "f", b ? flRealframetime : flFrametime);
}

ADE_FUNC(getCurrentGameState, l_Base, "[Depth (number)]", "Gets current FreeSpace state; if a depth is specified, the state at that depth is returned. (IE at the in-game options game, a depth of 1 would give you the game state, while the function defaults to 0, which would be the options screen.", "gamestate", "Current game state at specified depth, or invalid handle if no game state is active yet")
{
	int depth = 0;
	ade_get_args(L, "|i", &depth);

	if(depth > gameseq_get_depth())
		return ade_set_args(L, "o", l_GameState.Set(gamestate_h()));

	return ade_set_args(L, "o", l_GameState.Set(gamestate_h(gameseq_get_state(depth))));
}

ADE_FUNC(getCurrentMPStatus, l_Base, "NIL", "Gets this computers current MP status", "string", "Current MP status" )
{
	if ( MULTIPLAYER_MASTER )
		return ade_set_args(L, "s", "MULTIPLAYER_MASTER");

	if ( MULTIPLAYER_HOST )
		return ade_set_args(L, "s", "MULTIPLAYER_HOST");

	if ( MULTIPLAYER_CLIENT )
		return ade_set_args(L, "s", "MULTIPLAYER_CLIENT");

	if ( MULTIPLAYER_STANDALONE )
		return ade_set_args(L, "s", "MULTIPLAYER_STANDALONE");

	return ade_set_args(L, "s", "SINGLEPLAYER");
}

ADE_FUNC(getCurrentPlayer, l_Base, NULL, "Gets a handle of the currently used player.<br><b>Note:</b> If there is no current player then the first player will be returned, check the game state to make sure you have a valid player handle.", "player", "Player handle")
{
	return ade_set_args(L, "o", l_Player.Set(Player_num));
}

ADE_FUNC(setControlMode, l_Base, "NIL or enumeration LE_*_CONTROL", "Sets the current control mode for the game.", "string", "Current control mode")
{
	enum_h *e = NULL;
	if (!(ade_get_args(L, "|o", l_Enum.GetPtr(&e)))) {
		if (lua_game_control & LGC_NORMAL)
			return ade_set_args(L, "s", "NORMAL");
		else if (lua_game_control & LGC_STEERING)
			return ade_set_args(L, "s", "STEERING");
		else if (lua_game_control & LGC_FULL)
			return ade_set_args(L, "s", "FULL");
		else
			return ade_set_error(L, "s", "");
	}

	if (!e) {
		return ade_set_error(L, "s", "");
	}

	switch (e->index) {
		case LE_NORMAL_CONTROLS:
			lua_game_control |= LGC_NORMAL;
			lua_game_control &= ~(LGC_STEERING|LGC_FULL);
			return ade_set_args(L, "s", "NORMAL CONTROLS");
		case LE_LUA_STEERING_CONTROLS:
			lua_game_control |= LGC_STEERING;
			lua_game_control &= ~(LGC_NORMAL|LGC_FULL);
			return ade_set_args(L, "s", "LUA STEERING CONTROLS");
		case LE_LUA_FULL_CONTROLS:
			lua_game_control |= LGC_FULL;
			lua_game_control &= ~(LGC_STEERING|LGC_NORMAL);
			return ade_set_args(L, "s", "LUA FULL CONTROLS");
		default:
			return ade_set_error(L, "s", "");
	}
}

ADE_FUNC(setButtonControlMode, l_Base, "NIL or enumeration LE_*_BUTTON_CONTROL", "Sets the current control mode for the game.", "string", "Current control mode")
{
	enum_h *e = NULL;
	if (!(ade_get_args(L, "|o", l_Enum.GetPtr(&e)))) {
		if (lua_game_control & LGC_B_NORMAL)
			return ade_set_args(L, "s", "NORMAL");
		else if (lua_game_control & LGC_B_OVERRIDE)
			return ade_set_args(L, "s", "OVERRIDE");
		else if (lua_game_control & LGC_B_ADDITIVE)
			return ade_set_args(L, "s", "ADDITIVE");
		else
			return ade_set_error(L, "s", "");
	}

	if (!e) {
		return ade_set_error(L, "s", "");
	}

	switch (e->index) {
		case LE_NORMAL_BUTTON_CONTROLS:
			lua_game_control |= LGC_B_NORMAL;
			lua_game_control &= ~(LGC_B_ADDITIVE|LGC_B_OVERRIDE);
			return ade_set_args(L, "s", "NORMAL BUTTON CONTROL");
		case LE_LUA_ADDITIVE_BUTTON_CONTROL:
			lua_game_control |= LGC_B_ADDITIVE;
			lua_game_control &= ~(LGC_B_NORMAL|LGC_B_OVERRIDE);
			return ade_set_args(L, "s", "LUA OVERRIDE BUTTON CONTROL");
		case LE_LUA_OVERRIDE_BUTTON_CONTROL:
			lua_game_control |= LGC_B_OVERRIDE;
			lua_game_control &= ~(LGC_B_ADDITIVE|LGC_B_NORMAL);
			return ade_set_args(L, "s", "LUA ADDITIVE BUTTON CONTROL");
		default:
			return ade_set_error(L, "s", "");
	}
}

ADE_FUNC(getControlInfo, l_Base, NULL, "Gets the control info handle.", "control info", "control info handle")
{
	return ade_set_args(L, "o", l_Control_Info.Set(1));
}

ADE_FUNC(setTips, l_Base, "True or false", "Sets whether to display tips of the day the next time the current pilot enters the mainhall.", NULL, NULL)
{
	if (Player == NULL)
		return ADE_RETURN_NIL;

	bool tips = false;

	ade_get_args(L, "b", &tips);

	if (tips)
		Player->tips = 1;
	else
		Player->tips = 0;

	return ADE_RETURN_NIL;
}

ADE_FUNC(getGameDifficulty, l_Base, NULL, "Returns the difficulty level from 1-5, 1 being the lowest, (Very Easy) and 5 being the highest (Insane)", "integer", "Difficulty level as integer")
{
	return ade_set_args(L, "i", Game_skill_level+1);
}

ADE_FUNC(postGameEvent, l_Base, "gameevent Event", "Sets current game event. Note that you can crash FreeSpace 2 by posting an event at an improper time, so test extensively if you use it.", "boolean", "True if event was posted, false if passed event was invalid")
{
	gameevent_h *gh = NULL;
	if(!ade_get_args(L, "o", l_GameEvent.GetPtr(&gh)))
		return ade_set_error(L, "b", false);

	if(!gh->IsValid())
		return ade_set_error(L, "b", false);

	if (Om_tracker_flag) 
		Multi_options_g.protocol = NET_TCP;
	psnet_use_protocol(Multi_options_g.protocol);
	
	gameseq_post_event(gh->Get());

	return ADE_RETURN_TRUE;
}

//**********SUBLIBRARY: Base/Events
ade_lib l_Base_Events("GameEvents", &l_Base, NULL, "Freespace 2 game events");

ADE_INDEXER(l_Base_Events, "number Index/string Name", "Array of game events", "gameevent", "Game event, or invalid gameevent handle if index is invalid")
{
	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_GameEvent.Set(gameevent_h()));

	int idx = gameseq_get_event_idx(name);

	if(idx < 0)
	{
		idx = atoi(name);

		//Lua-->FS2
		idx--;

		if(idx < 0 || idx >= Num_gs_event_text)
			return ade_set_error(L, "o", l_GameEvent.Set(gameevent_h()));
	}

	return ade_set_args(L, "o", l_GameEvent.Set(gameevent_h(idx)));
}

ADE_FUNC(__len, l_Base_Events, NULL, "Number of events", "number", "Number of events")
{
	return ade_set_args(L, "i", Num_gs_event_text);
}

//**********SUBLIBRARY: Base/States
ade_lib l_Base_States("GameStates", &l_Base, NULL, "Freespace 2 states");

ADE_INDEXER(l_Base_States, "number Index/string Name", "Array of game states", "gamestate", "Game state, or invalid gamestate handle if index is invalid")
{
	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_GameState.Set(gamestate_h()));

	int idx = gameseq_get_state_idx(name);

	if(idx < 0)
	{
		idx = atoi(name);

		//Lua-->FS2
		idx--;

		if(idx < 0 || idx >= Num_gs_state_text)
			return ade_set_error(L, "o", l_GameState.Set(gamestate_h()));
	}

	return ade_set_args(L, "o", l_GameState.Set(gamestate_h(idx)));
}

ADE_FUNC(__len, l_Base_States, NULL, "Number of states", "number", "Number of states")
{
	return ade_set_args(L, "i", Num_gs_state_text);
}

//**********LIBRARY: CFILE
//WMC - It's on my to-do list! (Well, if I had one anyway)
//WMC - Did it. I had to invent a to-do list first, though.
//Ironically, I never actually put this on it.
ade_lib l_CFile("CFile", NULL, "cf", "CFile FS2 filesystem access");

int l_cf_get_path_id(char* n_path)
{
	size_t i;
	size_t path_len = strlen(n_path);

	char *buf = (char*) vm_malloc((path_len+1) * sizeof(char));
	
	if (!buf) 
		return CF_TYPE_INVALID;
		
	strcpy(buf, n_path);

	//Remove trailing slashes; avoid buffer overflow on 1-char strings
	i = path_len - 1;
	while(i < std::numeric_limits<size_t>::max() && (buf[i] == '\\' || buf[i] == '/'))
		buf[i--] = '\0';

	//Remove leading slashes
	i = 0;
	while(i < path_len && (buf[i] == '\\' || buf[i] == '/'))
		buf[i++] = '\0';

	//Use official DIR_SEPARATOR_CHAR
	for(i = 0; i < path_len; i++)
	{
		if(buf[i] == '\\' || buf[i] == '/')
			buf[i] = DIR_SEPARATOR_CHAR;
	}
	for(i = 0; i < CF_MAX_PATH_TYPES; i++)
	{
		if(Pathtypes[i].path != NULL && !stricmp(buf, Pathtypes[i].path)) {
			vm_free(buf);
			buf = NULL;
			return Pathtypes[i].index;
		}
	}

	vm_free(buf);
	buf = NULL;
	return CF_TYPE_INVALID;
}

ADE_FUNC(deleteFile, l_CFile, "string Filename, string Path", "Deletes given file. Path must be specified. Use a slash for the root directory.", "boolean", "True if deleted, false")
{
	char *n_filename = NULL;
	char *n_path = "";
	if(!ade_get_args(L, "ss", &n_filename, &n_path))
		return ade_set_error(L, "b", false);

	int path = CF_TYPE_INVALID;
	if(n_path != NULL && strlen(n_path))
		path = l_cf_get_path_id(n_path);

	if(path == CF_TYPE_INVALID)
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", cf_delete(n_filename, path) != 0);
}

ADE_FUNC(fileExists, l_CFile, "string Filename, [string Path = \"\", boolean CheckVPs = false]", "Checks if a file exists. Use a blank string for path for any directory, or a slash for the root directory.", "boolean", "True if file exists, false or nil otherwise")
{
	char *n_filename = NULL;
	char *n_path = "";
	bool check_vps = false;
	if(!ade_get_args(L, "s|sb", &n_filename, &n_path, &check_vps))
		return ADE_RETURN_NIL;

	int path = CF_TYPE_ANY;
	if(n_path != NULL && strlen(n_path))
		path = l_cf_get_path_id(n_path);

	if(path == CF_TYPE_INVALID)
		return ade_set_error(L, "b", false);

	if(!check_vps)
		return ade_set_args(L, "b", cf_exists(n_filename, path) != 0);
	else
		return ade_set_args(L, "b", cf_exists_full(n_filename, path ) != 0);
}

ADE_FUNC(openFile, l_CFile, "string Filename, [string Mode=\"r\", string Path = \"\"]",
		 "Opens a file. 'Mode' uses standard C fopen arguments. Use a blank string for path for any directory, or a slash for the root directory."
		 "Be EXTREMELY CAREFUL when using this function, as you may PERMANENTLY delete any file by accident",
		 "file",
		 "File handle, or invalid file handle if the specified file couldn't be opened")
{
	char *n_filename = NULL;
	char *n_mode = "r";
	char *n_path = "";
	if(!ade_get_args(L, "s|ss", &n_filename, &n_mode, &n_path))
		return ade_set_error(L, "o", l_File.Set(NULL));

	int type = CFILE_NORMAL;

	int path = CF_TYPE_ANY;
	if(n_path != NULL && strlen(n_path))
		path = l_cf_get_path_id(n_path);

	if(path == CF_TYPE_INVALID)
		return ade_set_error(L, "o", l_File.Set(NULL));

	CFILE *cfp = cfopen(n_filename, n_mode, type, path);
	
	if(!cf_is_valid(cfp))
		return ade_set_error(L, "o", l_File.Set(NULL));

	return ade_set_args(L, "o", l_File.Set(cfp));
}

ADE_FUNC(openTempFile, l_CFile, NULL, "Opens a temp file that is automatically deleted when closed", "file", "File handle, or invalid file handle if tempfile couldn't be created")
{
	return ade_set_args(L, "o", l_File.Set(ctmpfile()));
}

ADE_FUNC(renameFile, l_CFile, "string CurrentFilename, string NewFilename, string Path", "Renames given file. Path must be specified. Use a slash for the root directory.", "boolean", "True if file was renamed, otherwise false")
{
	char *n_filename = NULL;
	char *n_new_filename = NULL;
	char *n_path = "";
	if(!ade_get_args(L, "ss|s", &n_filename, &n_new_filename, &n_path))
		return ade_set_error(L, "b", false);

	int path = CF_TYPE_INVALID;
	if(n_path != NULL && strlen(n_path))
		path = l_cf_get_path_id(n_path);

	if(path == CF_TYPE_INVALID)
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", cf_rename(n_filename, n_new_filename, path) != 0);
}

//**********LIBRARY: Controls library
ade_lib l_Mouse("Controls", NULL, "io", "Controls library");

extern int mouse_inited;

ADE_FUNC(getMouseX, l_Mouse, NULL, "Gets Mouse X pos", "number", "Mouse x position, or 0 if mouse is not initialized yet")
{
	if(!mouse_inited)
		return ade_set_error(L, "i", 0);

	int x;

	mouse_get_pos(&x, NULL);

	return ade_set_args(L, "i", x);
}

ADE_FUNC(getMouseY, l_Mouse, NULL, "Gets Mouse Y pos", "number", "Mouse y position, or 0 if mouse is not initialized yet")
{
	if(!mouse_inited)
		return ade_set_error(L, "i", 0);

	int y;

	mouse_get_pos(NULL, &y);

	return ade_set_args(L, "i", y);
}

ADE_FUNC(isMouseButtonDown, l_Mouse, "{MOUSE_*_BUTTON enumeration}, [..., ...]", "Returns whether the specified mouse buttons are up or down", "boolean", "Whether specified mouse buttons are down, or false if mouse is not initialized yet")
{
	if(!mouse_inited)
		return ade_set_error(L, "b", false);

	enum_h *e[3] = {NULL, NULL, NULL};
	ade_get_args(L, "o|oo", l_Enum.GetPtr(&e[0]), l_Enum.GetPtr(&e[1]), l_Enum.GetPtr(&e[2]));	//Like a snake!

	bool rtn = false;
	int check_flags = 0;

	for(int i = 0; i < 3; i++)
	{
		if(e[i] == NULL)
			break;

		if(e[i]->index == LE_MOUSE_LEFT_BUTTON)
			check_flags |= MOUSE_LEFT_BUTTON;
		if(e[i]->index == LE_MOUSE_MIDDLE_BUTTON)
			check_flags |= MOUSE_MIDDLE_BUTTON;
		if(e[i]->index == LE_MOUSE_RIGHT_BUTTON)
			check_flags |= MOUSE_RIGHT_BUTTON;
	}

	if(mouse_down(check_flags))
		rtn = true;

	return ade_set_args(L, "b", rtn);
}

ADE_FUNC(setCursorImage, l_Mouse, "Image filename", "Sets mouse cursor image, and allows you to lock/unlock the image. (A locked cursor may only be changed with the unlock parameter)", "boolean", "true if successful, false otherwise")
{
	using namespace io::mouse;

	if(!mouse_inited || !Gr_inited)
		return ade_set_error(L, "b", false);

	if (Is_standalone)
		return ade_set_error(L, "b", false);

	char *s = NULL;
	enum_h *u = NULL; // This isn't used anymore

	if(!ade_get_args(L, "s|o", &s, l_Enum.GetPtr(&u)))
		return ade_set_error(L, "b", false);

	Cursor* cursor = CursorManager::get()->loadCursor(s);

	if (cursor == NULL)
	{
		return ade_set_error(L, "b", false);
	}

	CursorManager::get()->setCurrentCursor(cursor);
	return ade_set_args(L, "b", true);
}

ADE_FUNC(setCursorHidden, l_Mouse, "boolean hide[, boolean grab]", "Hides the cursor when <i>hide</i> is true, otherwise shows it. <i>grab</i> determines if "
				"the mouse will be restricted to the window. Set this to true when hiding the cursor while in game. By default grab will be true when we are in the game play state, false otherwise.", NULL, NULL)
{
	if(!mouse_inited)
		return ADE_RETURN_NIL;

	bool b = false;
	bool grab = gameseq_get_state() == GS_STATE_GAME_PLAY;
	ade_get_args(L, "b|b", &b, &grab);

	io::mouse::CursorManager::get()->showCursor(!b, grab);

	return ADE_RETURN_NIL;
}

ADE_FUNC(forceMousePosition, l_Mouse, "number, number (coordinates)", "function to force mouse position", "boolean", "if the operation succeeded or not")
{
	if(!mouse_inited)
		return ADE_RETURN_FALSE;

	if(!Gr_inited)
		return ADE_RETURN_FALSE;

	int x, y;
	if (!(ade_get_args(L, "ii", &x, &y)))
		return ADE_RETURN_FALSE;

	if (!((x >= 0) && (x <= gr_screen.max_w)))
		return ADE_RETURN_FALSE;

	if (!((y >= 0) && (y <= gr_screen.max_h)))
		return ADE_RETURN_FALSE;

	mouse_set_pos(x, y);

	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(MouseControlStatus, l_Mouse, "boolean", "Gets and sets the retail mouse control status", "boolean", "if the retail mouse is on or off")
{
	if(!mouse_inited)
		return ADE_RETURN_NIL;

	bool newVal = false;
	if (!ade_get_args(L, "*|b", &newVal))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
	{
		if (newVal)
		{
			Use_mouse_to_fly = 1;
		}
		else
		{
			Use_mouse_to_fly = 0;
		}
	}

	if (Use_mouse_to_fly)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(getMouseSensitivity, l_Mouse, NULL, "Gets mouse sensitivity setting", "number", "Mouse sensitivity in range of 0-9")
{
	return ade_set_args(L, "i", Mouse_sensitivity);
}

ADE_FUNC(getJoySensitivity, l_Mouse, NULL, "Gets joystick sensitivity setting", "number", "Joystick sensitivity in range of 0-9")
{
	return ade_set_args(L, "i", Joy_sensitivity);
}

ADE_FUNC(getJoyDeadzone, l_Mouse, NULL, "Gets joystick deadzone setting", "number", "Joystick deadzone in range of 0-9")
{
	return ade_set_args(L, "i", Joy_dead_zone_size / 5);
}

//trackir funcs
ADE_FUNC(updateTrackIR, l_Mouse, NULL, "Updates Tracking Data. Call before using get functions", "boolean", "Checks if trackir is available and updates variables, returns true if successful, otherwise false")
{
	if( !headtracking::isEnabled() )
		return ADE_RETURN_FALSE;

	if (!headtracking::query())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getTrackIRPitch, l_Mouse, NULL, "Gets pitch axis from last update", "number", "Pitch value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args( L, "f", headtracking::getStatus()->pitch);
}

ADE_FUNC(getTrackIRYaw, l_Mouse, NULL, "Gets yaw axis from last update", "number", "Yaw value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->yaw);
}

ADE_FUNC(getTrackIRRoll, l_Mouse, NULL, "Gets roll axis from last update", "number", "Roll value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->roll);
}

ADE_FUNC(getTrackIRX, l_Mouse, NULL, "Gets x position from last update", "number", "X value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->x);
}

ADE_FUNC(getTrackIRY, l_Mouse, NULL, "Gets y position from last update", "number", "Y value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->y);
}

ADE_FUNC(getTrackIRZ, l_Mouse, NULL, "Gets z position from last update", "number", "Z value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled() )
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->z);
}

//**********LIBRARY: HUD library
ade_lib l_HUD("HUD", NULL, "hu", "HUD library");

ADE_VIRTVAR(HUDDrawn, l_HUD, "boolean", "Current HUD draw status", "boolean", "If the HUD is drawn or not")
{
	bool to_draw = false;

	if(!ade_get_args(L, "*|b", &to_draw))
		return ADE_RETURN_NIL;

	if(ADE_SETTING_VAR)
	{
		if (to_draw)
			HUD_draw = 1;
		else
			HUD_draw = 0;
	}

	if (HUD_draw)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(HUDDisabledExceptMessages, l_HUD, "boolean", "Specifies if only the messages gauges of the hud are drawn", "boolean", "true if only the message gauges are drawn, false otherwise")
{
	bool to_draw = false;

	if (!ade_get_args(L, "*|b", &to_draw))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
	{
		hud_disable_except_messages(to_draw);
	}

	if (hud_disabled_except_messages())
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(setHUDGaugeColor, l_HUD, "number (index number of the gauge), [integer red, number green, number blue, number alpha]", "Color used to draw the gauge", "boolean", "If the operation was successful")
{
	int idx = -1; 
	int r = 0;
	int g = 0;
	int b = 0;
	int a = 0;

	if(!ade_get_args(L, "i|iiii", &idx, &r, &g, &b, &a))
		return ADE_RETURN_FALSE;

	if ((idx < 0) || (idx >= NUM_HUD_GAUGES))
		return ADE_RETURN_FALSE;

	gr_init_alphacolor(&HUD_config.clr[idx], r, g, b, a);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getHUDGaugeColor, l_HUD, "number (index number of the gauge)", "Color used to draw the gauge", "number, number, number, number", "Red, green, blue, and alpha of the gauge")
{
	int idx = -1;

	if(!ade_get_args(L, "i", &idx))
		return ADE_RETURN_NIL;

	if ((idx < 0) || (idx >= NUM_HUD_GAUGES))
		return ADE_RETURN_NIL;

	color c = HUD_config.clr[idx];
	
	return ade_set_args(L, "iiii", (int) c.red, (int) c.green, (int) c.blue, (int) c.alpha);	
}

ADE_FUNC(getHUDGaugeHandle, l_HUD, "string Name", "Returns a handle to a specified HUD gauge", "HudGauge", "HUD Gauge handle, or nil if invalid")
{
	char* name;
	if (!ade_get_args(L, "s", &name))
		return ADE_RETURN_NIL;
	HudGauge* gauge = NULL;

	gauge = hud_get_gauge(name);

	if (gauge == NULL)
		return ADE_RETURN_NIL;
	else
		return ade_set_args(L, "o", l_HudGauge.Set(*gauge));
}

//**********LIBRARY: Graphics
ade_lib l_Graphics("Graphics", NULL, "gr", "Graphics Library");

static float lua_Opacity = 1.0f;
static int lua_Opacity_type = GR_ALPHABLEND_FILTER;

//****SUBLIBRARY: Graphics/Cameras
ade_lib l_Graphics_Cameras("Cameras", &l_Graphics, NULL, "Cameras");

ADE_INDEXER(l_Graphics_Cameras, "number Index/string Name", "Gets camera", "camera", "Ship handle, or invalid ship handle if index was invalid")
{
	char *s = NULL;
	if(!ade_get_args(L, "*s", &s))
		return ade_set_error(L, "o", l_Camera.Set(camid()));

	camid cid = cam_lookup(s);
	if(!cid.isValid())
	{
		int cn = atoi(s);
		if(cn > 0)
		{
			//Lua-->FS2
			cn--;
			cid = cam_get_camera(cn);
		}
	}

	return ade_set_args(L, "o", l_Camera.Set(cid));
}

ADE_FUNC(__len, l_Graphics_Cameras, NULL, "Gets number of cameras", "number", "Number of cameras")
{
	return ade_set_args(L, "i", (int)cam_get_num());
}

//****SUBLIBRARY: Graphics/Fonts
ade_lib l_Graphics_Fonts("Fonts", &l_Graphics, NULL, "Font library");

ADE_FUNC(__len, l_Graphics_Fonts, NULL, "Number of loaded fonts", "number", "Number of loaded fonts")
{
	return ade_set_args(L, "i", font::FontManager::numberOfFonts());
}

ADE_INDEXER(l_Graphics_Fonts, "number Index/string Filename", "Array of loaded fonts", "font", "Font handle, or invalid font handle if index is invalid")
{
	if (lua_isnumber(L, 2))
	{
		int index = -1;

		if (!ade_get_args(L, "*i", &index))
			return ade_set_error(L, "o", l_Font.Set(font_h()));

		auto realIdx = index - 1;

		if (realIdx < 0 || realIdx >= font::FontManager::numberOfFonts())
		{
			LuaError(L, "Invalid font index %d specified, must be between 1 and %d!", index, font::FontManager::numberOfFonts());
		}

		return ade_set_args(L, "o", l_Font.Set(font_h(font::FontManager::getFont(index - 1))));
	}
	else
	{
		char *s = NULL;

		if(!ade_get_args(L, "*s", &s))
			return ade_set_error(L, "o", l_Font.Set(font_h()));

		return ade_set_args(L, "o", l_Font.Set(font_h(font::FontManager::getFont(s))));
	}
}

ADE_VIRTVAR(CurrentFont, l_Graphics, "font", "Current font", "font", NULL)
{
	font_h *newFh = NULL;

	if(!ade_get_args(L, "*|o", l_Font.GetPtr(&newFh)))
		return ade_set_error(L, "o", l_Font.Set(font_h()));

	if(ADE_SETTING_VAR && newFh->isValid()) {
		font::FontManager::setCurrentFont(newFh->Get());
	}
	
	return ade_set_args(L, "o", l_Font.Set(font_h(font::FontManager::getCurrentFont())));
}

//****SUBLIBRARY: Graphics/PostEffects
ade_lib l_Graphics_Posteffects("PostEffects", &l_Graphics, NULL, "Post processing effects");

ADE_INDEXER(l_Graphics_Posteffects, "number index", "Gets the name of the specified post processing index", "string", "post processing name or empty string on error")
{
	int index = -1;
	if(!ade_get_args(L, "*i", &index))
		return ade_set_error(L, "s", "");

	index--; // Lua -> C/C++

	if (index < 0)
		return ade_set_error(L, "s", "");

	SCP_vector<SCP_string> names;
	get_post_process_effect_names(names);
	names.push_back(SCP_string("lightshafts"));

	if (index >= (int) names.size())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", const_cast<char*>(names[index].c_str()));
}

ADE_FUNC(__len, l_Graphics_Posteffects, NULL, "Gets the number or available post processing effects", "number", "number of post processing effects or 0 on error")
{
	SCP_vector<SCP_string> names;
	get_post_process_effect_names(names);

	// Add one for lightshafts
	return ade_set_args(L, "i", ((int) names.size()) + 1);
}

ADE_FUNC(setPostEffect, l_Graphics, "string name, [number value=0]", "Sets the intensity of the specified post processing effect", "boolean", "true when successful, false otherwise")
{
	char* name = NULL;
	int intensity = 0;

	if (!ade_get_args(L, "s|i", &name, &intensity))
		return ADE_RETURN_FALSE;

	if (name == NULL || intensity < 0)
		return ADE_RETURN_FALSE;

	gr_post_process_set_effect(name, intensity);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(resetPostEffects, l_Graphics, NULL, "Resets all post effects to their default values", "boolean", "true if successful, false otherwise")
{
	gr_post_process_set_defaults();

	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(CurrentOpacityType, l_Graphics, "enumeration", "Current alpha blending type; uses ALPHABLEND_* enumerations", "enumeration", NULL)
{
	enum_h *alphatype = NULL;

	if(!ade_get_args(L, "*|o", l_Enum.GetPtr(&alphatype)))
		return ADE_RETURN_NIL;

	if(ADE_SETTING_VAR)
	{
		if(alphatype != NULL && alphatype->index == LE_ALPHABLEND_FILTER)
			lua_Opacity_type = GR_ALPHABLEND_FILTER;
		else
			lua_Opacity_type = GR_ALPHABLEND_NONE;
	}

	int rtn;
	switch(lua_Opacity_type)
	{
		case GR_ALPHABLEND_FILTER:
			rtn = LE_ALPHABLEND_FILTER;
			break;
		default:
			rtn = LE_ALPHABLEND_NONE;
	}

	return ade_set_args(L, "o", l_Enum.Set(enum_h(rtn)));
}

ADE_VIRTVAR(CurrentRenderTarget, l_Graphics, "texture", "Current rendering target", "texture", "Current rendering target, or invalid texture handle if screen is render target")
{
	int newtx = -1;

	if(ADE_SETTING_VAR && lua_isnil(L, 2))
	{
		bm_set_render_target(-1);
		return ade_set_args(L, "o", l_Texture.Set(gr_screen.rendering_to_texture));
	}
	else
	{

		if(!ade_get_args(L, "*|o", l_Texture.Get(&newtx)))
			return ade_set_error(L, "o", l_Texture.Set(-1));

		if(ADE_SETTING_VAR) {
			if(newtx > -1 && bm_is_valid(newtx))
				bm_set_render_target(newtx, 0);
			else
				bm_set_render_target(-1);
		}

		return ade_set_args(L, "o", l_Texture.Set(gr_screen.rendering_to_texture));
	}
}

ADE_FUNC(clearScreen, l_Graphics, "[integer red, number green, number blue, number alpha]", "Clears the screen to black, or the color specified.", NULL, NULL)
{
	int r,g,b,a;
	r=g=b=0;
	a=255;
	ade_get_args(L, "|iiii", &r, &g, &b, &a);

	//WMC - Set to valid values
	if(r != 0 || g != 0 || b != 0 || a!= 255)
	{
		CAP(r,0,255);
		CAP(g,0,255);
		CAP(b,0,255);
		CAP(a,0,255);
		gr_set_clear_color(r,g,b);
		gr_screen.current_clear_color.alpha = (ubyte)a;
		gr_clear();
		gr_set_clear_color(0,0,0);

		return ADE_RETURN_NIL;
	}

	gr_clear();
	return ADE_RETURN_NIL;
}

ADE_FUNC(createCamera, l_Graphics,
		 "string Name, [wvector Position, orientation Orientation]",
		 "Creates a new camera using the specified position and orientation (World)",
		 "camera",
		 "Camera handle, or invalid camera handle if camera couldn't be created")
{
	char *s = NULL;
	vec3d *v = &vmd_zero_vector;
	matrix_h *mh = NULL;
	if(!ade_get_args(L, "s|oo", &s, l_Vector.GetPtr(&v), l_Matrix.GetPtr(&mh)))
		return ADE_RETURN_NIL;

	matrix *mtx = &vmd_identity_matrix;
	if(mh != NULL)
		mtx = mh->GetMatrix();
	camid cid = cam_create(s, v, mtx);

	//Set position
	return ade_set_args(L, "o", l_Camera.Set(cid));
}

ADE_FUNC(isMenuStretched, l_Graphics, NULL, "Returns whether the standard interface is stretched", "boolean", "True if stretched, false if aspect ratio is maintained")
{
	if(!Gr_inited)
		return ade_set_error(L, "b", 0);

	return ade_set_args(L, "b", Cmdline_stretch_menu);
}

ADE_FUNC(getScreenWidth, l_Graphics, NULL, "Gets screen width", "number", "Width in pixels, or 0 if graphics are not initialized yet")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.max_w);
}

ADE_FUNC(getScreenHeight, l_Graphics, NULL, "Gets screen height", "number", "Height in pixels, or 0 if graphics are not initialized yet")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.max_h);
}

ADE_FUNC(getCenterWidth, l_Graphics, NULL, "Gets width of center monitor (should be used in conjuction with getCenterOffsetX)", "number", "Width of center monitor in pixels, or 0 if graphics are not initialized yet")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.center_w);
}

ADE_FUNC(getCenterHeight, l_Graphics, NULL, "Gets height of center monitor (should be used in conjuction with getCenterOffsetY)", "number", "Height of center monitor in pixels, or 0 if graphics are not initialized yet")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.center_h);
}

ADE_FUNC(getCenterOffsetX, l_Graphics, NULL, "Gets X offset of center monitor", "number", "X offset of center monitor in pixels")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.center_offset_x);
}

ADE_FUNC(getCenterOffsetY, l_Graphics, NULL, "Gets Y offset of center monitor", "number", "Y offset of center monitor in pixels")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.center_offset_y);
}

ADE_FUNC(getCurrentCamera, l_Graphics, "[boolean]", "Gets the current camera handle, if argument is <i>true</i> then it will also return the main camera when no custom camera is in use", "camera", "camera handle or invalid handle on error")
{
	camid current;

	bool rtnMain = false;

	ade_get_args(L, "|b", &rtnMain);

	if (!rtnMain || Viewer_mode & VM_FREECAMERA)
		current = cam_get_current();
	else
		current = Main_camera;

	return ade_set_args(L, "o", l_Camera.Set(current));
}

ADE_FUNC(getVectorFromCoords, l_Graphics,
		 "[number X=center, number Y=center, number Depth, boolean normalize = false]",
		 "Returns a vector through screen coordinates x and y. "
		 "If depth is specified, vector is extended to Depth units into space"
		 "If normalize is true, vector will be normalized.",
		 "vector",
		 "Vector, or zero vector on failure")
{
	int x = gr_screen.max_w/2;
	int y = gr_screen.max_h/2;
	float depth = 0.0f;
	bool normalize = false; 
	ade_get_args(L, "|iifb", &x, &y, &depth, &normalize);

	vec3d pos = vmd_zero_vector;

	bool in_frame = g3_in_frame() > 0;
	if(!in_frame) {
		g3_start_frame(0);

		vec3d cam_pos;
		matrix cam_orient;

		camid cid;
		if (Viewer_mode & VM_FREECAMERA)
			cid = cam_get_current();
		else
			cid = Main_camera;

		camera *cam = cid.getCamera();

		if (cam != NULL) {
			cam->get_info(&cam_pos, &cam_orient);
			g3_set_view_matrix(&cam_pos, &cam_orient, View_zoom);
		} else {
			g3_set_view_matrix(&Eye_position, &Eye_matrix, View_zoom);
		}

		gr_set_proj_matrix( Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}

	g3_point_to_vec(&pos, x, y);

	if(!in_frame) {
		gr_end_view_matrix();
		gr_end_proj_matrix();
		g3_end_frame();
	}

	if(depth)
		vm_vec_scale(&pos, depth);

	if (normalize)
		vm_vec_normalize_quick(&pos);

	vm_vec_add2(&pos, &View_position);

	return ade_set_args(L, "o", l_Vector.Set(pos));
}

ADE_FUNC(setTarget, l_Graphics, "[texture Texture]",
		"If texture is specified, sets current rendering surface to a texture."
		"Otherwise, sets rendering surface back to screen.",
		"boolean",
		"True if successful, false otherwise")
{
	if(!Gr_inited)
		return ade_set_error(L, "b", false);

	int idx = -1;
	ade_get_args(L, "|o", l_Texture.Get(&idx));

	return ade_set_args(L, "b", bm_set_render_target(idx, 0));
}

ADE_FUNC(setCamera, l_Graphics, "[camera handle Camera]", "Sets current camera, or resets camera if none specified", "boolean", "true if successful, false or nil otherwise")
{
	camid cid;
	if(!ade_get_args(L, "|o", l_Camera.Get(&cid)))
	{
		cam_reset_camera();
		return ADE_RETURN_NIL;
	}

	if(!cid.isValid())
		return ADE_RETURN_NIL;

	cam_set_camera(cid);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(setColor, l_Graphics, "integer Red, number Green, number Blue, [integer Alpha]", "Sets 2D drawing color; each color number should be from 0 (darkest) to 255 (brightest)", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int r,g,b,a=255;

	if(!ade_get_args(L, "iii|i", &r, &g, &b, &a))
		return ADE_RETURN_NIL;

	color ac;
	gr_init_alphacolor(&ac,r,g,b,a);
	gr_set_color_fast(&ac);

	return ADE_RETURN_NIL;
}

ADE_FUNC(setLineWidth, l_Graphics, "[number width=1.0]", "Sets the line width for lines. This call might fail if the specified width is not supported by the graphics implementation. Then the width will be the nearest supported value.", "boolean", "true if succeeded, false otherwise")
{
	if(!Gr_inited)
		return ADE_RETURN_FALSE;

	float width = 1.0f;

	ade_get_args(L, "|f", &width);

	if (width <= 0.0f)
	{
		return ADE_RETURN_FALSE;
	}

	gr_set_line_width(width);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(drawCircle, l_Graphics, "number Radius, number X, number Y, [boolean Filled=true]", "Draws a circle", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y,ra;
	bool fill = true;

	if(!ade_get_args(L, "iii|b", &ra,&x,&y,&fill))
		return ADE_RETURN_NIL;

	if (fill) {
		//WMC - Circle takes...diameter.
		gr_circle(x,y, ra*2, GR_RESIZE_NONE);
	} else {
		gr_unfilled_circle(x,y, ra*2, GR_RESIZE_NONE);
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawArc, l_Graphics, "number Radius, number X, number Y, number StartAngle, number EndAngle, [boolean Filled=true]", "Draws an arc", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y;
	float ra,angle_start,angle_end;
	bool fill = true;

	if(!ade_get_args(L, "fiiff|b", &ra,&x,&y,&angle_start,&angle_end,&fill)) {
		return ADE_RETURN_NIL;
	}

	gr_arc(x,y, ra, angle_start, angle_end, fill, GR_RESIZE_NONE);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawCurve, l_Graphics, "number X, number Y, number Radius", "Draws a curve", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y,ra,dir = 0;

	if(!ade_get_args(L, "iii|i", &x,&y,&ra, &dir))
		return ADE_RETURN_NIL;

	//WMC - direction should be settable at a certain point via enumerations.
	//Not gonna deal with it now.
    gr_curve(x, y, ra, dir, GR_RESIZE_NONE);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawGradientLine, l_Graphics, "number X1, number Y1, number X2, number Y2", "Draws a line from (x1,y1) to (x2,y2) with the CurrentColor that steadily fades out", NULL, NULL)
{
	if(!Gr_inited)
		return 0;

	int x1,y1,x2,y2;

	if(!ade_get_args(L, "iiii", &x1, &y1, &x2, &y2))
		return ADE_RETURN_NIL;

	gr_gradient(x1,y1,x2,y2,GR_RESIZE_NONE);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawLine, l_Graphics, "number X1, number Y1, number X2, number Y2", "Draws a line from (x1,y1) to (x2,y2) with CurrentColor", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x1,y1,x2,y2;

	if(!ade_get_args(L, "iiii", &x1, &y1, &x2, &y2))
		return ADE_RETURN_NIL;

	gr_line(x1,y1,x2,y2,GR_RESIZE_NONE);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawPixel, l_Graphics, "number X, number Y", "Sets pixel to CurrentColor", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y;

	if(!ade_get_args(L, "ii", &x, &y))
		return ADE_RETURN_NIL;

	gr_pixel(x,y,GR_RESIZE_NONE);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawPolygon, l_Graphics, "texture Texture, [vector Position={0,0,0}, orientation Orientation=nil, number Width=1.0, number Height=1.0]", "Draws a polygon. May not work properly in hooks other than On Object Render.", NULL, NULL)
{
	int tdx = -1;
	vec3d pos = vmd_zero_vector;
	matrix_h *mh = NULL;
	float width = 1.0f;
	float height = 1.0f;
	if(!ade_get_args(L, "o|ooff", l_Texture.Get(&tdx), l_Vector.Get(&pos), l_Matrix.GetPtr(&mh), &width, &height))
		return ADE_RETURN_NIL;

	if(!bm_is_valid(tdx))
		return ADE_RETURN_FALSE;

	matrix *orip = &vmd_identity_matrix;
	if(mh != NULL)
		orip = mh->GetMatrix();

	//Do 3D stuff
	bool in_frame = g3_in_frame() > 0;
	if(!in_frame)
		g3_start_frame(0);

	//gr_set_bitmap(tdx, lua_Opacity_type, GR_BITBLT_MODE_NORMAL, lua_Opacity);
	//g3_draw_polygon(&pos, orip, width, height, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
	material mat_params;
	material_set_unlit(&mat_params, tdx, lua_Opacity, lua_Opacity_type == GR_ALPHABLEND_FILTER ? true : false, false);
	g3_render_rect_oriented(&mat_params, &pos, orip, width, height);

	if(!in_frame)
		g3_end_frame();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(drawRectangle, l_Graphics, "number X1, number Y1, number X2, number Y2, [boolean Filled=true]", "Draws a rectangle with CurrentColor", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x1,y1,x2,y2;
	bool f=true;

	if(!ade_get_args(L, "iiii|b", &x1, &y1, &x2, &y2, &f))
		return ADE_RETURN_NIL;

	if(f)
	{
		gr_set_bitmap(0);  // gr_rect will use the last bitmaps info, so set to zero to flush any previous alpha state
		gr_rect(x1, y1, x2-x1, y2-y1, GR_RESIZE_NONE);
	}
	else
	{
		gr_line(x1,y1,x2,y1,GR_RESIZE_NONE);	//Top
		gr_line(x1,y2,x2,y2,GR_RESIZE_NONE); //Bottom
		gr_line(x1,y1,x1,y2,GR_RESIZE_NONE);	//Left
		gr_line(x2,y1,x2,y2,GR_RESIZE_NONE);	//Right
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawSphere, l_Graphics, "[number Radius = 1.0, vector Position]", "Draws a sphere with radius Radius at world vector Position. May not work properly in hooks other than On Object Render.", "boolean", "True if successful, false or nil otherwise")
{
	float rad = 1.0f;
	vec3d pos = vmd_zero_vector;
	ade_get_args(L, "|fo", &rad, l_Vector.Get(&pos));

	bool in_frame = g3_in_frame() > 0;
	if(!in_frame) {
		g3_start_frame(0);

		vec3d cam_pos;
		matrix cam_orient;

		camid cid;
		
		if (Viewer_mode & VM_FREECAMERA)
			cid = cam_get_current();
		else
			cid = Main_camera;

		camera *cam = cid.getCamera();

		if (cam != NULL) {
			cam->get_info(&cam_pos, &cam_orient);
			g3_set_view_matrix(&cam_pos, &cam_orient, View_zoom);
		} else {
			g3_set_view_matrix(&Eye_position, &Eye_matrix, View_zoom);
		}

		gr_set_proj_matrix( Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}

	vertex vtx;
	vtx.world = pos;
	g3_rotate_vertex(&vtx, &pos);
	g3_draw_sphere(&vtx, rad);

	if(!in_frame) {
		gr_end_view_matrix();
		gr_end_proj_matrix();
		g3_end_frame();
	}
	return ADE_RETURN_TRUE;
}

// Aardwolf's test code to render a model, supposed to emulate WMC's gr.drawModel function
ADE_FUNC(drawModel, l_Graphics, "model, position, orientation", "Draws the given model with the specified position and orientation - Use with extreme care, may not work properly in all scripting hooks.", "int", "Zero if successful, otherwise an integer error code")
{
	model_h *mdl = NULL;
	vec3d *v = &vmd_zero_vector;
	matrix_h *mh = NULL;
	if(!ade_get_args(L, "ooo", l_Model.GetPtr(&mdl), l_Vector.GetPtr(&v), l_Matrix.GetPtr(&mh)))
		return ade_set_args(L, "i", 1);

	if(mdl == NULL)
		return ade_set_args(L, "i", 2);

	int model_num = mdl->GetID();
	if(model_num < 0)
		return ade_set_args(L, "i", 3);

	//Handle angles
	matrix *orient = mh->GetMatrix();

	//Clip
	gr_set_clip(0, 0, gr_screen.max_w, gr_screen.max_h, GR_RESIZE_NONE);

	//Handle 3D init stuff
	g3_start_frame(1);

	vec3d cam_pos;
	matrix cam_orient;

	camid cid;
	if (Viewer_mode & VM_FREECAMERA)
		cid = cam_get_current();
	else
		cid = Main_camera;

	camera *cam = cid.getCamera();

	if (cam != NULL) {
		cam->get_info(&cam_pos, &cam_orient);
		g3_set_view_matrix(&cam_pos, &cam_orient, View_zoom);
	} else {
		g3_set_view_matrix(&Eye_position, &Eye_matrix, View_zoom);
	}

	gr_set_proj_matrix( Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	//Draw the ship!!
	model_clear_instance(model_num);
	model_set_detail_level(0);
	model_render_params render_info;

	render_info.set_detail_level_lock(0);

	model_render_immediate(&render_info, model_num, orient, v);

	//OK we're done
	gr_end_view_matrix();
	gr_end_proj_matrix();

	//Bye!!
	g3_end_frame();
	gr_reset_clip();

	return ade_set_args(L, "i", 0);
}

// Wanderer
ADE_FUNC(drawModelOOR, l_Graphics, "model Model, vector Position, matrix Orientation, [integer Flags]", "Draws the given model with the specified position and orientation - Use with extreme care, designed to operate properly only in On Object Render hooks.", "int", "Zero if successful, otherwise an integer error code")
{
	model_h *mdl = NULL;
	vec3d *v = &vmd_zero_vector;
	matrix_h *mh = NULL;
	int flags = MR_NORMAL;
	if(!ade_get_args(L, "ooo|i", l_Model.GetPtr(&mdl), l_Vector.GetPtr(&v), l_Matrix.GetPtr(&mh), &flags))
		return ade_set_args(L, "i", 1);

	if(mdl == NULL)
		return ade_set_args(L, "i", 2);

	polymodel *pm = mdl->Get();

	if (pm == NULL)
		return ade_set_args(L, "i", 3);

	int model_num = pm->id;

	if(model_num < 0)
		return ade_set_args(L, "i", 3);

	//Handle angles
	matrix *orient = mh->GetMatrix();

	//Draw the ship!!
	model_clear_instance(model_num);

	model_render_params render_info;
	render_info.set_flags(flags);

	model_render_immediate(&render_info, model_num, orient, v);

	return ade_set_args(L, "i", 0);
}

// Aardwolf's targeting brackets function
ADE_FUNC(drawTargetingBrackets, l_Graphics, "object Object, [boolean draw=true, int padding=5]",
	"Gets the edge positions of targeting brackets for the specified object. The brackets will only be drawn if draw is true or the default value of draw is used. Brackets are drawn with the current color. The brackets will have a padding (distance from the actual bounding box); the default value (used elsewhere in FS2) is 5.",
	"number,number,number,number",
	"Left, top, right, and bottom positions of the brackets, or nil if invalid")
{
	if(!Gr_inited) {
		return ADE_RETURN_NIL;
	}

	object_h *objh = NULL;
	bool draw_box = true;
	int padding = 5;

	if( !ade_get_args(L, "o|bi", l_Object.GetPtr(&objh), &draw_box, &padding) ) {
		return ADE_RETURN_NIL;
	}

	// The following code is mostly copied from
	// void hud_show_brackets(object *targetp, vertex *projected_v)
	// in hudtarget.cpp

	if( !objh->IsValid()) {
		return ADE_RETURN_NIL;
	}

	object *targetp = objh->objp;

	int x1,x2,y1,y2;
	int bound_rc, pof;
	int modelnum;
	bool entered_frame = false;
	SCP_list<CJumpNode>::iterator jnp;
	
	if ( !(g3_in_frame( ) > 0 ) )
	{
		g3_start_frame( 0 );
		entered_frame = true;
	}


	switch ( targetp->type ) {
		case OBJ_SHIP:
			modelnum = Ship_info[Ships[targetp->instance].ship_info_index].model_num;
			bound_rc = model_find_2d_bound_min( modelnum, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			if ( bound_rc != 0 ) {
				if ( entered_frame )
					g3_end_frame( );
				return ADE_RETURN_NIL;
			}
			break;
		case OBJ_DEBRIS:
			modelnum = Debris[targetp->instance].model_num;
			bound_rc = submodel_find_2d_bound_min( modelnum, Debris[targetp->instance].submodel_num, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			if ( bound_rc != 0 ) {
				if ( entered_frame )
					g3_end_frame( );
				return ADE_RETURN_NIL;
			}
			break;
		case OBJ_WEAPON:
			Assert(Weapon_info[Weapons[targetp->instance].weapon_info_index].subtype == WP_MISSILE);
			modelnum = Weapon_info[Weapons[targetp->instance].weapon_info_index].model_num;
			bound_rc = model_find_2d_bound_min( modelnum, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			break;
		case OBJ_ASTEROID:
			pof = Asteroids[targetp->instance].asteroid_subtype;
			modelnum = Asteroid_info[Asteroids[targetp->instance].asteroid_type].model_num[pof];
			bound_rc = model_find_2d_bound_min( modelnum, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			break;
		case OBJ_JUMP_NODE:
			for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
				if(jnp->GetSCPObject() == targetp)
					break;
			}
			
			modelnum = jnp->GetModelNumber();
			bound_rc = model_find_2d_bound_min( modelnum, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			break;
		default: //Someone passed an invalid pointer.
			if ( entered_frame )
				g3_end_frame( );
			return ADE_RETURN_NIL;
	}

	x1 -= padding;
	x2 += padding;
	y1 -= padding;
	y2 += padding;
	if ( draw_box ) {
		draw_brackets_square(x1, y1, x2, y2, GR_RESIZE_NONE);
	}

	if ( entered_frame )
		g3_end_frame( );

	return ade_set_args(L, "iiii", x1, y1, x2, y2);
}

ADE_FUNC(drawSubsystemTargetingBrackets, l_Graphics, "subsystem subsys, [boolean draw=true, boolean setColor=false]",
	"Gets the edge position of the targeting brackets drawn for a subsystem as if they were drawn on the HUD. Only actually draws the brackets if <i>draw</i> is true, optionally sets the color the as if it was drawn on the HUD",
	"number,number,number,number",
	"Left, top, right, and bottom positions of the brackets, or nil if invalid or off-screen")
{
	if(!Gr_inited) {
		return ADE_RETURN_NIL;
	}

	ship_subsys_h *sshp = NULL;
	bool draw = true;
	bool set_color = false;

	if( !ade_get_args(L, "o|bb", l_Subsystem.GetPtr(&sshp), &draw, &set_color) ) {
		return ADE_RETURN_NIL;
	}

	if (!sshp->IsValid())
	{
		return ADE_RETURN_NIL;
	}

	bool entered_frame = false;
	
	if ( !(g3_in_frame( ) > 0 ) )
	{
		g3_start_frame( 0 );
		entered_frame = true;
	}

	int coords[4];

	int in_sight = draw_subsys_brackets(sshp->ss, 24, 24, draw, set_color, coords);

	if ( entered_frame )
		g3_end_frame( );

	if (in_sight > 0)
	{
		return ade_set_args(L, "iiii", coords[0], coords[1], coords[2], coords[3]);
	}
	else
	{
		return ADE_RETURN_NIL;
	}
}

ADE_FUNC(drawOffscreenIndicator, l_Graphics, "object Object, [boolean draw=true, boolean setColor=false]",
	"Draws an off-screen indicator for the given object. The indicator will not be drawn if draw=false, but the coordinates will be returned in either case. The indicator will be drawn using the current color if setColor=true and using the IFF color of the object if setColor=false.",
	"number,number",
	"Coordinates of the indicator (at the very edge of the screen), or nil if object is on-screen")
{
	object_h *objh = NULL;
	bool draw = false;
	bool setcolor = false;
	vec2d outpoint = { -1.0f, -1.0f };
	
	if(!Gr_inited) {
		return ADE_RETURN_NIL;
	}
	
	if( !ade_get_args(L, "o|bb", l_Object.GetPtr(&objh), &draw, &setcolor) ) {
		return ADE_RETURN_NIL;
	}

	if( !objh->IsValid()) {
		return ADE_RETURN_NIL;
	}

	object *targetp = objh->objp;
	bool in_frame = g3_in_frame() > 0;

	if (!in_frame)
		g3_start_frame(0);

	vertex target_point;
	g3_rotate_vertex(&target_point, &targetp->pos);
	g3_project_vertex(&target_point);

	if (!in_frame)
		g3_end_frame();

	if(target_point.codes == 0)
		return ADE_RETURN_NIL;

	hud_target_clear_display_list();
	hud_target_add_display_list(targetp, &target_point, &targetp->pos, 5, NULL, NULL, TARGET_DISPLAY_DIST);

	size_t j, num_gauges;
	num_gauges = default_hud_gauges.size();

	for(j = 0; j < num_gauges; j++) {
		if (default_hud_gauges[j]->getObjectType() == HUD_OBJECT_OFFSCREEN) {
			HudGaugeOffscreen *offscreengauge = static_cast<HudGaugeOffscreen*>(default_hud_gauges[j]);
			
			offscreengauge->preprocess();
			offscreengauge->onFrame(flFrametime);

			if ( !offscreengauge->canRender() ) {
				break;
			}

			offscreengauge->resetClip();
			offscreengauge->setFont();
			int dir;
			float tri_separation;

			offscreengauge->calculatePosition(&target_point, &targetp->pos, &outpoint, &dir, &tri_separation);

			if (draw) {
				float distance = hud_find_target_distance(targetp, Player_obj);

				if (!setcolor)
					hud_set_iff_color(targetp, 1);

				offscreengauge->renderOffscreenIndicator(&outpoint, dir, distance, tri_separation, true);
			}

			offscreengauge->resize(&outpoint.x, &outpoint.y);

			break;
		}
	}

	if (outpoint.x >= 0 && outpoint.y >=0)
		return ade_set_args(L, "ii", (int)outpoint.x, (int)outpoint.y);
	else
		return ADE_RETURN_NIL;
}

#define MAX_TEXT_LINES		256
static char *BooleanValues[] = {"False", "True"};
static const int NextDrawStringPosInitial[] = {0, 0};
static int NextDrawStringPos[] = {NextDrawStringPosInitial[0], NextDrawStringPosInitial[1]};
ADE_FUNC(drawString, l_Graphics, "string Message, [number X1, number Y1, number X2, number Y2]",
		 "Draws a string. Use x1/y1 to control position, x2/y2 to limit textbox size."
		 "Text will automatically move onto new lines, if x2/y2 is specified."
		 "Additionally, calling drawString with only a string argument will automatically"
		 "draw that string below the previously drawn string (or 0,0 if no strings"
		 "have been drawn yet",
		 "number",
		 "Number of lines drawn, or 0 on failure")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	int x=NextDrawStringPos[0];
	int y = NextDrawStringPos[1];

	char *s = "(null)";
	int x2=-1,y2=-1;
	int num_lines = 0;

	if(lua_isboolean(L, 1))
	{
		bool b = false;
		if(!ade_get_args(L, "b|iiii", &b, &x, &y, &x2, &y2))
			return ade_set_error(L, "i", 0);

		if(b)
			s = BooleanValues[1];
		else
			s = BooleanValues[0];
	}
	else if(lua_isstring(L, 1))
	{
		if(!ade_get_args(L, "s|iiii", &s, &x, &y, &x2, &y2))
			return ade_set_error(L, "i", 0);
	}
	else
	{
		ade_get_args(L, "|*iiii", &x, &y, &x2, &y2);
	}

	NextDrawStringPos[0] = x;
	if(x2 < 0)
	{
		num_lines = 1;
		gr_string(x,y,s,GR_RESIZE_NONE);

		int height = 0;
		gr_get_string_size(NULL, &height, s);
		NextDrawStringPos[1] = y+height;
	}
	else
	{
		int linelengths[MAX_TEXT_LINES];
		const char *linestarts[MAX_TEXT_LINES];

		if (y2 >= 0 && y2 < y)
		{
			// Invalid y2 value
			Warning(LOCATION, "Illegal y2 value passed to drawString. Got %d y2 value but %d for y.", y2, y);
		
			int temp = y;
			y = y2;
			y2 = temp;
		}

		num_lines = split_str(s, x2-x, linelengths, linestarts, MAX_TEXT_LINES);

		//Make sure we don't go over size
		int line_ht = gr_get_font_height();
		num_lines = MIN(num_lines, (y2 - y) / line_ht);

		int curr_y = y;
		for(int i = 0; i < num_lines; i++)
		{
			//Contrary to WMC's previous comment, let's make a new string each line
			int len = linelengths[i];
			char *buf = new char[len+1];
			strncpy(buf, linestarts[i], len);
			buf[len] = '\0';

			//Draw the string
			gr_string(x,curr_y,buf,GR_RESIZE_NONE);

			//Free the string we made
			delete[] buf;

			//Increment line height
			curr_y += line_ht;
		}
		
		if (num_lines <= 0)
		{
			// If no line was drawn then we need to add one so the next line is 
			// aligned right
			curr_y += line_ht;
		}
		
		NextDrawStringPos[1] = curr_y;
	}
	return ade_set_error(L, "i", num_lines);
}

ADE_FUNC(getStringWidth, l_Graphics, "string String", "Gets string width", "number", "String width, or 0 on failure")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	char *s;
	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "i", 0);

	int w;

	gr_get_string_size(&w, NULL, s);
	
	return ade_set_args(L, "i", w);
}

ADE_FUNC(loadStreamingAnim, l_Graphics, "string Filename, [boolean loop, boolean reverse, boolean pause, boolean cache]",
		 "Plays a streaming animation, returning its handle. The optional booleans (except cache) can also be set via the handles virtvars<br>"
		 "cache is best set to false when loading animations that are only intended to play once, e.g. headz<br>"
		 "Remember to call the unload() function when you're finished using the animation to free up memory.",
		 "streaminganim",
		 "Streaming animation handle, or invalid handle if animation couldn't be loaded")
{
	char *s;
	int rc = -1;
	bool loop = false, reverse = false, pause = false, cache = true;

	if(!ade_get_args(L, "s|bbbb", &s, &loop, &reverse, &pause, &cache))
		return ADE_RETURN_NIL;

	streaminganim_h sah(s);
	if (loop == false) {
		sah.ga.direction |= GENERIC_ANIM_DIRECTION_NOLOOP;
	}
	if (reverse == true) {
		sah.ga.direction |= GENERIC_ANIM_DIRECTION_BACKWARDS;
	}
	if (pause == true) {
		sah.ga.direction |= GENERIC_ANIM_DIRECTION_PAUSED;
	}
	rc = generic_anim_stream(&sah.ga, cache);

	if(rc < 0)
		return ade_set_args(L, "o", l_streaminganim.Set(sah)); // this object should be "invalid", matches loadTexture behaviour

	return ade_set_args(L, "o", l_streaminganim.Set(sah));
}

ADE_FUNC(createTexture, l_Graphics, "[number Width=512, number Height=512, enumeration Type=TEXTURE_DYNAMIC]",
		 "Creates a texture for rendering to."
		 "Types are TEXTURE_STATIC - for infrequent rendering - and TEXTURE_DYNAMIC - for frequent rendering.",
		 "texture",
		 "New texture handle, or invalid texture handle if texture couldn't be created")
{
	int w=512;
	int h=512;
	enum_h *e = NULL;

	//GET ARGS
	ade_get_args(L, "|iio", &w, &h, l_Enum.GetPtr(&e));

	int t = BMP_FLAG_RENDER_TARGET_DYNAMIC;
	if(e != NULL)
	{
		if(e->index == LE_TEXTURE_STATIC)
			t = BMP_FLAG_RENDER_TARGET_STATIC;
		else if(e->index == LE_TEXTURE_DYNAMIC)
			t = BMP_FLAG_RENDER_TARGET_DYNAMIC;
	}

	int idx = bm_make_render_target(w, h, t);

	if(idx < 0)
		return ade_set_error(L, "o", l_Texture.Set(-1));

	return ade_set_args(L, "o", l_Texture.Set(idx));
}

ADE_FUNC(loadTexture, l_Graphics, "string Filename, [boolean LoadIfAnimation, boolean NoDropFrames]",
		 "Gets a handle to a texture. If second argument is set to true, animations will also be loaded."
		 "If third argument is set to true, every other animation frame will not be loaded if system has less than 48 MB memory."
		 "<br><strong>IMPORTANT:</strong> Textures will not be unload themselves unless you explicitly tell them to do so."
		 "When you are done with a texture, call the Unload() function to free up memory.",
		 "texture",
		 "Texture handle, or invalid texture handle if texture couldn't be loaded")
{
	char *s;
	int idx=-1;
	bool b=false;
	bool d=false;

	if(!ade_get_args(L, "s|bb", &s, &b, &d))
		return ade_set_error(L, "o", l_Texture.Set(-1));

	if(b == true) {
		idx = bm_load_animation(s, nullptr, nullptr, nullptr, nullptr, d);
	}
	if(idx < 0) {
		idx = bm_load(s);
	}

	if(idx < 0)
		return ade_set_error(L, "o", l_Texture.Set(-1));

	return ade_set_args(L, "o", l_Texture.Set(idx));
}

ADE_FUNC(drawImage, l_Graphics, "string Filename/texture Texture, [number X1=0, Y1=0, number X2, number Y2, number UVX1 = 0.0, number UVY1 = 0.0, number UVX2=1.0, number UVY2=1.0, number alpha=1.0]",
		 "Draws an image or texture. Any image extension passed will be ignored."
		 "The UV variables specify the UV value for each corner of the image. "
		 "In UV coordinates, (0,0) is the top left of the image; (1,1) is the lower right.",
		 "boolean",
		 "Whether image was drawn")
{
	if(!Gr_inited)
		return ade_set_error(L, "b", false);

	int idx;
	int x1 = 0;
	int y1 = 0;
	int x2=INT_MAX;
	int y2=INT_MAX;
	float uv_x1=0.0f;
	float uv_y1=0.0f;
	float uv_x2=1.0f;
	float uv_y2=1.0f;
	float alpha=1.0f;

	if(lua_isstring(L, 1))
	{
		char *s = NULL;
		if(!ade_get_args(L, "s|iiiifffff", &s,&x1,&y1,&x2,&y2,&uv_x1,&uv_y1,&uv_x2,&uv_y2,&alpha))
			return ade_set_error(L, "b", false);

		idx = Script_system.LoadBm(s);

		if(idx < 0)
			return ADE_RETURN_FALSE;
	}
	else
	{
		if(!ade_get_args(L, "o|iiiifffff", l_Texture.Get(&idx),&x1,&y1,&x2,&y2,&uv_x1,&uv_y1,&uv_x2,&uv_y2,&alpha))
			return ade_set_error(L, "b", false);
	}

	if(!bm_is_valid(idx))
		return ade_set_error(L, "b", false);

	int w, h;
	if(bm_get_info(idx, &w, &h) < 0)
		return ADE_RETURN_FALSE;

	if(x2!=INT_MAX)
		w = x2-x1;

	if(y2!=INT_MAX)
		h = y2-y1;

	gr_set_bitmap(idx, lua_Opacity_type, GR_BITBLT_MODE_NORMAL, alpha);
	bitmap_rect_list brl = bitmap_rect_list(x1, y1, w, h, uv_x1, uv_y1, uv_x2, uv_y2);
	gr_bitmap_list(&brl, 1, GR_RESIZE_NONE);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(drawMonochromeImage, l_Graphics, "string Filename/texture Texture, number X1, number Y1, [number X2, number Y2, number alpha=1.0]", "Draws a monochrome image using the current color", "boolean", "Whether image was drawn")
{
	if(!Gr_inited)
		return ade_set_error(L, "b", false);

	int idx;
	int x,y;
	int x2=INT_MAX;
	int y2=INT_MAX;
	int sx=0;
	int sy=0;
	bool m = false;
	float alpha=1.0;

	if(lua_isstring(L, 1))
	{
		char *s = NULL;
		if(!ade_get_args(L, "sii|iif", &s,&x,&y,&x2,&y2,&alpha))
			return ade_set_error(L, "b", false);

		idx = Script_system.LoadBm(s);

		if(idx < 0)
			return ADE_RETURN_FALSE;
	}
	else
	{
		if(!ade_get_args(L, "oii|iif", l_Texture.Get(&idx),&x,&y,&x2,&y2,&alpha))
			return ade_set_error(L, "b", false);
	}

	if(!bm_is_valid(idx))
		return ade_set_error(L, "b", false);

	int w, h;
	if(bm_get_info(idx, &w, &h) < 0)
		return ADE_RETURN_FALSE;

	if(sx < 0)
		sx = w + sx;

	if(sy < 0)
		sy = h + sy;
	
	if(x2!=INT_MAX)
		w = x2-x;

	if(y2!=INT_MAX)
		h = y2-y;

	gr_set_bitmap(idx, lua_Opacity_type, GR_BITBLT_MODE_NORMAL,alpha);
	gr_aabitmap_ex(x, y, w, h, sx, sy, GR_RESIZE_NONE, m);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getImageWidth, l_Graphics, "string Filename", "Gets image width", "number", "Image width, or 0 if filename is invalid")
{
	char *s;
	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "i", 0);

	int w;
	
	int idx = bm_load(s);

	if(idx < 0)
		return ade_set_error(L, "i", 0);

	bm_get_info(idx, &w);
	return ade_set_args(L, "i", w);
}

ADE_FUNC(getImageHeight, l_Graphics, "Image name", "Gets image height", "number", "Image height, or 0 if filename is invalid")
{
	char *s;
	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "i", 0);

	int h;
	
	int idx = bm_load(s);

	if(idx < 0)
		return ade_set_error(L, "i", 0);

	bm_get_info(idx, NULL, &h);
	return ade_set_args(L, "i", h);
}

ADE_FUNC(flashScreen, l_Graphics, "number Red, number Green, number Blue", "Flashes the screen", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int r,g,b;

	if(!ade_get_args(L, "iii", &r, &g, &b))
		return ADE_RETURN_NIL;

	gr_flash(r,g,b);

	return ADE_RETURN_NIL;
}

ADE_FUNC(loadModel, l_Graphics, "string Filename", "Loads the model - will not setup subsystem data, DO NOT USE FOR LOADING SHIP MODELS", "model", "Handle to a model")
{
	char *s;
	int model_num = -1;

	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "o", l_Model.Set(model_h(-1)));

	if (s[0] == '\0')
		return ade_set_error(L, "o", l_Model.Set(model_h(-1)));

	model_num = model_load(s, 0, NULL);

	return ade_set_args(L, "o", l_Model.Set(model_h(model_num)));
}

ADE_FUNC(hasViewmode, l_Graphics, "enumeration", "Specifies if the current viemode has the specified flag, see VM_* enumeration", "boolean", "true if flag is present, false otherwise")
{
	enum_h *type = NULL;

	if (!ade_get_args(L, "o", l_Enum.GetPtr(&type)))
		return ade_set_error(L, "b", false);

	if (type == NULL || !type->IsValid())
		return ade_set_error(L, "b", false);

	int bit = 0;

	switch(type->index)
	{
	case LE_VM_INTERNAL:
		return ade_set_args(L, "b", (Viewer_mode & ~(VM_CAMERA_LOCKED | VM_CENTERING)) == 0);	// z64: Ignore camera lock state and centering state
		break;

	case LE_VM_EXTERNAL:
		bit = VM_EXTERNAL;
		break;

	case LE_VM_OTHER_SHIP:
		bit = VM_OTHER_SHIP;
		break;

	case LE_VM_CHASE:
		bit = VM_CHASE;
		break;

	case LE_VM_DEAD_VIEW:
		bit = VM_DEAD_VIEW;
		break;

	case LE_VM_EXTERNAL_CAMERA_LOCKED:
		return ade_set_args(L, "b", (Viewer_mode & VM_CAMERA_LOCKED) && (Viewer_mode & VM_EXTERNAL));
		break;

	case LE_VM_CAMERA_LOCKED:
		bit = VM_CAMERA_LOCKED;
		break;

	case LE_VM_FREECAMERA:
		bit = VM_FREECAMERA;
		break;

	case LE_VM_PADLOCK_LEFT:
		bit = VM_PADLOCK_LEFT;
		break;

	case LE_VM_PADLOCK_REAR:
		bit = VM_PADLOCK_REAR;
		break;

	case LE_VM_PADLOCK_RIGHT:
		bit = VM_PADLOCK_RIGHT;
		break;

	case LE_VM_PADLOCK_UP:
		bit = VM_PADLOCK_UP;
		break;

	case LE_VM_TOPDOWN:
		bit = VM_TOPDOWN;
		break;

	case LE_VM_TRACK:
		bit = VM_TRACK;
		break;

	case LE_VM_WARP_CHASE:
		bit = VM_WARP_CHASE;
		break;

	case LE_VM_WARPIN_ANCHOR:
		bit = VM_WARPIN_ANCHOR;
		break;

	case LE_VM_CENTERING:
		bit = VM_CENTERING;
		break;

	default:
		LuaError(L, "Attempted to use hasViewmode with an invalid enumeration! Only VM_* enumerations are allowed!");
		return ade_set_error(L, "b", false);
		break;
	}

	return ade_set_args(L, "b", (Viewer_mode & bit) != 0);
}

ADE_FUNC(setClip, l_Graphics, "x, y, width, height", "Sets the clipping region to the specified rectangle. Most drawing functions are able to handle the offset.", "boolean", "true if successful, false otherwise")
{
	int x, y, width, height;

	if (!ade_get_args(L, "iiii", &x, &y, &width, &height))
		return ADE_RETURN_FALSE;

	gr_set_clip(x, y, width, height, GR_RESIZE_NONE);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(resetClip, l_Graphics, NULL, "Resets the clipping region that might have been set", "boolean", "true if successful, false otherwise")
{
	gr_reset_clip();

	return ADE_RETURN_TRUE;
}

//**********LIBRARY: Scripting Variables
ade_lib l_HookVar("HookVariables", NULL, "hv", "Hook variables repository");

//WMC: IMPORTANT
//Be very careful when modifying this library, as the Globals[] library does depend
//on the current number of items in the library. If you add _anything_, modify __len.
//Or run changes by me.

//*****LIBRARY: Scripting Variables
ade_lib l_HookVar_Globals("Globals", &l_HookVar);

ADE_INDEXER(l_HookVar_Globals, "number Index", "Array of current HookVariable names", "string", "Hookvariable name, or empty string if invalid index specified")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");

	//Get lib
	lua_getglobal(L, l_HookVar.GetName());
	int lib_ldx = lua_gettop(L);
	if(!lua_isuserdata(L, lib_ldx))
	{
		lua_pop(L, 1);
		return ade_set_error(L, "s", "");
	}

	//Get metatable
	lua_getmetatable(L, lib_ldx);
	int mtb_ldx = lua_gettop(L);
	if(!lua_istable(L, mtb_ldx))
	{
		lua_pop(L, 2);
		return ade_set_error(L, "s", "");
	}

	//Get ade members table
	lua_pushstring(L, "__ademembers");
	lua_rawget(L, mtb_ldx);
	int amt_ldx = lua_gettop(L);
	if(!lua_istable(L, amt_ldx))
	{
		lua_pop(L, 3);
		return ade_set_error(L, "s", "");
	}

	//List 'em
	char *keyname = NULL;
	int count = 1;
	lua_pushnil(L);
	while(lua_next(L, amt_ldx))
	{
		//Now on stack: Key, value
		lua_pushvalue(L, -2);
		keyname = (char *)lua_tostring(L, -1);
		if(strcmp(keyname, "Globals"))
		{
			if(count == idx)
			{
				//lib, mtb, amt, key, value, string go bye-bye
				lua_pop(L, 5);
				return ade_set_args(L, "s", keyname);
			}
			count++;
		}
		lua_pop(L, 2);	//Value, string
	}

	lua_pop(L, 3);	//lib, mtb, amt

	return ade_set_error(L, "s", "");
}

ADE_FUNC(__len, l_HookVar_Globals, NULL, "Number of HookVariables", "number", "Number of HookVariables")
{
	//Get metatable
	lua_getglobal(L, l_HookVar.GetName());
	int lib_ldx = lua_gettop(L);
	if(!lua_isuserdata(L, lib_ldx))
	{
		lua_pop(L, 1);
		return ade_set_error(L, "i", 0);
	}

	lua_getmetatable(L, lib_ldx);
	int mtb_ldx = lua_gettop(L);
	if(!lua_istable(L, mtb_ldx))
	{
		lua_pop(L, 2);
		return ade_set_error(L, "i", 0);
	}

	//Get ade members table
	lua_pushstring(L, "__ademembers");
	lua_rawget(L, mtb_ldx);
	int amt_ldx = lua_gettop(L);
	if(!lua_istable(L, amt_ldx))
	{
		lua_pop(L, 3);
		return ade_set_error(L, "i", 0);
	}

	int total_len = 0;
	lua_pushnil(L);
	while(lua_next(L, amt_ldx))
	{
		total_len++;
		lua_pop(L, 1);	//value
	}
	size_t num_sub = ade_manager::getInstance()->getEntry(l_HookVar.GetIdx()).Num_subentries;

	lua_pop(L, 3);

	//WMC - Return length, minus the 'Globals' library
	return ade_set_args(L, "i", (int)(total_len - num_sub));
}

//**********LIBRARY: Mission
ade_lib l_Mission("Mission", NULL, "mn", "Mission library");

// for use in creating faster metadata systems, use in conjunction with getSignature()
ADE_FUNC(getObjectFromSignature, l_Mission, "number Signature", "Gets a handle of an object from its signature", "object", "Handle of object with signaure, invalid handle if signature is not in use")
{
	int sig = -1;
	int objnum;
	if(!ade_get_args(L, "i", &sig))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if (sig == -1) {
		return ade_set_error(L, "o", l_Object.Set(object_h()));
	}

	objnum = obj_get_by_signature(sig);

	return ade_set_object_with_breed(L, objnum);
}

ADE_FUNC(evaluateSEXP, l_Mission, "string", "Runs the defined SEXP script", "boolean", "if the operation was successful")
{
	char *s;
	int r_val;

	if(!ade_get_args(L, "s", &s))
		return ADE_RETURN_FALSE;

	r_val = run_sexp(s);

	if (r_val == SEXP_TRUE)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(runSEXP, l_Mission, "string", "Runs the defined SEXP script", "boolean", "if the operation was successful")
{
	char *s;
	int r_val;
	char buf[8192];

	if(!ade_get_args(L, "s", &s))
		return ADE_RETURN_FALSE;

	while (is_white_space(*s))
		s++;
	if (*s != '(')
	{
		static bool Warned_about_runSEXP_parentheses = false;
		if (!Warned_about_runSEXP_parentheses)
		{
			Warned_about_runSEXP_parentheses = true;
			Warning(LOCATION, "Invalid SEXP syntax: SEXPs must be surrounded by parentheses.  For backwards compatibility, the string has been enclosed in parentheses.  This may not be correct in all use cases.");
		}
		// this is the old sexp handling method, which is incorrect
		snprintf(buf, 8191, "( when ( true ) ( %s ) )", s);
	}
	else
	{
		// this is correct usage
		snprintf(buf, 8191, "( when ( true ) %s )", s);
	}

	r_val = run_sexp(buf);

	if (r_val == SEXP_TRUE)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

//****SUBLIBRARY: Mission/Asteroids
ade_lib l_Mission_Asteroids("Asteroids", &l_Mission, NULL, "Asteroids in the mission");

ADE_INDEXER(l_Mission_Asteroids, "number Index", "Gets asteroid", "asteroid", "Asteroid handle, or invalid handle if invalid index specified")
{
	int idx = -1;
	if( !ade_get_args(L, "*i", &idx) ) {
		return ade_set_error( L, "o", l_Asteroid.Set( object_h() ) );
	}
	if( idx > -1 && idx < asteroid_count() ) {
		idx--; //Convert from Lua to C, as lua indices start from 1, not 0
		return ade_set_args( L, "o", l_Asteroid.Set( object_h( &Objects[Asteroids[idx].objnum] ), Objects[Asteroids[idx].objnum].signature ) );
	}

	return ade_set_error(L, "o", l_Asteroid.Set( object_h() ) );
}

ADE_FUNC(__len, l_Mission_Asteroids, NULL,
		 "Number of asteroids in mission. Note that the value returned is only good until an asteroid is destroyed, and so cannot be relied on for more than one frame.",
		 "number",
		 "Number of asteroids in the mission, or 0 if asteroids are not enabled")
{
	if(Asteroids_enabled) {
		return ade_set_args(L, "i", asteroid_count());
	}
	return ade_set_args(L, "i", 0);
}

//****SUBLIBRARY: Mission/Debris
ade_lib l_Mission_Debris("Debris", &l_Mission, NULL, "debris in the mission");

ADE_INDEXER(l_Mission_Debris, "number Index", "Array of debris in the current mission", "debris", "Debris handle, or invalid debris handle if index wasn't valid")
{
	int idx = -1;
	if( !ade_get_args( L, "*i", &idx ) ) {
		return ade_set_error(L, "o", l_Debris.Set(object_h()));
	}
	if( idx > -1 && idx < Num_debris_pieces ) {
		idx--; // Lua -> C
		if (Debris[idx].objnum == -1) //Somehow accessed an invalid debris piece
			return ade_set_error(L, "o", l_Debris.Set(object_h()));
		return ade_set_args(L, "o", l_Debris.Set(object_h(&Objects[Debris[idx].objnum]), Objects[Debris[idx].objnum].signature));
	}

	return ade_set_error(L, "o", l_Debris.Set(object_h()));
}

ADE_FUNC(__len, l_Mission_Debris, NULL, 
		 "Number of debris pieces in the mission. "
		 "Note that the value returned is only good until a piece of debris is destroyed, and so cannot be relied on for more than one frame.",
		 "number",
		 "Current number of debris particles")
{
	return ade_set_args(L, "i", Num_debris_pieces);
}

//****SUBLIBRARY: Mission/EscortShips
ade_lib l_Mission_EscortShips("EscortShips", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_EscortShips, "number Index", "Gets escort ship at specified index on escort list", "ship", "Specified ship, or invalid ship handle if invalid index")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	if(idx < 1 || idx > hud_escort_num_ships_on_list())
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	//Lua->FS2
	idx--;

	idx = hud_escort_return_objnum(idx);
	
	if(idx < 0)
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[idx])));
}

ADE_FUNC(__len, l_Mission_EscortShips, NULL, "Current number of escort ships", "number", "Current number of escort ships")
{
	return ade_set_args(L, "i", hud_escort_num_ships_on_list());
}

//****SUBLIBRARY: Mission/Events
ade_lib l_Mission_Events("Events", &l_Mission, NULL, "Events");

ADE_INDEXER(l_Mission_Events, "number Index/string Name", "Indexes events list", "event", "Event handle, or invalid event handle if index was invalid")
{
	char *s;
	if(!ade_get_args(L, "*s", &s))
		return ade_set_error(L, "o", l_Event.Set(-1));

	int i;
	for(i = 0; i < Num_mission_events; i++)
	{
		if(!stricmp(Mission_events[i].name, s))
			return ade_set_args(L, "o", l_Event.Set(i));
	}

	//Now try as a number
	i = atoi(s);
	if(i < 1 || i > Num_mission_events)
		return ade_set_error(L, "o", l_Event.Set(-1));

	//Lua-->FS2
	i--;

	return ade_set_args(L, "o", l_Event.Set(i));
}

ADE_FUNC(__len, l_Mission_Events, NULL, "Number of events in mission", "number", "Number of events in mission")
{
	return ade_set_args(L, "i", Num_mission_events);
}

//****SUBLIBRARY: Mission/SEXPVariables
ade_lib l_Mission_SEXPVariables("SEXPVariables", &l_Mission, NULL, "SEXP Variables");

ADE_INDEXER(l_Mission_SEXPVariables, "number Index/string Name", "Array of SEXP variables. Note that you can set a sexp variable using the array, eg \'SEXPVariables[\"newvariable\"] = \"newvalue\"\'", "sexpvariable", "Handle to SEXP variable, or invalid sexpvariable handle if index was invalid")
{
	char *name = NULL;
	char *newval = NULL;
	if(!ade_get_args(L, "*s|s", &name, &newval))
		return ade_set_error(L, "o", l_SEXPVariable.Set(sexpvar_h()));

	int idx = get_index_sexp_variable_name(name);
	if(idx < 0)
	{
		idx = atoi(name);

		//Lua-->FS2
		idx--;
	}

	if(idx < 0 || idx >= MAX_SEXP_VARIABLES)
	{
		if(ADE_SETTING_VAR && newval != NULL)
		{
			idx = sexp_add_variable(newval, name, lua_type(L, 2) == LUA_TNUMBER ? SEXP_VARIABLE_NUMBER : SEXP_VARIABLE_STRING);
		}

		//We have failed.
		if(idx < 0)
		{
			return ade_set_error(L, "o", l_SEXPVariable.Set(sexpvar_h()));
		}
	}
	else
	{
		if(ADE_SETTING_VAR && newval != NULL)
		{
			sexp_modify_variable(newval, idx, false);
		}
	}

	return ade_set_args(L, "o", l_SEXPVariable.Set(sexpvar_h(idx)));
}

ADE_FUNC(__len, l_Mission_SEXPVariables, NULL, "Current number of SEXP variables", "number", "Counts number of loaded SEXP Variables. May be slow.")
{
	return ade_set_args(L, "i", sexp_variable_count());
}

//****SUBLIBRARY: Mission/Ships
ade_lib l_Mission_Ships("Ships", &l_Mission, NULL, "Ships in the mission");

ADE_INDEXER(l_Mission_Ships, "number Index/string Name", "Gets ship", "ship", "Ship handle, or invalid ship handle if index was invalid")
{
	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	int idx = ship_name_lookup(name);

	if(idx > -1)
	{
		return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[Ships[idx].objnum]), Objects[Ships[idx].objnum].signature));
	}
	else
	{
		idx = atoi(name);
		if(idx > 0)
		{
			int count=1;

			for(int i = 0; i < MAX_SHIPS; i++)
			{
				if (Ships[i].objnum < 0 || Objects[Ships[i].objnum].type != OBJ_SHIP)
					continue;

				if(count == idx) {
					return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[Ships[i].objnum]), Objects[Ships[i].objnum].signature));
				}

				count++;
			}
		}
	}

	return ade_set_error(L, "o", l_Ship.Set(object_h()));
}

extern int ships_inited;
ADE_FUNC(__len, l_Mission_Ships, NULL,
		 "Number of ships in the mission. "
		 "This function is somewhat slow, and should be set to a variable for use in looping situations. "
		 "Note that the value returned is only good until a ship is destroyed, and so cannot be relied on for more than one frame.",
		 "number",
		 "Number of ships in the mission, or 0 if ships haven't been initialized yet")
{
	if(ships_inited)
		return ade_set_args(L, "i", ship_get_num_ships());
	else
		return ade_set_args(L, "i", 0);
}

//****SUBLIBRARY: Mission/Waypoints
ade_lib l_Mission_Waypoints("Waypoints", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Waypoints, "number Index", "Array of waypoints in the current mission", "waypoint", "Waypoint handle, or invalid waypoint handle if index was invalid")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Waypoint.Set(object_h()));

	//Remember, Lua indices start at 0.
	int count=0;

	object *ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list))
	{
		if (ptr->type == OBJ_WAYPOINT)
			count++;

		if(count == idx) {
			return ade_set_args(L, "o", l_Waypoint.Set(object_h(ptr)));
		}

		ptr = GET_NEXT(ptr);
	}

	return ade_set_error(L, "o", l_Weapon.Set(object_h()));
}

ADE_FUNC(__len, l_Mission_Waypoints, NULL, "Gets number of waypoints in mission. Note that this is only accurate for one frame.", "number", "Number of waypoints in the mission")
{
	uint count=0;
	for(uint i = 0; i < MAX_OBJECTS; i++)
	{
		if (Objects[i].type == OBJ_WAYPOINT)
			count++;
	}

	return ade_set_args(L, "i", count);
}

//****SUBLIBRARY: Mission/WaypointLists
ade_lib l_Mission_WaypointLists("WaypointLists", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_WaypointLists, "number Index/string WaypointListName", "Array of waypoint lists", "waypointlist", "Gets waypointlist handle")
{
	waypointlist_h wpl;
	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_WaypointList.Set(waypointlist_h()));

	wpl = waypointlist_h(name);

	if (!wpl.IsValid()) {
		int idx = atoi(name) - 1;
		wpl = waypointlist_h(find_waypoint_list_at_index(idx));
	}

	if (wpl.IsValid()) {
		return ade_set_args(L, "o", l_WaypointList.Set(wpl));
	}

	return ade_set_error(L, "o", l_WaypointList.Set(waypointlist_h()));
}

ADE_FUNC(__len, l_Mission_WaypointLists, NULL, "Number of waypoint lists in mission. Note that this is only accurate for one frame.", "number", "Number of waypoint lists in the mission")
{
	return ade_set_args(L, "i", Waypoint_lists.size());
}

//****SUBLIBRARY: Mission/Weapons
ade_lib l_Mission_Weapons("Weapons", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Weapons, "number Index", "Gets handle to a weapon object in the mission.", "weapon", "Weapon handle, or invalid weapon handle if index is invalid")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Weapon.Set(object_h()));

	//Remember, Lua indices start at 0.
	int count=1;

	for(int i = 0; i < MAX_WEAPONS; i++)
	{
		if (Weapons[i].weapon_info_index < 0 || Weapons[i].objnum < 0 || Objects[Weapons[i].objnum].type != OBJ_WEAPON)
			continue;

		if(count == idx) {
			return ade_set_args(L, "o", l_Weapon.Set(object_h(&Objects[Weapons[i].objnum])));
		}

		count++;
	}

	return ade_set_error(L, "o", l_Weapon.Set(object_h()));
}
ADE_FUNC(__len, l_Mission_Weapons, NULL, "Number of weapon objects in mission. Note that this is only accurate for one frame.", "number", "Number of weapon objects in mission")
{
	return ade_set_args(L, "i", Num_weapons);
}

//****SUBLIBRARY: Mission/Beams
ade_lib l_Mission_Beams("Beams", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Beams, "number Index", "Gets handle to a beam object in the mission.", "beam", "Beam handle, or invalid beam handle if index is invalid")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Beam.Set(object_h()));

	//Remember, Lua indices start at 0.
	int count=1;

	for(int i = 0; i < MAX_BEAMS; i++)
	{
		if (Beams[i].weapon_info_index < 0 || Beams[i].objnum < 0 || Objects[Beams[i].objnum].type != OBJ_BEAM)
			continue;

		if(count == idx) {
			return ade_set_args(L, "o", l_Beam.Set(object_h(&Objects[Beams[i].objnum])));
		}

		count++;
	}

	return ade_set_error(L, "o", l_Beam.Set(object_h()));
}
ADE_FUNC(__len, l_Mission_Beams, NULL, "Number of beam objects in mission. Note that this is only accurate for one frame.", "number", "Number of beam objects in mission")
{
	return ade_set_args(L, "i", Beam_count);
}

//****SUBLIBRARY: Campaign
ade_lib l_Campaign("Campaign", NULL, "ca", "Campaign Library");

ADE_FUNC(getNextMissionFilename, l_Campaign, NULL, "Gets next mission filename", "string", "Next mission filename, or nil if the next mission is invalid")
{
	if (Campaign.next_mission < 0 || Campaign.next_mission >= MAX_CAMPAIGN_MISSIONS) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", Campaign.missions[Campaign.next_mission].name);
}

ADE_FUNC(getPrevMissionFilename, l_Campaign, NULL, "Gets previous mission filename", "string", "Previous mission filename, or nil if the previous mission is invalid")
{
	if (Campaign.prev_mission < 0 || Campaign.prev_mission >= MAX_CAMPAIGN_MISSIONS) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", Campaign.missions[Campaign.prev_mission].name);
}

// DahBlount - This jumps to a mission, the reason it accepts a boolean value is so that players can return to campaign maps
ADE_FUNC(jumpToMission, l_Campaign, "string filename, [boolean hub]", "Jumps to a mission based on the filename. Optionally, the player can be sent to a hub mission without setting missions to skipped.", "boolean", "Jumps to a mission, or returns nil.")
{
	char *filename = NULL;;
	bool hub = false;
	if (!ade_get_args(L, "s|b", &filename, &hub))
		return ADE_RETURN_NIL;

	mission_campaign_jump_to_mission(filename, hub);

	return ADE_RETURN_TRUE;
}

// TODO: add a proper indexer type that returns a handle
// something like ca.Mission[filename/index]

//****SUBLIBRARY: Mission/Wings
ade_lib l_Mission_Wings("Wings", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Wings, "number Index/string WingName", "Wings in the mission", "wing", "Wing handle, or invalid wing handle if index or name was invalid")
{
	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_Wing.Set(-1));

	//MageKing17 - Make the count-ignoring version of the lookup and leave checking if the wing has any ships to the scripter
	int idx = wing_lookup(name);
	
	if(idx < 0)
	{
		idx = atoi(name);
		if(idx < 1 || idx > Num_wings)
			return ade_set_error(L, "o", l_Wing.Set(-1));

		idx--;	//Lua->FS2
	}

	return ade_set_args(L, "o", l_Wing.Set(idx));
}

ADE_FUNC(__len, l_Mission_Wings, NULL, "Number of wings in mission", "number", "Number of wings in mission")
{
	return ade_set_args(L, "i", Num_wings);
}

//****SUBLIBRARY: Mission/Teams
ade_lib l_Mission_Teams("Teams", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Teams, "number Index/string TeamName", "Teams in the mission", "team", "Team handle or invalid team handle if the requested team could not be found")
{
	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_Team.Set(-1));

	int idx = iff_lookup(name);
	
	if(idx < 0)
	{
		idx = atoi(name);

		idx--;	//Lua->FS2
	}

	if(idx < 0 || idx >= Num_iffs)
		return ade_set_error(L, "o", l_Team.Set(-1));

	return ade_set_args(L, "o", l_Team.Set(idx));
}

ADE_FUNC(__len, l_Mission_Teams, NULL, "Number of teams in mission", "number", "Number of teams in mission")
{
	return ade_set_args(L, "i", Num_iffs);
}

//****SUBLIBRARY: Mission/Messages
ade_lib l_Mission_Messages("Messages", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Messages, "number Index/string messageName", "Messages of the mission", "message", "Message handle or invalid handle on error")
{
	int idx = -1;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "*i", &idx))
			return ade_set_args(L, "o", l_Message.Set(-1));

		idx--; // Lua --> FS2

		idx += Num_builtin_messages;
	}
	else
	{
		char* name = NULL;

		if (!ade_get_args(L, "*s", &name))
			return ade_set_args(L, "o", l_Message.Set(-1));

		if (name == NULL)
			return ade_set_args(L, "o", l_Message.Set(-1));

		for (int i = Num_builtin_messages; i < (int) Messages.size(); i++)
		{
			if (!stricmp(Messages[i].name, name))
			{
				idx = i;
				break;
			}
		}
	}

	if (idx < Num_builtin_messages || idx >= (int) Messages.size())
		return ade_set_args(L, "o", l_Message.Set(-1));
	else
		return ade_set_args(L, "o", l_Message.Set(idx));
}

ADE_FUNC(__len, l_Mission_Messages, NULL, "Number of messages in the mission", "number", "Number of messages in mission")
{
	return ade_set_args(L, "i", (int) Messages.size() - Num_builtin_messages);
}

//****SUBLIBRARY: Mission/BuiltinMessages
ade_lib l_Mission_BuiltinMessages("BuiltinMessages", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_BuiltinMessages, "number Index/string messageName", "Built-in messages of the mission", "message", "Message handle or invalid handle on error")
{
	int idx = -1;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "*i", &idx))
			return ade_set_args(L, "o", l_Message.Set(-1));

		idx--; // Lua --> FS2
	}
	else
	{
		char* name = NULL;

		if (!ade_get_args(L, "*s", &name))
			return ade_set_args(L, "o", l_Message.Set(-1));

		if (name == NULL)
			return ade_set_args(L, "o", l_Message.Set(-1));

		for (int i = 0; i < Num_builtin_messages; i++)
		{
			if (!stricmp(Messages[i].name, name))
			{
				idx = i;
				break;
			}
		}
	}

	if (idx < 0 || idx >= Num_builtin_messages)
		return ade_set_args(L, "o", l_Message.Set(-1));
	else
		return ade_set_args(L, "o", l_Message.Set(idx));
}

ADE_FUNC(__len, l_Mission_BuiltinMessages, NULL, "Number of built-in messages in the mission", "number", "Number of messages in mission")
{
	return ade_set_args(L, "i", Num_builtin_messages);
}

//****SUBLIBRARY: Mission/Personas
ade_lib l_Mission_Personas("Personas", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Personas, "number Index/string name", "Personas of the mission", "persona", "Persona handle or invalid handle on error")
{
	int idx = -1;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "*i", &idx))
			return ade_set_args(L, "o", l_Persona.Set(-1));

		idx--; // Lua --> FS2
	}
	else
	{
		char* name = NULL;

		if (!ade_get_args(L, "*s", &name))
			return ade_set_args(L, "o", l_Persona.Set(-1));

		if (name == NULL)
			return ade_set_args(L, "o", l_Persona.Set(-1));

		idx = message_persona_name_lookup(name);
	}

	if (idx < 0 || idx >= Num_personas)
		return ade_set_args(L, "o", l_Persona.Set(-1));
	else
		return ade_set_args(L, "o", l_Persona.Set(idx));
}

ADE_FUNC(__len, l_Mission_Personas, NULL, "Number of personas in the mission", "number", "Number of messages in mission")
{
	return ade_set_args(L, "i", Num_personas);
}

ADE_FUNC(addMessage, l_Mission, "string name, string text[, persona persona]", "Adds a message", "message", "The new message or invalid handle on error")
{
	char* name = NULL;
	char* text = NULL;
	int personaIdx = -1;

	if (!ade_get_args(L, "ss|o", &name, &text, l_Persona.Get(&personaIdx)))
		return ade_set_error(L, "o", l_Message.Set(-1));

	if (name == NULL || text == NULL)
		return ade_set_error(L, "o", l_Message.Set(-1));

	if (personaIdx < 0 || personaIdx >= Num_personas)
		personaIdx = -1;

	add_message(name, text, personaIdx, 0);

	return ade_set_error(L, "o", l_Message.Set((int) Messages.size() - 1));
}

ADE_FUNC(sendMessage, l_Mission, "string sender, message message[, number delay=0.0[, enumeration priority = MESSAGE_PRIORITY_NORMAL[, boolean fromCommand = false]]]",
		 "Sends a message from the given source (not from a ship!) with the given priority or optionally sends it from the missions command source.<br>"
		 "If delay is specified the message will be delayed by the specified time in seconds<br>"
		 "If you pass <i>nil</i> as the sender then the message will not have a sender.",
		 "boolean", "true if successfull, false otherwise")
{
	char* sender = NULL;
	int messageIdx = -1;
	int priority = MESSAGE_PRIORITY_NORMAL;
	bool fromCommand = false;
	int messageSource = MESSAGE_SOURCE_SPECIAL;
	float delay = 0.0f;

	enum_h* ehp = NULL;

	// if first is nil then use no source
	if (lua_isnil(L, 1))
	{
		if (!ade_get_args(L, "*o|fob", l_Message.Get(&messageIdx), &delay, l_Enum.GetPtr(&ehp), &fromCommand))
			return ADE_RETURN_FALSE;

		messageSource = MESSAGE_SOURCE_NONE;
	}
	else
	{
		if (!ade_get_args(L, "so|fob", &sender, l_Message.Get(&messageIdx), &delay, l_Enum.GetPtr(&ehp), &fromCommand))
			return ADE_RETURN_FALSE;

		if (sender == NULL)
			return ADE_RETURN_FALSE;
	}

	if (fromCommand)
		messageSource = MESSAGE_SOURCE_COMMAND;

	if (messageIdx < 0 || messageIdx >= (int) Messages.size())
		return ADE_RETURN_FALSE;

	if (messageIdx < Num_builtin_messages)
	{
		LuaError(L, "Cannot send built-in messages!");
		return ADE_RETURN_FALSE;
	}

	if (delay < 0.0f)
	{
		LuaError(L, "Invalid negative delay of %f!", delay);
		return ADE_RETURN_FALSE;
	}

	if (ehp != NULL)
	{
		switch(ehp->index)
		{
		case LE_MESSAGE_PRIORITY_HIGH:
			priority = MESSAGE_PRIORITY_HIGH;
			break;
		case LE_MESSAGE_PRIORITY_NORMAL:
			priority = MESSAGE_PRIORITY_NORMAL;
			break;
		case LE_MESSAGE_PRIORITY_LOW:
			priority = MESSAGE_PRIORITY_LOW;
			break;
		default:
			LuaError(L, "Invalid enumeration used! Must be one of MESSAGE_PRIORITY_*.");
			return ADE_RETURN_FALSE;
		}
	}

	if (messageSource == MESSAGE_SOURCE_NONE)
		message_send_unique_to_player(Messages[messageIdx].name, NULL, MESSAGE_SOURCE_NONE, priority, 0, fl2i(delay * 1000.0f));
	else
		message_send_unique_to_player(Messages[messageIdx].name, (void*) sender, messageSource, priority, 0, fl2i(delay * 1000.0f));

	return ADE_RETURN_TRUE;
}

ADE_FUNC(sendTrainingMessage, l_Mission, "message message, number time[, number delay=0.0]",
		 "Sends a training message to the player. <i>time</i> is the amount in seconds to display the message, only whole seconds are used!",
		 "boolean", "true if successfull, false otherwise")
{
	int messageIdx = -1;
	float delay = 0.0f;
	int time = -1;

	if (!ade_get_args(L, "oi|f", l_Message.Get(&messageIdx), &time, &delay))
		return ADE_RETURN_FALSE;

	if (messageIdx < 0 || messageIdx >= (int) Messages.size())
		return ADE_RETURN_FALSE;

	if (delay < 0.0f)
	{
		LuaError(L, "Got invalid delay of %f seconds!", delay);
		return ADE_RETURN_FALSE;
	}

	if (time < 0)
	{
		LuaError(L, "Got invalid time of %d seconds!", time);
		return ADE_RETURN_FALSE;
	}

	message_training_queue(Messages[messageIdx].name, timestamp(fl2i(delay * 1000.0f)), time);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(createShip, l_Mission, "[string Name, shipclass Class=Shipclass[1], orientation Orientation=null, vector Position={0,0,0}]", "Creates a ship and returns a handle to it using the specified name, class, world orientation, and world position", "ship", "Ship handle, or invalid ship handle if ship couldn't be created")
{
	char *name = NULL;
	int sclass = -1;
	matrix_h *orient = NULL;
	vec3d pos = vmd_zero_vector;
	ade_get_args(L, "|sooo", &name, l_Shipclass.Get(&sclass), l_Matrix.GetPtr(&orient), l_Vector.Get(&pos));

	matrix *real_orient = &vmd_identity_matrix;
	if(orient != NULL)
	{
		real_orient = orient->GetMatrix();
	}
	
	int obj_idx = ship_create(real_orient, &pos, sclass, name);

	if(obj_idx > -1) {
		model_page_in_textures(Ship_info[sclass].model_num, sclass);

		return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[obj_idx]), Objects[obj_idx].signature));
	} else
		return ade_set_error(L, "o", l_Ship.Set(object_h()));
}

ADE_FUNC(createWaypoint, l_Mission, "[vector Position, waypointlist List]",
		 "Creates a waypoint",
		 "waypoint",
		 "Waypoint handle, or invalid waypoint handle if waypoint couldn't be created")
{
	vec3d *v3 = NULL;
	waypointlist_h *wlh = NULL;
	if(!ade_get_args(L, "|oo", l_Vector.GetPtr(&v3), l_WaypointList.GetPtr(&wlh)))
		return ade_set_error(L, "o", l_Waypoint.Set(object_h()));

	// determine where we need to create it - it looks like we were given a waypoint list but not a waypoint itself
	int waypoint_instance = -1;
	if (wlh && wlh->IsValid())
	{
		int wp_list_index = find_index_of_waypoint_list(wlh->wlp);
		int wp_index = (int) wlh->wlp->get_waypoints().size() - 1;
		waypoint_instance = calc_waypoint_instance(wp_list_index, wp_index);
	}
	int obj_idx = waypoint_add(v3 != NULL ? v3 : &vmd_zero_vector, waypoint_instance);

	if(obj_idx >= 0)
		return ade_set_args(L, "o", l_Waypoint.Set(object_h(&Objects[obj_idx])));
	else
		return ade_set_args(L, "o", l_Waypoint.Set(object_h()));
}

ADE_FUNC(createWeapon, l_Mission, "[weaponclass Class=WeaponClass[1], orientation Orientation=null, world vector Position={0,0,0}, object Parent = nil, number Group = -1",
		 "Creates a weapon and returns a handle to it. 'Group' is used for lighting grouping purposes;"
		 " for example, quad lasers would only need to act as one light source.",
		 "weapon",
		 "Weapon handle, or invalid weapon handle if weapon couldn't be created.")
{
	int wclass = -1;
	object_h *parent = NULL;
	int group = -1;
	matrix_h *orient = NULL;
	vec3d pos = vmd_zero_vector;
	ade_get_args(L, "|ooooi", l_Weaponclass.Get(&wclass), l_Matrix.GetPtr(&orient), l_Vector.Get(&pos), l_Object.GetPtr(&parent), &group);

	matrix *real_orient = &vmd_identity_matrix;
	if(orient != NULL)
	{
		real_orient = orient->GetMatrix();
	}

	int parent_idx = (parent && parent->IsValid()) ? OBJ_INDEX(parent->objp) : -1;

	int obj_idx = weapon_create(&pos, real_orient, wclass, parent_idx, group);

	if(obj_idx > -1)
		return ade_set_args(L, "o", l_Weapon.Set(object_h(&Objects[obj_idx]), Objects[obj_idx].signature));
	else
		return ade_set_error(L, "o", l_Weapon.Set(object_h()));
}

ADE_FUNC(getMissionFilename, l_Mission, NULL, "Gets mission filename", "string", "Mission filename, or empty string if game is not in a mission")
{
	return ade_set_args(L, "s", Game_current_mission_filename);
}

ADE_FUNC(startMission, l_Mission, "[Filename or MISSION_* enumeration, Briefing = true]", "Starts the defined mission", "boolean", "True, or false if the function fails")
{
	bool b = true;
	char s[MAX_FILENAME_LEN];
	char *str = s;

	if(lua_isstring(L, 1))
	{
		if (!ade_get_args(L, "s|b", &str, &b))
			return ade_set_args(L, "b", false);

	} else {
		enum_h *e = NULL;

		if (!ade_get_args(L, "o|b", l_Enum.GetPtr(&e), &b))
			return ade_set_args(L, "b", false);

		if (e->index == LE_MISSION_REPEAT) {
			if (Num_recent_missions > 0)  {
				strncpy( s, Recent_missions[0], MAX_FILENAME_LEN );
			} else {
				return ade_set_args(L, "b", false);
			}
		} else {
			return ade_set_args(L, "b", false);
		}
	}

	// no filename... bail
	if (str == NULL)
		return ade_set_args(L, "b", false);

	// if mission name has extension... it needs to be removed...
	char *file_ext;

	file_ext = strrchr(str, '.');
	if (file_ext)
		*file_ext = 0;

	// game is in MP mode... or if the file does not exist... bail
	if ((Game_mode & GM_MULTIPLAYER) || (cf_exists_full(str, CF_TYPE_MISSIONS) != 0))
		return ade_set_args(L, "b", false);

	// mission is already running...
	if (Game_mode & GM_IN_MISSION) {
		// TO DO... All the things needed if this function is called in any state of the game while mission is running.
		//    most likely all require 'stricmp(str, Game_current_mission_filename)' to make sure missions arent mixed
		//    but after that it might be possible to imprement method for jumping directly into already running 
		//    missions.
		return ade_set_args(L, "b", false);
	// if mission is not running
	} else {
		// due safety checks of the game_start_mission() function allow only main menu for now.
		if (gameseq_get_state(gameseq_get_depth()) == GS_STATE_MAIN_MENU) {
			strcpy_s( Game_current_mission_filename, str );
			if (b == true) {
				// start mission - go via briefing screen
				gameseq_post_event(GS_EVENT_START_GAME);
			} else {
				// start mission - enter the game directly
				gameseq_post_event(GS_EVENT_START_GAME_QUICK);
			}
			return ade_set_args(L, "b", true);
		}
	}
	return ade_set_args(L, "b", false);
}

ADE_FUNC(getMissionTime, l_Mission, NULL, "Game time in seconds since the mission was started; is affected by time compression", "number", "Mission time (seconds), or 0 if game is not in a mission")
{
	if(!(Game_mode & GM_IN_MISSION))
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "x", Missiontime);
}

//WMC - These are in freespace.cpp
ADE_FUNC(loadMission, l_Mission, "Mission name", "Loads a mission", "boolean", "True if mission was loaded, otherwise false")
{
	char *s;
	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "b", false);

	// clear post processing settings
	gr_post_process_set_defaults();

	//NOW do the loading stuff
	game_stop_time();
	get_mission_info(s, &The_mission, false);
	game_level_init();

	if(mission_load(s) == -1)
		return ADE_RETURN_FALSE;

	game_post_level_init();

	Game_mode |= GM_IN_MISSION;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(unloadMission, l_Mission, NULL, "Stops the current mission and unloads it", NULL, NULL)
{
	if(Game_mode & GM_IN_MISSION)
	{
		game_level_close();
		Game_mode &= ~GM_IN_MISSION;
		strcpy_s(Game_current_mission_filename, "");
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(simulateFrame, l_Mission, NULL, "Simulates mission frame", NULL, NULL)
{
	game_update_missiontime();
	game_simulation_frame();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(renderFrame, l_Mission, NULL, "Renders mission frame, but does not move anything", NULL, NULL)
{
	camid cid = game_render_frame_setup();
	game_render_frame( cid );
	game_render_post_frame();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(applyShudder, l_Mission, "number time, number intesity", "Applies a shudder effects to the camera. Time is in seconds. Intensity specifies the shudder effect strength, the Maxim has a value of 1440.", "boolean", "true if successfull, false otherwise")
{
	float time = -1.0f;
	float intensity = -1.0f;

	if (!ade_get_args(L, "ff", &time, &intensity))
		return ADE_RETURN_FALSE;

	if (time < 0.0f || intensity < 0.0f)
	{
		LuaError(L, "Illegal shudder values given. Must be bigger than zero, got time of %f and intensity of %f.", time, intensity);
		return ADE_RETURN_FALSE;
	}

	int int_time = fl2i(time * 1000.0f);

	game_shudder_apply(int_time, intensity * 0.01f);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(isInCampaign, l_Mission, NULL, "Get whether or not the current mission being played in a campaign (as opposed to the tech room's simulator)", "boolean", "true if in campaign, false if not")
{
	bool b = false;

	if (Game_mode & GM_CAMPAIGN_MODE) {
		b = true;
	}

	return ade_set_args(L, "b", b);
}

//**********LIBRARY: Tables
ade_lib l_Tables("Tables", NULL, "tb", "Tables library");

//*****SUBLIBRARY: Tables/ShipClasses
ade_lib l_Tables_ShipClasses("ShipClasses", &l_Tables, NULL, NULL);
ADE_INDEXER(l_Tables_ShipClasses, "number Index/string Name", "Array of ship classes", "shipclass", "Ship handle, or invalid ship handle if index is invalid")
{
	if(!ships_inited)
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	int idx = ship_info_lookup(name);
	
	if(idx < 0) {
		idx = atoi(name);
		if(idx < 1 || idx >= static_cast<int>(Ship_info.size()))
			return ade_set_error(L, "o", l_Shipclass.Set(-1));

		idx--;	//Lua->FS2
	}

	return ade_set_args(L, "o", l_Shipclass.Set(idx));
}

ADE_FUNC(__len, l_Tables_ShipClasses, NULL, "Number of ship classes", "number", "Number of ship classes, or 0 if ship classes haven't been loaded yet")
{
	if(!ships_inited)
		return ade_set_args(L, "i", 0);	//No ships loaded...should be 0

	return ade_set_args(L, "i", Ship_info.size());
}

//*****SUBLIBRARY: Tables/WeaponClasses
ade_lib l_Tables_WeaponClasses("WeaponClasses", &l_Tables, NULL, NULL);

extern int Weapons_inited;

ADE_INDEXER(l_Tables_WeaponClasses, "number Index/string WeaponName", "Array of weapon classes", "weapon", "Weapon class handle, or invalid weaponclass handle if index is invalid")
{
	if(!Weapons_inited)
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	char *name;
	if(!ade_get_args(L, "*s", &name))
		return 0;

	int idx = weapon_info_lookup(name);
	
	if(idx < 0) {
		idx = atoi(name);

		// atoi is good enough here, 0 is invalid anyway
		if (idx > 0)
		{
			idx--; // Lua --> C/C++
		}
		else
		{
			return ade_set_args(L, "o", l_Weaponclass.Set(-1));
		}
	}

	return ade_set_args(L, "o", l_Weaponclass.Set(idx));
}

ADE_FUNC(__len, l_Tables_WeaponClasses, NULL, "Number of weapon classes", "number", "Number of weapon classes, or 0 if weapon classes haven't been loaded yet")
{
	if(!Weapons_inited)
		return ade_set_args(L, "i", 0);

	return ade_set_args(L, "i", Num_weapon_types);
}

//*************************Testing stuff*************************
//This section is for stuff that's considered experimental.
ade_lib l_Testing("Testing", NULL, "ts", "Experimental or testing stuff");

ADE_FUNC(avdTest, l_Testing, NULL, "Test the AVD Physics code", NULL, NULL)
{
	static bool initialized = false;
	static avd_movement avd;

	if(!initialized)
	{
		avd.setAVD(10.0f, 3.0f, 1.0f, 1.0f, 0.0f);
		initialized = true;
	}
	for(int i = 0; i < 3000; i++)
	{
		float Pc, Vc;
		avd.get((float)i/1000.0f, &Pc, &Vc);
		gr_set_color(0, 255, 0);
		gr_pixel(i/10, gr_screen.clip_bottom - (int)(Pc*10.0f), GR_RESIZE_NONE);
		gr_set_color(255, 0, 0);
		gr_pixel(i/10, gr_screen.clip_bottom - (int)(Vc*10.0f), GR_RESIZE_NONE);

		avd.get(&Pc, &Vc);
		gr_set_color(255, 255, 255);
		gr_pixel((timestamp()%3000)/10, gr_screen.clip_bottom - (int)(Pc*10.0f), GR_RESIZE_NONE);
		gr_pixel((timestamp()%3000)/10, gr_screen.clip_bottom - (int)(Vc*10.0f), GR_RESIZE_NONE);
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(createParticle, l_Testing, "vector Position, vector Velocity, number Lifetime, number Radius, enumeration Type, [number Tracer length=-1, boolean Reverse=false, texture Texture=Nil, object Attached Object=Nil]",
		 "Creates a particle. Use PARTICLE_* enumerations for type."
		 "Reverse reverse animation, if one is specified"
		 "Attached object specifies object that Position will be (and always be) relative to.",
		 "particle",
		 "Handle to the created particle")
{
	particle::particle_info pi;
	pi.type = particle::PARTICLE_DEBUG;
	pi.optional_data = -1;
	pi.attached_objnum = -1;
	pi.attached_sig = -1;
	pi.reverse = 0;

	// Need to consume tracer_length parameter but it isn't used anymore
	float temp;

	enum_h *type = NULL;
	bool rev=false;
	object_h *objh=NULL;
	if(!ade_get_args(L, "ooffo|fboo", l_Vector.Get(&pi.pos), l_Vector.Get(&pi.vel), &pi.lifetime, &pi.rad, l_Enum.GetPtr(&type), &temp, &rev, l_Texture.Get((int*)&pi.optional_data), l_Object.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(type != NULL)
	{
		switch(type->index)
		{
			case LE_PARTICLE_DEBUG:
				pi.type = particle::PARTICLE_DEBUG;
				break;
			case LE_PARTICLE_FIRE:
				pi.type = particle::PARTICLE_FIRE;
				break;
			case LE_PARTICLE_SMOKE:
				pi.type = particle::PARTICLE_SMOKE;
				break;
			case LE_PARTICLE_SMOKE2:
				pi.type = particle::PARTICLE_SMOKE2;
				break;
			case LE_PARTICLE_BITMAP:
				if (pi.optional_data < 0)
				{
					LuaError(L, "Invalid texture specified for createParticle()!");
				}

				pi.type = particle::PARTICLE_BITMAP;
				break;
		}
	}

	if(rev)
		pi.reverse = 0;

	if(objh != NULL && objh->IsValid())
	{
		pi.attached_objnum = OBJ_INDEX(objh->objp);
		pi.attached_sig = objh->objp->signature;
	}

    particle::WeakParticlePtr p = particle::create(&pi);

	if (!p.expired())
		return ade_set_args(L, "o", l_Particle.Set(particle_h(p)));
	else
		return ADE_RETURN_NIL;
}

ADE_FUNC(getStack, l_Testing, NULL, "Generates an ADE stackdump", "string", "Current Lua stack")
{
	char buf[10240] = {'\0'};
	ade_stackdump(L, buf);
	return ade_set_args(L, "s", buf);
}

ADE_FUNC(isCurrentPlayerMulti, l_Testing, NULL, "Returns whether current player is a multiplayer pilot or not.", "boolean", "Whether current player is a multiplayer pilot or not")
{
	if(Player == NULL)
		return ade_set_error(L, "b", false);

	if(!(Player->flags & PLAYER_FLAGS_IS_MULTI))
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

// Om_tracker_flag should already be set in FreeSpace.cpp, needed to determine if PXO is enabled from the registry
extern int Om_tracker_flag; // needed for FS2OpenPXO config

ADE_FUNC(isPXOEnabled, l_Testing, NULL, "Returns whether PXO is currently enabled in the configuration.", "boolean", "Whether PXO is enabled or not")
{
	if(!(Om_tracker_flag))
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(playCutscene, l_Testing, NULL, "Forces a cutscene by the specified filename string to play. Should really only be used in a non-gameplay state (i.e. start of GS_STATE_BRIEFING) otherwise odd side effects may occur. Highly Experimental.", "string", NULL)
{
	//This whole thing is a quick hack and can probably be done way better, but is currently functioning fine for my purposes.
	char *filename;

	if (!ade_get_args(L, "s", &filename))
		return ADE_RETURN_FALSE;

	movie::play(filename);

	return ADE_RETURN_TRUE;
}

// *************************Helper functions*********************
//WMC - This should be used anywhere that an 'object' is set, so
//that scripters can get access to as much relevant data to that
//object as possible.
//It should also be updated as new types are added to Lua.
int ade_set_object_with_breed(lua_State *L, int obj_idx)
{
	if(obj_idx < 0 || obj_idx >= MAX_OBJECTS)
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	object *objp = &Objects[obj_idx];

	switch(objp->type)
	{
		case OBJ_SHIP:
			return ade_set_args(L, "o", l_Ship.Set(object_h(objp)));
		case OBJ_ASTEROID:
			return ade_set_args(L, "o", l_Asteroid.Set(object_h(objp)));
		case OBJ_DEBRIS:
			return ade_set_args(L, "o", l_Debris.Set(object_h(objp)));
		case OBJ_WAYPOINT:
			return ade_set_args(L, "o", l_Waypoint.Set(object_h(objp)));
		case OBJ_WEAPON:
			return ade_set_args(L, "o", l_Weapon.Set(object_h(objp)));
		case OBJ_BEAM:
			return ade_set_args(L, "o", l_Beam.Set(object_h(objp)));
		default:
			return ade_set_args(L, "o", l_Object.Set(object_h(objp)));
	}
}

//###########################################################
//########################<IMPORTANT>########################
//###########################################################
//If you are a coder who wants to add libraries, functions,
//or objects to Lua, then you want to be above this point.
//###########################################################
//########################</IMPORTANT>#######################
//###########################################################

// *************************Housekeeping*************************
//WMC - The miraculous lines of code that make Lua debugging worth something.
lua_Debug Ade_debug_info;
char debug_stack[4][32];

void ade_debug_ret(lua_State *L, lua_Debug *ar)
{
	Assert(L != NULL);
	Assert(ar != NULL);
	lua_getstack(L, 1, ar);
	lua_getinfo(L, "nSlu", ar);
	memcpy(&Ade_debug_info, ar, sizeof(lua_Debug));

	int n;
	for (n = 0; n < 4; n++) {
		debug_stack[n][0] = '\0';
	}

	for (n = 0; n < 4; n++) {
		if (lua_getstack(L,n+1, ar) == 0)
			break;
		lua_getinfo(L,"n", ar);
		if (ar->name == NULL)
			break;
		strcpy_s(debug_stack[n],ar->name);
	}
}

//WMC - because the behavior of the return keyword
//was changed, I now have to use this in hooks.
static int ade_return_hack(lua_State *L)
{
	int i = 0;
	int num = lua_gettop(L);
	for(i = 0; i < num; i++)
	{
		lua_pushvalue(L, i+1);
	}

	return num;
}

static void *vm_lua_alloc(void*, void *ptr, size_t, size_t nsize) {
	if (nsize == 0)
	{
		vm_free(ptr);
		return NULL;
	}
	else
	{
		return vm_realloc(ptr, nsize);
	}
}

//Inits LUA
//Note that "libraries" must end with a {NULL, NULL}
//element
int script_state::CreateLuaState()
{
	mprintf(("LUA: Opening LUA state...\n"));
	lua_State *L = lua_newstate(vm_lua_alloc, nullptr);

	if(L == NULL)
	{
		Warning(LOCATION, "Could not initialize Lua");
		return 0;
	}

	//*****INITIALIZE AUXILIARY LIBRARIES
	mprintf(("LUA: Initializing base Lua libraries...\n"));
	luaL_openlibs(L);

	//*****DISABLE DANGEROUS COMMANDS
	lua_pushstring(L, "os");
	lua_rawget(L, LUA_GLOBALSINDEX);
	int os_ldx = lua_gettop(L);
	if(lua_istable(L, os_ldx))
	{
		lua_pushstring(L, "execute");
		lua_pushnil(L);
		lua_rawset(L, os_ldx);
		lua_pushstring(L, "remove");
		lua_pushnil(L);
		lua_rawset(L, os_ldx);
		lua_pushstring(L, "rename");
		lua_pushnil(L);
		lua_rawset(L, os_ldx);
	}
	lua_pop(L, 1);	//os table

	//*****SET DEBUG HOOKS
#ifndef NDEBUG
	lua_sethook(L, ade_debug_ret, LUA_MASKRET, 0);
#endif

	//*****INITIALIZE ADE
	uint i;
	mprintf(("LUA: Beginning ADE initialization\n"));
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		//WMC - Do only toplevel table entries, doi
		if(ade_manager::getInstance()->getEntry(i).ParentIdx == UINT_MAX)			//WMC - oh hey, we're done with the meaty point in < 10 lines.
			ade_manager::getInstance()->getEntry(i).SetTable(L, LUA_GLOBALSINDEX, LUA_GLOBALSINDEX);	//Oh the miracles of OOP.
	}

	//*****INITIALIZE RETURN HACK FUNCTION
	lua_pushstring(L, "ade_return_hack");
	lua_pushboolean(L, 0);
	lua_pushcclosure(L, ade_return_hack, 2);
	lua_setglobal(L, "ade_return_hack");

	//*****INITIALIZE ENUMERATION CONSTANTS
	mprintf(("ADE: Initializing enumeration constants...\n"));
	enum_h eh;
	for(i = 0; i < Num_enumerations; i++)
	{
		eh.index = Enumerations[i].def;
		eh.is_constant = true;

		ade_set_args(L, "o", l_Enum.Set(eh));
		lua_setglobal(L, Enumerations[i].name);
	}

	//*****ASSIGN LUA SESSION
	mprintf(("ADE: Assigning Lua session...\n"));
	SetLuaSession(L);

//	(void)l_BitOps.GetName();

	return 1;
}

void script_state::EndLuaFrame()
{
	memcpy(NextDrawStringPos, NextDrawStringPosInitial, sizeof(NextDrawStringPos));
}

void ade_output_toc(FILE *fp, ade_table_entry *ate)
{
	Assert(fp != NULL);
	Assert(ate != NULL);

	//WMC - sanity checking
	if(ate->Name == NULL && ate->ShortName == NULL) {
		Warning(LOCATION, "Found ade_table_entry with no name or shortname");
		return;
	}

	fputs("<dd>", fp);

	if(ate->Name == NULL)
	{
		fprintf(fp, "<a href=\"#%s\">%s", ate->ShortName, ate->ShortName);
	}
	else
	{
		fprintf(fp, "<a href=\"#%s\">%s", ate->Name, ate->Name);
		if(ate->ShortName)
			fprintf(fp, " (%s)", ate->ShortName);
	}
	fputs("</a>", fp);

	if(ate->Description)
		fprintf(fp, " - %s\n", ate->Description);

	fputs("</dd>\n", fp);
}

static bool sort_table_entries(const ade_table_entry* left, const ade_table_entry* right) {
	const char* leftCmp = left->Name != nullptr ? left->Name : left->ShortName;
	const char* rightCmp = right->Name != nullptr ? right->Name : left->ShortName;

	if (leftCmp == nullptr) {
		return false;
	}
	if (rightCmp == nullptr) {
		return false;
	}

	SCP_string leftStr(leftCmp);
	std::transform(std::begin(leftStr), std::end(leftStr), std::begin(leftStr), ::tolower);

	SCP_string rightStr(rightCmp);
	std::transform(std::begin(rightStr), std::end(rightStr), std::begin(rightStr), ::tolower);

	return leftStr < rightStr;
}

static bool sort_doc_entries(const ade_table_entry* left, const ade_table_entry* right) {
	if (left->Instanced == right->Instanced) {
		// Same type -> compare as normal
		return sort_table_entries(left, right);
	}
	if (left->Instanced) {
		return true;
	}
	return false;
}

void script_state::OutputLuaMeta(FILE *fp)
{
	uint i;
	ade_table_entry *ate;
	fputs("<dl>\n", fp);

	SCP_vector<ade_table_entry*> table_entries;

	//***TOC: Libraries
	fputs("<dt><b>Libraries</b></dt>\n", fp);
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		ate = &ade_manager::getInstance()->getEntry(i);
		if(ate->ParentIdx == UINT_MAX && ate->Type == 'o' && ate->Instanced) {
			table_entries.push_back(ate);
		}
	}
	std::sort(std::begin(table_entries), std::end(table_entries), sort_table_entries);
	for (auto entry : table_entries) {
		ade_output_toc(fp, entry);
	}
	table_entries.clear();

	//***TOC: Objects
	fputs("<dt><b>Types</b></dt>\n", fp);
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		ate = &ade_manager::getInstance()->getEntry(i);
		if(ate->ParentIdx == UINT_MAX && ate->Type == 'o' && !ate->Instanced) {
			table_entries.push_back(ate);
		}
	}
	std::sort(std::begin(table_entries), std::end(table_entries), sort_table_entries);
	for (auto entry : table_entries) {
		ade_output_toc(fp, entry);
	}
	table_entries.clear();

	//***TOC: Enumerations
	fputs("<dt><b><a href=\"#Enumerations\">Enumerations</a></b></dt>", fp);

	//***End TOC
	fputs("</dl><br/><br/>", fp);

	//***Everything
	fputs("<dl>\n", fp);
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		ate = &ade_manager::getInstance()->getEntry(i);
		if(ate->ParentIdx == UINT_MAX)
			table_entries.push_back(ate);
	}

	std::sort(std::begin(table_entries), std::end(table_entries), sort_doc_entries);
	for (auto entry : table_entries) {
		entry->OutputMeta(fp);
	}
	table_entries.clear();

	//***Enumerations
	fprintf(fp, "<dt id=\"Enumerations\"><h2>Enumerations</h2></dt>");
	for(i = 0; i < Num_enumerations; i++)
	{
		//WMC - This is in case we ever want to add descriptions to enums.
		//fprintf(fp, "<dd><dl><dt><b>%s</b></dt><dd>%s</dd></dl></dd>", Enumerations[i].name, Enumerations[i].desc);

		//WMC - Otherwise, just use this.
		fprintf(fp, "<dd><b>%s</b></dd>", Enumerations[i].name);
	}
	fputs("</dl>\n", fp);

	//***End LUA
	fputs("</dl>\n", fp);
}
