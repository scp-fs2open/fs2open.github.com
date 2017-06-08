#include "editor.h"

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

#include "globalincs/pstypes.h" // vm_init
#include "osapi/osregistry.h" // os_registry_init
#include "io/timer.h" // timer_init
#include "cfile/cfile.h" // cfile_chdir, cfile_init
#include "localization/localize.h" // lcl_init
#include "graphics/2d.h" // gr_init
#include "graphics/font.h" // gr_font_init
#include "io/key.h" // key_init
#include "io/mouse.h" // mouse_init
#include "iff_defs/iff_defs.h" // iff_init
#include "object/object.h" // obj_init
#include "species_defs/species_defs.h" // species_init
#include "mission/missionbriefcommon.h" // mission_brief_init
#include "ai/ai.h" // ai_init
#include "ai/ai_profiles.h" // ai_profiles_init
#include "ship/ship.h" // armor_init, ship_init
#include "weapon/weapon.h" // weapon_init
#include "stats/medals.h" // parse_medal_tbl
#include "nebula/neb.h" // neb2_init
#include "starfield/starfield.h" // stars_init, stars_pre_level_init, stars_post_level_init
#include "osapi/osapi.h" // os_get_window, os_set_window.
#include "graphics/font.h"
#include "hud/hudsquadmsg.h"
#include "globalincs/linklist.h"

#include "ui/QtGraphicsOperations.h"
#include "ai/aigoals.h"

#include "object.h"
#include <FredApplication.h>

extern int Xstr_inited;

extern int Nmodel_num;
extern int Nmodel_instance_num;
extern matrix Nmodel_orient;
extern int Nmodel_bitmap;

extern void allocate_mission_text(size_t size);

extern void parse_init(bool basic = false);

char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];
char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];

namespace fso {
namespace fred {

void initialize(const std::string& cfilepath, int argc, char* argv[], Editor* editor, InitializerCallback listener) {

#ifndef NDEBUG
	outwnd_init();

	load_filter_info();
#endif

	std::setlocale(LC_ALL, "C");

	std::vector<std::pair<std::function<void(void)>, SubSystem>> initializers =
		{{ std::bind(os_init, Osreg_company_name, Osreg_app_name, nullptr, nullptr), SubSystem::OS },
		 { timer_init,                                                               SubSystem::Timer },
		 { [&]() {
			 // this should enable mods - Kazan
			 parse_cmdline(argc, argv);
		 },                                                                          SubSystem::CommandLine },
		 { [&cfilepath](void) {
			 // cfile needs a file path...
			 auto file_path = cfilepath + DIR_SEPARATOR_STR + "qtfred.exe";

			 cfile_init(file_path.c_str());
		 },                                                                          SubSystem::CFile },
		 { []() {
			 lcl_init(FS2_OPEN_DEFAULT_LANGUAGE);

			 // Goober5000 - force init XSTRs (so they work, but only work in English, based on above comment)
			 Xstr_inited = 1;
		 },                                                                          SubSystem::Locale },
		 { [=]() {
			 std::unique_ptr<QtGraphicsOperations> graphicsOps(new QtGraphicsOperations(editor));
			 gr_init(std::move(graphicsOps));
		 },                                                                          SubSystem::Graphics },
		 { font::init,                                                               SubSystem::Fonts },
		 { key_init,                                                                 SubSystem::Keyboard },
		 { mouse_init,                                                               SubSystem::Mouse },
		 { particle::ParticleManager::init,                                          SubSystem::Particles },
		 { iff_init,                                                                 SubSystem::Iff },
		 { obj_init,                                                                 SubSystem::Objects },
		 { species_init,                                                             SubSystem::Species },
		 { mission_brief_common_init,                                                SubSystem::MissionBrief },
		 { ai_init,                                                                  SubSystem::AI },
		 { ai_profiles_init,                                                         SubSystem::AIProfiles },
		 { armor_init,                                                               SubSystem::Armor },
		 { weapon_init,                                                              SubSystem::Weapon },
		 { parse_medal_tbl,                                                          SubSystem::Medals },
		 { ship_init,                                                                SubSystem::Ships },
		 { []() { parse_init(); },                                                   SubSystem::Parse },
		 { neb2_init,                                                                SubSystem::Nebulas },
		 { []() {
			 stars_init();
			 stars_pre_level_init(true);
			 stars_post_level_init();
		 },                                                                          SubSystem::Stars }};

	for (const auto& initializer : initializers) {
		const auto& init_function = initializer.first;
		const auto& which = initializer.second;
		listener(which);
		init_function();
	}
}

void shutdown() {
	audiostream_close();
	snd_close();
	gr_close();

	os_cleanup();

#ifndef NDEBUG
	outwnd_close();
#endif
}

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
	connect(this, &Editor::missionChanged, this, &Editor::updateAllRenderers);

	// When a mission was loaded we need to notify everyone that the mission has changed
	connect(this, &Editor::missionLoaded, [this](const std::string&) { missionChanged(); });

	fredApp->runAfterInit([this]() { initialSetup(); });
}

FredRenderer* Editor::createRenderer(os::Viewport* renderView) {
	std::unique_ptr<FredRenderer> renderer(new FredRenderer(this, renderView));

	auto ptr = renderer.get();
	_renderers.push_back(std::move(renderer));

	return ptr;
}

void Editor::update() {
	// Do updates for all renderers
	for (auto& renderer : _renderers) {
		renderer->game_do_frame(currentObject);
	}
}

void Editor::loadMission(const std::string& filepath) {
	if (parse_main(filepath.c_str())) {
		throw mission_load_error("Parse error");
	}

	obj_merge_created_list();

	for (auto& renderer : _renderers) {
		renderer->view_orient = Parse_viewer_orient;
		renderer->view_pos = Parse_viewer_pos;
	}
	stars_post_level_init();

	missionLoaded(filepath);
}
void Editor::unmark_all() {
	if (numMarked > 0) {
		for (auto i = 0; i < MAX_OBJECTS; i++) {
			Objects[i].flags.remove(Object::Object_Flags::Marked);
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
		numMarked--;
		if (obj == currentObject) {  // need to find a new index
			object *ptr;

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

		updateAllRenderers();
	}
}

void Editor::resetPhysics() {
	int physics_speed = 100;
	int physics_rot = 20;

	physics_info view_physics;
	physics_init(&view_physics);
	view_physics.max_vel.xyz.x *= physics_speed / 3.0f;
	view_physics.max_vel.xyz.y *= physics_speed / 3.0f;
	view_physics.max_vel.xyz.z *= physics_speed / 3.0f;
	view_physics.max_rear_vel *= physics_speed / 3.0f;
	view_physics.max_rotvel.xyz.x *= physics_rot / 30.0f;
	view_physics.max_rotvel.xyz.y *= physics_rot / 30.0f;
	view_physics.max_rotvel.xyz.z *= physics_rot / 30.0f;
	view_physics.flags |= PF_ACCELERATES | PF_SLIDE_ENABLED;

	for (auto& renderer : _renderers) {
		renderer->view_physics = view_physics;
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

	for (auto& renderer : _renderers) {
		renderer->resetView();
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
	resetPhysics();

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

	createNewMission();
}

void Editor::setupCurrentObjectIndices(int selectedObj) {
	// TODO: Handle other object types

	if (query_valid_object(selectedObj)) {
		currentObject = selectedObj;

		return;
	}

	if (selectedObj == -1 || !Num_objects) {
		currentObject = -1;
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
void Editor::updateAllRenderers() {
	// This takes all renderers and issues an update request for each of them. For now that is only one but this allows
	// us to expand FRED to multiple view ports in the future.
	for (auto& renderer : _renderers) {
		renderer->scheduleUpdate();
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
	if (obj == -1)
		return -1;
	
	Objects[obj].phys_info.speed = 33.0f;

	ship* shipp = &Ships[Objects[obj].instance];
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
		object* temp_objp;
		ship* temp_shipp = NULL;

		// find the first player ship
		for (temp_objp = GET_FIRST(&obj_used_list); temp_objp != END_OF_LIST(&obj_used_list); temp_objp = GET_NEXT(temp_objp)) {
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
		temp_max_hull_strength = (float)shipp->special_exp_blast;
	} else {
		temp_max_hull_strength = sip->max_hull_strength;
	}

	Ai_info[shipp->ai_index].kamikaze_damage = (int)std::min(1000.0f, 200.0f + (temp_max_hull_strength / 4.0f));

	missionChanged();

	return obj;
}

bool Editor::query_ship_name_duplicate(int ship) {
	int i;

	for (i = 0; i < MAX_SHIPS; i++)
		if ((i != ship) && (Ships[i].objnum != -1))
			if (!stricmp(Ships[i].ship_name, Ships[ship].ship_name))
				return true;

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
int Editor::getCurrentObject() {
	return currentObject;
}
void Editor::hideMarkedObjects() {
	object *ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags[Object::Object_Flags::Marked]) {
			ptr->flags.set(Object::Object_Flags::Hidden);
			unmarkObject(OBJ_INDEX(ptr));
		}

		ptr = GET_NEXT(ptr);
	}

	updateAllRenderers();
}
void Editor::showHiddenObjects() {
	object *ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		ptr->flags.remove(Object::Object_Flags::Hidden);
		ptr = GET_NEXT(ptr);
	}

	updateAllRenderers();
}

} // namespace fred
} // namespace fso
