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
#include <globalincs/pstypes.h>
#include <globalincs/safe_strings.h>
#include <hud/hudsquadmsg.h>
#include <iff_defs/iff_defs.h>
#include <jumpnode/jumpnode.h>
#include <math/vecmat.h>
#include <mission/missionbriefcommon.h>
#include <mission/missiongoals.h>
#include <mission/missionmessage.h>
#include <mission/missionparse.h>
#include <model/model.h>
#include <nebula/neb.h>
#include <object/object.h>
#include <object/waypoint.h>
#include <parse/parselo.h>
#include <ship/ship.h>
#include <species_defs/species_defs.h>
#include <starfield/nebula.h>
#include <starfield/starfield.h>
#include <weapon/weapon.h>

#include <wx/image.h>
#include <wx/string.h>
//#include <wx/xrc/xmlres.h>
#include <wx/wx.h>

#include <stdlib.h>

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

bool wxFRED2::OnInit() {
	//wxXmlResource::Get()->InitAllHandlers();
	//InitXmlResource();
	//wxFREDMission* the_Mission = new wxFREDMission();
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
			if (Ship_info[j].flags & SIF_DEFAULT_PLAYER_SHIP) {
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
	Nebula_pitch = (int)((float)(rand() & 0x0fff) * 360.0f / 4096.0f);
	Nebula_bank = (int)((float)(rand() & 0x0fff) * 360.0f / 4096.0f);
	Nebula_heading = (int)((float)(rand() & 0x0fff) * 360.0f / 4096.0f);
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
	The_mission.flags = 0;
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
	Create_player(&vmd_zero_vector, &vmd_identity_matrix);
}

OIN_t wxFRED2::Object_create(MTIN_t mtin, vec3d *pos, matrix *orient, WIN_t win = -1) {
	OIN_t obj;
		if (obj >= 0) {
			// Successfully created ship
			int n;

			n = Objects[obj].instance;
			Ships[n].arrival_cue = alloc_sexp("true", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
			Ships[n].departure_cue = alloc_sexp("false", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
			Ships[n].cargo1 = 0;
		}

	if (obj >= 0) {
		// Successfully created object
		obj_merge_created_list();
		Document.Modify(true);
	}

	return obj;
}

// TODO: dup_object in fred has wierdness with waypoints. Need to figure out how to do that SANELY
OIN_t wxFRED2::Object_copy(object *objp) {
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
			throw Fred_exception("Unable to create new player start point. You have reached the maximum limit.\n");

		} else if (The_mission.game_type & MISSION_TYPE_SINGLE) {
			throw Fred_exception("You can't have more than one player start in single player missions.\n");

		} else if (The_mission.game_type & MISSION_TYPE_TRAINING) {
			throw Fred_exception("You can't have more than one player start in training missions.\n");
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
			throw Fred_exception("Could not create ship for player start.\n");
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

	if (Ship_info[type].flags & SIF_NO_FRED) {
		// Is a Non-FRED-able object
		throw Fred_exception("Unable to create Non-FRED-able object");
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
		throw Fred_exception("Unable to create new ship. (Unknown cause)");
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
	resolve_parse_flags(&Objects[obj], Iff_info[shipp->team].default_parse_flags, Iff_info[shipp->team].default_parse_flags2);

	// default shield setting
	shipp->special_shield = -1;
	z1 = Shield_sys_teams[shipp->team];
	z2 = Shield_sys_types[type];
	if (((z1 == 1) && (z2 != 0)) || (z2 == 1)) {
		Objects[obj].flags |= OF_NO_SHIELDS;
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
		if (!(sip->flags & SIF_SMALL_SHIP)) {
			shipp->orders_accepted = ship_get_default_orders_accepted(sip);
			shipp->orders_accepted &= ~DEPART_ITEM;
		}

	} else {
		// Else, no orders
		shipp->orders_accepted = 0;
	}

	// calc kamikaze stuff
	if (shipp->use_special_explosion) {
		temp_max_hull_strength = (float)shipp->special_exp_blast;
	} else {
		temp_max_hull_strength = sip->max_hull_strength;
	}

	Ai_info[shipp->ai_index].kamikaze_damage = (int)std::min(1000.0f, 200.0f + (temp_max_hull_strength / 4.0f));

	int n;
	n = Objects[obj].instance;
	Ships[n].arrival_cue = alloc_sexp("true", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
	Ships[n].departure_cue = alloc_sexp("false", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
	Ships[n].cargo1 = 0;

	obj_merge_created_list();
	return obj;
}

OIN_t wxFRED2::Create_waypoint(vec3d *pos, WIN_t win) {
	int obj = waypoint_add(pos, win);


	if (obj >= 0) {
		Document.Modify(true);
		obj_merge_created_list();
	} else {
		throw Fred_exception("Unable to create waypoint (Unknown cause)");
	}

	return obj;
}

