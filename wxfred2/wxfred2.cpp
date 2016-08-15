/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "wxfred2.h"

#include "frmFRED2.h"
#include "base/wxFRED_base.h"

#include <ai/ai.h>
#include <ai/aigoals.h>
#include <ai/ai_profiles.h>
#include <asteroid/asteroid.h>
#include <bmpman/bmpman.h>
#include <gamesnd/eventmusic.h>
#include <globalincs/globals.h>
#include <globalincs/linklist.h>
#include <globalincs/mspdb_callstack.h>
#include <globalincs/pstypes.h>
#include <globalincs/safe_strings.h>
#include <hud/hudsquadmsg.h>
#include <iff_defs/iff_defs.h>
#include <jumpnode/jumpnode.h>
#include <localization/localize.h>
#include <math/vecmat.h>
#include <mission/missionbriefcommon.h>
#include <mission/missiongoals.h>
#include <mission/missionmessage.h>
#include <mission/missionparse.h>
#include <model/model.h>
#include <nebula/neb.h>
#include <object/object.h>
#include <object/waypoint.h>
#include <osapi/outwnd.h>
#include <parse/parselo.h>
#include <ship/ship.h>
#include <ship/ship_flags.h>
#include <species_defs/species_defs.h>
#include <starfield/nebula.h>
#include <starfield/starfield.h>
#include <weapon/weapon.h>

#include <wx/dir.h>
#include <wx/image.h>
#include <wx/string.h>
//#include <wx/xrc/xmlres.h>
#include <wx/wx.h>

#include <stdlib.h>
#include <algorithm>

// Referenced variables and functions from code.lib
char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];
char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];
int Show_cpu = 0;
int Fred_running = 1;	// z64: :V:'s cheap hack to let the codebase know that fred-specific routines should be run
float Sun_spot = 0.0f;

// "Private" variables and functions
// from starfield.cpp
extern int Nmodel_num;
extern matrix Nmodel_orient;
extern int Nmodel_bitmap;

IMPLEMENT_APP(wxFRED2)

wxFRED2::~wxFRED2() {
	SCP_mspdbcs_Cleanup();
}

bool wxFRED2::OnInit() {
	//wxXmlResource::Get()->InitAllHandlers();
	//InitXmlResource();
	//wxFREDMission* the_Mission = new wxFREDMission();

	// Filepaths
	Fred_base_dir = wxGetCwd().ToStdString();
	Fred_exe_dir = Fred_base_dir + DIR_SEPARATOR_STR + "wxfred2.exe";

	// Init FSO codebase
	Init_FSO();

	// Init GUI
	wxChar* title = NULL;
	// Init image handlers before frmFRED2 (wxFormBuilder workaround)
	wxImage::AddHandler(new wxPNGHandler);
	frmFRED2 *Fred_gui = new frmFRED2(title, 50, 50, 800, 600);
	SetTopWindow(Fred_gui);
	Fred_gui->Show(TRUE);

	return true;
}

// Mission Management Methods

void wxFRED2::Mission_clear() {
	const char *str;
	int i, j, count;

	wxDateTime time;

	/** Interface calls before calling this method:

	Breifing_dialog->reset_editor()
	fiction_viewer_reset();
	cmd_brief_reset();
	set_cur_indices(-1);
	unmark_all();
	set_physics_controls();
	*/

	/** Interface calls after calling this method:
	fred_render_init();
	*/

	extern void allocate_mission_text(int size);	// from parse/parselo.h
	allocate_mission_text(MISSION_TEXT_SIZE);

	The_mission.cutscenes.clear();
	mission_event_shutdown();

	// Set the asteroid field to defaults
	Asteroid_field.num_initial_asteroids = 0;
	Asteroid_field.speed = 0.0f;
	vm_vec_make(&Asteroid_field.min_bound, -1000.0f, -1000.0f, -1000.0f);
	vm_vec_make(&Asteroid_field.max_bound, 1000.0f, 1000.0f, 1000.0f);
	vm_vec_make(&Asteroid_field.inner_min_bound, -500.0f, -500.0f, -500.0f);
	vm_vec_make(&Asteroid_field.inner_max_bound, 500.0f, 500.0f, 500.0f);
	Asteroid_field.has_inner_bound = 0;
	Asteroid_field.field_type = FT_ACTIVE;
	Asteroid_field.debris_genre = DG_ASTEROID;
	Asteroid_field.field_debris_type[0] = -1;
	Asteroid_field.field_debris_type[1] = -1;
	Asteroid_field.field_debris_type[2] = -1;

	strcpy_s(Mission_parse_storm_name, "none");

	obj_init();
	model_free_all();
	ai_init();
	ai_profiles_init();
	ship_init();

	jumpnode_level_close();
	waypoint_level_close();

	Num_wings = 0;
	for (i = 0; i < MAX_WINGS; i++) {
		Wings[i].wave_count = 0;
		Wings[i].wing_squad_filename[0] = '\0';
		Wings[i].wing_insignia_texture = -1;
	}

	for (i = 0; i < MAX_IFFS; i++) {
		Shield_sys_teams[i] = 0;
	}

	for (i = 0; i < MAX_SHIP_CLASSES; i++) {
		Shield_sys_types[i] = 0;
	}

	Num_ai_dock_names = 0;
	Num_reinforcements = 0;

	// Set default Mission Specs
	str = wxGetUserName().c_str().AsChar();
	strcpy_s(The_mission.name, "Untitled");
	strncpy(The_mission.author, str, NAME_LENGTH - 1);

	time = wxGetCurrentTime();
	str = time.Format("%x at %X").c_str().AsChar();
	strcpy_s(The_mission.created, str);
	strcpy_s(The_mission.modified, The_mission.created);

	strcpy_s(The_mission.notes, "This is a wxFRED2 created mission.\n");
	strcpy_s(The_mission.mission_desc, "Put mission description here\n");
	The_mission.game_type = MISSION_TYPE_SINGLE;
	strcpy_s(The_mission.squad_name, "");
	strcpy_s(The_mission.squad_filename, "");
	The_mission.num_respawns = 3;
	The_mission.max_respawn_delay = -1;

	Player_starts = 0;
	Num_teams = 1;

	// reset alternate name & callsign stuff
	for (i = 0; i < MAX_SHIPS; i++) {
		strcpy_s(Fred_alt_names[i], "");
		strcpy_s(Fred_callsigns[i], "");
	}

	// set up the default ship types for all teams.  For now, this is the same class
	// of ships for all teams
	for (i = 0; i < MAX_TVT_TEAMS; i++) {
		count = 0;
		for (j = 0; j < MAX_SHIP_CLASSES; j++) {
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
		for (j = 0; j < MAX_WEAPON_TYPES; j++) {
			if (Weapon_info[j].wi_flags & WIF_PLAYER_ALLOWED) {
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

	*Mission_text = *Mission_text_raw = EOF_CHAR;
	Mission_text[1] = Mission_text_raw[1] = 0;

	waypoint_parse_init();
	Num_mission_events = 0;
	Num_goals = 0;

	init_sexp();
	messages_init();
	brief_reset();
	debrief_reset();
	//ship_init();
	event_music_reset_choices();
	clear_texture_replacements();

	mission_parse_reset_alt();
	mission_parse_reset_callsign();

	strcpy(Cargo_names[0], "Nothing");
	Num_cargo = 1;

	// reset background bitmaps and suns
	stars_pre_level_init();
	Nebula_index = 0;
	Mission_palette = 1;
	Nebula_pitch = (int) ((float) (rand() & 0x0fff) * 360.0f / 4096.0f);
	Nebula_bank = (int) ((float) (rand() & 0x0fff) * 360.0f / 4096.0f);
	Nebula_heading = (int) ((float) (rand() & 0x0fff) * 360.0f / 4096.0f);
	Neb2_awacs = -1.0f;
	Neb2_poof_flags = 0;
	strcpy_s(Neb2_texture_name, "");
	for (i = 0; i < MAX_NEB2_POOFS; i++) {
		Neb2_poof_flags |= (1 << i);
	}

	Nmodel_flags = DEFAULT_NMODEL_FLAGS;
	Nmodel_num = -1;
	vm_set_identity(&Nmodel_orient);
	Nmodel_bitmap = -1;

	The_mission.contrail_threshold = CONTRAIL_THRESHOLD_DEFAULT;

	// Goober5000
	The_mission.command_persona = Default_command_persona;
	strcpy_s(The_mission.command_sender, DEFAULT_COMMAND);

	// Goober5000: reset ALL mission flags, not just nebula!
	The_mission.flags.reset();
	The_mission.support_ships.max_support_ships = -1;	// negative means infinite
	The_mission.support_ships.max_hull_repair_val = 0.0f;
	The_mission.support_ships.max_subsys_repair_val = 100.0f;
	The_mission.ai_profile = &Ai_profiles[Default_ai_profile];

	nebula_init(Nebula_filenames[Nebula_index], Nebula_pitch, Nebula_bank, Nebula_heading);

	char palette_filename[1024];
	strcpy_s(palette_filename, "gamepalette1-01");
	//	sprintf( palette_filename, "gamepalette%d-%02d", 1, Mission_palette+1 );
	mprintf(("Loading palette %s\n", palette_filename));
	palette_load_table(palette_filename);

	strcpy_s(The_mission.loading_screen[GR_640], "");
	strcpy_s(The_mission.loading_screen[GR_1024], "");
	strcpy_s(The_mission.skybox_model, "");
	vm_set_identity(&The_mission.skybox_orientation);
	strcpy_s(The_mission.envmap_name, "");
	The_mission.skybox_flags = DEFAULT_NMODEL_FLAGS;

	// no sound environment
	The_mission.sound_environment.id = -1;

	ENVMAP = -1;

	Document.Modify(false);
}

void wxFRED2::Mission_new(void) {
	Mission_reset();
	Mission_filename[0] = '\0';
	//	docFREDDoc_ptr->autosave("nothing");
	//	Undo_count = 0;
};

bool wxFRED2::Mission_load(const wxString infile) {
	wxMessageBox(_T("Sorry! This feature has not been implemented yet.\nPlease encourage a developer to get right on this. :)"), _T("Unimplemented feature"), wxOK);
	return false;
}

void wxFRED2::Mission_reset() {
	Mission_clear();
	Create_player(&vmd_zero_vector, &vmd_identity_matrix, -1);
}

OIN_t wxFRED2::Copy_object(OIN_t oin) {
	OIN_t oin_copy = -1;
	object* objp;

	if (oin < 0) {
		throw Fred_exception("Error: Could not copy object - invalid input");
		return -1;
	}

	objp = &Objects[oin];

	switch (objp->type) {
	case OBJ_JUMP_NODE:
		oin_copy = Create_jumpnode(&objp->pos);
		break;

	case OBJ_SHIP:
	case OBJ_START:
		int inst;		// Index in Ships[] to object
		int inst_copy;	// Index in Ships[] to object's copy

		inst = objp->instance;

		// MSVC doesn't like function pointers to class members, so gotta call directly
		if (objp->type == OBJ_SHIP) {
			oin_copy = Create_ship(&objp->pos, &objp->orient, Ships[inst].ship_info_index);
		} else {
			oin_copy = Create_player(&objp->pos, &objp->orient, Ships[inst].ship_info_index);
		}
		Assert(oin_copy >= 0);

		// New ship created, now copy all relevant settings
		// Ship info
		inst_copy = Objects[oin_copy].instance;
		Ships[inst_copy].team = Ships[inst].team;
		Ships[inst_copy].arrival_cue = dup_sexp_chain(Ships[inst].arrival_cue);
		Ships[inst_copy].departure_cue = dup_sexp_chain(Ships[inst].departure_cue);
		Ships[inst_copy].cargo1 = Ships[inst].cargo1;
		Ships[inst_copy].arrival_location = Ships[inst].arrival_location;
		Ships[inst_copy].departure_location = Ships[inst].departure_location;
		Ships[inst_copy].arrival_delay = Ships[inst].arrival_delay;
		Ships[inst_copy].departure_delay = Ships[inst].departure_delay;
		Ships[inst_copy].weapons = Ships[inst].weapons;
		Ships[inst_copy].hotkey = Ships[inst].hotkey;

		// AI Info
		ai_info* aip1;  // ai_info of object
		ai_info* aip2;  // ai_info of object's copy

		aip1 = &Ai_info[Ships[inst].ai_index];
		aip2 = &Ai_info[Ships[inst_copy].ai_index];

		aip2->behavior = aip1->behavior;
		aip2->ai_class = aip1->ai_class;
		for (int i = 0; i < MAX_AI_GOALS; i++) {
			aip2->goals[i] = aip1->goals[i];
		}

		if (aip1->ai_flags & AIF_KAMIKAZE)
			aip2->ai_flags |= AIF_KAMIKAZE;
		if (aip1->ai_flags & AIF_NO_DYNAMIC)
			aip2->ai_flags |= AIF_NO_DYNAMIC;

		aip2->kamikaze_damage = aip1->kamikaze_damage;

		// Physics, Hull, and Shield
		object *objp_copy;      // object* to copy

		objp_copy = &Objects[oin_copy];
		objp_copy->phys_info.speed = objp->phys_info.speed;
		objp_copy->phys_info.fspeed = objp->phys_info.fspeed;
		objp_copy->hull_strength = objp->hull_strength;
		objp_copy->shield_quadrant[0] = objp->shield_quadrant[0];

		// Subsystems
		ship_subsys* subp1;     // ship_subsys* of object 
		ship_subsys* subp2;     // ship_subsys* of object's copy

		subp1 = GET_FIRST(&Ships[inst].subsys_list);
		subp2 = GET_FIRST(&Ships[inst_copy].subsys_list);
		while (subp2 != END_OF_LIST(&Ships[inst_copy].subsys_list)) {
			Assert(subp1 != END_OF_LIST(&Ships[inst].subsys_list));
			subp2->current_hits = subp1->current_hits;
			subp2 = GET_NEXT(subp2);
			subp1 = GET_NEXT(subp1);
		}

		// Reinforcements
		for (int i = 0, j = 0; i < Num_reinforcements; i++)
			if (!stricmp(Reinforcements[i].name, Ships[inst].ship_name)) {
				// object has reinforcements, so object's copy will have reinforcements, too
				if (Num_reinforcements < MAX_REINFORCEMENTS) {
					j = Num_reinforcements;
					Num_reinforcements++;
					strcpy_s(Reinforcements[j].name, Ships[inst_copy].ship_name);
					Reinforcements[j].type = Reinforcements[i].type;
					Reinforcements[j].uses = Reinforcements[i].uses;
				} else {
					warning_stack.push_back("Warning: Copy_object - Could not create reinforcements for copied object. (Too many reinforcements already)\n");
				}

				break;
			}
		break;

	case OBJ_WAYPOINT:
		oin_copy = Create_waypoint(&objp->pos);
		break;

	default:
		throw Fred_exception("Error: Could not copy object - invalid object type\n");
	}

	return oin_copy;
}


void wxFRED2::Copy_objects(SCP_vector<OIN_t> &selq) {
	SCP_vector<OIN_t> wp_queue;
	SCP_vector<OIN_t> new_selq;
	OIN_t new_oin;
	bool has_errors = false;

	if (selq.size() == 0) {
		throw Fred_exception("Error: Couldn't copy objects (nothing in selection queue)\n");
		return;
	}

	for (auto it = selq.begin(); it != selq.end(); ++it) {
		if (Objects[*it].type == OBJ_WAYPOINT) {
			// Defer copying waypoints until later
			wp_queue.push_back(*it);

		} else {
			try {
				new_oin = Copy_object(*it);

			} catch (const Fred_exception& e) {
				// Couldn't copy this one, so skip it
				has_errors = true;

				warning_stack.push_back(e.what());
				continue;
			}
		}
	}

	if (!wp_queue.empty()) {
		int wp_inst = -1;   // Waypoint instance to append a new waypoint to. Value of -1 makes a new list
		int wpl_idx1 = -1;   // Previous WPL
		int wpl_idx2;        // Current WPL

		// Assumption: Waypoint's instances are a simple hash which contains their list idx and their sequence idx
		// within that list, and is sortable (see /object/waypoint.cpp)
		std::sort(wp_queue.begin(), wp_queue.end());

		for (auto it = wp_queue.begin(); it != wp_queue.end(); ++it) {
			wpl_idx2 = calc_waypoint_list_index(Objects[*it].instance);

			if (wpl_idx1 != wpl_idx2) {
				// New list detected, reset vars
				wp_inst = -1;
				wpl_idx1 = wpl_idx2;
			}

			// Copy the waypoint. If wp_inst = -1 a new list is formed
			new_oin = waypoint_add(&Objects[*it].pos, wp_inst);

			if (new_oin >= 0) {
				// Keep the instance for next iteration
				wp_inst = Objects[new_oin].instance;

				// Copy complete, push to selection
				new_selq.push_back(new_oin);

			} else {
				// Couln't copy this one for some reason
				has_errors = true;
				warning_stack.push_back("Error: couldn't copy waypoint");
			}
		}

		obj_merge_created_list();
	}

	if (has_errors) {
		throw Fred_exception("One or more errors occured during Copy_objects. Please review the warning stack.\n");
	}

	// Copy job done
	Document.Modify(true);
	selq = new_selq;

	return;
}

OIN_t wxFRED2::Create_jumpnode(vec3d *pos) {
	OIN_t obj;

	CJumpNode jnp(pos);
	obj = jnp.GetSCPObjectNumber();

	Assert(obj >= 0);
	Jump_nodes.push_back(std::move(jnp));

	obj_merge_created_list();
	return obj;
}

OIN_t wxFRED2::Create_player(vec3d *pos, matrix *orient, MTIN_t type) {
	OIN_t obj = -1;

	// Check that we're able to add a new player start
	if (Player_starts >= 1) {
		if (Player_starts >= MAX_PLAYERS) {
			throw Fred_exception("Error: Unable to create new player start point. You have reached the maximum limit.\n");

		} else if (The_mission.game_type & MISSION_TYPE_SINGLE) {
			throw Fred_exception("Error: You can't have more than one player start in single player missions.\n");

		} else if (The_mission.game_type & MISSION_TYPE_TRAINING) {
			throw Fred_exception("Error: You can't have more than one player start in training missions.\n");
		}

	} else {
		// Else, try creating player start

		if (type < 0) {
			type = MTIN_player_default;
		}

		try {
			obj = Create_ship(pos, orient, type);

		} catch (Fred_exception &e) {
			// Rethrow with more descriptive message
			// TODO: Include player start number in message
			throw Fred_exception("Error: Could not create ship for player start.\n");
		}

		Assert(obj >= 0);
		Player_starts++;
		Objects[obj].type = OBJ_START;

		// be sure arrival/departure cues are set
		Ships[Objects[obj].instance].arrival_cue = Locked_sexp_true;
		Ships[Objects[obj].instance].departure_cue = Locked_sexp_false;
		obj_merge_created_list();
		Document.Modify(true);
	}

	return obj;
}

OIN_t wxFRED2::Create_ship(vec3d *pos, matrix *orient, MTIN_t type) {
	int obj, z1, z2;
	float temp_max_hull_strength;
	ship_info *sip;

	if (Ship_info[type].flags[Ship::Info_Flags::No_fred]) {
		// Is a Non-FRED-able object
		throw Fred_exception("Error: Unable to create Non-FRED-able object");
		return -1;
	}

	// ship::ship_create() tries to look in the cwd for the .pof's when loading in new models.
	// Fred's currently in somewhere like <fs2path>[/modpath]/data/missions/ so we have to hop to <fs2path> in order to continue
	SCP_string pwd;

	pwd = wxGetCwd().ToStdString();
	wxSetWorkingDirectory(Fred_base_dir);
	obj = ship_create(orient, pos, type);
	wxSetWorkingDirectory(pwd);

	if (obj < 0) {
		throw Fred_exception("Error: Unable to create new ship. (Unknown cause)");
		return -1;
	}

	Objects[obj].phys_info.speed = 33.0f;

	ship *shipp = &Ships[Objects[obj].instance];
	sip = &Ship_info[shipp->ship_info_index];

	// Probably not needed, since we're not naming the ship
	/*
	if (query_ship_name_duplicate(Objects[obj].instance))
	fix_ship_name(Objects[obj].instance);
	*/

	// default stuff according to species and IFF
	shipp->team = Species_info[Ship_info[shipp->ship_info_index].species].default_iff;
	resolve_parse_flags(&Objects[obj], Iff_info[shipp->team].default_parse_flags);

	// default shield setting
	shipp->special_shield = -1;
	z1 = Shield_sys_teams[shipp->team];
	z2 = Shield_sys_types[type];
	if (((z1 == 1) && (z2 != 0)) || (z2 == 1)) {
		Objects[obj].flags.set(Object::Object_Flags::No_shields);
	}

	object *temp_objp;
	ship *temp_shipp = NULL;

	// find the first player ship
	for (temp_objp = GET_FIRST(&obj_used_list); temp_objp != END_OF_LIST(&obj_used_list); temp_objp = GET_NEXT(temp_objp)) {
		if (temp_objp->type == OBJ_START) {
			temp_shipp = &Ships[temp_objp->instance];
			break;
		}
	}

	// set orders if ship is on same team as the player, or if player couldn't be found
	if (temp_shipp == NULL || shipp->team == temp_shipp->team) {
		// if this ship is not a small ship, then make the orders be the default orders without
		// the depart item
		if (!(sip->is_small_ship())) {
			shipp->orders_accepted = ship_get_default_orders_accepted(sip);
			shipp->orders_accepted &= ~DEPART_ITEM;
		}

	} else {
		// Else, no orders
		shipp->orders_accepted = 0;
	}

	// calc kamikaze stuff
	if (shipp->use_special_explosion) {
		temp_max_hull_strength = (float) shipp->special_exp_blast;
	} else {
		temp_max_hull_strength = sip->max_hull_strength;
	}

	Ai_info[shipp->ai_index].kamikaze_damage = (int) std::min(1000.0f, 200.0f + (temp_max_hull_strength / 4.0f));

	int n;
	n = Objects[obj].instance;
	Ships[n].arrival_cue = alloc_sexp("true", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
	Ships[n].departure_cue = alloc_sexp("false", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
	Ships[n].cargo1 = 0;

	obj_merge_created_list();
	return obj;
}

OIN_t wxFRED2::Create_waypoint(vec3d *pos, OIN_t oin) {
	int inst;   // Object instance of the waypoint indexed by oin, if supplied
	int obj;    // Object instance of the new waypoint

	if (oin >= 0) {
		if (Objects[oin].type == OBJ_WAYPOINT) {
			inst = Objects[oin].instance;
		} else {
			inst = -1;
		}
	}

	obj = waypoint_add(pos, inst);

	if (obj >= 0) {
		Document.Modify(true);
		obj_merge_created_list();
	} else {
		throw Fred_exception("Error: Unable to create waypoint (Unknown cause)");
	}

	return obj;
}

void wxFRED2::Init_FSO(void) {
	SCP_mspdbcs_Initialise();

#ifndef NDEBUG
	// TODO Maybe use wxWidget's windows instead
	outwnd_init();
#endif

	SDL_SetMainReady();
	memory::init();

	srand((unsigned) time(NULL));
	//	init_pending_messages();

	os_init(Osreg_class_name, Osreg_app_name);

	timer_init();

	Assert(Fred_base_dir.size() > 0); //-V805

	// sigh... this should enable proper reading of cmdline_fso.cfg - Goober5000
	cfile_chdir(Fred_base_dir.c_str());

	// this should enable mods - Kazan
	// parse_cmdline(__argc, __argv);

	// TODO: Print cmdline if DEBUG

	// d'oh
	if (cfile_init(Fred_exe_dir.c_str())) {
		wxMessageBox("Failed to init cfile!");	// TODO: Make this aware of cmdline instances. If it's cmdline, output message to console instead of making a messagebox
		wxExit();
	}

	// Load game_settings.tbl
	// mod_table_init();

	// initialize localization module. Make sure this is done AFTER initialzing OS.
	// NOTE : Fred should ALWAYS run in English. Otherwise it might swap in another language
	// when saving - which would cause inconsistencies when externalizing to tstrings.tbl via Exstr
	// trust me on this :)
	lcl_init(FS2_OPEN_DEFAULT_LANGUAGE);

	// Goober5000 - force init XSTRs (so they work, but only work in English, based on above comment)
	extern int Xstr_inited;
	Xstr_inited = 1;

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
}

bool wxFRED2::Has_warnings() {
	bool retval;

	(warning_stack.size() > 0) ? (retval = true) : (retval = false);
	return retval;
}

void wxFRED2::Get_warnings(SCP_vector<SCP_string> &warnings) {
	warnings = warning_stack;

	warning_stack.clear();
}

void wxFRED2::Ignore_warnings() {
	warning_stack.clear();
}

