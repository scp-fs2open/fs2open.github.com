#include "Editor.h"

#include <array>
#include <vector>
#include <stdexcept>
#include <clocale>

#include <SDL.h>
#include <sound/audiostr.h>
#include <parse/parselo.h>
#include <missionui/fictionviewer.h>
#include <mission/missiongoals.h>
#include <asteroid/asteroid.h>
#include <jumpnode/jumpnode.h>
#include <util.h>
#include <mission/missionmessage.h>
#include <gamesnd/eventmusic.h>
#include <starfield/nebula.h>
#include <object/objectdock.h>
#include <localization/fhash.h>

#include "iff_defs/iff_defs.h" // iff_init
#include "object/object.h" // obj_init
#include "species_defs/species_defs.h" // species_init
#include "weapon/weapon.h" // weapon_init
#include "nebula/neb.h" // neb2_init
#include "starfield/starfield.h" // stars_init, stars_pre_level_init, stars_post_level_init
#include "hud/hudsquadmsg.h"
#include "globalincs/linklist.h"

#include "ui/QtGraphicsOperations.h"

#include "object.h"
#include "management.h"
#include "FredApplication.h"

namespace {

int query_referenced_in_ai_goals(int type, const char* name) {
	int i;

	for (i = 0; i < MAX_AI_INFO; i++) {  // loop through all Ai_info entries
		if (Ai_info[i].shipnum >= 0) {  // skip if unused
			if (query_referenced_in_ai_goals(Ai_info[i].goals, type, name)) {
				return Ai_info[i].shipnum | SRC_SHIP_ORDER;
			}
		}
	}

	for (i = 0; i < MAX_WINGS; i++) {
		if (Wings[i].wave_count) {
			if (query_referenced_in_ai_goals(Wings[i].ai_goals, type, name)) {
				return i | SRC_WING_ORDER;
			}
		}
	}

	return 0;
}

// Used in the FRED drop-down menu and in error_check_initial_orders
// NOTE: Certain goals (Form On Wing, Rearm, Chase Weapon, Fly To Ship) aren't listed here.  This may or may not be intentional,
// but if they are added in the future, it will be necessary to verify correct functionality in the various FRED dialog functions.
ai_goal_list Ai_goal_list[] = {
	{ "Waypoints",				AI_GOAL_WAYPOINTS,			0 },
	{ "Waypoints once",			AI_GOAL_WAYPOINTS_ONCE,		0 },
	{ "Attack",					AI_GOAL_CHASE | AI_GOAL_CHASE_WING,	0 },
	{ "Attack any ship",		AI_GOAL_CHASE_ANY,			0 },
	{ "Attack ship class",		AI_GOAL_CHASE_SHIP_CLASS,	0 },
	{ "Guard",					AI_GOAL_GUARD | AI_GOAL_GUARD_WING, 0 },
	{ "Disable ship",			AI_GOAL_DISABLE_SHIP,		0 },
	{ "Disarm ship",			AI_GOAL_DISARM_SHIP,		0 },
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

}

char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];
char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];

extern void allocate_parse_text(size_t size);

extern int Nmodel_num;
extern int Nmodel_instance_num;
extern matrix Nmodel_orient;
extern int Nmodel_bitmap;

namespace fso {
namespace fred {
	
Editor::Editor() : currentObject{ -1 }, Shield_sys_teams(Iff_info.size(), 0), Shield_sys_types(MAX_SHIP_CLASSES, 0) {
	connect(fredApp, &FredApplication::onIdle, this, &Editor::update);

	// When the mission changes we need to update all renderers
	connect(this, &Editor::missionChanged, this, &Editor::updateAllViewports);

	// When a mission was loaded we need to notify everyone that the mission has changed
	connect(this, &Editor::missionLoaded, this, [this](const std::string&) { missionChanged(); });

	fredApp->runAfterInit([this]() { initialSetup(); });
}

EditorViewport* Editor::createEditorViewport(os::Viewport* renderView) {
	std::unique_ptr<FredRenderer> renderer(new FredRenderer(renderView));
	std::unique_ptr<EditorViewport> viewport(new EditorViewport(this, std::move(renderer)));

	auto ptr = viewport.get();
	_viewports.push_back(std::move(viewport));

	return ptr;
}

void Editor::update() {
	// Do updates for all renderers
	for (auto& viewport : _viewports) {
		viewport->game_do_frame(currentObject);
	}
}

bool Editor::loadMission(const std::string& mission_name, int flags) {
	char name[512], * old_name;
	int i, j, k, ob;
	int used_pool[MAX_WEAPON_TYPES];
	object* objp;

	// activate the localizer hash table
	fhash_flush();

	clearMission();

	std::string filepath = mission_name;
	auto res = cf_find_file_location(filepath.c_str(), CF_TYPE_MISSIONS);
	if (res.found) {
		// We found this file in the CFile system
		if (res.offset == 0) {
			// This is a "real" file. Since we now have an absolute path we can use that to make sure we only deal with
			// absolute paths which makes sure that the "recent mission" menu has correct file names
			filepath = res.full_name;
		}
	}

	if (!parse_main(filepath.c_str(), flags)) {
		if (flags & MPF_IMPORT_FSM) {
			SCP_string msg;
			sprintf(msg, "Unable to import the file \"%s\".", filepath.c_str());

			_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error,
																  "Load Error",
																  msg,
																  { DialogButton::Ok });
		} else {
			SCP_string msg;
			sprintf(msg, "Unable to load the file \"%s\".", filepath.c_str());
			_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error,
																  "Load Error",
																  msg,
																  { DialogButton::Ok });
		}
		createNewMission();

		return false;
	}

	if ((Num_unknown_ship_classes > 0) || (Num_unknown_weapon_classes > 0) || (Num_unknown_loadout_classes > 0)) {
		if (flags & MPF_IMPORT_FSM) {
			SCP_string msg;
			sprintf(msg,
					"Fred encountered unknown ship/weapon classes when importing \"%s\" (path \"%s\"). You will have to manually edit the converted mission to correct this.",
					The_mission.name,
					filepath.c_str());

			_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Warning,
																  "Unknown Ship classes",
																  msg,
																  { DialogButton::Ok });
		} else {
			_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Warning,
																  "Unknown Ship classes",
																  "Fred encountered unknown ship/weapon classes when parsing the mission file. This may be due to mission disk data you do not have.",
																  { DialogButton::Ok });
		}
	}

	obj_merge_created_list();
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if (objp->flags[Object::Object_Flags::Player_ship]) {
			Assert(objp->type == OBJ_SHIP);
			objp->type = OBJ_START;
			//			Player_starts++;
		}

		objp = GET_NEXT(objp);
	}

	for (i = 0; i < Num_wings; i++) {
		for (j = 0; j < Wings[i].wave_count; j++) {
			ob = Ships[Wings[i].ship_index[j]].objnum;
			wing_objects[i][j] = ob;
			Ships[Wings[i].ship_index[j]].wingnum = i;
			Ships[Wings[i].ship_index[j]].arrival_cue = Locked_sexp_false;
		}

		// fix old ship names for ships in wings if needed
		while (j--) {
			if ((Objects[wing_objects[i][j]].type == OBJ_SHIP)
				|| (Objects[wing_objects[i][j]].type == OBJ_START)) {  // don't change player ship names
				wing_bash_ship_name(name, Wings[i].name, j + 1);
				old_name = Ships[Wings[i].ship_index[j]].ship_name;
				if (stricmp(name, old_name) != 0) {  // need to fix name
					update_sexp_references(old_name, name);
					ai_update_goal_references(REF_TYPE_SHIP, old_name, name);
					update_texture_replacements(old_name, name);
					for (k = 0; k < Num_reinforcements; k++) {
						if (!strcmp(old_name, Reinforcements[k].name)) {
							Assert(strlen(name) < NAME_LENGTH);
							strcpy_s(Reinforcements[k].name, name);
						}
					}

					strcpy_s(Ships[Wings[i].ship_index[j]].ship_name, name);
				}
			}
		}
	}

	for (i = 0; i < Num_teams; i++) {
		generate_team_weaponry_usage_list(i, used_pool);
		for (j = 0; j < Team_data[i].num_weapon_choices; j++) {
			// The amount used in wings is always set by a static loadout entry so skip any that were set by Sexp variables
			if ((!strlen(Team_data[i].weaponry_pool_variable[j]))
				&& (!strlen(Team_data[i].weaponry_amount_variable[j]))) {
				// convert weaponry_pool to be extras available beyond the current ships weapons
				Team_data[i].weaponry_count[j] -= used_pool[Team_data[i].weaponry_pool[j]];
				if (Team_data[i].weaponry_count[j] < 0) {
					Team_data[i].weaponry_count[j] = 0;
				}

				// zero the used pool entry
				used_pool[Team_data[i].weaponry_pool[j]] = 0;
			}
		}
		// double check the used pool is empty
		for (j = 0; j < static_cast<int>(Weapon_info.size()); j++) {
			if (used_pool[j] != 0) {
				Warning(LOCATION,
						"%s is used in wings of team %d but was not in the loadout. Fixing now",
						Weapon_info[j].name,
						i + 1);

				// add the weapon as a new entry
				Team_data[i].weaponry_pool[Team_data[i].num_weapon_choices] = j;
				Team_data[i].weaponry_count[Team_data[i].num_weapon_choices] = used_pool[j];
				strcpy_s(Team_data[i].weaponry_amount_variable[Team_data[i].num_weapon_choices], "");
				strcpy_s(Team_data[i].weaponry_pool_variable[Team_data[i].num_weapon_choices++], "");
			}
		}
	}

	Assert(Mission_palette >= 0);
	Assert(Mission_palette <= 98);

	// go through all ships and translate their callsign and alternate name indices
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		// if this is a ship, check it, and mark its possible alternate name down in the auxiliary array
		if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->instance >= 0)) {
			if (Ships[objp->instance].alt_type_index >= 0) {
				strcpy_s(Fred_alt_names[objp->instance], mission_parse_lookup_alt_index(Ships[objp->instance].alt_type_index));

				// also zero it
				Ships[objp->instance].alt_type_index = -1;
			}

			if (Ships[objp->instance].callsign_index >= 0) {
				strcpy_s(Fred_callsigns[objp->instance], mission_parse_lookup_callsign_index(Ships[objp->instance].callsign_index));

				// also zero it
				Ships[objp->instance].callsign_index = -1;
			}
		}

		objp = GET_NEXT(objp);
	}

	for (auto& viewport : _viewports) {
		viewport->view_orient = Parse_viewer_orient;
		viewport->view_pos = Parse_viewer_pos;
	}
	stars_post_level_init();

	missionLoaded(filepath);

	return true;
}
void Editor::unmark_all() {
	if (numMarked > 0) {
		for (auto i = 0; i < MAX_OBJECTS; i++) {
			Objects[i].flags.remove(Object::Object_Flags::Marked);
			if (Objects[i].type != OBJ_NONE) {
				// Only emit signals for valid objects
				objectMarkingChanged(i, false);
			}
		}

		numMarked = 0;
		setupCurrentObjectIndices(-1);

		missionChanged();
	}
}
void Editor::markObject(int obj) {
	Assert(query_valid_object(obj));
	if (!(Objects[obj].flags[Object::Object_Flags::Marked])) {
		Objects[obj].flags.set(Object::Object_Flags::Marked);  // set as marked
		objectMarkingChanged(obj, true);
		numMarked++;

		if (currentObject == -1) {
			setupCurrentObjectIndices(obj);
		}

		updateAllViewports();
	}
}
void Editor::unmarkObject(int obj) {
	Assert(query_valid_object(obj));
	if (Objects[obj].flags[Object::Object_Flags::Marked]) {
		Objects[obj].flags.remove(Object::Object_Flags::Marked);
		objectMarkingChanged(obj, false);
		numMarked--;
		if (obj == currentObject) {  // need to find a new index
			object* ptr;

			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->flags[Object::Object_Flags::Marked]) {
					setupCurrentObjectIndices(OBJ_INDEX(ptr));  // found one
					return;
				}

				ptr = GET_NEXT(ptr);
			}

			setupCurrentObjectIndices(-1);  // can't find one; nothing is marked.
		}

		missionChanged();
	}
}

void Editor::clearMission() {
	// clean up everything we need to before we reset back to defaults.
#if 0
    if (Briefing_dialog){
        Briefing_dialog->reset_editor();
    }
#endif

	allocate_parse_text(PARSE_TEXT_SIZE);

	The_mission.cutscenes.clear();
	fiction_viewer_reset();
	cmd_brief_reset();
	mission_event_shutdown();

	Asteroid_field.num_initial_asteroids = 0;  // disable asteroid field by default.
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
	model_free_all();                // Free all existing models
	ai_init();
	ship_init();
	jumpnode_level_close();
	waypoint_level_close();

	Num_wings = 0;
	for (auto i = 0; i < MAX_WINGS; i++) {
		Wings[i].wave_count = 0;
		Wings[i].wing_squad_filename[0] = '\0';
		Wings[i].wing_insignia_texture = -1;
	}

	Shield_sys_teams.clear();
	Shield_sys_teams.resize(Iff_info.size(), 0);

	Num_ai_dock_names = 0;
	Num_reinforcements = 0;
	setupCurrentObjectIndices(-1);

	auto userName = getUsername();

	time_t currentTime;
	time(&currentTime);
	auto tm_info = localtime(&currentTime);
	char time_buffer[26];
	strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

	strcpy_s(The_mission.name, "Untitled");
	strcpy_s(The_mission.author, userName.c_str());
	The_mission.author[NAME_LENGTH - 1] = 0;
	strcpy_s(The_mission.created, time_buffer);
	strcpy_s(The_mission.modified, The_mission.created);
	strcpy_s(The_mission.notes, "This is a FRED2_OPEN created mission.\n");
	strcpy_s(The_mission.mission_desc, "Put mission description here\n");
	The_mission.game_type = MISSION_TYPE_SINGLE;
	strcpy_s(The_mission.squad_name, "");
	strcpy_s(The_mission.squad_filename, "");
	The_mission.num_respawns = 3;
	The_mission.max_respawn_delay = -1;

	Player_starts = 0;
	Num_teams = 1;

	// reset alternate name & callsign stuff
	for (auto i = 0; i < MAX_SHIPS; i++) {
		strcpy_s(Fred_alt_names[i], "");
		strcpy_s(Fred_callsigns[i], "");
	}

	// set up the default ship types for all teams.  For now, this is the same class
	// of ships for all teams
	for (auto i = 0; i < MAX_TVT_TEAMS; i++) {
		auto count = 0;
		for (auto j = 0; j < static_cast<int>(Ship_info.size()); j++) {
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
		for (auto j = 0; j < static_cast<int>(Weapon_info.size()); j++) {
			if (Weapon_info[j].wi_flags[Weapon::Info_Flags::Player_allowed]) {
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

	waypoint_parse_init();
	Num_mission_events = 0;
	Num_goals = 0;
	unmark_all();
	obj_init();
	model_free_all();                // Free all existing models

	for (auto& viewport : _viewports) {
		viewport->resetView();
	}

	init_sexp();
	messages_init();
	brief_reset();
	debrief_reset();
	event_music_reset_choices();
	clear_texture_replacements();

	Event_annotations.clear();

	mission_parse_reset_alt();        // alternate ship type names
	mission_parse_reset_callsign();

	strcpy(Cargo_names[0], "Nothing");
	Num_cargo = 1;
	for (auto& viewport : _viewports) {
		viewport->resetViewPhysics();
	}

	// reset background bitmaps and suns
	stars_pre_level_init();
	Nebula_index = 0;
	Mission_palette = 1;
	Nebula_pitch = (int) ((float) (Random::next() & 0x0fff) * 360.0f / 4096.0f);
	Nebula_bank = (int) ((float) (Random::next() & 0x0fff) * 360.0f / 4096.0f);
	Nebula_heading = (int) ((float) (Random::next() & 0x0fff) * 360.0f / 4096.0f);
	Neb2_awacs = -1.0f;
	Neb2_poof_flags = 0;
	strcpy_s(Neb2_texture_name, "");
	for (size_t i = 0; i < MAX_NEB2_POOFS; i++) {
		Neb2_poof_flags |= (1 << i);
	}

	Nmodel_flags = DEFAULT_NMODEL_FLAGS;
	Nmodel_num = -1;
	Nmodel_instance_num = -1;
	vm_set_identity(&Nmodel_orient);
	Nmodel_bitmap = -1;

	The_mission.contrail_threshold = CONTRAIL_THRESHOLD_DEFAULT;

	// Goober5000
	The_mission.command_persona = Default_command_persona;
	strcpy_s(The_mission.command_sender, DEFAULT_COMMAND);

	// Goober5000: reset ALL mission flags, not just nebula!
	The_mission.flags.reset();
	The_mission.support_ships.max_support_ships = -1;    // negative means infinite
	The_mission.support_ships.max_hull_repair_val = 0.0f;
	The_mission.support_ships.max_subsys_repair_val = 100.0f;
	The_mission.ai_profile = &Ai_profiles[Default_ai_profile];

	nebula_init(Nebula_filenames[Nebula_index], Nebula_pitch, Nebula_bank, Nebula_heading);

	strcpy_s(The_mission.loading_screen[GR_640], "");
	strcpy_s(The_mission.loading_screen[GR_1024], "");
	strcpy_s(The_mission.skybox_model, "");
	vm_set_identity(&The_mission.skybox_orientation);
	strcpy_s(The_mission.envmap_name, "");
	The_mission.skybox_flags = DEFAULT_NMODEL_FLAGS;

	// no sound environment
	The_mission.sound_environment.id = -1;

	ENVMAP = -1;

	// free memory from all parsing so far -- see also the stop_parse() in player_select_close() which frees all tbls found during game_init()
	stop_parse();
	// however, FRED expects to parse comments from the raw buffer, so we need a nominal string for that
	allocate_parse_text(1);

	missionLoaded("");
}

void Editor::initialSetup() {
	// Get the default player ship
	Default_player_model = get_default_player_ship_index();

	Id_select_type_jump_node = (int) (Ship_info.size() + 1);
	Id_select_type_waypoint = (int) (Ship_info.size());

	createNewMission();
}

void Editor::setupCurrentObjectIndices(int selectedObj) {
	if (query_valid_object(selectedObj)) {
		currentObject = selectedObj;

		cur_ship = cur_wing = -1;
		cur_waypoint_list = nullptr;
		cur_waypoint = nullptr;

		if ((Objects[selectedObj].type == OBJ_SHIP) || (Objects[selectedObj].type == OBJ_START)) {
			cur_ship = Objects[selectedObj].instance;
			cur_wing = Ships[cur_ship].wingnum;
			if (cur_wing >= 0) {
				for (auto i = 0; i < Wings[cur_wing].wave_count; i++) {
					if (wing_objects[cur_wing][i] == currentObject) {
						cur_wing_index = i;
						break;
					}
				}
			}
		} else if (Objects[selectedObj].type == OBJ_WAYPOINT) {
			cur_waypoint = find_waypoint_with_instance(Objects[selectedObj].instance);
			Assert(cur_waypoint != nullptr);
			cur_waypoint_list = cur_waypoint->get_parent_list();
		}

		currentObjectChanged(currentObject);
		return;
	}

	if (selectedObj == -1 || !Num_objects) {
		currentObject = cur_ship = cur_wing = -1;
		cur_waypoint_list = nullptr;
		cur_waypoint = nullptr;

		currentObjectChanged(currentObject);
		return;
	}

	object* ptr;
	if (query_valid_object(currentObject)) {
		ptr = Objects[currentObject].next;
	} else {
		ptr = GET_FIRST(&obj_used_list);
	}

	if (ptr == END_OF_LIST(&obj_used_list)) {
		ptr = ptr->next;
	}

	Assert(ptr != END_OF_LIST(&obj_used_list));
	currentObject = OBJ_INDEX(ptr);
	Assert(ptr->type != OBJ_NONE);

	cur_ship = cur_wing = -1;
	cur_waypoint_list = nullptr;
	cur_waypoint = nullptr;

	if (ptr->type == OBJ_SHIP) {
		cur_ship = ptr->instance;
		cur_wing = Ships[cur_ship].wingnum;
		for (auto i = 0; i < Wings[cur_wing].wave_count; i++) {
			if (wing_objects[cur_wing][i] == currentObject) {
				cur_wing_index = i;
				break;
			}
		}
	} else if (ptr->type == OBJ_WAYPOINT) {
		cur_waypoint = find_waypoint_with_instance(ptr->instance);
		Assert(cur_waypoint != nullptr);
		cur_waypoint_list = cur_waypoint->get_parent_list();
	}

	currentObjectChanged(currentObject);
}
void Editor::selectObject(int objId) {
	if (objId < 0) {
		unmark_all();
	} else {
		markObject(objId);
	}

	setupCurrentObjectIndices(objId);  // select the new object

	missionChanged();
}
void Editor::updateAllViewports() {
	// This takes all renderers and issues an update request for each of them. For now that is only one but this allows
	// us to expand FRED to multiple view ports in the future.
	for (auto& viewport : _viewports) {
		viewport->needsUpdate();
	}
}

int Editor::create_player(int  /*num*/, vec3d* pos, matrix* orient, int type, int  /*init*/) {
	int obj;

	if (type == -1) {
		type = Default_player_model;
	}

	Assert(type >= 0);
	Assert(Player_starts < MAX_PLAYERS);
	Player_starts++;
	obj = create_ship(orient, pos, type);
	Objects[obj].type = OBJ_START;

	// be sure arrival/departure cues are set
	Ships[Objects[obj].instance].arrival_cue = Locked_sexp_true;
	Ships[Objects[obj].instance].departure_cue = Locked_sexp_false;
	obj_merge_created_list();

	missionChanged();

	return obj;
}

int Editor::create_ship(matrix* orient, vec3d* pos, int ship_type) {
	int obj, z1, z2;
	float temp_max_hull_strength;
	ship_info* sip;

	obj = ship_create(orient, pos, ship_type);
	if (obj == -1) {
		return -1;
	}

	Objects[obj].phys_info.speed = 33.0f;

	ship* shipp = &Ships[Objects[obj].instance];
	sip = &Ship_info[shipp->ship_info_index];

	if (query_ship_name_duplicate(Objects[obj].instance)) {
		fix_ship_name(Objects[obj].instance);
	}

	// default stuff according to species and IFF
	shipp->team = Species_info[Ship_info[shipp->ship_info_index].species].default_iff;
	resolve_parse_flags(&Objects[obj], Iff_info[shipp->team].default_parse_flags);

	// default shield setting
	shipp->special_shield = -1;

	z1 = Shield_sys_teams[shipp->team];
	z2 = Shield_sys_types[ship_type];
	if (((z1 == 1) && z2) || (z2 == 1)) {
		Objects[obj].flags.set(Object::Object_Flags::No_shields);
	}

	// set orders according to whether the ship is on the player ship's team
	{
		object* temp_objp;
		ship* temp_shipp = nullptr;

		// find the first player ship
		for (temp_objp = GET_FIRST(&obj_used_list); temp_objp != END_OF_LIST(&obj_used_list);
			 temp_objp = GET_NEXT(temp_objp)) {
			if (temp_objp->type == OBJ_START) {
				temp_shipp = &Ships[temp_objp->instance];
				break;
			}
		}

		// set orders if teams match, or if player couldn't be found
		if (temp_shipp == nullptr || shipp->team == temp_shipp->team) {
			// if this ship is not a small ship, then make the orders be the default orders without
			// the depart item
			if (!(sip->is_small_ship())) {
				shipp->orders_accepted = ship_get_default_orders_accepted(sip);
				shipp->orders_accepted.erase(DEPART_ITEM);
			}
		} else {
			shipp->orders_accepted.clear();
		}
	}

	// calc kamikaze stuff
	if (shipp->use_special_explosion) {
		temp_max_hull_strength = (float) shipp->special_exp_blast;
	} else {
		temp_max_hull_strength = sip->max_hull_strength;
	}

	Ai_info[shipp->ai_index].kamikaze_damage = (int) std::min(1000.0f, 200.0f + (temp_max_hull_strength / 4.0f));

	missionChanged();

	return obj;
}

bool Editor::query_ship_name_duplicate(int ship) {
	int i;

	for (i = 0; i < MAX_SHIPS; i++) {
		if ((i != ship) && (Ships[i].objnum != -1)) {
			if (!stricmp(Ships[i].ship_name, Ships[ship].ship_name)) {
				return true;
			}
		}
	}

	return false;
}

void Editor::fix_ship_name(int ship) {
	int i = 1;

	do {
		sprintf(Ships[ship].ship_name, "U.R.A. Moron %d", i++);
	} while (query_ship_name_duplicate(ship));
}

void Editor::createNewMission() {
	clearMission();
	create_player(0, &vmd_zero_vector, &vmd_identity_matrix);
}
void Editor::hideMarkedObjects() {
	object* ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags[Object::Object_Flags::Marked]) {
			ptr->flags.set(Object::Object_Flags::Hidden);
			unmarkObject(OBJ_INDEX(ptr));
		}

		ptr = GET_NEXT(ptr);
	}

	updateAllViewports();
}
void Editor::showHiddenObjects() {
	object* ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		ptr->flags.remove(Object::Object_Flags::Hidden);
		ptr = GET_NEXT(ptr);
	}

	updateAllViewports();
}
int Editor::create_waypoint(vec3d* pos, int waypoint_instance) {
	int obj = waypoint_add(pos, waypoint_instance);

	missionChanged();

	return obj;
}
int Editor::getNumMarked() {
	return numMarked;
}
int Editor::dup_object(object* objp) {

	int i, j, n, inst, obj = -1;
	ai_info* aip1, * aip2;
	object* objp1, * objp2;
	ship_subsys* subp1, * subp2;
	static int waypoint_instance(-1);

	if (!objp) {
		waypoint_instance = -1;
		return 0;
	}

	inst = objp->instance;
	if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
		obj = create_ship(&objp->orient, &objp->pos, Ships[inst].ship_info_index);
		if (obj == -1) {
			return -1;
		}

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
		aip1->behavior = aip2->behavior;
		aip1->ai_class = aip2->ai_class;
		for (i = 0; i < MAX_AI_GOALS; i++) {
			aip1->goals[i] = aip2->goals[i];
		}

		if (aip2->ai_flags[AI::AI_Flags::Kamikaze]) {
			aip1->ai_flags.set(AI::AI_Flags::Kamikaze);
		}
		if (aip2->ai_flags[AI::AI_Flags::No_dynamic]) {
			aip2->ai_flags.set(AI::AI_Flags::No_dynamic);
		}

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
			subp1->current_hits = subp2->current_hits;
			subp1 = GET_NEXT(subp1);
			subp2 = GET_NEXT(subp2);
		}

		for (i = 0; i < Num_reinforcements; i++) {
			if (!stricmp(Reinforcements[i].name, Ships[inst].ship_name)) {
				if (Num_reinforcements < MAX_REINFORCEMENTS) {
					j = Num_reinforcements++;
					strcpy_s(Reinforcements[j].name, Ships[n].ship_name);
					Reinforcements[j].type = Reinforcements[i].type;
					Reinforcements[j].uses = Reinforcements[i].uses;
				}

				break;
			}
		}

	} else if (objp->type == OBJ_WAYPOINT) {
		obj = create_waypoint(&objp->pos, waypoint_instance);
		waypoint_instance = Objects[obj].instance;
	}

	if (obj == -1) {
		return -1;
	}

	Objects[obj].pos = objp->pos;
	Objects[obj].orient = objp->orient;
	Objects[obj].flags.set(Object::Object_Flags::Temp_marked);
	return obj;
}

int Editor::common_object_delete(int obj) {
	char msg[255], * name;
	int i, z, r, type;
	object* objp;
	SCP_list<CJumpNode>::iterator jnp;

	type = Objects[obj].type;
	if (type == OBJ_START) {
		i = Objects[obj].instance;
		if (Player_starts < 2) {  // player 1 start
			_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error,
																  "Error",
																  "Must have at least 1 player starting point!",
																  { DialogButton::Ok });

			unmarkObject(obj);
			return 1;
		}

		Assert((i >= 0) && (i < MAX_SHIPS));
		sprintf(msg, "Player %d", i + 1);
		name = msg;
		r = reference_handler(name, REF_TYPE_PLAYER, obj);
		if (r) {
			return r;
		}

		if (Ships[i].wingnum >= 0) {
			r = delete_ship_from_wing(i);
			if (r) {
				return r;
			}
		}

		Objects[obj].type = OBJ_SHIP;  // was allocated as a ship originally, so remove as such.
		invalidate_references(name, REF_TYPE_PLAYER);

		// check if any ship is docked with this ship and break dock if so
		while (object_is_docked(&Objects[obj])) {
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
		waypoint* wpt = find_waypoint_with_instance(Objects[obj].instance);
		Assert(wpt != NULL);
		waypoint_list* wp_list = wpt->get_parent_list();
		int index = calc_waypoint_list_index(Objects[obj].instance);
		int count = (int) wp_list->get_waypoints().size();

		// we'll end up deleting the path, so check for path references
		if (count == 1) {
			name = wp_list->get_name();
			r = reference_handler(name, REF_TYPE_PATH, obj);
			if (r) {
				return r;
			}
		}

		// check for waypoint references
		sprintf(msg, "%s:%d", wp_list->get_name(), index + 1);
		name = msg;
		r = reference_handler(name, REF_TYPE_WAYPOINT, obj);
		if (r) {
			return r;
		}

		// at this point we've confirmed we want to delete it

		invalidate_references(name, REF_TYPE_WAYPOINT);
		if (count == 1) {
			invalidate_references(wp_list->get_name(), REF_TYPE_PATH);
		}

		// the actual removal code has been moved to this function in waypoints.cpp
		waypoint_remove(wpt);

	} else if (type == OBJ_SHIP) {
		name = Ships[Objects[obj].instance].ship_name;
		r = reference_handler(name, REF_TYPE_SHIP, obj);
		if (r) {
			return r;
		}

		z = Objects[obj].instance;
		if (Ships[z].wingnum >= 1) {
			invalidate_references(name, REF_TYPE_SHIP);
			r = delete_ship_from_wing(z);
			if (r) {
				return r;
			}

		} else if (Ships[z].wingnum >= 0) {
			r = delete_ship_from_wing(z);
			if (r) {
				return r;
			}

			invalidate_references(name, REF_TYPE_SHIP);
		}

		for (i = 0; i < Num_reinforcements; i++) {
			if (!stricmp(name, Reinforcements[i].name)) {
				delete_reinforcement(i);
				break;
			}
		}

		// check if any ship is docked with this ship and break dock if so
		while (object_is_docked(&Objects[obj])) {
			ai_do_objects_undocked_stuff(&Objects[obj], dock_get_first_docked_object(&Objects[obj]));
		}

	} else if (type == OBJ_POINT) {
		/*
		 TODO: Implement vriefing dialog

		Assert(Briefing_dialog);
		Briefing_dialog->delete_icon(Objects[obj].instance);
		Update_window = 1;
		 */
		return 0;

	} else if (type == OBJ_JUMP_NODE) {
		for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
			if (jnp->GetSCPObject() == &Objects[obj]) {
				break;
			}
		}

		// come on, WMC, we don't want to call obj_delete twice...
		// fool the destructor into not calling obj_delete yet
		Objects[obj].type = OBJ_NONE;

		// now call the destructor
		Jump_nodes.erase(jnp);

		// now restore the jump node type so that the below unmark and obj_delete will work
		Objects[obj].type = OBJ_JUMP_NODE;
	}

	unmarkObject(obj);

	//we need to call obj_delete() even if obj is a jump node
	//the statement "delete Objects[obj].jnp" deletes the jnp object
	//obj_delete() frees up the object slot where the node used to reside.
	//if we don't call this then the node will still show up in fred and you can try to delete it twice
	//this causes an ugly crash.
	obj_delete(obj);

	missionChanged();
	return 0;
}

int Editor::delete_object(int obj) {
	int r;

	r = common_object_delete(obj);
	return r;
}

void Editor::setActiveViewport(EditorViewport* viewport) {
	_lastActiveViewport = viewport;
}

int Editor::reference_handler(const char* name, int type, int obj) {

	char msg[2048], text[128], type_name[128];
	int r, n, node;

	switch (type) {
	case REF_TYPE_SHIP:
		sprintf(type_name, "Ship \"%s\"", name);
		break;

	case REF_TYPE_WING:
		sprintf(type_name, "Wing \"%s\"", name);
		break;

	case REF_TYPE_PLAYER:
		strcpy_s(type_name, name);
		break;

	case REF_TYPE_WAYPOINT:
		sprintf(type_name, "Waypoint \"%s\"", name);
		break;

	case REF_TYPE_PATH:
		sprintf(type_name, "Waypoint path \"%s\"", name);
		break;

	default:
		Error(LOCATION, "Type unknown for object \"%s\".  Let Hoffos know now!", name);
	}

	r = query_referenced_in_sexp(type, name, &node);
	if (r) {
		n = r & SRC_DATA_MASK;
		switch (r & SRC_MASK) {
		case SRC_SHIP_ARRIVAL:
			sprintf(text, "the arrival cue of ship \"%s\"", Ships[n].ship_name);
			break;

		case SRC_SHIP_DEPARTURE:
			sprintf(text, "the departure cue of ship \"%s\"", Ships[n].ship_name);
			break;

		case SRC_WING_ARRIVAL:
			sprintf(text, "the arrival cue of wing \"%s\"", Wings[n].name);
			break;

		case SRC_WING_DEPARTURE:
			sprintf(text, "the departure cue of wing \"%s\"", Wings[n].name);
			break;

		case SRC_EVENT:
			if (*Mission_events[n].name) {
				sprintf(text, "event \"%s\"", Mission_events[n].name);
			} else {
				sprintf(text, "event #%d", n);
			}

			break;

		case SRC_MISSION_GOAL:
			if (*Mission_goals[n].name) {
				sprintf(text, "mission goal \"%s\"", Mission_goals[n].name);
			} else {
				sprintf(text, "mission goal #%d", n);
			}

			break;

		case SRC_DEBRIEFING:
			sprintf(text, "debriefing #%d", n);
			break;

		case SRC_BRIEFING:
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

		r = sexp_reference_handler(node, r, msg);
		if (r == 1) {
			if (obj >= 0) {
				unmarkObject(obj);
			}

			return 1;
		}

		if (r == 2) {
			delete_flag = 1;
			return 2;
		}
	}

	r = query_referenced_in_ai_goals(type, name);
	if (r) {
		n = r & SRC_DATA_MASK;
		switch (r & SRC_MASK) {
		case SRC_SHIP_ORDER:
			sprintf(text, "ship \"%s\"", Ships[n].ship_name);
			break;

		case SRC_WING_ORDER:
			sprintf(text, "wing \"%s\"", Wings[n].name);
			break;

		default:  // very bad.  Someone added an sexp somewhere and didn't change this.
			Error(LOCATION, "\"%s\" referenced by an unknown initial orders source!  "
				"Run for the hills and let Hoffoss know right now!", name);
		}

		sprintf(msg, "%s is referenced by the initial orders of %s (possibly \n"
			"more initial orders).  Do you want to delete it anyway?\n\n"
			"(click Cancel to go to the reference)", type_name, text);

		r = orders_reference_handler(r, msg);
		if (r == 1) {
			if (obj >= 0) {
				unmarkObject(obj);
			}

			return 1;
		}

		if (r == 2) {
			delete_flag = 1;
			return 2;
		}
	}

	if ((type != REF_TYPE_SHIP) && (type != REF_TYPE_WING)) {
		return 0;
	}

	for (n = 0; n < Num_reinforcements; n++) {
		if (!stricmp(name, Reinforcements[n].name)) {
			break;
		}
	}

	if (n < Num_reinforcements) {
		sprintf(msg, "Ship \"%s\" is a reinforcement unit.\n"
			"Do you want to delete it anyway?", name);

		_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", msg, { DialogButton::Ok });
		r = 0; // TODO: Make this a Yes/No choice once we have a generic dialog interface
		if (r == 0) {
			if (obj >= 0) {
				unmarkObject(obj);
			}

			return 1;
		}
	}

	return 0;
}

int Editor::orders_reference_handler(int  /*code*/, char* msg) {
	auto r = _lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Warning,
																   "Warning",
																   msg,
																   { DialogButton::Yes,
																	 DialogButton::No,
																	 DialogButton::Cancel });
	if (r == DialogButton::No) {
		return 1;
	}

	if (r == DialogButton::Yes) {
		return 0;
	}

	// TODO: add a generic dialog system for showing these dialogs
	/*
	ShipGoalsDlg dlg_goals;

	auto n = code & SRC_DATA_MASK;
	switch (code & SRC_MASK) {
	case SRC_SHIP_ORDER:
		unmark_all();
		mark_object(Ships[n].objnum);

		dlg_goals.self_ship = n;
		dlg_goals.DoModal();
		if (!query_initial_orders_empty(Ai_info[Ships[n].ai_index].goals))
			if ((Ships[n].wingnum >= 0) && (query_initial_orders_conflict(Ships[n].wingnum)))
				Fred_main_wnd->MessageBox("This ship's wing also has initial orders", "Possible conflict");

		break;

	case SRC_WING_ORDER:
		unmark_all();
		mark_wing(n);

		dlg_goals.self_wing = n;
		dlg_goals.DoModal();
		if (query_initial_orders_conflict(n))
			Fred_main_wnd->MessageBox("One or more ships of this wing also has initial orders", "Possible conflict");

		break;

	default:  // very bad.  Someone added an sexp somewhere and didn't change this.
		Error(LOCATION, "Unknown initial order reference source");
	}
	*/

	delete_flag = 1;
	return 2;
}
int Editor::sexp_reference_handler(int  /*node*/, int  /*code*/, const char* msg) {
	auto r = _lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Warning,
																   "Warning",
																   msg,
																   { DialogButton::Yes,
																	 DialogButton::No,
																	 DialogButton::Cancel });
	if (r == DialogButton::No) {
		return 1;
	}

	if (r == DialogButton::Yes) {
		return 0;
	}

	// TODO: add a generic dialog system for showing these dialogs
	/*
	switch (code & SRC_MASK) {
	case SRC_SHIP_ARRIVAL:
	case SRC_SHIP_DEPARTURE:
		if (!Ship_editor_dialog.GetSafeHwnd())
			Ship_editor_dialog.Create();

		Ship_editor_dialog.SetWindowPos(&Fred_main_wnd->wndTop, 0, 0, 0, 0,
										SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		Ship_editor_dialog.ShowWindow(SW_RESTORE);

		Ship_editor_dialog.select_sexp_node = node;
		unmark_all();
		mark_object(Ships[code & SRC_DATA_MASK].objnum);
		break;

	case SRC_WING_ARRIVAL:
	case SRC_WING_DEPARTURE:
		if (!Wing_editor_dialog.GetSafeHwnd())
			Wing_editor_dialog.Create();

		Wing_editor_dialog.SetWindowPos(&Fred_main_wnd->wndTop, 0, 0, 0, 0,
										SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		Wing_editor_dialog.ShowWindow(SW_RESTORE);

		Wing_editor_dialog.select_sexp_node = node;
		unmark_all();
		mark_wing(code & SRC_DATA_MASK);
		break;

	case SRC_EVENT:
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

	case SRC_MISSION_GOAL: {
		CMissionGoalsDlg dlg;

		dlg.select_sexp_node = node;
		dlg.DoModal();
		break;
	}

	case SRC_DEBRIEFING: {
		debriefing_editor_dlg dlg;

		dlg.select_sexp_node = node;
		dlg.DoModal();
		break;
	}

	case SRC_BRIEFING: {
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
	 */

	delete_flag = 1;
	return 2;
}
int Editor::delete_ship_from_wing(int ship) {
	char name[NAME_LENGTH];
	int i, r, wing, end;

	wing = Ships[ship].wingnum;
	if (wing >= 0) {
		if (Wings[wing].wave_count == 1) {
			cur_wing = -1;
			r = delete_wing(wing, 1);
			if (r) {
				if (r == 2) {
					delete_flag = 1;
				}

				return r;
			}

		} else {
			i = Wings[wing].wave_count;
			end = i - 1;
			while (i--) {
				if (wing_objects[wing][i] == Ships[ship].objnum) {
					break;
				}
			}

			Assert(i != -1);  // Error, object should be in wing.
			if (Wings[wing].special_ship == i) {
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

			if (Wings[wing].threshold >= Wings[wing].wave_count) {
				Wings[wing].threshold = Wings[wing].wave_count - 1;
			}

			Wings[wing].wave_count--;
			if (Wings[wing].wave_count && (Wings[wing].threshold >= Wings[wing].wave_count)) {
				Wings[wing].threshold = Wings[wing].wave_count - 1;
			}
		}
	}

	missionChanged();
	return 0;
}
int Editor::invalidate_references(const char* name, int type) {
	char new_name[512];
	int i;

	sprintf(new_name, "<%s>", name);
	update_sexp_references(name, new_name);
	ai_update_goal_references(type, name, new_name);
	update_texture_replacements(name, new_name);
	for (i = 0; i < Num_reinforcements; i++) {
		if (!stricmp(name, Reinforcements[i].name)) {
			strcpy_s(Reinforcements[i].name, new_name);
		}
	}

	return 0;
}
void Editor::ai_update_goal_references(int type, const char* old_name, const char* new_name) {
	int i;

	for (i = 0; i < MAX_AI_INFO; i++) {  // loop through all Ai_info entries
		if (Ai_info[i].shipnum != -1) {  // skip if unused
			::ai_update_goal_references(Ai_info[i].goals, type, old_name, new_name);
		}
	}

	for (i = 0; i < MAX_WINGS; i++) {
		if (Wings[i].wave_count) {
			::ai_update_goal_references(Wings[i].ai_goals, type, old_name, new_name);
		}
	}
}
void Editor::update_texture_replacements(const char* old_name, const char* new_name) {
	for (SCP_vector<texture_replace>::iterator ii = Fred_texture_replacements.begin();
		 ii != Fred_texture_replacements.end(); ++ii) {
		if (!stricmp(ii->ship_name, old_name))
			strcpy_s(ii->ship_name, new_name);
	}
}
int Editor::rename_ship(int ship, char* name) {
	int i;

	Assert(ship >= 0);
	Assert(strlen(name) < NAME_LENGTH);

	update_sexp_references(Ships[ship].ship_name, name);
	ai_update_goal_references(REF_TYPE_SHIP, Ships[ship].ship_name, name);
	update_texture_replacements(Ships[ship].ship_name, name);
	for (i = 0; i < Num_reinforcements; i++) {
		if (!stricmp(Ships[ship].ship_name, Reinforcements[i].name)) {
			strcpy_s(Reinforcements[i].name, name);
		}
	}

	strcpy_s(Ships[ship].ship_name, name);

	missionChanged();

	return 0;
}
void Editor::delete_reinforcement(int num) {
	int i;

	for (i = num; i < Num_reinforcements - 1; i++) {
		Reinforcements[i] = Reinforcements[i + 1];
	}

	Num_reinforcements--;
	missionChanged();
}
int Editor::check_wing_dependencies(int wing_num) {
	const char* name = Wings[wing_num].name;
	return reference_handler(name, REF_TYPE_WING, -1);
}
int Editor::set_reinforcement(const char* name, int state) {
	int i, index, cur = -1;

	for (i = 0; i < Num_reinforcements; i++) {
		if (!stricmp(Reinforcements[i].name, name)) {
			cur = i;
		}
	}

	if (!state && (cur != -1)) {
		Num_reinforcements--;
		Reinforcements[cur] = Reinforcements[Num_reinforcements];

		// clear the ship/wing flag for this reinforcement
		index = ship_name_lookup(name);
		if (index != -1) {
			Ships[index].flags.remove(Ship::Ship_Flags::Reinforcement);
		} else {
			index = wing_name_lookup(name);
			if (index != -1) {
				Wings[index].flags.remove(Ship::Wing_Flags::Reinforcement);
			}
		}
		if (index == -1) {
			Int3();                // get allender -- coudln't find ship/wing for clearing reinforcement flag
		}

		missionChanged();
		return -1;
	}

	if (state && (cur == -1) && (Num_reinforcements < MAX_REINFORCEMENTS)) {
		Assert(strlen(name) < NAME_LENGTH);
		strcpy_s(Reinforcements[Num_reinforcements].name, name);
		Reinforcements[Num_reinforcements].uses = 1;
		Reinforcements[Num_reinforcements].arrival_delay = 0;
		memset(Reinforcements[Num_reinforcements].no_messages, 0, MAX_REINFORCEMENT_MESSAGES * NAME_LENGTH);
		memset(Reinforcements[Num_reinforcements].yes_messages, 0, MAX_REINFORCEMENT_MESSAGES * NAME_LENGTH);
		Num_reinforcements++;

		// set the reinforcement flag on the ship or wing
		index = ship_name_lookup(name);
		if (index != -1) {
			Ships[index].flags.set(Ship::Ship_Flags::Reinforcement);
		} else {
			index = wing_name_lookup(name);
			if (index != -1) {
				Wings[index].flags.set(Ship::Wing_Flags::Reinforcement);
			}
		}
		if (index == -1) {
			Int3();                // get allender -- coudln't find ship/wing for setting reinforcement flag
		}

		missionChanged();
		return 1;
	}

	// this code will take care of setting the bits for the ship/wing flags
	if (state && (cur != -1)) {
		// set the reinforcement flag on the ship or wing
		index = ship_name_lookup(name);
		if (index != -1) {
			Ships[index].flags.set(Ship::Ship_Flags::Reinforcement);
		} else {
			index = wing_name_lookup(name);
			if (index != -1) {
				Wings[index].flags.set(Ship::Wing_Flags::Reinforcement);
			}
		}
		missionChanged();
		if (index == -1) {
			Int3();                // get allender -- coudln't find ship/wing for setting reinforcement flag
		}
	}

	return 0;
}
void Editor::remove_wing(int wing_num) {

	int i, total;
	object* ptr;

	if (check_wing_dependencies(wing_num)) {
		return;
	}

	total = Wings[wing_num].wave_count;
	for (i = 0; i < total; i++) {
		ptr = &Objects[wing_objects[wing_num][i]];
		if (ptr->type == OBJ_SHIP) {
			remove_ship_from_wing(ptr->instance, 0);
		} else if (ptr->type == OBJ_START) {
			remove_player_from_wing(ptr->instance, 0);
		}
	}

	Assert(!Wings[wing_num].wave_count);

	Wings[wing_num].wave_count = 0;
	Wings[wing_num].wing_squad_filename[0] = '\0';
	Wings[wing_num].wing_insignia_texture = -1;

	if (cur_wing == wing_num) {
		set_cur_wing(cur_wing = -1);  // yes, one '=' is correct.
	}

	free_sexp2(Wings[wing_num].arrival_cue);
	free_sexp2(Wings[wing_num].departure_cue);

	Num_wings--;

	update_custom_wing_indexes();

	missionChanged();
}
void Editor::generate_wing_weaponry_usage_list(int* arr, int wing) {
	int i, j;
	ship_weapon* swp;

	if (wing < 0) {
		return;
	}

	i = Wings[wing].wave_count;
	while (i--) {
		swp = &Ships[Wings[wing].ship_index[i]].weapons;
		j = swp->num_primary_banks;
		while (j--) {
			if (swp->primary_bank_weapons[j] >= 0 && swp->primary_bank_weapons[j] < static_cast<int>(Weapon_info.size())) {
				arr[swp->primary_bank_weapons[j]]++;
			}
		}

		j = swp->num_secondary_banks;
		while (j--) {
			if (swp->secondary_bank_weapons[j] >= 0 && swp->secondary_bank_weapons[j] < static_cast<int>(Weapon_info.size())) {
				arr[swp->secondary_bank_weapons[j]] += (int) floor(
					(swp->secondary_bank_ammo[j] * swp->secondary_bank_capacity[j] / 100.0f
						/ Weapon_info[swp->secondary_bank_weapons[j]].cargo_size) + 0.5f);
			}
		}
	}
}
void Editor::generate_team_weaponry_usage_list(int team, int* arr) {
	int i;

	for (i = 0; i < MAX_WEAPON_TYPES; i++) {
		arr[i] = 0;
	}

	if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
		Assert (team >= 0 && team < MAX_TVT_TEAMS);

		for (i = 0; i < MAX_TVT_WINGS_PER_TEAM; i++) {
			generate_wing_weaponry_usage_list(arr, TVT_wings[(team * MAX_TVT_WINGS_PER_TEAM) + i]);
		}
	} else {
		for (i = 0; i < MAX_STARTING_WINGS; i++) {
			generate_wing_weaponry_usage_list(arr, Starting_wings[i]);
		}
	}
}
void Editor::delete_marked() {
	object* ptr, * next;

	delete_flag = 0;
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		next = GET_NEXT(ptr);
		if (ptr->flags[Object::Object_Flags::Marked]) {
			if (delete_object(OBJ_INDEX(ptr)) == 2) {  // user went to a reference, so don't get in the way.
				break;
			}
		}

		ptr = next;
	}

	if (!delete_flag) {
		setupCurrentObjectIndices(-1);
	}

	missionChanged();
}
void Editor::select_next_subsystem() {

	object* objp;

	if (currentObject < 0) {
		cancel_select_subsystem();
	}

	objp = &Objects[currentObject];

	// check if cur object is ship type
	if (objp->type == OBJ_SHIP) {

		// check if same ship
		if (Render_subsys.ship_obj == objp) {

			// if already on, advance to next
			if (Render_subsys.do_render) {
				if (!get_next_visible_subsys(&Ships[objp->instance], &Render_subsys.cur_subsys)) {
					cancel_select_subsystem();
				}
			} else {
				Int3();
			}
		} else {
			// clean up
			cancel_select_subsystem();

			// set up new and advance to first
			Render_subsys.do_render = true;
			Render_subsys.ship_obj = objp;
			if (!get_next_visible_subsys(&Ships[objp->instance], &Render_subsys.cur_subsys)) {
				cancel_select_subsystem();
			}
		}
	} else {
		// not ship type
		cancel_select_subsystem();
	}
}
void Editor::select_previous_subsystem() {
	if (!Render_subsys.do_render) {
		return;
	}

	if ((currentObject < 0) || (Objects[currentObject].type != OBJ_SHIP)
		|| (&Objects[currentObject] != Render_subsys.ship_obj)) {
		cancel_select_subsystem();
		return;
	}

	if (!get_prev_visible_subsys(&Ships[Objects[currentObject].instance], &Render_subsys.cur_subsys)) {
		cancel_select_subsystem();
	}

}
void Editor::cancel_select_subsystem() {
	Render_subsys.do_render = false;
	Render_subsys.ship_obj = NULL;
	Render_subsys.cur_subsys = NULL;
	updateAllViewports();
}
int Editor::get_visible_sub_system_count(ship * shipp) {
	int count = 0;
	ship_subsys* cur_subsys;

	for (cur_subsys = GET_FIRST(&shipp->subsys_list); cur_subsys != END_OF_LIST(&shipp->subsys_list);
		 cur_subsys = GET_NEXT(cur_subsys)) {
		if (cur_subsys->system_info->subobj_num != -1) {
			count++;
		}
	}

	return count;
}
int Editor::get_next_visible_subsys(ship * shipp, ship_subsys * *next_subsys) {
	int count = get_visible_sub_system_count(shipp);

	// return don't try to display
	if (count == 0) {
		return 0;
	}

	// first timer
	if (*next_subsys == NULL) {
		*next_subsys = &shipp->subsys_list;
	}

	// look before wrap
	for (*next_subsys = GET_NEXT(*next_subsys); *next_subsys != END_OF_LIST(&shipp->subsys_list);
		 *next_subsys = GET_NEXT(*next_subsys)) {
		if ((*next_subsys)->system_info->subobj_num != -1) {
			updateAllViewports();
			return 1;
		}
	}

	// look for first after wrap
	for (*next_subsys = GET_FIRST(&shipp->subsys_list); *next_subsys != END_OF_LIST(&shipp->subsys_list);
		 *next_subsys = GET_NEXT(*next_subsys)) {
		if ((*next_subsys)->system_info->subobj_num != -1) {
			updateAllViewports();
			return 1;
		}
	}

	Int3();    // should be impossible to miss
	return 0;
}
int Editor::get_prev_visible_subsys(ship * shipp, ship_subsys * *prev_subsys) {
	int count = get_visible_sub_system_count(shipp);

	// return don't try to display
	if (count == 0) {
		return 0;
	}

	// first timer
	Assert(*prev_subsys != NULL);

	// look before wrap
	for (*prev_subsys = GET_PREV(*prev_subsys); *prev_subsys != END_OF_LIST(&shipp->subsys_list);
		 *prev_subsys = GET_PREV(*prev_subsys)) {
		if ((*prev_subsys)->system_info->subobj_num != -1) {
			updateAllViewports();
			return 1;
		}
	}

	// look for first after wrap
	for (*prev_subsys = GET_LAST(&shipp->subsys_list); *prev_subsys != END_OF_LIST(&shipp->subsys_list);
		 *prev_subsys = GET_PREV(*prev_subsys)) {
		if ((*prev_subsys)->system_info->subobj_num != -1) {
			updateAllViewports();
			return 1;
		}
	}

	Int3();    // should be impossible to miss
	return 0;
}
bool Editor::global_error_check() {
	int z;

	z = global_error_check_impl();
	if (!z) {
		_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Information,
															  "Woohoo!",
															  "No errors were detected in this mission",
															  { DialogButton::Ok });
	}

	for (z = 0; z < obj_count; z++) {
		if (err_flags[z]) {
			delete[] names[z];
		}
	}

	obj_count = 0;

	return !z;
}
int Editor::global_error_check_impl() {

	char buf[256];
	const char* str;
	int bs, i, j, n, s, t, z, ai, count, ship, wing, obj, team, point, multi;
	object* ptr;
	brief_stage* sp;

	g_err = multi = 0;
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		multi = 1;
	}

//	if (!stricmp(The_mission.name, "Untitled"))
//		if (error("You haven't given this mission a title yet.\nThis is done from the Mission Specs Editor (Shift-N)."))
//			return 1;

	// cycle though all the objects and verify every possible aspect of them
	obj_count = t = 0;
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		names[obj_count] = NULL;
		err_flags[obj_count] = 0;
		i = ptr->instance;
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
			if (i < 0 || i >= MAX_SHIPS) {
				return internal_error("An object has an illegal ship index");
			}

			z = Ships[i].ship_info_index;
			if ((z < 0) || (z >= static_cast<int>(Ship_info.size()))) {
				return internal_error("A ship has an illegal class");
			}

			if (ptr->type == OBJ_START) {
				t++;
				if (!(Ship_info[z].flags[Ship::Info_Flags::Player_ship])) {
					ptr->type = OBJ_SHIP;
					Player_starts--;
					t--;
					if (error("Invalid ship type for a player.  Ship has been reset to non-player ship.")) {
						return 1;
					}
				}

				for (n = count = 0; n < MAX_SHIP_PRIMARY_BANKS; n++) {
					if (Ships[i].weapons.primary_bank_weapons[n] >= 0) {
						count++;
					}
				}

				if (!count) {
					if (error("Player \"%s\" has no primary weapons.  Should have at least 1", Ships[i].ship_name)) {
						return 1;
					}
				}

				for (n = count = 0; n < MAX_SHIP_SECONDARY_BANKS; n++) {
					if (Ships[i].weapons.secondary_bank_weapons[n] >= 0) {
						count++;
					}
				}
			}

			if (Ships[i].objnum != OBJ_INDEX(ptr)) {
				return internal_error("Object/ship references are corrupt");
			}

			names[obj_count] = Ships[i].ship_name;
			wing = Ships[i].wingnum;
			if (wing >= 0) {  // ship is part of a wing, so check this
				if (wing < 0 || wing >= MAX_WINGS) {  // completely out of range?
					return internal_error("A ship has an illegal wing index");
				}

				j = Wings[wing].wave_count;
				if (!j) {
					return internal_error("A ship is in a non-existent wing");
				}

				if (j < 0 || j > MAX_SHIPS_PER_WING) {
					return internal_error("Invalid number of ships in wing \"%s\"", Wings[z].name);
				}

				while (j--) {
					if (wing_objects[wing][j] == OBJ_INDEX(ptr)) {  // look for object in wing's table
						break;
					}
				}

				if (j < 0) {
					return internal_error("Ship/wing references are corrupt");
				}

				// wing squad logo check - Goober5000
				if (strlen(Wings[wing].wing_squad_filename) > 0) //-V805
				{
					if (The_mission.game_type & MISSION_TYPE_MULTI) {
						if (error("Wing squad logos are not displayed in multiplayer games.")) {
							return 1;
						}
					} else {
						if (ptr->type == OBJ_START) {
							if (error(
								"A squad logo was assigned to the player's wing.  The player's squad logo will be displayed instead of the wing squad logo on ships in this wing.")) {
								return 1;
							}
						}
					}
				}
			}

			if ((Ships[i].flags[Ship::Ship_Flags::Kill_before_mission]) && (Ships[i].hotkey >= 0)) {
				if (error("Ship flagged as \"destroy before mission start\" has a hotkey assignment")) {
					return 1;
				}
			}

			if ((Ships[i].flags[Ship::Ship_Flags::Kill_before_mission]) && (ptr->type == OBJ_START)) {
				if (error("Player start flagged as \"destroy before mission start\"")) {
					return 1;
				}
			}
		} else if (ptr->type == OBJ_WAYPOINT) {
			int waypoint_num;
			waypoint_list* wp_list = find_waypoint_list_with_instance(i, &waypoint_num);

			if (wp_list == NULL) {
				return internal_error("Object references an illegal waypoint path number");
			}

			if (waypoint_num < 0 || (uint) waypoint_num >= wp_list->get_waypoints().size()) {
				return internal_error("Object references an illegal waypoint number in path");
			}

			sprintf(buf, "%s:%d", wp_list->get_name(), waypoint_num + 1);
			names[obj_count] = new char[strlen(buf) + 1];
			strcpy(names[obj_count], buf);
			err_flags[obj_count] = 1;
		} else if (ptr->type == OBJ_POINT) {
			// TODO: Fix this once the briefing dialog exists
			/*
			if (!Briefing_dialog) {
				return internal_error("Briefing icon detected when not in briefing edit mode");
			}
			 */
			//Shouldn't be needed anymore.
			//If we really do need it, call me and I'll write a is_valid function for jumpnodes -WMC
		} else if (ptr->type == OBJ_JUMP_NODE) {
			//nothing needs to be done here, we just need to make sure the else doesn't occur
		} else {
			return internal_error("An unknown object type (%d) was detected", ptr->type);
		}

		for (i = 0; i < obj_count; i++) {
			if (names[i] && names[obj_count]) {
				if (!stricmp(names[i], names[obj_count])) {
					return internal_error("Duplicate object names (%s)", names[i]);
				}
			}
		}

		obj_count++;
		ptr = GET_NEXT(ptr);
	}

	if (t != Player_starts) {
		return internal_error("Total number of player ships is incorrect");
	}

	if (obj_count != Num_objects) {
		return internal_error("Num_objects is incorrect");
	}

	count = 0;
	for (i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum >= 0) {  // is ship being used?
			count++;
			if (!query_valid_object(Ships[i].objnum)) {
				return internal_error("Ship uses an unused object");
			}

			z = Objects[Ships[i].objnum].type;
			if ((z != OBJ_SHIP) && (z != OBJ_START)) {
				return internal_error("Object should be a ship, but isn't");
			}

			if (fred_check_sexp(Ships[i].arrival_cue, OPR_BOOL, "arrival cue of ship \"%s\"", Ships[i].ship_name)) {
				return -1;
			}

			if (fred_check_sexp(Ships[i].departure_cue, OPR_BOOL, "departure cue of ship \"%s\"", Ships[i].ship_name)) {
				return -1;
			}

			if (Ships[i].arrival_location != ARRIVE_AT_LOCATION) {
				if (Ships[i].arrival_anchor < 0) {
					if (error("Ship \"%s\" requires a valid arrival target", Ships[i].ship_name)) {
						return 1;
					}
				}
			}

			if (Ships[i].departure_location != DEPART_AT_LOCATION) {
				if (Ships[i].departure_anchor < 0) {
					if (error("Ship \"%s\" requires a valid departure target", Ships[i].ship_name)) {
						return 1;
					}
				}
			}

			ai = Ships[i].ai_index;
			if (ai < 0 || ai >= MAX_AI_INFO) {
				return internal_error("AI index out of range for ship \"%s\"", Ships[i].ship_name);
			}

			if (Ai_info[ai].shipnum != i) {
				return internal_error("AI/ship references are corrupt");
			}

			if ((str = error_check_initial_orders(Ai_info[ai].goals, i, -1)) != nullptr) {
				if (*str == '*') {
					return internal_error("Initial orders error for ship \"%s\"\n\n%s", Ships[i].ship_name, str + 1);
				} else if (*str == '!') {
					return 1;
				} else if (error("Initial orders error for ship \"%s\"\n\n%s", Ships[i].ship_name, str)) {
					return 1;
				}
			}


			for (dock_instance* dock_ptr = Objects[Ships[i].objnum].dock_list; dock_ptr != NULL;
				 dock_ptr = dock_ptr->next) {
				obj = OBJ_INDEX(dock_ptr->docked_objp);

				if (!query_valid_object(obj)) {
					return internal_error("Ship \"%s\" initially docked with non-existant ship", Ships[i].ship_name);
				}

				if (Objects[obj].type != OBJ_SHIP && Objects[obj].type != OBJ_START) {
					return internal_error("Ship \"%s\" initially docked with non-ship object", Ships[i].ship_name);
				}

				ship = get_ship_from_obj(obj);
				if (!ship_docking_valid(i, ship) && !ship_docking_valid(ship, i)) {
					return internal_error("Docking illegal between \"%s\" and \"%s\" (initially docked)",
										  Ships[i].ship_name,
										  Ships[ship].ship_name);
				}

				auto dock_list = get_docking_list(Ship_info[Ships[i].ship_info_index].model_num);
				point = dock_ptr->dockpoint_used;
				if (point < 0 || point >= (int)dock_list.size()) {
					internal_error("Invalid docker point (\"%s\" initially docked with \"%s\")",
								   Ships[i].ship_name,
								   Ships[ship].ship_name);
				}

				dock_list = get_docking_list(Ship_info[Ships[ship].ship_info_index].model_num);
				point = dock_find_dockpoint_used_by_object(dock_ptr->docked_objp, &Objects[Ships[i].objnum]);
				if (point < 0 || point >= (int)dock_list.size()) {
					internal_error("Invalid dockee point (\"%s\" initially docked with \"%s\")",
								   Ships[i].ship_name,
								   Ships[ship].ship_name);
				}
			}

			wing = Ships[i].wingnum;
			bool is_in_loadout_screen = (ptr->type == OBJ_START);
			if (!is_in_loadout_screen && wing >= 0) {
				if (multi && The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
					for (n = 0; n < MAX_TVT_WINGS; n++) {
						if (!strcmp(Wings[wing].name, TVT_wing_names[n])) {
							is_in_loadout_screen = true;
							break;
						}
					}
				} else {
					for (n = 0; n < MAX_STARTING_WINGS; n++) {
						if (!strcmp(Wings[wing].name, Starting_wing_names[n])) {
							is_in_loadout_screen = true;
							break;
						}
					}
				}
			}
			if (is_in_loadout_screen) {
				int illegal = 0;
				z = Ships[i].ship_info_index;
				for (n = 0; n < MAX_SHIP_PRIMARY_BANKS; n++) {
					if (Ships[i].weapons.primary_bank_weapons[n] >= 0
						&& !Ship_info[z].allowed_weapons[Ships[i].weapons.primary_bank_weapons[n]]) {
						illegal++;
					}
				}

				for (n = 0; n < MAX_SHIP_SECONDARY_BANKS; n++) {
					if (Ships[i].weapons.secondary_bank_weapons[n] >= 0
						&& !Ship_info[z].allowed_weapons[Ships[i].weapons.secondary_bank_weapons[n]]) {
						illegal++;
					}
				}

				if (illegal && error("%d illegal weapon(s) found on ship \"%s\"", illegal, Ships[i].ship_name)) {
					return 1;
				}
			}
		}
	}

	if (count != ship_get_num_ships()) {
		return internal_error("num_ships is incorrect");
	}

	count = 0;
	for (i = 0; i < MAX_WINGS; i++) {
		team = -1;
		j = Wings[i].wave_count;
		if (j) {  // is wing being used?
			count++;
			if (j < 0 || j > MAX_SHIPS_PER_WING) {
				return internal_error("Invalid number of ships in wing \"%s\"", Wings[i].name);
			}

			while (j--) {
				obj = wing_objects[i][j];
				if (obj < 0 || obj >= MAX_OBJECTS) {
					return internal_error("Wing_objects has an illegal object index");
				}

				if (!query_valid_object(obj)) {
					return internal_error("Wing_objects references an unused object");
				}

// Now, at this point, we can assume several things.  We have a valid object because
// we passed query_valid_object(), and all valid objects were already checked above,
// so this object has valid information, such as the instance.

				if ((Objects[obj].type == OBJ_SHIP) || (Objects[obj].type == OBJ_START)) {
					ship = Objects[obj].instance;
					wing_bash_ship_name(buf, Wings[i].name, j + 1);
					if (stricmp(buf, Ships[ship].ship_name) != 0) {
						return internal_error("Ship \"%s\" in wing should be called \"%s\"",
											  Ships[ship].ship_name,
											  buf);
					}

					int ship_type = ship_query_general_type(ship);
					if (ship_type < 0 || !(Ship_types[ship_type].flags[Ship::Type_Info_Flags::AI_can_form_wing])) {
						if (error("Ship \"%s\" is an illegal type to be in a wing", Ships[ship].ship_name)) {
							return 1;
						}
					}
				} else {
					return internal_error("Wing_objects of \"%s\" references an illegal object type", Wings[i].name);
				}

				if (Ships[ship].wingnum != i) {
					return internal_error("Wing/ship references are corrupt");
				}

				if (ship != Wings[i].ship_index[j]) {
					return internal_error("Ship/wing references are corrupt");
				}

				if (team < 0) {
					team = Ships[ship].team;
				} else if (team != Ships[ship].team && team < 999) {
					if (error("ship teams mixed within same wing (\"%s\")", Wings[i].name)) {
						return 1;
					}
				}
			}

			if ((Wings[i].special_ship < 0) || (Wings[i].special_ship >= Wings[i].wave_count)) {
				return internal_error("Special ship out of range for \"%s\"", Wings[i].name);
			}

			if (Wings[i].num_waves < 0) {
				return internal_error("Number of waves for \"%s\" is negative", Wings[i].name);
			}

			if ((Wings[i].threshold < 0) || (Wings[i].threshold >= Wings[i].wave_count)) {
				return internal_error("Threshold for \"%s\" is invalid", Wings[i].name);
			}

			if (Wings[i].threshold + Wings[i].wave_count > MAX_SHIPS_PER_WING) {
				Wings[i].threshold = MAX_SHIPS_PER_WING - Wings[i].wave_count;
				if (error("Threshold for wing \"%s\" is higher than allowed.  Reset to %d",
						  Wings[i].name,
						  Wings[i].threshold)) {
					return 1;
				}
			}

			for (j = 0; j < obj_count; j++) {
				if (names[j]) {
					if (!stricmp(names[j], Wings[i].name)) {
						return internal_error("Wing name is also used by an object (%s)", names[j]);
					}
				}
			}

			if (fred_check_sexp(Wings[i].arrival_cue, OPR_BOOL, "arrival cue of wing \"%s\"", Wings[i].name)) {
				return -1;
			}

			if (fred_check_sexp(Wings[i].departure_cue, OPR_BOOL, "departure cue of wing \"%s\"", Wings[i].name)) {
				return -1;
			}

			if (Wings[i].arrival_location != ARRIVE_AT_LOCATION) {
				if (Wings[i].arrival_anchor < 0) {
					if (error("Wing \"%s\" requires a valid arrival target", Wings[i].name)) {
						return 1;
					}
				}
			}

			if (Wings[i].departure_location != DEPART_AT_LOCATION) {
				if (Wings[i].departure_anchor < 0) {
					if (error("Wing \"%s\" requires a valid departure target", Wings[i].name)) {
						return 1;
					}
				}
			}

			if ((str = error_check_initial_orders(Wings[i].ai_goals, -1, i)) != nullptr) {
				if (*str == '*') {
					return internal_error("Initial orders error for wing \"%s\"\n\n%s", Wings[i].name, str + 1);
				} else if (*str == '!') {
					return 1;
				} else if (error("Initial orders error for wing \"%s\"\n\n%s", Wings[i].name, str)) {
					return 1;
				}
			}

		}
	}

	if (count != Num_wings) {
		return internal_error("Num_wings is incorrect");
	}

	SCP_list<waypoint_list>::iterator ii;
	for (ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++ii) {
		for (z = 0; z < obj_count; z++) {
			if (names[z]) {
				if (!stricmp(names[z], ii->get_name())) {
					return internal_error("Waypoint path name is also used by an object (%s)", names[z]);
				}
			}
		}

		for (j = 0; (uint) j < ii->get_waypoints().size(); j++) {
			sprintf(buf, "%s:%d", ii->get_name(), j + 1);
			for (z = 0; z < obj_count; z++) {
				if (names[z]) {
					if (!stricmp(names[z], buf)) {
						break;
					}
				}
			}

			if (z == obj_count) {
				return internal_error("Waypoint \"%s\" not linked to an object", buf);
			}
		}
	}

	if (Player_starts > MAX_PLAYERS) {
		return internal_error("Number of player starts exceeds max limit");
	}

	if (!multi && (Player_starts > 1)) {
		if (error("Multiple player starts exist, but this is a single player mission")) {
			return 1;
		}
	}

	if (Num_reinforcements > MAX_REINFORCEMENTS) {
		return internal_error("Number of reinforcements exceeds max limit");
	}

	for (i = 0; i < Num_reinforcements; i++) {
		z = 0;
		for (ship = 0; ship < MAX_SHIPS; ship++) {
			if ((Ships[ship].objnum >= 0) && !stricmp(Ships[ship].ship_name, Reinforcements[i].name)) {
				z = 1;
				break;
			}
		}

		for (wing = 0; wing < MAX_WINGS; wing++) {
			if (Wings[wing].wave_count && !stricmp(Wings[wing].name, Reinforcements[i].name)) {
				z = 1;
				break;
			}
		}

		if (!z) {
			return internal_error("Reinforcement name not found in ships or wings");
		}
	}

/*	for (i=0; i<num_messages; i++) {
		if (Messages[i].num_times < 0)
			return internal_error("Number of times to play message is negative");

		z = Messages[i].who_from;
		if (z < -1 || z >= MAX_SHIPS)  // hacked!  -1 should be illegal..
			return internal_error("Message originator index is out of range");

		if (Ships[z].objnum == -1)
			return internal_error("Message originator points to nonexistant ship");

		if (fred_check_sexp(Messages[i].sexp, OPR_BOOL,
			"Message formula from \"%s\"", Ships[Messages[i].who_from].ship_name))
				return -1;
	}*/

	Assert(
		(Player_start_shipnum >= 0) && (Player_start_shipnum < MAX_SHIPS) && (Ships[Player_start_shipnum].objnum >= 0));
	i = global_error_check_player_wings(multi);
	if (i) {
		return i;
	}

	for (i = 0; i < Num_mission_events; i++) {
		if (fred_check_sexp(Mission_events[i].formula, OPR_NULL, "mission event \"%s\"", Mission_events[i].name)) {
			return -1;
		}
	}

	for (i = 0; i < Num_goals; i++) {
		if (fred_check_sexp(Mission_goals[i].formula, OPR_BOOL, "mission goal \"%s\"", Mission_goals[i].name)) {
			return -1;
		}
	}

	for (bs = 0; bs < Num_teams; bs++) {
		for (s = 0; s < Briefings[bs].num_stages; s++) {
			sp = &Briefings[bs].stages[s];
			t = sp->num_icons;
			for (i = 0; i < t - 1; i++) {
				for (j = i + 1; j < t; j++) {
					if ((sp->icons[i].id > 0) && (sp->icons[i].id == sp->icons[j].id)) {
						if (error("Duplicate icon IDs %d in briefing stage %d", sp->icons[i].id, s + 1)) {
							return 1;
						}
					}
				}
			}
		}
	}

	for (j = 0; j < Num_teams; j++) {
		for (i = 0; i < Debriefings[j].num_stages; i++) {
			if (fred_check_sexp(Debriefings[j].stages[i].formula, OPR_BOOL, "debriefing stage %d", i + 1)) {
				return -1;
			}
		}
	}

	// for all wings, be sure that the orders accepted for all ships are the same for all ships
	// in the wing
	for (i = 0; i < MAX_WINGS; i++) {
		int starting_wing;

		if (!Wings[i].wave_count) {
			continue;
		}

		// determine if this wing is a starting wing of the player
		starting_wing = (ship_starting_wing_lookup(Wings[i].name) != -1);

		// first, be sure this isn't a reinforcement wing.
		if (starting_wing && (Wings[i].flags[Ship::Wing_Flags::Reinforcement])) {
			if (error(
				"Starting Wing %s marked as reinforcement.  This wing\nshould either be renamed, or unmarked as reinforcement.",
				Wings[i].name)) {
// Goober5000				return 1;
			}
		}

		std::set<size_t> default_orders;
		for (j = 0; j < Wings[i].wave_count; j++) {
			const std::set<size_t>& orders = Ships[Wings[i].ship_index[j]].orders_accepted;
			if (j == 0) {
				default_orders = orders;
			} else if (default_orders != orders) {
				if (error(
					"%s and %s will accept different orders. All ships in a wing must accept the same Player Orders.",
					Ships[Wings[i].ship_index[j]].ship_name,
					Ships[Wings[i].ship_index[0]].ship_name)) {
					return 1;
				}
			}
		}

/* Goober5000 - this is not necessary
		// make sure that these ignored orders are the same for all starting wings of the player
		if ( starting_wing ) {
			if ( starting_orders == -1 ) {
				starting_orders = default_orders;
			} else {
				if ( starting_orders != default_orders ) {
					if ( error("Player starting wing %s has orders which don't match other starting wings\n", Wings[i].name) ){
// Goober5000						return 1;
					}
				}
			}
		}
*/
	}

	//This should never ever be a problem -WMC
	/*
	if (Num_jump_nodes < 0){
		return internal_error("Jump node count is illegal");
	}*/

	// FIXME: This call was in the original function but the code of that function was entirely commented out
	//fred_check_message_personas();

	return g_err;
}
int Editor::error(const char* msg, ...) {
	char buf[2048];
	va_list args;

	va_start(args, msg);
	vsnprintf(buf, sizeof(buf) - 1, msg, args);
	va_end(args);
	buf[sizeof(buf) - 1] = '\0';

	g_err = 1;
	if (_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error,
															  "Error",
															  buf,
															  { DialogButton::Ok, DialogButton::Cancel })
		== DialogButton::Ok) {
		return 0;
	}

	return 1;
}
int Editor::internal_error(const char* msg, ...) {
	char buf[2048];
	va_list args;

	va_start(args, msg);
	vsnprintf(buf, sizeof(buf) - 1, msg, args);
	va_end(args);
	buf[sizeof(buf) - 1] = '\0';

	g_err = 1;

#ifndef NDEBUG
	char buf2[2048];

	sprintf_safe(buf2, "%s\n\nThis is an internal error.  Please let Jason\n"
		"know about this so he can fix it.  Click cancel to debug.", buf);

	if (_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error,
															  "Internal Error",
															  buf2,
															  { DialogButton::Ok, DialogButton::Cancel })
		== DialogButton::Cancel)
		Int3();  // drop to debugger so the problem can be analyzed.

#else
	_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", buf, { DialogButton::Ok });
#endif

	return -1;
}
int Editor::fred_check_sexp(int sexp, int type, const char* msg, ...) {
	SCP_string buf, sexp_buf, error_buf;
	int err = 0, z, faulty_node;
	va_list args;

	va_start(args, msg);
	vsprintf(buf, msg, args);
	va_end(args);

	if (sexp == -1)
		return 0;

	z = check_sexp_syntax(sexp, type, 1, &faulty_node);
	if (!z)
		return 0;

	convert_sexp_to_string(sexp_buf, sexp, SEXP_ERROR_CHECK_MODE);
	truncate_message_lines(sexp_buf, 30);
	sprintf(error_buf,
			"Error in %s: %s\n\nIn sexpression: %s\n\n(Bad node appears to be: %s)",
			buf.c_str(),
			sexp_error_message(z),
			sexp_buf.c_str(),
			Sexp_nodes[faulty_node].text);

	if (z < 0 && z > -100)
		err = 1;

	if (err)
		return internal_error("%s", error_buf.c_str());

	if (error("%s", error_buf.c_str()))
		return 1;

	return 0;
}
const char* Editor::error_check_initial_orders(ai_goal* goals, int ship, int wing) {
	char *source;
	int i, j, flag, found, inst, team, team2;
	object *ptr;

	if (ship >= 0) {
		source = Ships[ship].ship_name;
		team = Ships[ship].team;
		for (i=0; i<MAX_AI_GOALS; i++)
			if (!ai_query_goal_valid(ship, goals[i].ai_mode)) {
				if (error("Order \"%s\" isn't allowed for ship \"%s\"", get_order_name(goals[i].ai_mode), source))
					return "!";
			}

	} else {
		Assert(wing >= 0);
		Assert(Wings[wing].wave_count > 0);
		source = Wings[wing].name;
		team = Ships[Objects[wing_objects[wing][0]].instance].team;
		for (j=0; j<Wings[wing].wave_count; j++)
			for (i=0; i<MAX_AI_GOALS; i++)
				if (!ai_query_goal_valid(Wings[wing].ship_index[j], goals[i].ai_mode)) {
					if (error("Order \"%s\" isn't allowed for ship \"%s\"", get_order_name(goals[i].ai_mode),
											 Ships[Wings[wing].ship_index[j]].ship_name))
						return "!";
				}
	}

	for (i=0; i<MAX_AI_GOALS; i++) {
		switch (goals[i].ai_mode) {
		case AI_GOAL_NONE:
		case AI_GOAL_CHASE_ANY:
		case AI_GOAL_CHASE_SHIP_CLASS:
		case AI_GOAL_UNDOCK:
		case AI_GOAL_KEEP_SAFE_DISTANCE:
		case AI_GOAL_PLAY_DEAD:
		case AI_GOAL_PLAY_DEAD_PERSISTENT:
		case AI_GOAL_WARP:
			flag = 0;
			break;

		case AI_GOAL_WAYPOINTS:
		case AI_GOAL_WAYPOINTS_ONCE:
			flag = 1;
			break;

		case AI_GOAL_DOCK:
			if (ship < 0)
				return "Wings can't dock";
			FALLTHROUGH;

		case AI_GOAL_DESTROY_SUBSYSTEM:
		case AI_GOAL_CHASE:
		case AI_GOAL_GUARD:
		case AI_GOAL_DISARM_SHIP:
		case AI_GOAL_DISABLE_SHIP:
		case AI_GOAL_EVADE_SHIP:
		case AI_GOAL_STAY_NEAR_SHIP:
		case AI_GOAL_IGNORE:
		case AI_GOAL_IGNORE_NEW:
			flag = 2;
			break;

		case AI_GOAL_CHASE_WING:
		case AI_GOAL_GUARD_WING:
			flag = 3;
			break;

		case AI_GOAL_STAY_STILL:
			flag = 4;
			break;

		default:
			return "*Invalid goal type";
		}

		found = 0;
		if (flag > 0) {
			if (*goals[i].target_name == '<')
				return "Invalid target";

			if (!stricmp(goals[i].target_name, source)) {
				if (ship >= 0)
					return "Target of ship's goal is itself";
				else
					return "Target of wing's goal is itself";
			}
		}

		inst = team2 = -1;
		if (flag == 1) {  // target waypoint required
			if (find_matching_waypoint_list(goals[i].target_name) == NULL)
				return "*Invalid target waypoint path name";

		} else if (flag == 2) {  // target ship required
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
					inst = ptr->instance;
					if (!stricmp(goals[i].target_name, Ships[inst].ship_name)) {
						found = 1;
						break;
					}
				}

				ptr = GET_NEXT(ptr);
			}

			if (!found)
				return "*Invalid target ship name";

			if (wing >= 0) {  // check if target ship is in wing
				if (Ships[inst].wingnum == wing && Objects[Ships[inst].objnum].type != OBJ_START)
					return "Target ship of wing's goal is within said wing";
			}

			team2 = Ships[inst].team;

		} else if (flag == 3) {  // target wing required
			for (j=0; j<MAX_WINGS; j++)
				if (Wings[j].wave_count && !stricmp(Wings[j].name, goals[i].target_name))
					break;

			if (j >= MAX_WINGS)
				return "*Invalid target wing name";

			if (ship >= 0) {  // check if ship is in target wing
				if (Ships[ship].wingnum == j)
					return "Target wing of ship's goal is same wing said ship is part of";
			}

			team2 = Ships[Objects[wing_objects[j][0]].instance].team;

		} else if (flag == 4) {
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
					inst = ptr->instance;
					if (!stricmp(goals[i].target_name, Ships[inst].ship_name)) {
						found = 2;
						break;
					}

				} else if (ptr->type == OBJ_WAYPOINT) {
					if (!stricmp(goals[i].target_name, object_name(OBJ_INDEX(ptr)))) {
						found = 1;
						break;
					}
				}

				ptr = GET_NEXT(ptr);
			}

			if (!found)
				return "*Invalid target ship or waypoint name";

			if (found == 2) {
				if (wing >= 0) {  // check if target ship is in wing
					if (Ships[inst].wingnum == wing && Objects[Ships[inst].objnum].type != OBJ_START)
						return "Target ship of wing's goal is within said wing";
				}

				team2 = Ships[inst].team;
			}
		}

		switch (goals[i].ai_mode) {
		case AI_GOAL_DESTROY_SUBSYSTEM:
			Assert(flag == 2 && inst >= 0);
			if (ship_get_subsys_index(&Ships[inst], goals[i].docker.name) < 0)
				return "Unknown subsystem type";

			break;

		case AI_GOAL_DOCK: {
			int dock1 = -1, dock2 = -1, model1, model2;

			Assert(flag == 2 && inst >= 0);
			if (!ship_docking_valid(ship, inst))
				return "Docking illegal between given ship types";

			model1 = Ship_info[Ships[ship].ship_info_index].model_num;
			auto model1Docks = get_docking_list(model1);
			for (j = 0; j < (int)model1Docks.size(); ++j) {
				if (!stricmp(goals[i].docker.name, model1Docks[j].c_str())) {
					dock1 = j;
					break;
				}
			}

			model2 = Ship_info[Ships[inst].ship_info_index].model_num;
			auto model2Docks = get_docking_list(model2);
			for (j = 0; j < (int)model2Docks.size(); ++j) {
				if (!stricmp(goals[i].dockee.name, model2Docks[j].c_str())) {
					dock2 = j;
					break;
				}
			}

			if (dock1 < 0)
				return "Invalid docker point";

			if (dock2 < 0)
				return "Invalid dockee point";

			if ((dock1 >= 0) && (dock2 >= 0)) {
				if ( !(model_get_dock_index_type(model1, dock1) & model_get_dock_index_type(model2, dock2)) )
					return "Dock points are incompatible";
			}

			break;
		}
		}

		switch (goals[i].ai_mode) {
		case AI_GOAL_GUARD:
		case AI_GOAL_GUARD_WING:
			if (team != team2) {	//	MK, added support for TEAM_NEUTRAL.  Won't this work?
				if (ship >= 0)
					return "Ship assigned to guard a different team";
				else
					return "Wing assigned to guard a different team";
			}

			break;

		case AI_GOAL_CHASE:
		case AI_GOAL_CHASE_WING:
		case AI_GOAL_DESTROY_SUBSYSTEM:
		case AI_GOAL_DISARM_SHIP:
		case AI_GOAL_DISABLE_SHIP:
			if (team == team2) {
				if (ship >= 0)
					return "Ship assigned to attack same team";
				else
					return "Wings assigned to attack same team";
			}

			break;
		}
	}

	return NULL;
}
const char* Editor::get_order_name(int order) {
	if (order == AI_GOAL_NONE)  // special case
		return "None";

	for (auto& entry : Ai_goal_list)
		if (entry.def & order)
			return entry.name;

	return "???";
}
const ai_goal_list* Editor::getAi_goal_list()
{
	return Ai_goal_list;
}
int Editor::getAigoal_list_size() {
	return sizeof(Ai_goal_list) / sizeof(ai_goal_list);
}
SCP_vector<SCP_string> Editor::get_docking_list(int model_index) {
	int i;
	polymodel *pm;
	SCP_vector<SCP_string> out;

	pm = model_get(model_index);
	out.reserve(pm->n_docks);

	for (i=0; i<pm->n_docks; i++)
		out.push_back(pm->docking_bays[i].name);

	return out;
}
int Editor::global_error_check_player_wings(int multi) {
	int i, z, err;
	int starting_wing_count[MAX_STARTING_WINGS];
	int tvt_wing_count[MAX_TVT_WINGS];

	object *ptr;
	SCP_string starting_wing_list = "";
	SCP_string tvt_wing_list = "";

	// check team wings in tvt
	if ( multi && The_mission.game_type & MISSION_TYPE_MULTI_TEAMS )
	{
		for (i=0; i<MAX_TVT_WINGS; i++)
		{
			if (ship_tvt_wing_lookup(TVT_wing_names[i]) == -1)
			{
				if (error("%s wing is required for multiplayer team vs. team missions", TVT_wing_names[i]))
					return 1;
			}
		}
	}

//	// player's wing must have a true arrival
//	free_sexp2(Wings[z].arrival_cue);
//	Wings[z].arrival_cue = Locked_sexp_true;

	// Check to be sure that any player wing doesn't have > 1 wave for multiplayer
	if ( multi )
	{
		if ( The_mission.game_type & MISSION_TYPE_MULTI_TEAMS )
		{
			for (i=0; i<MAX_TVT_WINGS; i++)
			{
				if (TVT_wings[i] >= 0 && Wings[TVT_wings[i]].num_waves > 1)
				{
					Wings[TVT_wings[i]].num_waves = 1;
					if (error("%s wing must contain only 1 wave.\nThis change has been made for you.", TVT_wing_names[i]))
						return 1;
				}
			}
		}
		else
		{
			for (i=0; i<MAX_STARTING_WINGS; i++)
			{
				if (Starting_wings[i] >= 0 && Wings[Starting_wings[i]].num_waves > 1)
				{
					Wings[Starting_wings[i]].num_waves = 1;
					if (error("%s wing must contain only 1 wave.\nThis change has been made for you.", Starting_wing_names[i]))
						return 1;
				}
			}
		}
	}

	// check number of ships in player wing
	if ( multi && The_mission.game_type & MISSION_TYPE_MULTI_TEAMS )
	{
		for (i=0; i<MAX_TVT_WINGS; i++)
		{
			if (TVT_wings[i] >= 0 && Wings[TVT_wings[i]].wave_count > 4)
			{
				if (error("%s wing has too many ships.  Should only have 4 max.", TVT_wing_names[i]))
					return 1;
			}
		}
	}
	else
	{
		for (i=0; i<MAX_STARTING_WINGS; i++)
		{
			if (Starting_wings[i] >= 0 && Wings[Starting_wings[i]].wave_count > 4)
			{
				if (error("%s wing has too many ships.  Should only have 4 max.", Starting_wing_names[i]))
					return 1;
			}
		}
	}

	// check arrival delay in tvt
	if ( multi && The_mission.game_type & MISSION_TYPE_MULTI_TEAMS )
	{
		for (i=0; i<MAX_TVT_WINGS; i++)
		{
			if (TVT_wings[i] >= 0 && Wings[TVT_wings[i]].arrival_delay > 0)
			{
				if (error("%s wing shouldn't have a non-zero arrival delay", TVT_wing_names[i]))
					return 1;
			}
		}
	}

	// check mixed-species in a wing for multi missions
	if (multi)
	{
		if ( The_mission.game_type & MISSION_TYPE_MULTI_TEAMS )
		{
			for (i=0; i<MAX_TVT_WINGS; i++)
			{
				if (TVT_wings[i] >= 0)
				{
					if (global_error_check_mixed_player_wing(TVT_wings[i]))
						return 1;
				}
			}
		}
		else
		{
			for (i=0; i<MAX_STARTING_WINGS; i++)
			{
				if (Starting_wings[i] >= 0)
				{
					if (global_error_check_mixed_player_wing(Starting_wings[i]))
						return 1;
				}
			}
		}
	}

	for (i=0; i<MAX_STARTING_WINGS; i++)
	{
		starting_wing_count[i] = 0;

		if (i < MAX_STARTING_WINGS - 1)
		{
			starting_wing_list += Starting_wing_names[i];
			if (MAX_STARTING_WINGS > 2)
				starting_wing_list += ",";
			starting_wing_list += " ";
		}
		else
		{
			starting_wing_list += "or ";
			starting_wing_list += Starting_wing_names[i];
		}
	}
	for (i=0; i<MAX_TVT_WINGS; i++)
	{
		tvt_wing_count[i] = 0;

		if (i < MAX_TVT_WINGS - 1)
		{
			tvt_wing_list += TVT_wing_names[i];
			if (MAX_TVT_WINGS > 2)
				tvt_wing_list += ",";
			tvt_wing_list += " ";
		}
		else
		{
			tvt_wing_list += "or ";
			tvt_wing_list += TVT_wing_names[i];
		}
	}

	// check players in wings
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list))
	{
		int ship_instance = ptr->instance;
		err = 0;

		// this ship is a player?
		if (ptr->type == OBJ_START)
		{
			// check if this ship is in a wing
			z = Ships[ship_instance].wingnum;
			if (z < 0)
			{
				err = 1;
			}
			else
			{
				int in_starting_wing = 0;
				int in_tvt_wing = 0;

				// check which wing the player is in
				for (i=0; i<MAX_STARTING_WINGS; i++)
				{
					if (Starting_wings[i] == z)
					{
						in_starting_wing = 1;
						starting_wing_count[i]++;
					}
				}
				for (i=0; i<MAX_TVT_WINGS; i++)
				{
					if (TVT_wings[i] == z)
					{
						in_tvt_wing = 1;
						tvt_wing_count[i]++;
					}
				}

				// make sure the player belongs to his proper wing
				if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS)
				{
					if (!in_tvt_wing)
					{
						err = 1;
					}
				}
				else
				{
					if (!in_starting_wing)
					{
						err = 1;
					}
				}
			}

			if (err)
			{
				if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS)
				{
					if (error("Player %s should be part of %s wing", Ships[ship_instance].ship_name, tvt_wing_list.c_str()))
						return 1;
				}
				else
				{
					if (error("Player %s should be part of %s wing", Ships[ship_instance].ship_name, starting_wing_list.c_str()))
						return 1;
				}
			}
		}

		ptr = GET_NEXT(ptr);
	}

	// check all wings in tvt have players
	if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS)
	{
		for (i=0; i<MAX_TVT_WINGS; i++)
		{
			if (!tvt_wing_count[i])
			{
				if (error("%s wing doesn't contain any players (which it should)", TVT_wing_names[i]))
					return 1;
			}
		}
	}

	return 0;
}
int Editor::global_error_check_mixed_player_wing(int w) {
	int i, s, species = -1, mixed = 0;

	for (i=0; i<Wings[w].wave_count; i++) {
		s = Wings[w].ship_index[i];
		if (species < 0)
			species = Ship_info[Ships[s].ship_info_index].species;
		else if (Ship_info[Ships[s].ship_info_index].species != species)
			mixed = 1;
	}

	if (mixed)
		if (error("%s wing must all be of the same species", Wings[w].name))
			return 1;

	return 0;
}

bool Editor::compareShieldSysData(const std::vector<int>& teams, const std::vector<int>& types) const {
	Assert(Shield_sys_teams.size() == teams.size());
	Assert(Shield_sys_types.size() == types.size());

	return (Shield_sys_teams == teams) && (Shield_sys_types == types);
}

void Editor::exportShieldSysData(std::vector<int>& teams, std::vector<int>& types) const {
	teams = Shield_sys_teams;
	types = Shield_sys_types;
}

void Editor::importShieldSysData(const std::vector<int>& teams, const std::vector<int>& types) {
	Assert(Shield_sys_teams.size() == teams.size());
	Assert(Shield_sys_types.size() == types.size());

	Shield_sys_teams = teams;
	Shield_sys_types = types;

	for (int i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum >= 0) {
			int z = Shield_sys_teams[Ships[i].team];
			if (!Shield_sys_types[Ships[i].ship_info_index])
				z = 0;
			else if (Shield_sys_types[Ships[i].ship_info_index] == 1)
				z = 1;

			if (!z)
				Objects[Ships[i].objnum].flags.remove(Object::Object_Flags::No_shields);
			else if (z == 1)
				Objects[Ships[i].objnum].flags.set(Object::Object_Flags::No_shields);
		}
	}
}

// adapted from shield_sys_dlg OnInitDialog()
// 0 = has shields, 1 = no shields, 2 = conflict/inconsistent
void Editor::normalizeShieldSysData() {
	std::vector<int> teams(Iff_info.size(), 0);
	std::vector<int> types(MAX_SHIP_CLASSES, 0);

	for (int i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum >= 0) {
			int z = (Objects[Ships[i].objnum].flags[Object::Object_Flags::No_shields]) ? 1 : 0;
			if (!teams[Ships[i].team])
				Shield_sys_teams[Ships[i].team] = z;
			else if (Shield_sys_teams[Ships[i].team] != z)
				Shield_sys_teams[Ships[i].team] = 2;

			if (!types[Ships[i].ship_info_index])
				Shield_sys_types[Ships[i].ship_info_index] = z;
			else if (Shield_sys_types[Ships[i].ship_info_index] != z)
				Shield_sys_types[Ships[i].ship_info_index] = 2;

			teams[Ships[i].team]++;
			types[Ships[i].ship_info_index]++;
		}
	}
}

void Editor::strip_quotation_marks(SCP_string& str) {
	SCP_string::size_type idx = 0;
	while ((idx = str.find('\"', idx)) != SCP_string::npos) {
		str.erase(idx, 1);
	}
}

void Editor::pad_with_newline(SCP_string& str, size_t max_size) {
	size_t len = str.size();
	if (!len || (str.back() != '\n' && len < max_size)) {
		str += "\n";
	}
}


} // namespace fred
} // namespace fso
