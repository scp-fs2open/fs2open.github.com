/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "stdafx.h"
#include "FRED.h"
#include "MainFrm.h"
#include "FREDDoc.h"
#include "FREDView.h"
#include "FredRender.h"
#include "ai/aigoals.h"
#include "ship/ship.h"
#include "globalincs/linklist.h"
#include "globalincs/version.h"
#include "globalincs/alphacolors.h"
#include "math/curve.h"
#include "mission/missiongrid.h"
#include "mission/missionparse.h"
#include "mission/missionmessage.h"
#include "mission/missiongoals.h"
#include "mission/missionbriefcommon.h"
#include "model/modelreplace.h"
#include "Management.h"
#include "cfile/cfile.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "weapon/weapon.h"
#include "io/key.h"
#include "parse/parselo.h"
#include "math/fvi.h"
#include "starfield/starfield.h"
#include "parse/sexp.h"
#include "parse/sexp/sexp_lookup.h"
#include "io/mouse.h"
#include "mission/missioncampaign.h"
#include "wing.h"
#include "MessageEditorDlg.h"
#include "EventEditor.h"
#include "MissionGoalsDlg.h"
#include "MissionCutscenesDlg.h"
#include "ShieldSysDlg.h"
#include "gamesnd/eventmusic.h"
#include "DebriefingEditorDlg.h"
#include "starfield/nebula.h"
#include "asteroid/asteroid.h"
#include "hud/hudsquadmsg.h"
#include "jumpnode/jumpnode.h"
#include "stats/medals.h"
#include "localization/localize.h" 
#include "osapi/osregistry.h"
#include "localization/fhash.h"
#include "io/timer.h"
#include "nebula/neb.h"
#include "nebula/neblightning.h"
#include "species_defs/species_defs.h"
#include "lighting/lighting_profiles.h"
#include "osapi/osapi.h"
#include "graphics/font.h"
#include "object/objectdock.h"
#include "gamesnd/gamesnd.h"
#include "iff_defs/iff_defs.h"
#include "menuui/techmenu.h"
#include "missionui/fictionviewer.h"
#include "mod_table/mod_table.h"
#include "libs/ffmpeg/FFmpeg.h"
#include "scripting/scripting.h"
#include "scripting/global_hooks.h"
#include "utils/Random.h"

#include <direct.h>
#include "cmdline/cmdline.h"

#define SDL_MAIN_HANDLED
#include <SDL_main.h>

#define MAX_DOCKS 1000

#define UNKNOWN_USER		"Unknown"

extern void ssm_init();	// Need this to populate Ssm_info so OPF_SSM_CLASS does something. -MageKing17

int cur_wing = -1;
int cur_wing_index;
int cur_object_index = -1;
int cur_ship = -1;
int cur_ship_type_combo_index = 0;
waypoint *cur_waypoint = NULL;
waypoint_list *cur_waypoint_list = NULL;
int delete_flag;
int bypass_update = 0;
int Update_ship = 0;
int Update_wing = 0;

char Fred_exe_dir[512] = "";
char Fred_base_dir[512] = "";

char Fred_alt_names[MAX_SHIPS][NAME_LENGTH+1];
char Fred_callsigns[MAX_SHIPS][NAME_LENGTH+1];

extern void allocate_parse_text(size_t size);

// object numbers for ships in a wing.
int wing_objects[MAX_WINGS][MAX_SHIPS_PER_WING];

char *Docking_bay_list[MAX_DOCKS];

// Goober5000
SCP_vector<bool> Show_iff;

CCriticalSection CS_cur_object_index;

// Used in the FRED drop-down menu and in error_check_initial_orders
// NOTE: Certain goals (Form On Wing, Rearm, Chase Weapon, Fly To Ship) aren't listed here.  This may or may not be intentional,
// but if they are added in the future, it will be necessary to verify correct functionality in the various FRED dialog functions.
ai_goal_list Ai_goal_list[] = {
	{ "Waypoints",				AI_GOAL_WAYPOINTS,			0 },
	{ "Waypoints once",			AI_GOAL_WAYPOINTS_ONCE,		0 },
	{ "Attack",					AI_GOAL_CHASE,				0 },
	{ "Attack",					AI_GOAL_CHASE_WING,			0 },	// duplicate needed because we can no longer use bitwise operators
	{ "Attack any ship",		AI_GOAL_CHASE_ANY,			0 },
	{ "Attack ship class",		AI_GOAL_CHASE_SHIP_CLASS,	0 },
	{ "Guard",					AI_GOAL_GUARD,				0 },
	{ "Guard",					AI_GOAL_GUARD_WING,			0 },	// duplicate needed because we can no longer use bitwise operators
	{ "Disable ship",			AI_GOAL_DISABLE_SHIP,		0 },
	{ "Disable ship (tactical)",AI_GOAL_DISABLE_SHIP_TACTICAL, 0 },
	{ "Disarm ship",			AI_GOAL_DISARM_SHIP,		0 },
	{ "Disarm ship (tactical)",	AI_GOAL_DISARM_SHIP_TACTICAL, 0 },
	{ "Destroy subsystem",		AI_GOAL_DESTROY_SUBSYSTEM,	0 },
	{ "Dock",					AI_GOAL_DOCK,				0 },
	{ "Undock",					AI_GOAL_UNDOCK,				0 },
	{ "Warp",					AI_GOAL_WARP,				0 },
	{ "Evade ship",				AI_GOAL_EVADE_SHIP,			0 },
	{ "Ignore ship",			AI_GOAL_IGNORE,				0 },
	{ "Ignore ship (new)",		AI_GOAL_IGNORE_NEW,			0 },
	{ "Stay near ship",			AI_GOAL_STAY_NEAR_SHIP,		0 },
	{ "Keep safe distance",		AI_GOAL_KEEP_SAFE_DISTANCE,	0 },
	{ "Stay still",				AI_GOAL_STAY_STILL,			0 },
	{ "Play dead",				AI_GOAL_PLAY_DEAD,			0 },
	{ "Play dead (persistent)",	AI_GOAL_PLAY_DEAD_PERSISTENT,		0 }
};

int Ai_goal_list_size = sizeof(Ai_goal_list) / sizeof(ai_goal_list);

// internal function prototypes
void set_cur_indices(int obj);
int common_object_delete(int obj);
int create_waypoint(vec3d *pos, int waypoint_instance);
int create_ship(matrix *orient, vec3d *pos, int ship_type);
int query_ship_name_duplicate(int ship);
char *reg_read_string( char *section, char *name, char *default_value );

// Note that the parameter here is max *length*, not max buffer size.  Leave room for the null-terminator!
void string_copy(char *dest, const CString &src, size_t max_len, bool modify)
{
	if (modify)
		if (strcmp(src, dest))
			set_modified();

	auto len = strlen(src);
	if (len > max_len)
		len = max_len;

	strncpy(dest, src, len);
	dest[len] = 0;
}

void string_copy(SCP_string &dest, const CString &src, bool modify)
{
	if (modify)
		if (strcmp(src, dest.c_str()))
			set_modified();

	dest = src;
}

// converts a multiline string (one with newlines in it) into a windows format multiline
// string (newlines changed to '\r\n').
void convert_multiline_string(CString &dest, const SCP_string &src)
{
	dest = src.c_str();
	dest.Replace("\n", "\r\n");
}

// converts a multiline string (one with newlines in it) into a windows format multiline
// string (newlines changed to '\r\n').
void convert_multiline_string(CString &dest, const char *src)
{
	dest = src;
	dest.Replace("\n", "\r\n");
}

// Converts a windows format multiline CString back into a normal multiline string.
void deconvert_multiline_string(char *dest, const CString &str, size_t max_len)
{
	// leave room for the null terminator
	memset(dest, 0, max_len + 1);
	strncpy(dest, (LPCTSTR) str, max_len);

	replace_all(dest, "\r\n", "\n", max_len);
}

// ditto for SCP_string
void deconvert_multiline_string(SCP_string &dest, const CString &str)
{
	dest = str;
	replace_all(dest, "\r\n", "\n");
}

void strip_quotation_marks(CString& str) { str.Remove('\"'); }

void pad_with_newline(CString& str, int max_size) {
	int len = str.GetLength();
	if (!len) {
		len = 1;
	}
	if (str[len - 1] != '\n' && len < max_size) {
		str += _T("\n");
	}
}

void lcl_fred_replace_stuff(CString &text)
{
	// this should be kept in sync with the function in localize.cpp
	text.Replace("\"", "$quote");
	text.Replace(";", "$semicolon");
	text.Replace("/", "$slash");
	text.Replace("\\", "$backslash");
}

CString get_display_name_for_text_box(const char *orig_name)
{
	auto p = get_pointer_to_first_hash_symbol(orig_name);
	if (p)
	{
		// use the same logic as in end_string_at_first_hash_symbol, but rewritten for CString
		CString display_name(orig_name, static_cast<int>(p - orig_name));
		display_name.TrimRight();
		return display_name;
	}
	else
		return "<none>";
}

void fred_preload_all_briefing_icons()
{
	for (SCP_vector<briefing_icon_info>::iterator ii = Briefing_icon_info.begin(); ii != Briefing_icon_info.end(); ++ii)
	{
		generic_anim_load(&ii->regular);
		hud_anim_load(&ii->fade);
		hud_anim_load(&ii->highlight);
	}
}

bool fred_init(std::unique_ptr<os::GraphicsOperations>&& graphicsOps)
{
	int i;

	SDL_SetMainReady();

	Random::seed(static_cast<unsigned int>(time(nullptr)));
	init_pending_messages();

	os_init(Osreg_class_name, Osreg_app_name);

	timer_init();

	Assert(strlen(Fred_base_dir) > 0); //-V805

	// sigh... this should enable proper reading of cmdline_fso.cfg - Goober5000
	cfile_chdir(Fred_base_dir);

	// this should enable mods - Kazan
	if (!parse_cmdline(__argc, __argv)) {
		// Command line contained an option that terminates the program immediately
		exit(1);
	}

#ifndef NDEBUG
	#if FS_VERSION_REVIS == 0
		mprintf(("Fred2 Open version: %i.%i.%i\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD));
	#else
		mprintf(("Fred2 Open version: %i.%i.%i.%i\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, FS_VERSION_REVIS));
	#endif

	extern void cmdline_debug_print_cmdline();
	cmdline_debug_print_cmdline();
#endif

	// d'oh
	if(cfile_init(Fred_exe_dir)){
		exit(1);
	}

	// Load game_settings.tbl
	mod_table_init();

	// initialize localization module. Make sure this is done AFTER initializing OS.
	// NOTE: Fred should ALWAYS run without localization. Otherwise it might swap in another language
	// when saving - which would cause inconsistencies when externalizing to tstrings.tbl via Exstr
	// trust me on this :)
	lcl_init(LCL_UNTRANSLATED);

	// Goober5000 - force init XSTRs (so they work, but only work untranslated, based on above comment)
	extern bool Xstr_inited;
	Xstr_inited = true;

#ifndef NDEBUG
	load_filter_info();
#endif

	//CFREDView *window = CFREDView::GetView();
	//HWND hwndApp = window->GetSafeHwnd();
	//os_set_window((uint) hwndApp);

	snd_init();

	// Not ready for this yet
  //	Cmdline_nospec = 1;
 // 	Cmdline_noglow = 1;
 	Cmdline_window = 1;

	gr_init(std::move(graphicsOps), GR_OPENGL, 640, 480, 32);
	gr_set_gamma(3.0f);

	io::mouse::CursorManager::get()->showCursor(false);

	font::init();					// loads up all fonts  

	curves_init();
	particle::ParticleManager::init();

	gamesnd_parse_soundstbl(true);
	iff_init();						// Goober5000
	species_init();					// Kazan
	gamesnd_parse_soundstbl(false);

	brief_icons_init();

	hud_init_comm_orders();		// Goober5000

	// wookieejedi
	// load in the controls and defaults including the controlconfigdefault.tbl
	// this allows the sexp tree in key-pressed to actually match what the game will use
	// especially useful when a custom Controlconfigdefaults.tbl is used
	control_config_common_init();

	rank_init();
	traitor_init();
	medals_init();			// get medal names for sexpression usage

	key_init();
	mouse_init();

	model_init();
	virtual_pof_init();

	event_music_init();

	alpha_colors_init();

	// get fireball IDs for sexpression usage
	// (we don't need to init the entire system via fireball_init, we just need the information)
	fireball_parse_tbl();

	animation::ModelAnimationParseHelper::parseTables();

	sexp_startup(); // Must happen before ship init for LuaAI

	obj_init();
	armor_init();
	ai_init();
	ai_profiles_init();
	weapon_init();
	glowpoint_init();
	ship_init();

	techroom_intel_init();
	hud_positions_init();

	asteroid_init();
	mission_brief_common_init();

	neb2_init();						// fullneb stuff
	nebl_init();						// neb lightning
	stars_init();
	ssm_init();		// The game calls this after stars_init(), and we need Ssm_info initialized for OPF_SSM_CLASS. -MageKing17

	lighting_profiles::load_profiles();

	// To avoid breaking current mods which do not support scripts in FRED we only initialize the scripting
	// system if a special mod_table option is set
	if (Enable_scripts_in_fred) {
		// Note: Avoid calling any non-script functions after this line and before OnGameInit->run(), lest they run before scripting has completely initialized.
		script_init();			//WMC
	}

	Script_system.RunInitFunctions();
	Scripting_game_init_run = true;	// set this immediately before OnGameInit so that OnGameInit *itself* will run
	if (scripting::hooks::OnGameInit->isActive()) {
		scripting::hooks::OnGameInit->run();
	}
	//Technically after the splash screen, but the best we can do these days. Since the override is hard-deprecated, we don't need to check it.
	if (scripting::hooks::OnSplashScreen->isActive()) {
		scripting::hooks::OnSplashScreen->run();
	}

	// A non-deprecated hook that runs after the splash screen has faded out.
	if (scripting::hooks::OnSplashEnd->isActive()) {
		scripting::hooks::OnSplashEnd->run();
	}

	libs::ffmpeg::initialize();


	// for fred specific replacement texture stuff
	Fred_texture_replacements.clear();

	// Goober5000
	Show_iff.clear();
	for (i = 0; i < (int)Iff_info.size(); i++)
		Show_iff.push_back(true);

	// Goober5000
	strcpy_s(Voice_abbrev_briefing, "");
	strcpy_s(Voice_abbrev_campaign, "");
	strcpy_s(Voice_abbrev_command_briefing, "");
	strcpy_s(Voice_abbrev_debriefing, "");
	strcpy_s(Voice_abbrev_message, "");
	strcpy_s(Voice_abbrev_mission, "");
	Voice_no_replace_filenames = false;
	strcpy_s(Voice_script_entry_format, "Sender: $sender\r\nPersona: $persona\r\nFile: $filename\r\nMessage: $message");
	Voice_export_selection = 0;

	Show_waypoints = TRUE;


	// initialize and activate external string hash table
	// make sure to do here so that we don't parse the table files into the hash table - waste of space
	fhash_init();
	fhash_activate();

	fred_preload_all_briefing_icons(); //phreak.  This needs to be done or else the briefing icons won't show up
	fiction_viewer_reset();
	cmd_brief_reset();

	// mission creation requires the existence of a timestamp snapshot
	timer_start_frame();

	mission_campaign_clear();
	create_new_mission();

	gr_reset_clip();
	g3_start_frame(0);
	g3_set_view_matrix(&eye_pos, &eye_orient, 0.5f);
	
	Fred_main_wnd -> init_tools();

	return true;
}

void set_physics_controls()
{
	physics_init(&view_physics);
	view_physics.max_vel.xyz.x *= physics_speed / 3.0f;
	view_physics.max_vel.xyz.y *= physics_speed / 3.0f;
	view_physics.max_vel.xyz.z *= physics_speed / 3.0f;
	view_physics.max_rear_vel *= physics_speed / 3.0f;

	view_physics.max_rotvel.xyz.x *= physics_rot / 30.0f;
	view_physics.max_rotvel.xyz.y *= physics_rot / 30.0f;
	view_physics.max_rotvel.xyz.z *= physics_rot / 30.0f;
	view_physics.flags |= PF_ACCELERATES | PF_SLIDE_ENABLED;
	theApp.write_ini_file(1);
}

int create_object_on_grid(int waypoint_instance)
{
	int obj = -1;
	float rval;
	vec3d dir,pos;

	g3_point_to_vec_delayed(&dir, marking_box.x2, marking_box.y2);

	rval = fvi_ray_plane(&pos, &The_grid->center, &The_grid->gmatrix.vec.uvec, &view_pos, &dir, 0.0f);

	if (rval>=0.0f) {
		unmark_all();
		obj = create_object(&pos, waypoint_instance);
		if (obj >= 0) {
			mark_object(obj);
			FREDDoc_ptr->autosave("object create");

		} else if (obj == -1)
			Fred_main_wnd->MessageBox("Maximum ship limit reached.  Can't add any more ships.");
	}

	return obj;
}

void fix_ship_name(int ship)
{
	int i = 1;

	do {
		sprintf(Ships[ship].ship_name, "U.R.A. Moron %d", i++);
	} while (query_ship_name_duplicate(ship));
}

int create_ship(matrix *orient, vec3d *pos, int ship_type)
{
	int obj, z1, z2;
	float temp_max_hull_strength;
	ship_info *sip;

	// Save the Current Working dir to restore in a minute - fred is being stupid
	char pwd[MAX_PATH_LEN];
	getcwd(pwd, MAX_PATH_LEN); // get the present working dir - probably <fs2path>[/modpath]/data/missions/

	// "pop" and cfile_chdirs off the stack
	chdir(Fred_base_dir);

	obj = ship_create(orient, pos, ship_type);
	if (obj == -1)
		return -1;

	// ok, done with file io, restore the pwd
	chdir(pwd);

	Objects[obj].phys_info.speed = 33.0f;

	ship *shipp = &Ships[Objects[obj].instance];
	sip = &Ship_info[shipp->ship_info_index];

	if (query_ship_name_duplicate(Objects[obj].instance))
		fix_ship_name(Objects[obj].instance);

	// default stuff according to species and IFF
	shipp->team = Species_info[Ship_info[shipp->ship_info_index].species].default_iff;
	resolve_parse_flags(&Objects[obj], Iff_info[shipp->team].default_parse_flags);

	// default shield setting
	shipp->special_shield = -1;
	z1 = Shield_sys_teams[shipp->team];
	z2 = Shield_sys_types[ship_type];
    if (((z1 == 1) && z2) || (z2 == 1))
        Objects[obj].flags.set(Object::Object_Flags::No_shields);

	// set orders according to whether the ship is on the player ship's team
	{
		object *temp_objp;
		ship *temp_shipp = NULL;

		// find the first player ship
		for (temp_objp = GET_FIRST(&obj_used_list); temp_objp != END_OF_LIST(&obj_used_list); temp_objp = GET_NEXT(temp_objp))
		{
			if (temp_objp->type == OBJ_START)
			{
				temp_shipp = &Ships[temp_objp->instance];
				break;
			}
		}

		// set orders if teams match, or if player couldn't be found
		if (temp_shipp == NULL || shipp->team == temp_shipp->team)
		{
			// if this ship is not a small ship, then make the orders be the default orders without
			// the depart item
			if (!(sip->is_small_ship()))
			{
				shipp->orders_accepted = ship_get_default_orders_accepted( sip );
				shipp->orders_accepted.erase(DEPART_ITEM);
			}
		}
		else
		{
			shipp->orders_accepted.clear();
		}
	}
	
	// calc kamikaze stuff
	if (shipp->use_special_explosion)
	{
		temp_max_hull_strength = (float)shipp->special_exp_blast;
	}
	else
	{
		temp_max_hull_strength = sip->max_hull_strength;
	}

	Ai_info[shipp->ai_index].kamikaze_damage = (int) std::min(1000.0f, 200.0f + (temp_max_hull_strength / 4.0f));

	return obj;
}

int query_ship_name_duplicate(int ship)
{
	int i;

	for (i=0; i<MAX_SHIPS; i++)
		if ((i != ship) && (Ships[i].objnum != -1))
			if (!stricmp(Ships[i].ship_name, Ships[ship].ship_name))
				return 1;

	return 0;
}

void copy_bits(int *dest, int src, int mask)
{
	*dest &= ~mask;
	*dest |= src & mask;
}

int dup_object(object *objp)
{
	int i, j, n, inst, obj = -1;
	ai_info *aip1, *aip2;
	object *objp1, *objp2;
	ship_subsys *subp1, *subp2;
	static int waypoint_instance(-1);

	if (!objp) {
		waypoint_instance = -1;
		return 0;
	}

	inst = objp->instance;
	if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
		obj = create_ship(&objp->orient, &objp->pos, Ships[inst].ship_info_index);
		if (obj == -1)
			return -1;

		n = Objects[obj].instance;
		Ships[n].team = Ships[inst].team;
		Ships[n].arrival_cue = dup_sexp_chain(Ships[inst].arrival_cue);
		Ships[n].departure_cue = dup_sexp_chain(Ships[inst].departure_cue);
		Ships[n].cargo1 = Ships[inst].cargo1;
		Ships[n].arrival_location = Ships[inst].arrival_location;
		Ships[n].departure_location = Ships[inst].departure_location;
		Ships[n].arrival_delay = Ships[inst].arrival_delay;
		Ships[n].departure_delay = Ships[inst].departure_delay;
		Ships[n].weapons = Ships[inst].weapons;
		Ships[n].hotkey = Ships[inst].hotkey;

		aip1 = &Ai_info[Ships[n].ai_index];
		aip2 = &Ai_info[Ships[inst].ai_index];
		aip1->ai_class = aip2->ai_class;
		for (i=0; i<MAX_AI_GOALS; i++)
			aip1->goals[i] = aip2->goals[i];

		if (aip2->ai_flags[AI::AI_Flags::Kamikaze])
			aip1->ai_flags.set(AI::AI_Flags::Kamikaze);
		if (aip2->ai_flags[AI::AI_Flags::No_dynamic])
			aip2->ai_flags.set(AI::AI_Flags::No_dynamic);

		aip1->kamikaze_damage = aip2->kamikaze_damage;

		objp1 = &Objects[obj];
		objp2 = &Objects[Ships[inst].objnum];
		objp1->phys_info.speed = objp2->phys_info.speed;
		objp1->phys_info.fspeed = objp2->phys_info.fspeed;
		objp1->hull_strength = objp2->hull_strength;
		objp1->shield_quadrant[0] = objp2->shield_quadrant[0];

		subp1 = GET_FIRST(&Ships[n].subsys_list);
		subp2 = GET_FIRST(&Ships[inst].subsys_list);
		while (subp1 != END_OF_LIST(&Ships[n].subsys_list)) {
			Assert(subp2 != END_OF_LIST(&Ships[inst].subsys_list));
			subp1 -> current_hits = subp2 -> current_hits;
			subp1 = GET_NEXT(subp1);
			subp2 = GET_NEXT(subp2);
		}

		for (i=0; i<Num_reinforcements; i++)
			if (!stricmp(Reinforcements[i].name, Ships[inst].ship_name)) {
				if (Num_reinforcements < MAX_REINFORCEMENTS) {
					j = Num_reinforcements++;
					strcpy_s(Reinforcements[j].name, Ships[n].ship_name);
					Reinforcements[j].type = Reinforcements[i].type;
					Reinforcements[j].uses = Reinforcements[i].uses;
				}

				break;
			}

	} else if (objp->type == OBJ_WAYPOINT) {
		obj = create_waypoint(&objp->pos, waypoint_instance);
		waypoint_instance = Objects[obj].instance;
	}

	if (obj == -1)
		return -1;

	Objects[obj].pos = objp->pos;
	Objects[obj].orient = objp->orient;
    Objects[obj].flags.set(Object::Object_Flags::Temp_marked);
	return obj;
}

int create_object(vec3d *pos, int waypoint_instance)
{
	int obj, n;

	if (cur_ship_type_combo_index == static_cast<int>(Id_select_type_waypoint)) {
		obj = create_waypoint(pos, waypoint_instance);
	} else if (cur_ship_type_combo_index == static_cast<int>(Id_select_type_jump_node)) {
		CJumpNode jnp(pos);
		obj = jnp.GetSCPObjectNumber();
		Jump_nodes.push_back(std::move(jnp));
	} else {  // creating a ship
		int ship_class = m_new_ship_type_combo_box.GetShipClass(cur_ship_type_combo_index);
		if (ship_class < 0 || ship_class >= ship_info_size())
			return -1;

		obj = create_ship(nullptr, pos, ship_class);
		if (obj == -1)
			return -1;

		n = Objects[obj].instance;
		Ships[n].arrival_cue = alloc_sexp("true", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
		Ships[n].departure_cue = alloc_sexp("false", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
		Ships[n].cargo1 = 0;
	}

	if (obj < 0)
		return obj;

	obj_merge_created_list();
	set_modified();
	Update_window = 1;
	return obj;
}

int create_player(vec3d *pos, matrix *orient, int type)
{
	int obj;

	if (type == -1){
		type = get_default_player_ship_index();
	}
	Assert(type >= 0);

	obj = create_ship(orient, pos, type);
	Objects[obj].type = OBJ_START;

	Assert(Player_starts < MAX_PLAYERS);
	Player_starts++;
	if (Player_start_shipnum < 0) {
		Player_start_shipnum = Objects[obj].instance;
	}

	// be sure arrival/departure cues are set
	Ships[Objects[obj].instance].arrival_cue = Locked_sexp_true;
	Ships[Objects[obj].instance].departure_cue = Locked_sexp_false;
	obj_merge_created_list();
	set_modified();
	return obj;
}

int create_waypoint(vec3d *pos, int waypoint_instance)
{
	int obj = waypoint_add(pos, waypoint_instance);
	set_modified();
	return obj;
}

void create_new_mission()
{
	reset_mission();
	*Mission_filename = 0;
	FREDDoc_ptr->autosave("nothing");
	Undo_count = 0;
}

void reset_mission()
{
	clear_mission();

	create_player(&vmd_zero_vector, &vmd_identity_matrix);

	stars_post_level_init();
}

void clear_mission(bool fast_reload)
{
	char *str;
	int i, j, count;
	CTime t;

	// clean up everything we need to before we reset back to defaults.
	if (Briefing_dialog){
		Briefing_dialog->reset_editor();
	}

	allocate_parse_text( PARSE_TEXT_SIZE );

	mission_init(&The_mission);

	obj_init();

	if (!fast_reload)
		model_free_all();				// Free all existing models

	ai_init();
	asteroid_level_init();
	ship_level_init();
	nebula_init(Nebula_index, Nebula_pitch, Nebula_bank, Nebula_heading);

	Shield_sys_teams.clear();
	Shield_sys_teams.resize(Iff_info.size(), 0);

	for (i=0; i<MAX_SHIP_CLASSES; i++){
		Shield_sys_types[i] = 0;
	}

	set_cur_indices(-1);

	str = reg_read_string("SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "RegisteredOwner", NULL);
	if (!str) {
		str = reg_read_string("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "RegisteredOwner", NULL);
		if (!str) {
			str = getenv("USERNAME");
			if (!str){
				str = UNKNOWN_USER;
			}
		}
	}

	time_t currentTime;
	time(&currentTime);
	auto timeinfo = localtime(&currentTime);

	strcpy_s(The_mission.name, "Untitled");
	The_mission.author = str;
	time_to_mission_info_string(timeinfo, The_mission.created, DATE_TIME_LENGTH - 1);
	strcpy_s(The_mission.modified, The_mission.created);
	strcpy_s(The_mission.notes, "This is a FRED2_OPEN created mission.");
	strcpy_s(The_mission.mission_desc, "Put mission description here");

	apply_default_custom_data(&The_mission);

	// reset alternate name & callsign stuff
	for(i=0; i<MAX_SHIPS; i++){
		strcpy_s(Fred_alt_names[i], "");
		strcpy_s(Fred_callsigns[i], "");
	}

	// set up the default ship types for all teams.  For now, this is the same class
	// of ships for all teams
	for (i=0; i<MAX_TVT_TEAMS; i++) {
		count = 0;
		for ( j = 0; j < ship_info_size(); j++ ) {
			if (Ship_info[j].flags[Ship::Info_Flags::Default_player_ship]) {
				Team_data[i].ship_list[count] = j;
				strcpy_s(Team_data[i].ship_list_variables[count], "");
				Team_data[i].ship_count[count] = 5;
				strcpy_s(Team_data[i].ship_count_variables[count], "");
				count++;
			}
		}
		Team_data[i].num_ship_choices = count;

		count = 0;
		for ( j = 0; j < weapon_info_size(); j++ ) {
			if (Weapon_info[j].wi_flags[Weapon::Info_Flags::Default_player_weapon]) {
				if (Weapon_info[j].subtype == WP_LASER) {
					Team_data[i].weaponry_count[count] = 16;
				} else {
					Team_data[i].weaponry_count[count] = 500;
				}
				Team_data[i].weaponry_pool[count] = j; 
				strcpy_s(Team_data[i].weaponry_pool_variable[count], "");
				strcpy_s(Team_data[i].weaponry_amount_variable[count], "");
				count++;
			}
			Team_data[i].weapon_required[j] = false;
		}
		Team_data[i].num_weapon_choices = count; 
	}

	unmark_all();
	fred_render_init();
	set_physics_controls();

	Event_annotations.clear();
	Fred_migrated_immobile_ships.clear();

	// free memory from all parsing so far -- see also the stop_parse() in player_select_close() which frees all tbls found during game_init()
	stop_parse();
	// however, FRED expects to parse comments from the raw buffer, so we need a nominal string for that
	allocate_parse_text(1);

	set_modified(FALSE);
	Update_window = 1;
}

int query_valid_object(int index)
{
	int obj_found = FALSE;
	object *ptr;

	if (index < 0 || index >= MAX_OBJECTS || Objects[index].type == OBJ_NONE)
		return FALSE;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		Assert(ptr->type != OBJ_NONE);
		if (OBJ_INDEX(ptr) == index)
			obj_found = TRUE;
		
		ptr = GET_NEXT(ptr);
	}

	Assert(obj_found);  // just to make sure it's in the list like it should be.	
	return TRUE;
}

int query_valid_ship(int index)
{
	int obj_found = FALSE;
	object *ptr;

	if (index < 0 || index >= MAX_OBJECTS || Objects[index].type != OBJ_SHIP)
		return FALSE;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		Assert(ptr->type != OBJ_NONE);
		if (OBJ_INDEX(ptr) == index)
			obj_found = TRUE;
		
		ptr = GET_NEXT(ptr);
	}

	Assert(obj_found);  // just to make sure it's in the list like it should be.	
	return TRUE;
}

int query_valid_waypoint(int index)
{
	int obj_found = FALSE;
	object *ptr;

	if (index < 0 || index >= MAX_OBJECTS || Objects[index].type != OBJ_WAYPOINT)
		return FALSE;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		Assert(ptr->type != OBJ_NONE);
		if (OBJ_INDEX(ptr) == index)
			obj_found = TRUE;
		
		ptr = GET_NEXT(ptr);
	}

	Assert(obj_found);  // just to make sure it's in the list like it should be.	
	return TRUE;
}

// Sets the current object to whatever is specified or advances to the next object
// in the list if nothing is passed.
void set_cur_object_index(int obj)
{
	if (obj < 0)
		unmark_all();
	else
		mark_object(obj);

	set_cur_indices(obj);  // select the new object
	Update_ship = Update_wing = 1;
	Waypoint_editor_dialog.initialize_data(1);
	Jumpnode_editor_dialog.initialize_data(1);
	Update_window = 1;
}

// changes the currently selected wing.  It is assumed that cur_wing == cur_ship's wing
// number.  Don't call this if this won't be true, or else you'll screw things up.
void set_cur_wing(int wing)
{
	cur_wing = wing;
/*	if (cur_ship != -1)
		Assert(cur_wing == Ships[cur_ship].wingnum);
	if ((cur_object_index != -1) && (Objects[cur_object_index].type == OBJ_SHIP))
		Assert(cur_wing == Ships[Objects[cur_object_index].instance].wingnum);*/
	Update_wing = 1;
	Update_window = 1;
}

// sets up the various cur_* global variables related to the selecting of an object.  This
// is an internal function that shouldn't typically get called directly.  Use set_cur_object_index() instead.
void set_cur_indices(int obj)
{
	int i;
	object *ptr;
	CSingleLock sync(&CS_cur_object_index);

	sync.Lock();  // Don't modify until it's unlocked (if it's locked elsewhere).
	if (query_valid_object(obj)) {
		cur_object_index = obj;
		cur_ship = cur_wing = -1;
		cur_waypoint_list = NULL;
		cur_waypoint = NULL;

		if ((Objects[obj].type == OBJ_SHIP) || (Objects[obj].type == OBJ_START)) {
			cur_ship = Objects[obj].instance;
			cur_wing = Ships[cur_ship].wingnum;
			if (cur_wing >= 0)
				for (i=0; i<Wings[cur_wing].wave_count; i++)
					if (wing_objects[cur_wing][i] == cur_object_index) {
						cur_wing_index = i;
						break;
					}

		} else if (Objects[obj].type == OBJ_WAYPOINT) {
			cur_waypoint = find_waypoint_with_instance(Objects[obj].instance);
			Assert(cur_waypoint != NULL);
			cur_waypoint_list = cur_waypoint->get_parent_list();
		}

		return;
	}

	if (obj == -1 || !Num_objects) {
		cur_object_index = cur_ship = cur_wing = -1;
		cur_waypoint_list = NULL;
		cur_waypoint = NULL;
		return;
	}

	if (query_valid_object(cur_object_index))
		ptr = Objects[cur_object_index].next;
	else
		ptr = GET_FIRST(&obj_used_list);

	if (ptr == END_OF_LIST(&obj_used_list))
		ptr = ptr->next;

	Assert(ptr != END_OF_LIST(&obj_used_list));
	cur_object_index = OBJ_INDEX(ptr);
	Assert(ptr->type != OBJ_NONE);
	cur_ship = cur_wing = -1;
	cur_waypoint_list = NULL;
	cur_waypoint = NULL;

	if (ptr->type == OBJ_SHIP) {
		cur_ship = ptr->instance;
		cur_wing = Ships[cur_ship].wingnum;
		for (i=0; i<Wings[cur_wing].wave_count; i++)
			if (wing_objects[cur_wing][i] == cur_object_index) {
				cur_wing_index = i;
				break;
			}

	} else if (ptr->type == OBJ_WAYPOINT) {
		cur_waypoint = find_waypoint_with_instance(ptr->instance);
		Assert(cur_waypoint != NULL);
		cur_waypoint_list = cur_waypoint->get_parent_list();
	}
}

int update_dialog_boxes()
{
	int z;

	nprintf(("Fred routing", "updating dialog boxes\n"));

	// check wing first, since ships are dependent on wings, but not the reverse
	z = Wing_editor_dialog.update_data(0);
	if (z) {
		nprintf(("Fred routing", "wing dialog save failed\n"));
		Wing_editor_dialog.SetWindowPos(&Fred_main_wnd->wndTop, 0, 0, 0, 0,
			SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

		return z;
	}

	z = Ship_editor_dialog.update_data(0);
	if (z) {
		nprintf(("Fred routing", "ship dialog save failed\n"));
		Ship_editor_dialog.SetWindowPos(&Fred_main_wnd->wndTop, 0, 0, 0, 0,
			SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

		return z;
	}

	z = Waypoint_editor_dialog.update_data(0);
	if (z) {
		nprintf(("Fred routing", "waypoint dialog save failed\n"));
		Waypoint_editor_dialog.SetWindowPos(&Fred_main_wnd->wndTop, 0, 0, 0, 0,
			SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

		return z;
	}

	z = Jumpnode_editor_dialog.update_data();
	if (z) {
		nprintf(("Fred routing", "jumpnode dialog save failed\n"));
		Jumpnode_editor_dialog
			.SetWindowPos(&Fred_main_wnd->wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

		return z;
	}

	update_map_window();
	return 0;
}

int delete_object(int obj)
{
	int r;

	Ship_editor_dialog.bypass_all++;
	r = common_object_delete(obj);
	Ship_editor_dialog.bypass_all--;
	return r;
}

int delete_object(object *ptr)
{
	int r;

	Ship_editor_dialog.bypass_all++;
	r = common_object_delete(OBJ_INDEX(ptr));
	Ship_editor_dialog.bypass_all--;
	return r;
}

int delete_ship(int ship)
{
	int r;

	Ship_editor_dialog.bypass_all++;
	r = common_object_delete(Ships[ship].objnum);
	Ship_editor_dialog.bypass_all--;
	return r;
}

int common_object_delete(int obj)
{
	char msg[255];
	const char *name;
	int i, z, r, type;
	object *objp;
	SCP_list<CJumpNode>::iterator jnp;

	type = Objects[obj].type;
	if (type == OBJ_START) {
		i = Objects[obj].instance;
		if (Player_starts < 2) {  // player 1 start
			Fred_main_wnd->MessageBox("Must have at least 1 player starting point!",
				NULL, MB_OK | MB_ICONEXCLAMATION);

			unmark_object(obj);
			return 1;
		}

		Assert((i >= 0) && (i < MAX_SHIPS));
		sprintf(msg, "Player %d", i + 1);
		name = msg;
		r = reference_handler(name, sexp_ref_type::PLAYER, obj);
		if (r)
			return r;

		if (Ships[i].wingnum >= 0) {
			r = delete_ship_from_wing(i);
			if (r)
				return r;
		}

		Objects[obj].type = OBJ_SHIP;  // was allocated as a ship originally, so remove as such.
		invalidate_references(name, sexp_ref_type::PLAYER);

		// check if any ship is docked with this ship and break dock if so
		while (object_is_docked(&Objects[obj]))
		{
			ai_do_objects_undocked_stuff(&Objects[obj], dock_get_first_docked_object(&Objects[obj]));
		}

		if (Player_start_shipnum == i) {  // need a new single player start.
			objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				if (objp->type == OBJ_START) {
					Player_start_shipnum = objp->instance;
					break;
				}

				objp = GET_NEXT(objp);
			}
		}

		Player_starts--;

	} else if (type == OBJ_WAYPOINT) {
		waypoint *wpt = find_waypoint_with_instance(Objects[obj].instance);
		Assert(wpt != NULL);
		waypoint_list *wp_list = wpt->get_parent_list();
		int index = calc_waypoint_list_index(Objects[obj].instance);
		int count = (int) wp_list->get_waypoints().size();

		// we'll end up deleting the path, so check for path references
		if (count == 1) {
			name = wp_list->get_name();
			r = reference_handler(name, sexp_ref_type::WAYPOINT_PATH, obj);
			if (r)
				return r;
		}

		// check for waypoint references
		sprintf(msg, "%s:%d", wp_list->get_name(), index + 1);
		name = msg;
		r = reference_handler(name, sexp_ref_type::WAYPOINT, obj);
		if (r)
			return r;

		// at this point we've confirmed we want to delete it

		invalidate_references(name, sexp_ref_type::WAYPOINT);
		if (count == 1) {
			invalidate_references(wp_list->get_name(), sexp_ref_type::WAYPOINT_PATH);
		}

		// the actual removal code has been moved to this function in waypoints.cpp
		waypoint_remove(wpt);

	} else if (type == OBJ_SHIP) {
		name = Ships[Objects[obj].instance].ship_name;
		r = reference_handler(name, sexp_ref_type::SHIP, obj);
		if (r)
			return r;

		z = Objects[obj].instance;
		if (Ships[z].wingnum >= 1) {
			invalidate_references(name, sexp_ref_type::SHIP);
			r = delete_ship_from_wing(z);
			if (r)
				return r;

		} else if (Ships[z].wingnum >= 0) {
			r = delete_ship_from_wing(z);
			if (r)
				return r;

			invalidate_references(name, sexp_ref_type::SHIP);
		}

		for (i=0; i<Num_reinforcements; i++)
			if (!stricmp(name, Reinforcements[i].name)) {
				delete_reinforcement(i);
				break;
			}

		// check if any ship is docked with this ship and break dock if so
		while (object_is_docked(&Objects[obj]))
		{
			ai_do_objects_undocked_stuff(&Objects[obj], dock_get_first_docked_object(&Objects[obj]));
		}

	} else if (type == OBJ_POINT) {
		Assert(Briefing_dialog);
		Briefing_dialog->delete_icon(Objects[obj].instance);
		Update_window = 1;
		return 0;

	} else if (type == OBJ_JUMP_NODE) {
		for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
			if(jnp->GetSCPObject() == &Objects[obj])
				break;
		}

		// come on, WMC, we don't want to call obj_delete twice...
		// fool the destructor into not calling obj_delete yet
		Objects[obj].type = OBJ_NONE;
		
		// now call the destructor
		if (jnp != Jump_nodes.end())
			Jump_nodes.erase(jnp);

		// now restore the jump node type so that the below unmark and obj_delete will work
		Objects[obj].type = OBJ_JUMP_NODE;
	}

	unmark_object(obj);

	//we need to call obj_delete() even if obj is a jump node
	//the statement "delete Objects[obj].jnp" deletes the jnp object
	//obj_delete() frees up the object slot where the node used to reside.
	//if we don't call this then the node will still show up in fred and you can try to delete it twice
	//this causes an ugly crash.
	obj_delete(obj);
	set_modified();
	Update_window = 1;
	return 0;
}

void delete_marked()
{
	object *ptr, *next;

	delete_flag = 0;
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		next = GET_NEXT(ptr);
		if (ptr->flags[Object::Object_Flags::Marked])
			if (delete_object(ptr) == 2)  // user went to a reference, so don't get in the way.
				break;
		
		ptr = next;
	}

	if (!delete_flag)
		set_cur_object_index(-1);

	Update_window = 1;
}

void delete_reinforcement(int num)
{
	int i;

	for (i=num; i<Num_reinforcements-1; i++)
		Reinforcements[i] = Reinforcements[i + 1];

	Num_reinforcements--;
	set_modified();
}

// delete ship, removing it from its wing if necessary.
int delete_ship_from_wing(int ship)
{
	char name[NAME_LENGTH];
	int i, r, wing, end;

	wing = Ships[ship].wingnum;
	if (wing >= 0) {
		if (Wings[wing].wave_count == 1) {
			cur_wing = -1;
			Update_wing = 1;
			r = delete_wing(wing, 1);
			if (r) {
				if (r == 2){
					delete_flag = 1;
				}

				return r;
			}
		
		} else {
			i = Wings[wing].wave_count;
			end = i - 1;
			while (i--){
				if (wing_objects[wing][i] == Ships[ship].objnum){
					break;
				}
			}

			Assert(i != -1);  // Error, object should be in wing.
			if (Wings[wing].special_ship == i){
				Wings[wing].special_ship = 0;
			} else if (Wings[wing].special_ship > i) {
				Wings[wing].special_ship--;
			}

			if (i != end) {
				wing_objects[wing][i] = wing_objects[wing][end];
				Wings[wing].ship_index[i] = Wings[wing].ship_index[end];
				if (Objects[wing_objects[wing][i]].type == OBJ_SHIP) {
					wing_bash_ship_name(name, Wings[wing].name, i + 1);
					rename_ship(Wings[wing].ship_index[i], name);
				}
			}

			if (Wings[wing].threshold >= Wings[wing].wave_count){
				Wings[wing].threshold = Wings[wing].wave_count - 1;
			}

			Wings[wing].wave_count--;
			if (Wings[wing].wave_count && (Wings[wing].threshold >= Wings[wing].wave_count)){
				Wings[wing].threshold = Wings[wing].wave_count - 1;
			}
		}
	}

	set_modified();
	return 0;
}

//	Return true if current object is valid and is in a wing.
//	Else return false.
int query_object_in_wing(int obj)
{
	if (query_valid_object(obj)){
		if (Ships[Objects[obj].instance].wingnum != -1){
			return TRUE;
		}
	}
	
	return FALSE;
}

void mark_object(int obj)
{
	Assert(query_valid_object(obj));
	if (!(Objects[obj].flags[Object::Object_Flags::Marked])) {
        Objects[obj].flags.set(Object::Object_Flags::Marked);  // set as marked
		Marked++;
		Update_window = 1;
		if (cur_object_index == -1){
			set_cur_object_index(obj);
		}
		Update_ship = Update_wing = 1;
		Waypoint_editor_dialog.initialize_data(1);
		Jumpnode_editor_dialog.initialize_data(1);
	}
}

void unmark_object(int obj)
{
	Assert(query_valid_object(obj));
	if (Objects[obj].flags[Object::Object_Flags::Marked]) {
        Objects[obj].flags.remove(Object::Object_Flags::Marked);
		Marked--;
		Update_window = 1;
		if (obj == cur_object_index) {  // need to find a new index
			object *ptr;

			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->flags[Object::Object_Flags::Marked]) {
					set_cur_object_index(OBJ_INDEX(ptr));  // found one
					return;
				}

				ptr = GET_NEXT(ptr);
			}

			set_cur_object_index(-1);  // can't find one; nothing is marked.
		}
		Update_ship = Update_wing = 1;
		Waypoint_editor_dialog.initialize_data(1);
		Jumpnode_editor_dialog.initialize_data(1);
	}
}

// clears the marked flag of all objects (so nothing is marked)
void unmark_all()
{
	int i;

	if (Marked) {
		for (i=0; i<MAX_OBJECTS; i++){
            Objects[i].flags.remove(Object::Object_Flags::Marked);
		}

		Marked = 0;
		Update_window = 1;
		set_cur_object_index(-1);
	}
}

void clear_menu(CMenu *ptr)
{
	int count;

	count = ptr->GetMenuItemCount();
	while (count--){
		ptr->DeleteMenu(count, MF_BYPOSITION);
	}
}

void generate_wing_popup_menu(CMenu *mptr, int first_id, int state)
{
	int i, z, columns, rows, count;

	columns = 1;
	rows = Num_wings;
	while (rows > 25) {
		columns++;
		rows = Num_wings / columns;
	}

	count = rows + 1;
	for (i=0; i<MAX_WINGS; i++){
		if (Wings[i].wave_count) {
			z = state | MF_STRING;
			if (!count--) {
				count = rows;
				z |= MF_MENUBARBREAK;
			}

			mptr->AppendMenu(z, first_id + i, Wings[i].name);
		}
	}

	mptr->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);
}

void generate_ship_popup_menu(CMenu *mptr, int first_id, int state, int filter)
{
	int z, ship, columns, rows, count, num_ships;
	object *ptr;

	columns = 1;
	num_ships = ship_get_num_ships();
	rows = num_ships;
	while (rows > 25) {
		columns++;
		rows = num_ships / columns;
	}

	count = rows + 1;
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) || ((ptr->type == OBJ_START) && (filter & SHIP_FILTER_PLAYERS))) {
			z = 1;
			if (filter & SHIP_FILTER_FLYABLE) {
				if (!Ship_info[Ships[get_ship_from_obj(ptr)].ship_info_index].is_flyable()){
					z = 0;
				}
			}

			if (z) {
				z = state | MF_STRING;
				if (!count--) {
					count = rows;
					z |= MF_MENUBARBREAK;
				}

				ship = ptr->instance;
				mptr->AppendMenu(z, first_id + ship, Ships[ship].ship_name);
			}
		}

		ptr = GET_NEXT(ptr);
	}

	mptr->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);
}

// Alternate string lookup function, taking a CString instead.  The reason that it's here,
// instead of parselo.cpp, is because the class CString require an include of windows.h,
// which everyone wants to avoid including in any freespace header files.  So..
int string_lookup(const CString &str1, char *strlist[], int max)
{
	int	i;

	for (i=0; i<max; i++) {
		Assert(strlen(strlist[i]));

		if (!stricmp((LPCTSTR) str1, strlist[i])){
			return i;
		}
	}

	return -1;
}

int gray_menu_tree(CMenu *base)
{
	int i, z, count = 0;
	CMenu *submenu;

	i = base->GetMenuItemCount();
	while (i--) {
		if ((submenu = base->GetSubMenu(i))>0) {
			if (gray_menu_tree(submenu)) {
				count++;
			} else {
				base->EnableMenuItem(i, MF_GRAYED | MF_BYPOSITION);
			}

		} else {
			z = base->GetMenuState(i, MF_BYPOSITION);
			if (z == MF_ENABLED){
				count++;
			}
		}
	}

	return count;
}

int query_initial_orders_conflict(int wing)
{
	int i, z;

	Assert(wing != -1);
	if (wing == -1){
		return 0;
	}

	if (query_initial_orders_empty(Wings[wing].ai_goals)){
		return 0;
	}

	i = Wings[wing].wave_count;  // wing has orders, now check ships.
	while (i--) {
		z = Ships[Objects[wing_objects[wing][i]].instance].ai_index;
		if (!query_initial_orders_empty(Ai_info[z].goals)){  // ship also has orders
			return 1;
		}
	}

	return 0;
}

int query_initial_orders_empty(ai_goal *ai_goals)
{
	int i;

	for (i=0; i<MAX_AI_GOALS; i++){
		if (ai_goals[i].ai_mode != AI_GOAL_NONE){
			return 0;
		}
	}

	return 1;
}

int set_reinforcement(char *name, int state)
{
	int i, index, cur = -1;

	for (i=0; i<Num_reinforcements; i++){
		if (!stricmp(Reinforcements[i].name, name)){
			cur = i;
		}
	}

	if (!state && (cur != -1)) {
		Num_reinforcements--;
		Reinforcements[cur] = Reinforcements[Num_reinforcements];

		// clear the ship/wing flag for this reinforcement
		index = ship_name_lookup(name);
		if ( index != -1 ){
            Ships[index].flags.remove(Ship::Ship_Flags::Reinforcement);
		} else {
			index = wing_name_lookup(name);
			if ( index != -1 ){
                Wings[index].flags.remove(Ship::Wing_Flags::Reinforcement);
			}
		}
		if (index == -1 ){
			Int3();				// get allender -- coudln't find ship/wing for clearing reinforcement flag
		}

		set_modified();
		return -1;
	}

	if (state && (cur == -1) && (Num_reinforcements < MAX_REINFORCEMENTS)) {
		Assert(strlen(name) < NAME_LENGTH);
		strcpy_s(Reinforcements[Num_reinforcements].name, name);
		Reinforcements[Num_reinforcements].uses = 1;
		Reinforcements[Num_reinforcements].arrival_delay = 0;
		memset( Reinforcements[Num_reinforcements].no_messages, 0, MAX_REINFORCEMENT_MESSAGES * NAME_LENGTH );
		memset( Reinforcements[Num_reinforcements].yes_messages, 0, MAX_REINFORCEMENT_MESSAGES * NAME_LENGTH );
		Num_reinforcements++;

		// set the reinforcement flag on the ship or wing
		index = ship_name_lookup(name);
		if ( index != -1 ){
			Ships[index].flags.set(Ship::Ship_Flags::Reinforcement);
		} else {
			index = wing_name_lookup(name);
			if ( index != -1 ){
                Wings[index].flags.set(Ship::Wing_Flags::Reinforcement);
			}
		}
		if ( index == -1 ){
			Int3();				// get allender -- coudln't find ship/wing for setting reinforcement flag
		}

		set_modified();
		return 1;
	}

	// this code will take care of setting the bits for the ship/wing flags
	if ( state && (cur != -1) ) {
		// set the reinforcement flag on the ship or wing
		index = ship_name_lookup(name);
		if ( index != -1 ){
			Ships[index].flags.set(Ship::Ship_Flags::Reinforcement);
		} else {
			index = wing_name_lookup(name);
			if ( index != -1 ){
                Wings[index].flags.set(Ship::Wing_Flags::Reinforcement);
			}
		}
		if ( index == -1 ){
			Int3();				// get allender -- coudln't find ship/wing for setting reinforcement flag
		}
	}

	return 0;
}

int get_docking_list(int model_index)
{
	int i;
	polymodel *pm;

	pm = model_get(model_index);
	Assert(pm->n_docks <= MAX_DOCKS);
	for (i=0; i<pm->n_docks; i++)
		Docking_bay_list[i] = pm->docking_bays[i].name;

	return pm->n_docks;
}

// DA 1/7/99 These ship names are not variables
int rename_ship(int ship, const char *name)
{
	int i;

	Assert(ship >= 0);
	Assert(strlen(name) < NAME_LENGTH);

	// we may not need to rename it
	if (strcmp(Ships[ship].ship_name, name) == 0)
		return 0;

	update_sexp_references(Ships[ship].ship_name, name);
	ai_update_goal_references(sexp_ref_type::SHIP, Ships[ship].ship_name, name);
	update_texture_replacements(Ships[ship].ship_name, name);
	for (i=0; i<Num_reinforcements; i++)
		if (!stricmp(Ships[ship].ship_name, Reinforcements[i].name)) {
			strcpy_s(Reinforcements[i].name, name);
		}

	strcpy_s(Ships[ship].ship_name, name);
	if (ship == cur_ship)
		Ship_editor_dialog.m_ship_name = _T(name);

	return 0;
}

int invalidate_references(const char *name, sexp_ref_type type)
{
	char new_name[512];
	int i;

	sprintf(new_name, "<%s>", name);
	update_sexp_references(name, new_name);
	ai_update_goal_references(type, name, new_name);
	update_texture_replacements(name, new_name);
	for (i=0; i<Num_reinforcements; i++)
		if (!stricmp(name, Reinforcements[i].name)) {
			strcpy_s(Reinforcements[i].name, new_name);
		}

	return 0;
}

int internal_integrity_check()
{
	int i;

	for (i=0; i<(int)Mission_events.size(); i++)
		verify_sexp_tree(Mission_events[i].formula);

	for (i=0; i<(int)Mission_goals.size(); i++)
		verify_sexp_tree(Mission_goals[i].formula);

	for (i=0; i<MAX_WINGS; i++)
		if (Wings[i].wave_count) {
			verify_sexp_tree(Wings[i].arrival_cue);
			verify_sexp_tree(Wings[i].departure_cue);
		}

	for (i=0; i<MAX_SHIPS; i++)
		if (Ships[i].objnum >= 0) {
			verify_sexp_tree(Ships[i].arrival_cue);
			verify_sexp_tree(Ships[i].departure_cue);
			if (Ships[i].ai_index < 0)
				Assert(0);
			if (Ai_info[Ships[i].ai_index].shipnum != i)
				Int3();
		}

	return 0;
}

void correct_marking()
{
	object *ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags[Object::Object_Flags::Marked]) {
			if (ptr->flags[Object::Object_Flags::Hidden])
				unmark_object(OBJ_INDEX(ptr));

			else switch (ptr->type) {
				case OBJ_WAYPOINT:
					if (!Show_waypoints)
						unmark_object(OBJ_INDEX(ptr));
					break;

				case OBJ_START:
					if (!Show_starts || !Show_ships)
						unmark_object(OBJ_INDEX(ptr));
					break;

				case OBJ_SHIP:
					if (!Show_ships)
						unmark_object(OBJ_INDEX(ptr));

					if (!Show_iff[Ships[ptr->instance].team])
						unmark_object(OBJ_INDEX(ptr));

					break;
			}
		}

		ptr = GET_NEXT(ptr);
	}
}

// Fills a combo box with a list of all docking points of type 'type' on ship 'ship'.
// Item data is the actual docking point index.
void set_valid_dock_points(int ship, int type, CComboBox *box)
{
	int i, z, num, model;

	model = Ship_info[Ships[ship].ship_info_index].model_num;
	num = model_get_num_dock_points(model);
	for (i=0; i<num; i++)
		if (model_get_dock_index_type(model, i) & type) {
			z = box->AddString(model_get_dock_name(model, i));
			box->SetItemData(z, i);
		}

	Assert(box->GetCount());
}

// Given an object index, find the ship index for that object.
int get_ship_from_obj(int obj)
{
	if ((Objects[obj].type == OBJ_SHIP) || (Objects[obj].type == OBJ_START))
		return Objects[obj].instance;

	Int3();
	return 0;
}

int get_ship_from_obj(object *objp)
{
	if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START))
		return objp->instance;

	Int3();
	return 0;
}

void ai_update_goal_references(sexp_ref_type type, const char *old_name, const char *new_name)
{
	int i;

	for (i=0; i<MAX_AI_INFO; i++)  // loop through all Ai_info entries
		if (Ai_info[i].shipnum != -1)  // skip if unused
			ai_update_goal_references(Ai_info[i].goals, type, old_name, new_name);

	for (i=0; i<MAX_WINGS; i++)
		if (Wings[i].wave_count)
			ai_update_goal_references(Wings[i].ai_goals, type, old_name, new_name);
}

std::pair<int, sexp_src> query_referenced_in_ai_goals(sexp_ref_type type, const char *name)
{
	int i;

	for (i=0; i<MAX_AI_INFO; i++)  // loop through all Ai_info entries
		if (Ai_info[i].shipnum >= 0)  // skip if unused
			if (query_referenced_in_ai_goals(Ai_info[i].goals, type, name))
				return std::make_pair(Ai_info[i].shipnum, sexp_src::SHIP_ORDER);

	for (i=0; i<MAX_WINGS; i++)
		if (Wings[i].wave_count)
			if (query_referenced_in_ai_goals(Wings[i].ai_goals, type, name))
				return std::make_pair(i, sexp_src::WING_ORDER);

	return std::make_pair(-1, sexp_src::NONE);
}

int advanced_stricmp(char *one, char *two)
{
	if (!one && !two)
		return 0;

	if (!one)
		return -1;

	if (!two)
		return 1;

	return stricmp(one, two);
}

// returns 0: go ahead change object
//			  1: don't change it
//			  2: abort (they used cancel to go to reference)
int reference_handler(const char *name, sexp_ref_type type, int obj)
{
	char msg[2048], text[128], type_name[128];
	int r, node;

	switch (type) {
		case sexp_ref_type::SHIP:
			sprintf(type_name, "Ship \"%s\"", name);
			break;

		case sexp_ref_type::WING:
			sprintf(type_name, "Wing \"%s\"", name);
			break;

		case sexp_ref_type::PLAYER:
			strcpy_s(type_name, name);
			break;

		case sexp_ref_type::WAYPOINT:
			sprintf(type_name, "Waypoint \"%s\"", name);
			break;

		case sexp_ref_type::WAYPOINT_PATH:
			sprintf(type_name, "Waypoint path \"%s\"", name);
			break;

		default:
			Error(LOCATION, "Type unknown for object \"%s\".  Let Hoffos know now!", name);
	}

	auto pair = query_referenced_in_sexp(type, name, node);
	int n = pair.first;
	sexp_src source = pair.second;

	if (source != sexp_src::NONE) {
		switch (source) {
			case sexp_src::SHIP_ARRIVAL:
				sprintf(text, "the arrival cue of ship \"%s\"", Ships[n].ship_name);
				break;

			case sexp_src::SHIP_DEPARTURE:
				sprintf(text, "the departure cue of ship \"%s\"", Ships[n].ship_name);
				break;

			case sexp_src::WING_ARRIVAL:
				sprintf(text, "the arrival cue of wing \"%s\"", Wings[n].name);
				break;

			case sexp_src::WING_DEPARTURE:
				sprintf(text, "the departure cue of wing \"%s\"", Wings[n].name);
				break;

			case sexp_src::EVENT:
				if (!Mission_events[n].name.empty())
					sprintf(text, "event \"%s\"", Mission_events[n].name.c_str());
				else
					sprintf(text, "event #%d", n);

				break;

			case sexp_src::MISSION_GOAL:
				if (!Mission_goals[n].name.empty())
					sprintf(text, "mission goal \"%s\"", Mission_goals[n].name.c_str());
				else
					sprintf(text, "mission goal #%d", n);

				break;

			case sexp_src::MISSION_CUTSCENE:
				if (The_mission.cutscenes[n].filename[0] != '\0')
					sprintf(text, "mission cutscene \"%s\"", The_mission.cutscenes[n].filename);
				else
					sprintf(text, "mission cutscene #%d", n);

				break;

			case sexp_src::DEBRIEFING:
				sprintf(text, "debriefing #%d", n);
				break;

			case sexp_src::BRIEFING:
				sprintf(text, "briefing #%d", n);
				break;

			default:  // very bad.  Someone added an sexp somewhere and didn't change this.
				Warning(LOCATION, "\"%s\" referenced by an unknown sexp source!  "
					"Run for the hills and let Hoffoss know right now!", name);

				delete_flag = 1;
				return 2;
		}

		sprintf(msg, "%s is referenced by %s (possibly more sexps).\n"
			"Do you want to delete it anyway?\n\n"
			"(click Cancel to go to the reference)", type_name, text);

		r = sexp_reference_handler(node, source, n, msg);
		if (r == 1) {
			if (obj >= 0)
				unmark_object(obj);

			return 1;
		}

		if (r == 2) {
			delete_flag = 1;
			return 2;
		}
	}

	pair = query_referenced_in_ai_goals(type, name);
	n = pair.first;
	source = pair.second;

	if (source != sexp_src::NONE) {
		switch (source) {
			case sexp_src::SHIP_ORDER:
				sprintf(text, "ship \"%s\"", Ships[n].ship_name);
				break;

			case sexp_src::WING_ORDER:
				sprintf(text, "wing \"%s\"", Wings[n].name);
				break;

			default:  // very bad.  Someone added an sexp somewhere and didn't change this.
				Error(LOCATION, "\"%s\" referenced by an unknown initial orders source!  "
					"Run for the hills and let Hoffoss know right now!", name);
		}

		sprintf(msg, "%s is referenced by the initial orders of %s (possibly \n"
			"more initial orders).  Do you want to delete it anyway?\n\n"
			"(click Cancel to go to the reference)", type_name, text);

		r = orders_reference_handler(source, n, msg);
		if (r == 1) {
			if (obj >= 0)
				unmark_object(obj);

			return 1;
		}

		if (r == 2) {
			delete_flag = 1;
			return 2;
		}
	}

	if ((type != sexp_ref_type::SHIP) && (type != sexp_ref_type::WING))
		return 0;

	for (n=0; n<Num_reinforcements; n++)
		if (!stricmp(name, Reinforcements[n].name))
			break;

	if (n < Num_reinforcements) {
		sprintf(msg, "Ship \"%s\" is a reinforcement unit.\n"
			"Do you want to delete it anyway?", name);

		r = Fred_main_wnd->MessageBox(msg, NULL, MB_YESNO | MB_ICONEXCLAMATION);
		if (r == IDNO) {
			if (obj >= 0)
				unmark_object(obj);

			return 1;
		}
	}

	return 0;
}

int orders_reference_handler(sexp_src source, int source_index, char *msg)
{
	int r = Fred_main_wnd->MessageBox(msg, "Warning", MB_YESNOCANCEL | MB_ICONEXCLAMATION);
	if (r == IDNO)
		return 1;

	if (r == IDYES)
		return 0;

	ShipGoalsDlg dlg_goals;

	switch (source) {
		case sexp_src::SHIP_ORDER:
			unmark_all();
			mark_object(Ships[source_index].objnum);

			dlg_goals.self_ship = source_index;
			dlg_goals.DoModal();
			if (!query_initial_orders_empty(Ai_info[Ships[source_index].ai_index].goals))
				if ((Ships[source_index].wingnum >= 0) && (query_initial_orders_conflict(Ships[source_index].wingnum)))
					Fred_main_wnd->MessageBox("This ship's wing also has initial orders", "Possible conflict");

			break;

		case sexp_src::WING_ORDER:
			unmark_all();
			mark_wing(source_index);

			dlg_goals.self_wing = source_index;
			dlg_goals.DoModal();
			if (query_initial_orders_conflict(source_index))
				Fred_main_wnd->MessageBox("One or more ships of this wing also has initial orders", "Possible conflict");

			break;

		default:  // very bad.  Someone added an sexp somewhere and didn't change this.
			Error(LOCATION, "Unknown initial order reference source");
	}

	delete_flag = 1;
	return 2;
}

int sexp_reference_handler(int node, sexp_src source, int source_index, char *msg)
{
	int r, n;

	r = Fred_main_wnd->MessageBox(msg, "Warning", MB_YESNOCANCEL | MB_ICONEXCLAMATION);
	if (r == IDNO)
		return 1;

	if (r == IDYES)
		return 0;

	n = source_index;
	switch (source) {
		case sexp_src::SHIP_ARRIVAL:
		case sexp_src::SHIP_DEPARTURE:
			if (!Ship_editor_dialog.GetSafeHwnd())
				Ship_editor_dialog.Create();

			Ship_editor_dialog.SetWindowPos(&Fred_main_wnd->wndTop, 0, 0, 0, 0,
				SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
			Ship_editor_dialog.ShowWindow(SW_RESTORE);

			Ship_editor_dialog.select_sexp_node = node;
			unmark_all();
			mark_object(Ships[n].objnum);
			break;

		case sexp_src::WING_ARRIVAL:
		case sexp_src::WING_DEPARTURE:
			if (!Wing_editor_dialog.GetSafeHwnd())
				Wing_editor_dialog.Create();

			Wing_editor_dialog.SetWindowPos(&Fred_main_wnd->wndTop, 0, 0, 0, 0,
				SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
			Wing_editor_dialog.ShowWindow(SW_RESTORE);

			Wing_editor_dialog.select_sexp_node = node;
			unmark_all();
			mark_wing(n);
			break;

		case sexp_src::EVENT:
			if (Message_editor_dlg) {
				Fred_main_wnd->MessageBox("You must close the message editor before the event editor can be opened");
				break;
			}

			if (!Event_editor_dlg) {
				Event_editor_dlg = new event_editor;
				Event_editor_dlg->select_sexp_node = node;
				Event_editor_dlg->Create(event_editor::IDD);
			}

			Event_editor_dlg->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
			Event_editor_dlg->ShowWindow(SW_RESTORE);
			break;

		case sexp_src::MISSION_GOAL: {
			CMissionGoalsDlg dlg;

			dlg.select_sexp_node = node;
			dlg.DoModal();
			break;
		}

		case sexp_src::MISSION_CUTSCENE: {
			CMissionCutscenesDlg dlg;

			dlg.select_sexp_node = node;
			dlg.DoModal();
			break;
		}

		case sexp_src::DEBRIEFING: {
			debriefing_editor_dlg dlg;

			dlg.select_sexp_node = node;
			dlg.DoModal();
			break;
		}

		case sexp_src::BRIEFING: {
			if (!Briefing_dialog) {
				Briefing_dialog = new briefing_editor_dlg;
				Briefing_dialog->create();
			}

			Briefing_dialog->SetWindowPos(&Briefing_dialog->wndTop, 0, 0, 0, 0,
				SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
			Briefing_dialog->ShowWindow(SW_RESTORE);
			Briefing_dialog->focus_sexp(node);
			break;
		}

		default:  // very bad.  Someone added an sexp somewhere and didn't change this.
			Error(LOCATION, "Unknown sexp reference source");
	}

	delete_flag = 1;
	return 2;
}

char *object_name(int obj)
{
	static char text[80];
	waypoint_list *wp_list;
	int waypoint_num;

	if (!query_valid_object(obj))
		return "*none*";

	switch (Objects[obj].type) {
		case OBJ_SHIP:
		case OBJ_START:
			return Ships[Objects[obj].instance].ship_name;

		case OBJ_WAYPOINT:
			wp_list = find_waypoint_list_with_instance(Objects[obj].instance, &waypoint_num);
			Assert(wp_list != NULL);
			sprintf(text, "%s:%d", wp_list->get_name(), waypoint_num + 1);
			return text;

		case OBJ_POINT:
			return "Briefing icon";
	}

	return "*unknown*";
}

const char *get_order_name(ai_goal_mode order)
{
	int i;

	if (order == AI_GOAL_NONE)  // special case
		return "None";

	for (i=0; i<Ai_goal_list_size; i++)
		if (Ai_goal_list[i].def == order)
			return Ai_goal_list[i].name;

	return "???";
}

void object_moved(object *objp)
{
	if (objp->type == OBJ_WAYPOINT)
	{
		waypoint *wpt = find_waypoint_with_instance(objp->instance);
		Assert(wpt != NULL);
		wpt->set_pos(&objp->pos);
	}

	if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) // do we have a ship?
	{
		// reset the already-handled flag (inefficient, but it's FRED, so who cares)
        for (int i = 0; i < MAX_OBJECTS; i++)
            Objects[i].flags.remove(Object::Object_Flags::Docked_already_handled);

		// move all docked objects docked to me
		dock_move_docked_objects(objp);
	}
}

// determine if all the ships in a given wing are all marked or not.
int query_whole_wing_marked(int wing)
{
	int count = 0;
	object *ptr;

	if (!Wings[wing].wave_count)
		return 0;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags[Object::Object_Flags::Marked])
			if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START))
				if (Ships[get_ship_from_obj(ptr)].wingnum == wing)
					count++;

		ptr = GET_NEXT(ptr);
	}

	if (count == Wings[wing].wave_count)
		return 1;

	return 0;
}

void generate_ship_usage_list(int *arr, int wing) 
{
	int i; 

	if (wing < 0) {
		return;
	}
	
	i = Wings[wing].wave_count;
	while (i--) {
		arr[Ships[Wings[wing].ship_index[i]].ship_info_index]++; 
	}
}

void generate_weaponry_usage_list(int *arr, int wing)
{
	int i, j;
	ship_weapon *swp;

	if (wing < 0)
		return;

	i = Wings[wing].wave_count;
	while (i--) {
		swp = &Ships[Wings[wing].ship_index[i]].weapons;
		j = swp->num_primary_banks;
		while (j--) {
			if (swp->primary_bank_weapons[j] >= 0 && swp->primary_bank_weapons[j] < weapon_info_size()) {
				arr[swp->primary_bank_weapons[j]]++;
			}
		}

		j = swp->num_secondary_banks;
		while (j--) {
			if (swp->secondary_bank_weapons[j] >=0 && swp->secondary_bank_weapons[j] < weapon_info_size()) {
				arr[swp->secondary_bank_weapons[j]] += (int) floor((swp->secondary_bank_ammo[j] * swp->secondary_bank_capacity[j] / 100.0f / Weapon_info[swp->secondary_bank_weapons[j]].cargo_size) + 0.5f);
			}
		}
	}
}

void generate_weaponry_usage_list(int team, int *arr)
{
	int i;

	for (i=0; i<MAX_WEAPON_TYPES; i++)
		arr[i] = 0;
	 
    if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
		Assert (team >= 0 && team < MAX_TVT_TEAMS);

		for (i=0; i<MAX_TVT_WINGS_PER_TEAM; i++) {
			generate_weaponry_usage_list(arr, TVT_wings[(team * MAX_TVT_WINGS_PER_TEAM) + i]);
		}
	}
	else {
		for (i=0; i<MAX_STARTING_WINGS; i++) {
			generate_weaponry_usage_list(arr, Starting_wings[i]);
		}
	}
}

CJumpNode *jumpnode_get_by_name(const CString& name)
{
	CJumpNode *jnp = jumpnode_get_by_name((LPCTSTR) name);
	return jnp;
}

// function which adds all current ships in the Fred mission to the passed in combo box.  useful for
// building up ship lists for arrival/departure targets
void management_add_ships_to_combo( CComboBox *box, int flags )
{
	object *objp;
	int id, i, restrict_to_players;

	box->ResetContent();
	
	// add the "special" targets, i.e. any friendly, any hostile, etc.
	if (flags & SHIPS_2_COMBO_SPECIAL)
	{
		for (restrict_to_players = 0; restrict_to_players < 2; restrict_to_players++)
		{
			for (i = 0; i < (int)Iff_info.size(); i++)
			{
				char tmp[NAME_LENGTH + 15];
				stuff_special_arrival_anchor_name(tmp, i, restrict_to_players, 0);

				id = box->AddString(tmp);
				box->SetItemData(id, get_special_anchor(tmp));
			}
		}
	}

	// either add all ships to the list, or only add ships with docking bays.
	if ( flags & SHIPS_2_COMBO_ALL_SHIPS ) {
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if ( ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && !(objp->flags[Object::Object_Flags::Marked]) ) {
				id = box->AddString(Ships[get_ship_from_obj(objp)].ship_name);
				box->SetItemData(id, get_ship_from_obj(objp));
			}
		}
	} else if ( flags & SHIPS_2_COMBO_DOCKING_BAY_ONLY ) {
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if ( ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && !(objp->flags[Object::Object_Flags::Marked]) ) {
				polymodel *pm;

				// determine if this ship has a docking bay
				pm = model_get( Ship_info[Ships[objp->instance].ship_info_index].model_num );
				Assert( pm );
				if ( pm->ship_bay && (pm->ship_bay->num_paths > 0) ) {
					id = box->AddString(Ships[get_ship_from_obj(objp)].ship_name);
					box->SetItemData(id, get_ship_from_obj(objp));
				}
			}
		}
	}
}

char *reg_read_string( char *section, char *name, char *default_value )
{
	HKEY hKey = NULL;
	DWORD dwType, dwLen;
	char keyname[1024];
	static char tmp_string_data[1024];
	LONG lResult;

	strcpy_s( keyname, section );

	lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,							// Where it is
											 keyname,								// name of key
											 NULL,									// DWORD reserved
											 KEY_QUERY_VALUE,						// Allows all changes
											 &hKey );								// Location to store key

	if ( lResult != ERROR_SUCCESS )	{
		mprintf(( "Error opening registry key '%s'\n", keyname ));
		goto Cleanup;
	}

	if ( !name )	 {
		mprintf(( "No variable name passed\n" ));
		goto Cleanup;
	}

	dwLen = 1024;
	lResult = RegQueryValueEx( hKey,									// Handle to key
									 name,											// The values name
									 NULL,											// DWORD reserved
	                         &dwType,										// What kind it is
									 (ubyte *)&tmp_string_data,						// value to set
									 &dwLen );								// How many bytes to set
																				
	if ( lResult != ERROR_SUCCESS )	{
		mprintf(( "Error reading registry key '%s'\n", name ));
		goto Cleanup;
	}

	default_value = tmp_string_data;

Cleanup:
	if ( hKey )
		RegCloseKey(hKey);

	return default_value;
}

// Goober5000
int wing_is_player_wing(int wing)
{
	int i;

	if (wing < 0)
		return 0;

	if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS)
	{
		for (i=0; i<MAX_TVT_WINGS; i++)
		{
			if (wing == TVT_wings[i])
				return 1;
		}
	}
	else
	{
		for (i=0; i<MAX_STARTING_WINGS; i++)
		{
			if (wing == Starting_wings[i])
				return 1;
		}
	}

	return 0;
}

// Goober5000
// This must be done when either the wing name or the custom name is changed.
// (It's also duplicated in FS2, in post_process_mission, for setting the indexes at mission load.)
void update_custom_wing_indexes()
{
	int i;

	for (i = 0; i < MAX_STARTING_WINGS; i++)
	{
		Starting_wings[i] = wing_name_lookup(Starting_wing_names[i], 1);
	}

	for (i = 0; i < MAX_SQUADRON_WINGS; i++)
	{
		Squadron_wings[i] = wing_name_lookup(Squadron_wing_names[i], 1);
	}

	for (i = 0; i < MAX_TVT_WINGS; i++)
	{
		TVT_wings[i] = wing_name_lookup(TVT_wing_names[i], 1);
	}
}

// Goober5000
void stuff_special_arrival_anchor_name(char *buf, int iff_index, int restrict_to_players, int retail_format)
{
	char *iff_name = Iff_info[iff_index].iff_name;

	// stupid retail hack
	if (retail_format && !stricmp(iff_name, "hostile") && !restrict_to_players)
		iff_name = "enemy";

	if (restrict_to_players)
		sprintf(buf, "<any %s player>", iff_name);
	else
		sprintf(buf, "<any %s>", iff_name);

	strlwr(buf);
}

// Goober5000
void stuff_special_arrival_anchor_name(char *buf, int anchor_num, int retail_format)
{
	// filter out iff
	int iff_index = anchor_num;
	iff_index &= ~SPECIAL_ARRIVAL_ANCHOR_FLAG;
	iff_index &= ~SPECIAL_ARRIVAL_ANCHOR_PLAYER_FLAG;

	// filter players
	int restrict_to_players = (anchor_num & SPECIAL_ARRIVAL_ANCHOR_PLAYER_FLAG);

	// get name
	stuff_special_arrival_anchor_name(buf, iff_index, restrict_to_players, retail_format);
}

// Goober5000
void update_texture_replacements(const char *old_name, const char *new_name)
{
	for (SCP_vector<texture_replace>::iterator ii = Fred_texture_replacements.begin(); ii != Fred_texture_replacements.end(); ++ii)
	{
		if (!stricmp(ii->ship_name, old_name))
			strcpy_s(ii->ship_name, new_name);
	}
}

void time_to_mission_info_string(const std::tm* src, char* dest, size_t dest_max_len)
{
	std::strftime(dest, dest_max_len, "%x at %X", src);
}
