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
#include <ai/aigoals.h>

#include "iff_defs/iff_defs.h" // iff_init
#include "object/object.h" // obj_init
#include "species_defs/species_defs.h" // species_init
#include "ship/ship.h" // armor_init, ship_init
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

}

char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];
char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];

extern void allocate_mission_text(size_t size);

extern int Nmodel_num;
extern int Nmodel_instance_num;
extern matrix Nmodel_orient;
extern int Nmodel_bitmap;

namespace fso {
namespace fred {

Editor::Editor() : currentObject{ -1 } {
	// We need to do this the hard way since MSVC 2013 is too stupid to support array initializers...
	for (auto& a : Shield_sys_teams) {
		a = 0;
	}
	for (auto& a : Shield_sys_types) {
		a = 0;
	}

	connect(fredApp, &FredApplication::onIdle, this, &Editor::update);

	// When the mission changes we need to update all renderers
	connect(this, &Editor::missionChanged, this, &Editor::updateAllViewports);

	// When a mission was loaded we need to notify everyone that the mission has changed
	connect(this, &Editor::missionLoaded, [this](const std::string&) { missionChanged(); });

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

void Editor::loadMission(const std::string& filepath) {
	if (parse_main(filepath.c_str())) {
		throw mission_load_error("Parse error");
	}

	obj_merge_created_list();

	for (auto& viewport : _viewports) {
		viewport->view_orient = Parse_viewer_orient;
		viewport->view_pos = Parse_viewer_pos;
	}
	stars_post_level_init();

	missionLoaded(filepath);
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

		missionChanged();
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

	allocate_mission_text(MISSION_TEXT_SIZE);

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
	ai_profiles_init();
	ship_init();
	jumpnode_level_close();
	waypoint_level_close();

	Num_wings = 0;
	for (auto i = 0; i < MAX_WINGS; i++) {
		Wings[i].wave_count = 0;
		Wings[i].wing_squad_filename[0] = '\0';
		Wings[i].wing_insignia_texture = -1;
	}

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
		for (auto j = 0; j < MAX_WEAPON_TYPES; j++) {
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

	*Mission_text = *Mission_text_raw = EOF_CHAR;
	Mission_text[1] = Mission_text_raw[1] = 0;

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
	ship_init();
	event_music_reset_choices();
	clear_texture_replacements();

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
	Nebula_pitch = (int) ((float) (rand() & 0x0fff) * 360.0f / 4096.0f);
	Nebula_bank = (int) ((float) (rand() & 0x0fff) * 360.0f / 4096.0f);
	Nebula_heading = (int) ((float) (rand() & 0x0fff) * 360.0f / 4096.0f);
	Neb2_awacs = -1.0f;
	Neb2_poof_flags = 0;
	strcpy_s(Neb2_texture_name, "");
	for (auto i = 0; i < MAX_NEB2_POOFS; i++) {
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
		cur_waypoint_list = NULL;
		cur_waypoint = NULL;

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
			Assert(cur_waypoint != NULL);
			cur_waypoint_list = cur_waypoint->get_parent_list();
		}

		currentObjectChanged(currentObject);
		return;
	}

	if (selectedObj == -1 || !Num_objects) {
		currentObject = cur_ship = cur_wing = -1;
		cur_waypoint_list = NULL;
		cur_waypoint = NULL;

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
	cur_waypoint_list = NULL;
	cur_waypoint = NULL;

	if (ptr->type == OBJ_SHIP) {
		cur_ship = ptr->instance;
		cur_wing = Ships[cur_ship].wingnum;
		for (auto i=0; i<Wings[cur_wing].wave_count; i++) {
			if (wing_objects[cur_wing][i] == currentObject) {
				cur_wing_index = i;
				break;
			}
		}
	} else if (ptr->type == OBJ_WAYPOINT) {
		cur_waypoint = find_waypoint_with_instance(ptr->instance);
		Assert(cur_waypoint != NULL);
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

int Editor::create_player(int num, vec3d* pos, matrix* orient, int type, int init) {
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
		ship* temp_shipp = NULL;

		// find the first player ship
		for (temp_objp = GET_FIRST(&obj_used_list); temp_objp != END_OF_LIST(&obj_used_list);
			 temp_objp = GET_NEXT(temp_objp)) {
			if (temp_objp->type == OBJ_START) {
				temp_shipp = &Ships[temp_objp->instance];
				break;
			}
		}

		// set orders if teams match, or if player couldn't be found
		if (temp_shipp == NULL || shipp->team == temp_shipp->team) {
			// if this ship is not a small ship, then make the orders be the default orders without
			// the depart item
			if (!(sip->is_small_ship())) {
				shipp->orders_accepted = ship_get_default_orders_accepted(sip);
				shipp->orders_accepted &= ~DEPART_ITEM;
			}
		} else {
			shipp->orders_accepted = 0;
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
			_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", "Must have at least 1 player starting point!", { DialogButton::Ok });

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

int Editor::orders_reference_handler(int code, char* msg) {
	auto r = _lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Warning, "Warning", msg, { DialogButton::Yes, DialogButton::No, DialogButton::Cancel });
	if (r == DialogButton::No)
		return 1;

	if (r == DialogButton::Yes)
		return 0;

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
int Editor::sexp_reference_handler(int node, int code, const char* msg) {
	auto r = _lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Warning, "Warning", msg, { DialogButton::Yes, DialogButton::No, DialogButton::Cancel });
	if (r == DialogButton::No)
		return 1;

	if (r == DialogButton::Yes)
		return 0;

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
	for (i=0; i<Num_reinforcements; i++)
		if (!stricmp(Ships[ship].ship_name, Reinforcements[i].name)) {
			strcpy_s(Reinforcements[i].name, name);
		}

	strcpy_s(Ships[ship].ship_name, name);

	missionChanged();

	return 0;
}
void Editor::delete_reinforcement(int num) {
	int i;

	for (i=num; i<Num_reinforcements-1; i++)
		Reinforcements[i] = Reinforcements[i + 1];

	Num_reinforcements--;
	missionChanged();
}
int Editor::delete_wing(int wing_num, int bypass) {
	int i, r, total;

	if (already_deleting_wing)
		return 0;

	r = check_wing_dependencies(wing_num);
	if (r)
		return r;

	already_deleting_wing = 1;
	for (i = 0; i<Num_reinforcements; i++)
		if (!stricmp(Wings[wing_num].name, Reinforcements[i].name)) {
			delete_reinforcement(i);
			break;
		}

	invalidate_references(Wings[wing_num].name, REF_TYPE_WING);
	if (!bypass) {
		total = Wings[wing_num].wave_count;
		for (i = 0; i<total; i++)
			delete_object(wing_objects[wing_num][i]);
	}

	Wings[wing_num].wave_count = 0;
	Wings[wing_num].wing_squad_filename[0] = '\0';
	Wings[wing_num].wing_insignia_texture = -1;

	if (cur_wing == wing_num)
		set_cur_wing(-1);

	free_sexp2(Wings[wing_num].arrival_cue);
	free_sexp2(Wings[wing_num].departure_cue);

	Num_wings--;
	missionChanged();

	update_custom_wing_indexes();

	already_deleting_wing = 0;
	return 0;
}
void Editor::set_cur_wing(int wing) {
	cur_wing = wing;
/*	if (cur_ship != -1)
		Assert(cur_wing == Ships[cur_ship].wingnum);
	if ((cur_object_index != -1) && (Objects[cur_object_index].type == OBJ_SHIP))
		Assert(cur_wing == Ships[Objects[cur_object_index].instance].wingnum);*/
	updateAllViewports();
	// TODO: Add notification for a changed selection
}
int Editor::check_wing_dependencies(int wing_num) {
	const char* name = Wings[wing_num].name;
	return reference_handler(name, REF_TYPE_WING, -1);
}
void Editor::update_custom_wing_indexes() {
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

} // namespace fred
} // namespace fso
