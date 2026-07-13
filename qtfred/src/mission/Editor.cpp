#include "Editor.h"

#include <array>
#include <vector>
#include <stdexcept>
#include <clocale>

#include <SDL.h>
#include <ai/ai.h>
#include <parse/parselo.h>
#include <mission/missiongoals.h>
#include <mission/missionparse.h>
#include <asteroid/asteroid.h>
#include <jumpnode/jumpnode.h>
#include <prop/prop.h>
#include <util.h>
#include <mission/missionmessage.h>
#include <missioneditor/common.h>
#include <missioneditor/sexp_annotation_model.h>
#include <missioneditor/missionsave.h>
#include <missioneditor/objectduplication.h>
#include <gamesnd/eventmusic.h>
#include <starfield/nebula.h>
#include <object/objectdock.h>
#include <localization/fhash.h>
#include <scripting/global_hooks.h>

#include "iff_defs/iff_defs.h" // iff_init
#include "object/object.h" // obj_init
#include "species_defs/species_defs.h" // species_init
#include "weapon/weapon.h" // weapon_init
#include "nebula/neb.h" // neb2_init
#include "starfield/starfield.h" // stars_init, stars_pre_level_init, stars_post_level_init
#include "hud/hudsquadmsg.h"
#include "globalincs/linklist.h"
#include "globalincs/utility.h"

#include "ui/QtGraphicsOperations.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include "object.h"
#include "management.h"
#include "util.h"
#include "FredApplication.h"

namespace {

std::pair<int, sexp_src> query_referenced_in_ai_goals(sexp_ref_type type, const char* name) {
	int i;

	for (i = 0; i < MAX_AI_INFO; i++) {  // loop through all Ai_info entries
		if (Ai_info[i].shipnum >= 0) {  // skip if unused
			if (query_referenced_in_ai_goals(Ai_info[i].goals, type, name)) {
				return std::make_pair(Ai_info[i].shipnum, sexp_src::SHIP_ORDER);
			}
		}
	}

	for (i = 0; i < MAX_WINGS; i++) {
		if (Wings[i].wave_count) {
			if (query_referenced_in_ai_goals(Wings[i].ai_goals, type, name)) {
				return std::make_pair(i, sexp_src::WING_ORDER);
			}
		}
	}

	return std::make_pair(-1, sexp_src::NONE);
}

// Used in the FRED drop-down menu and in ErrorChecker::checkInitialOrders
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

}

char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];
char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];

extern void allocate_parse_text(size_t size);

namespace fso {
namespace fred {
	
Editor::Editor() : currentObject{ -1 }, Shield_sys_teams(Iff_info.size(), GlobalShieldStatus::HasShields), Shield_sys_types(MAX_SHIP_CLASSES, GlobalShieldStatus::HasShields) {
	connect(fredApp, &FredApplication::onIdle, this, &Editor::update);

	// When the mission changes we need to update all renderers
	connect(this, &Editor::missionChanged, this, &Editor::updateAllViewports);

	// When a mission was loaded we need to notify everyone that the mission has changed
	connect(this, &Editor::missionLoaded, this, [this](const std::string&) { missionChanged(); });

	_autosaveDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/autosave/";
	QDir().mkpath(_autosaveDirectory);

	_autosaveTimer = new QTimer(this);
	_autosaveTimer->setSingleShot(false);
	connect(_autosaveTimer, &QTimer::timeout, this, &Editor::performTimedAutosave);

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

void Editor::maybeUseAutosave(std::string& filepath)
{
	const QString qpath       = QString::fromStdString(filepath);
	const QString basename    = QFileInfo(qpath).fileName();
	const QString autosavePath = _autosaveDirectory + basename;

	const QFileInfo autosaveInfo(autosavePath);
	if (!autosaveInfo.exists())
		return;

	const QFileInfo originalInfo(qpath);
	if (autosaveInfo.lastModified() <= originalInfo.lastModified())
		return;

	if (_lastActiveViewport == nullptr || _lastActiveViewport->dialogProvider == nullptr)
		return;

	const auto result = _lastActiveViewport->dialogProvider->showButtonDialog(
		DialogType::Question,
		"Autosave Recovery",
		"An autosave file for this mission is newer than the original. Load the autosave instead?",
		{ DialogButton::Yes, DialogButton::No });

	if (result == DialogButton::Yes) {
		filepath = autosavePath.toStdString();
	}
}

void Editor::startAutosaveTimer(int intervalSeconds) {
	_autosaveTimer->stop();
	if (intervalSeconds > 0)
		_autosaveTimer->start(intervalSeconds * 1000);
}

void Editor::stopAutosaveTimer() {
	_autosaveTimer->stop();
}

void Editor::setCurrentMissionPath(const QString& path) {
	_currentMissionPath = path;
}

void Editor::performTimedAutosave() {
	QString savePath;
	if (_currentMissionPath.isEmpty()) {
		savePath = _autosaveDirectory + "untitled_autosave.fs2";
	} else {
		savePath = _autosaveDirectory + QFileInfo(_currentMissionPath).fileName();
	}
	autosaveDue(savePath);
}

bool Editor::loadMission(const std::string& mission_name, int flags) {
	char name[512], * old_name;
	int i, j, ob;
	object* objp;

	// activate the localizer hash table
	fhash_flush();

	clearMission(flags & MPF_FAST_RELOAD);

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

	// message 1: required version
	if (!parse_main(filepath.c_str(), flags)) {
		auto term = (flags & MPF_IMPORT_FSM) ? "import" : "load";
		SCP_string msg;

		// the version will have been assigned before loading was aborted
		if (!gameversion::check_at_least(The_mission.required_fso_version)) {
			msg += "The file \"";
			msg += filepath;
			msg += "\" cannot be ";
			msg += term;
			msg += "ed because it requires FSO version ";
			msg += format_version(The_mission.required_fso_version, true);
			msg += ".";
		}
		else {
			msg += "Unable to ";
			msg += term;
			msg += " the file \"";
			msg += filepath;
			msg += "\".";
		}

		_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error,
																"Load Error",
																msg,
																{ DialogButton::Ok });
		createNewMission();

		return false;
	}

	// message 2: unknown classes
	if ((Num_unknown_ship_classes > 0) || (Num_unknown_prop_classes > 0) || (Num_unknown_weapon_classes > 0) || (Num_unknown_loadout_classes > 0)) {
		if (flags & MPF_IMPORT_FSM) {
			SCP_string msg;
			sprintf(msg,
					"Fred encountered unknown ship/prop/weapon classes when importing \"%s\" (path \"%s\"). You will have to manually edit the converted mission to correct this.",
					The_mission.name.c_str(),
					filepath.c_str());

			_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Warning,
																  "Unknown object classes",
																  msg,
																  { DialogButton::Ok });
		} else {
			_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Warning,
																  "Unknown object classes",
																  "Fred encountered unknown ship/prop/weapon classes when parsing the mission file. This may be due to mission disk data you do not have.",
																  { DialogButton::Ok });
		}
	}

	// message 3: warning about saving under a new version
	if (!(flags & MPF_IMPORT_FSM) && (The_mission.required_fso_version != LEGACY_MISSION_VERSION) && (MISSION_VERSION > The_mission.required_fso_version)) {
		SCP_string msg = "This mission's file format is ";
		msg += format_version(The_mission.required_fso_version, true);
		msg += ".  When you save this mission, the file format will be migrated to ";
		msg += format_version(MISSION_VERSION, true);
		msg += ".  FSO versions earlier than this will not be able to load the mission.";

		_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Information,
															  "Mission File Format",
															  msg,
															  { DialogButton::Ok });
	}

	// message 4: check for "immobile" flag migration
	if (!Fred_migrated_immobile_ships.empty()) {
		SCP_string msg = "The \"immobile\" ship flag has been superseded by the \"don't-change-position\", and \"don't-change-orientation\" flags.  "
			"All ships which previously had \"Does Not Move\" checked in the ship flags editor will now have both \"Does Not Change Position\" and "
			"\"Does Not Change Orientation\" checked.\n\nWould you like to open the error checker now to review any potential issues "
			"involving these flags?\n\nThe following ships have been migrated:";

		for (int shipnum : Fred_migrated_immobile_ships) {
			msg += "\n\t";
			msg += Ships[shipnum].ship_name;
		}

		truncate_message_lines(msg, 30);
		auto z = _lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Question,
														"Ship Flag Migration",
														msg,
														{ DialogButton::Yes, DialogButton::No });
		if (z == DialogButton::Yes) {
			// Consumed by FredView::autoRunErrorChecker after loadMission returns.
			_lastActiveViewport->Error_checker_force_display_potentials_once = true;
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
			if ((Objects[wing_objects[i][j]].type == OBJ_SHIP) || (Objects[wing_objects[i][j]].type == OBJ_START)) {  // don't change player ship names
				wing_bash_ship_name(name, Wings[i].name, j + 1);
				old_name = Ships[Wings[i].ship_index[j]].ship_name;
				if (stricmp(name, old_name) != 0) {  // need to fix name
					update_sexp_references(old_name, name);
					ai_update_goal_references(sexp_ref_type::SHIP, old_name, name);
					update_texture_replacements(old_name, name);
					int k = find_item_with_string(Reinforcements, &reinforcements::name, old_name);
					if (k >= 0) {
						Assert(strlen(name) < NAME_LENGTH);
						strcpy_s(Reinforcements[k].name, name);
					}

					// bash it again so that we handle display names if needed
					wing_bash_ship_name(&Ships[Wings[i].ship_index[j]], &Wings[i], j + 1, true);
				}
			}
		}
	} 

	for (i = 0; i < Num_teams; i++) {
		generate_team_weaponry_usage_list(i, _weapon_usage[i]);
		for (j = 0; j < Team_data[i].num_weapon_choices; j++) {
			// The amount used in wings is always set by a static loadout entry so skip any that were set by Sexp variables
			if ((!strlen(Team_data[i].weaponry_pool_variable[j]))
				&& (!strlen(Team_data[i].weaponry_amount_variable[j]))) {
				// convert weaponry_pool to be extras available beyond the current ships weapons
				Team_data[i].weaponry_count[j] -= _weapon_usage[i][Team_data[i].weaponry_pool[j]];
				if (Team_data[i].weaponry_count[j] < 0) {
					Team_data[i].weaponry_count[j] = 0;
				}

				// zero the used pool entry
				_weapon_usage[i][Team_data[i].weaponry_pool[j]] = 0;
			}
		}
		// Weapons used in wings but missing from the loadout pool are flagged by the error checker.
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
		viewport->camera.view_orient = Parse_viewer_orient;
		viewport->camera.view_pos = Parse_viewer_pos;
	}

	if (flags & MPF_IS_TEMPLATE) {
		// reset fields that should not carry over from the template source
		The_mission.name = "Untitled";
		The_mission.author = getUsername();

		time_t currentTime;
		time(&currentTime);
		auto timeinfo = localtime(&currentTime);
		time_to_mission_info_string(timeinfo, The_mission.created, DATE_TIME_LENGTH - 1);
		strcpy_s(The_mission.modified, The_mission.created);

		strcpy_s(The_mission.notes, "This is a FRED2_OPEN created mission.");
		strcpy_s(The_mission.mission_desc, "Put mission description here");

		for (auto& viewport : _viewports) {
			viewport->reset();
		}
	}

	stars_post_level_init();

	missionLoaded(filepath);

	// This hook will allow for scripts to know when a mission has been loaded
	// which will then allow them to update any LuaEnums that may be related to sexps
	if (scripting::hooks::FredOnMissionLoad->isActive()) {
		scripting::hooks::FredOnMissionLoad->run();
	}

	if (!(flags & MPF_FAST_RELOAD)) {
		// TODO(Phase 3): _undoStack->clear()
	}

	return true;
}
void Editor::clean_up_selections() {
	unmark_all();
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

		updateAllViewports();
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

		updateAllViewports();
	}
}

void Editor::clearMission(bool fast_reload) {
	// clean up everything we need to before we reset back to defaults.
	clean_up_selections();

	allocate_parse_text(PARSE_TEXT_SIZE);

	mission_init(&The_mission);

	obj_init();

	if (!fast_reload)
		model_free_all();                // Free all existing models

	ai_init();
	props_level_init();
	asteroid_level_init();
	ship_level_init();
	nebula_init(Nebula_index, Nebula_pitch, Nebula_bank, Nebula_heading);

	Shield_sys_teams.clear();
	Shield_sys_teams.resize(Iff_info.size(), GlobalShieldStatus::HasShields);

	for (int i = 0; i < MAX_SHIP_CLASSES; i++) {
		Shield_sys_types[i] = GlobalShieldStatus::HasShields;
	}

	setupCurrentObjectIndices(-1);

	auto userName = getUsername();

	time_t currentTime;
	time(&currentTime);
	auto timeinfo = localtime(&currentTime);

	The_mission.name = "Untitled";
	The_mission.author = userName;
	time_to_mission_info_string(timeinfo, The_mission.created, DATE_TIME_LENGTH - 1);
	strcpy_s(The_mission.modified, The_mission.created);
	strcpy_s(The_mission.notes, "This is a FRED2_OPEN created mission.");
	strcpy_s(The_mission.mission_desc, "Put mission description here");
	apply_default_custom_data(&The_mission);

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

	for (auto& viewport : _viewports) {
		viewport->reset();
	}

	Event_annotations.clear();
	Fred_migrated_immobile_ships.clear();

	// free memory from all parsing so far -- see also the stop_parse() in player_select_close() which frees all tbls found during game_init()
	stop_parse();
	// however, FRED expects to parse comments from the raw buffer, so we need a nominal string for that
	allocate_parse_text(1);

}

void Editor::initialSetup() {
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
}
void Editor::updateAllViewports() {
	// This takes all renderers and issues an update request for each of them. For now that is only one but this allows
	// us to expand FRED to multiple view ports in the future.
	for (auto& viewport : _viewports) {
		viewport->needsUpdate();
	}
}

int Editor::create_player(vec3d* pos, matrix* orient, int type) {
	int obj;

	if (type == -1) {
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

	missionChanged();

	return obj;
}

int Editor::create_ship(matrix* orient, vec3d* pos, int ship_type) {
	int obj;
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

	auto z1 = Shield_sys_teams[shipp->team];
	auto z2 = Shield_sys_types[ship_type];
	if (((z1 == GlobalShieldStatus::NoShields) && z2 != GlobalShieldStatus::HasShields) || (z2 == GlobalShieldStatus::NoShields)) {
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
	create_player(&vmd_zero_vector, &vmd_identity_matrix);
	stars_post_level_init();
	// TODO(Phase 3): _undoStack->clear()
	missionLoaded("");
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
void Editor::lockMarkedObjects() {
	object* ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags[Object::Object_Flags::Marked]) {
			ptr->flags.set(Object::Object_Flags::Locked_from_editing);
			unmarkObject(OBJ_INDEX(ptr));
		}

		ptr = GET_NEXT(ptr);
	}

	updateAllViewports();
}
void Editor::unlockAllObjects() {
	object* ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		ptr->flags.remove(Object::Object_Flags::Locked_from_editing);
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

	int inst, obj = -1;
	static int waypoint_instance(-1);

	if (!objp) {
		waypoint_instance = -1;
		return 0;
	}

	inst = objp->instance;
	if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
		bool clone_as_player_start = false;
		bool player_start_demoted = false;
		if (objp->type == OBJ_START && (The_mission.game_type & MISSION_TYPE_MULTI)) {
			if (Player_starts < MAX_PLAYERS) {
				clone_as_player_start = true;
			} else {
				player_start_demoted = true;
			}
		}

		if (clone_as_player_start) {
			obj = create_player(&objp->pos, &objp->orient, Ships[inst].ship_info_index);
		} else {
			obj = create_ship(&objp->orient, &objp->pos, Ships[inst].ship_info_index);
		}
		if (obj == -1) {
			return -1;
		}

		int n = Objects[obj].instance;

		// Copy every editable per-ship field.  See clone_ship_instance_data() in
		// code/missioneditor/objectduplication.cpp for the canonical field list.
		clone_ship_instance_data(inst, n);

		// Reinforcement-list propagation: if the source ship is listed as a
		// reinforcement, add a new entry pointing at the duplicate.
		int i = find_item_with_string(Reinforcements, &reinforcements::name, Ships[inst].ship_name);
		if (i >= 0) {
			Reinforcements.push_back(Reinforcements[i]);
			strcpy_s(Reinforcements.back().name, Ships[n].ship_name);
		}

		if (player_start_demoted && _lastActiveViewport) {
			SCP_string message;
			sprintf(message,
				"Cannot create another player start. This mission already has the maximum of %d. "
				"The duplicate has been created as a regular ship instead.",
				MAX_PLAYERS);
			_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Information,
				"Player Starts",
				message,
				{ DialogButton::Ok });
		}

	} else if (objp->type == OBJ_WAYPOINT) {
		// waypoint_instance < 0 means create_waypoint() will create a fresh list;
		// copy editable per-list properties from the source list in that case.
		bool is_first_of_new_list = (waypoint_instance < 0);
		int src_list_idx = calc_waypoint_list_index(objp->instance);
		obj = create_waypoint(&objp->pos, waypoint_instance);
		if (obj == -1) {
			return -1;
		}
		waypoint_instance = Objects[obj].instance;
		if (is_first_of_new_list) {
			clone_waypoint_path_instance_data(src_list_idx, calc_waypoint_list_index(waypoint_instance));
		}
	} else if (objp->type == OBJ_JUMP_NODE) {
		CJumpNode* src_jnp = jumpnode_get_by_objp(objp);
		CJumpNode jnp(&objp->pos);
		if (src_jnp) {
			clone_jump_node_instance_data(*src_jnp, jnp);
		}
		obj = jnp.GetSCPObjectNumber();
		Jump_nodes.push_back(std::move(jnp));
	} else if (objp->type == OBJ_PROP) {
		prop* propp = prop_id_lookup(inst);
		if (propp != nullptr) {
			obj = prop_create(&objp->orient, &objp->pos, propp->prop_info_index);
			if (obj != -1) {
				clone_prop_instance_data(inst, Objects[obj].instance);
			}
		}
	}

	if (obj == -1) {
		return -1;
	}

	Objects[obj].pos = objp->pos;
	Objects[obj].orient = objp->orient;
	Objects[obj].flags.set(Object::Object_Flags::Temp_marked);

	// Sync the viewport's layer-membership map for the new object so it shows
	// up in the same layer as the source (per-object fred_layer was already
	// copied by the clone_*_instance_data helpers).
	if (_lastActiveViewport) {
		_lastActiveViewport->registerObjectInLayer(obj);
	}

	return obj;
}

int Editor::common_object_delete(int obj) {
	char msg[255];
	const char *name;
	int i, z, r, type;
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

		Assertion((i >= 0) && (i < MAX_SHIPS), "Invalid ship index %d in player-start delete path", i); // NOLINT(readability-simplify-boolean-expr)
		sprintf(msg, "Player %d", i + 1);
		name = msg;
		r = reference_handler(name, sexp_ref_type::PLAYER, obj);
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
		invalidate_references(name, sexp_ref_type::PLAYER);

		// check if any ship is docked with this ship and break dock if so
		while (object_is_docked(&Objects[obj])) {
			ai_do_objects_undocked_stuff(&Objects[obj], dock_get_first_docked_object(&Objects[obj]));
		}

		ensure_valid_player_start_shipnum();  // may need a new single player start

		Player_starts--;

	} else if (type == OBJ_WAYPOINT) {
		waypoint* wpt = find_waypoint_with_instance(Objects[obj].instance);
		Assert(wpt != NULL);
		waypoint_list* wp_list = wpt->get_parent_list();
		auto count = wp_list->get_waypoints().size();

		// we'll end up deleting the path, so check for path references
		if (count == 1) {
			name = wp_list->get_name();
			r = reference_handler(name, sexp_ref_type::WAYPOINT_PATH, obj);
			if (r) {
				return r;
			}
		}

		// check for waypoint references
		waypoint_stuff_name(msg, *wpt);
		name = msg;
		r = reference_handler(name, sexp_ref_type::WAYPOINT, obj);
		if (r) {
			return r;
		}

		// at this point we've confirmed we want to delete it

		invalidate_references(name, sexp_ref_type::WAYPOINT);
		if (count == 1) {
			invalidate_references(wp_list->get_name(), sexp_ref_type::WAYPOINT_PATH);
		}

		// save info needed to update shifted waypoint references after removal
		int deleted_index = wpt->get_index();
		char list_name[NAME_LENGTH];
		strcpy_s(list_name, wp_list->get_name());

		// the actual removal code has been moved to this function in waypoints.cpp
		waypoint_remove(wpt);

		// update SEXP and AI goal references for waypoints that shifted down
		for (int wi = deleted_index; wi < (int)count - 1; wi++) {
			char old_wpt_name[NAME_LENGTH];
			char new_wpt_name[NAME_LENGTH];
			waypoint_stuff_name(old_wpt_name, list_name, wi + 2);	// old 1-based number
			waypoint_stuff_name(new_wpt_name, list_name, wi + 1);	// new 1-based number
			update_sexp_references(old_wpt_name, new_wpt_name);
			ai_update_goal_references(sexp_ref_type::WAYPOINT, old_wpt_name, new_wpt_name);
		}

	} else if (type == OBJ_SHIP) {
		name = Ships[Objects[obj].instance].ship_name;
		r = reference_handler(name, sexp_ref_type::SHIP, obj);
		if (r) {
			return r;
		}

		z = Objects[obj].instance;
		if (Ships[z].wingnum >= 1) {
			invalidate_references(name, sexp_ref_type::SHIP);
			r = delete_ship_from_wing(z);
			if (r) {
				return r;
			}

		} else if (Ships[z].wingnum >= 0) {
			r = delete_ship_from_wing(z);
			if (r) {
				return r;
			}

			invalidate_references(name, sexp_ref_type::SHIP);
		}

		delete_reinforcement(name);

		// check if any ship is docked with this ship and break dock if so
		while (object_is_docked(&Objects[obj])) {
			ai_do_objects_undocked_stuff(&Objects[obj], dock_get_first_docked_object(&Objects[obj]));
		}

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
		if (jnp != Jump_nodes.end())
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

	return 0;
}

int Editor::delete_object(int obj) {
	int r;

	r = common_object_delete(obj);
	missionChanged();
	return r;
}

void Editor::setActiveViewport(EditorViewport* viewport) {
	_lastActiveViewport = viewport;
}

int Editor::reference_handler(const char* name, sexp_ref_type type, int obj) {

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
			if (!Mission_events[n].name.empty()) {
				sprintf(text, "event \"%s\"", Mission_events[n].name.c_str());
			} else {
				sprintf(text, "event #%d", n);
			}

			break;

		case sexp_src::MISSION_GOAL:
			if (!Mission_goals[n].name.empty()) {
				sprintf(text, "mission goal \"%s\"", Mission_goals[n].name.c_str());
			} else {
				sprintf(text, "mission goal #%d", n);
			}

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

			return 2;
		}

		sprintf(msg, "%s is referenced by %s (possibly more sexps).\n"
			"Do you want to delete it anyway?\n\n"
			"(click Cancel to go to the reference)", type_name, text);

		r = sexp_reference_handler(node, source, n, msg);
		if (r == 1) {
			if (obj >= 0) {
				unmarkObject(obj);
			}

			return 1;
		}

		if (r == 2) {
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
			if (obj >= 0) {
				unmarkObject(obj);
			}

			return 1;
		}

		if (r == 2) {
			return 2;
		}
	}

	if ((type != sexp_ref_type::SHIP) && (type != sexp_ref_type::WING)) {
		return 0;
	}

	n = find_item_with_string(Reinforcements, &reinforcements::name, name);
	if (n >= 0) {
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

int Editor::orders_reference_handler(sexp_src /*source*/, int /*source_index*/, char* msg) {
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

	return 2;
}
int Editor::sexp_reference_handler(int  /*node*/, sexp_src /*source*/, int /*source_index*/, const char* msg) {
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
					// bash it again for the display name
					wing_bash_ship_name(&Ships[Wings[wing].ship_index[i]], &Wings[wing], i + 1, true);
				}
			}

			const auto max_threshold_before = MAX_SHIPS_PER_WING - Wings[wing].wave_count;
			if (Wings[wing].threshold > max_threshold_before) {
				Wings[wing].threshold = max_threshold_before;
			}

			Wings[wing].wave_count--;
			if (Wings[wing].wave_count) {
				const auto max_threshold_after = MAX_SHIPS_PER_WING - Wings[wing].wave_count;
				if (Wings[wing].threshold > max_threshold_after) {
					Wings[wing].threshold = max_threshold_after;
				}
			}
		}
	}

	missionChanged();
	return 0;
}
int Editor::invalidate_references(const char* name, sexp_ref_type type) {
	char new_name[512];
	int i;

	sprintf(new_name, "<%s>", name);
	update_sexp_references(name, new_name);
	ai_update_goal_references(type, name, new_name);
	update_texture_replacements(name, new_name);
	i = find_item_with_string(Reinforcements, &reinforcements::name, name);
	if (i >= 0)
		strcpy_s(Reinforcements[i].name, new_name);

	return 0;
}
void Editor::ai_update_goal_references(sexp_ref_type type, const char* old_name, const char* new_name) {
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
int Editor::rename_ship(int ship, const char* name) {
	Assert(ship >= 0);
	Assert(strlen(name) < NAME_LENGTH);

	// we may not need to rename it
	if (strcmp(Ships[ship].ship_name, name) == 0)
		return 0;

	update_sexp_references(Ships[ship].ship_name, name);
	ai_update_goal_references(sexp_ref_type::SHIP, Ships[ship].ship_name, name);
	update_texture_replacements(Ships[ship].ship_name, name);
	int i = find_item_with_string(Reinforcements, &reinforcements::name, Ships[ship].ship_name);
	if (i >= 0)
		strcpy_s(Reinforcements[i].name, name);

	// keep the ship registry in sync
	auto reg_it = Ship_registry_map.find(Ships[ship].ship_name);
	if (reg_it != Ship_registry_map.end()) {
		int reg_idx = reg_it->second;
		Ship_registry_map.erase(reg_it);
		strcpy_s(Ship_registry[reg_idx].name, name);
		Ship_registry_map[name] = reg_idx;
	}

	strcpy_s(Ships[ship].ship_name, name);

	// if this name has a hash, create a default display name
	if (get_pointer_to_first_hash_symbol(Ships[ship].ship_name))
	{
		Ships[ship].display_name = Ships[ship].ship_name;
		end_string_at_first_hash_symbol(Ships[ship].display_name);
		Ships[ship].flags.set(Ship::Ship_Flags::Has_display_name);
	}
	// otherwise reset the display name
	else
	{
		Ships[ship].display_name = "";
		Ships[ship].flags.remove(Ship::Ship_Flags::Has_display_name);
	}

	missionChanged();

	return 0;
}
void Editor::delete_reinforcement(const char* name) {
	int i = find_item_with_string(Reinforcements, &reinforcements::name, name);
	if (i < 0)
		return;

	Reinforcements.erase(Reinforcements.begin() + i);
	missionChanged();
}
int Editor::check_wing_dependencies(int wing_num) {
	const char* name = Wings[wing_num].name;
	return reference_handler(name, sexp_ref_type::WING, -1);
}
int Editor::set_reinforcement(const char* name, int state) {
	int index;
	int cur = find_item_with_string(Reinforcements, &reinforcements::name, name);

	if (!state && (cur != -1)) {
		Reinforcements.erase(Reinforcements.begin() + cur);

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

	if (state && (cur == -1)) {
		Assert(strlen(name) < NAME_LENGTH);
		Reinforcements.emplace_back(name);

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
void Editor::disband_wing(int wing_num) {

	int i, total;
	object* ptr;

	if (check_wing_dependencies(wing_num)) {
		return;
	}

	delete_reinforcement(Wings[wing_num].name);

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
void Editor::generate_ship_usage_list(int* arr, int wing) {
	int i; 

	if (wing < 0) {
		return;
	}

	i = Wings[wing].wave_count;
	while (i--) {
		arr[Ships[Wings[wing].ship_index[i]].ship_info_index]++; 
	}
}
void Editor::updateStartingWingLoadoutUseCounts() {
	memset(_ship_usage, 0, sizeof(int) * MAX_TVT_TEAMS * MAX_SHIP_CLASSES);

	if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) { 
		for (int i = 0; i<MAX_TVT_TEAMS; i++) {
			for (int j = 0; j<MAX_TVT_WINGS_PER_TEAM; j++) {
				generate_ship_usage_list(_ship_usage[i], TVT_wings[(i*MAX_TVT_WINGS_PER_TEAM) + j]);
			}			
			generate_team_weaponry_usage_list(i, _weapon_usage[i]);
		}
	}
	else {
		for (int i = 0; i < MAX_STARTING_WINGS; i++) {
			generate_ship_usage_list(_ship_usage[0], Starting_wings[i]);
		}
		generate_team_weaponry_usage_list(0, _weapon_usage[0]);
	}	
}
void Editor::delete_marked() {
	object* ptr, * next;
	bool navigated_to_reference = false;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		next = GET_NEXT(ptr);
		if (ptr->flags[Object::Object_Flags::Marked]) {
			if (delete_object(OBJ_INDEX(ptr)) == 2) {  // user went to a reference, so don't get in the way.
				navigated_to_reference = true;
				break;
			}
		}

		ptr = next;
	}

	if (!navigated_to_reference) {
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
void Editor::select_next_object()
{
	object* ptr;

	if (EMPTY(&obj_used_list))
		return;

	if (query_valid_object(currentObject)) {
		ptr = Objects[currentObject].next;
		if (ptr == END_OF_LIST(&obj_used_list))
			ptr = GET_NEXT(ptr);
	} else {
		ptr = GET_FIRST(&obj_used_list);
	}

	if (getNumMarked() > 1) {
		// Cycle current object through marked objects only
		while (!Objects[OBJ_INDEX(ptr)].flags[Object::Object_Flags::Marked]) {
			ptr = GET_NEXT(ptr);
			if (ptr == END_OF_LIST(&obj_used_list))
				ptr = GET_NEXT(ptr);
		}
		setupCurrentObjectIndices(OBJ_INDEX(ptr));
	} else {
		if (getNumMarked())
			unmarkObject(currentObject);
		markObject(OBJ_INDEX(ptr));
	}
}
void Editor::select_previous_object()
{
	int arr[MAX_OBJECTS], i = 0, n = 0;
	object* ptr;

	if (EMPTY(&obj_used_list))
		return;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (currentObject == OBJ_INDEX(ptr))
			i = n;
		arr[n++] = OBJ_INDEX(ptr);
		ptr = GET_NEXT(ptr);
	}

	Assert(n);
	if (query_valid_object(currentObject)) {
		i--;
		if (i < 0)
			i = n - 1;
	} else {
		i = n - 1;
	}

	if (getNumMarked() > 1) {
		// Cycle current object through marked objects only
		while (!Objects[arr[i]].flags[Object::Object_Flags::Marked]) {
			i--;
			if (i < 0)
				i = n - 1;
		}
		setupCurrentObjectIndices(arr[i]);
	} else {
		if (getNumMarked())
			unmarkObject(currentObject);
		markObject(arr[i]);
	}
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
const ai_goal_list* Editor::getAi_goal_list()
{
	return Ai_goal_list;
}
int Editor::getAigoal_list_size() {
	// sizeof works here because Ai_goal_list is defined as an array in this same TU (above);
	// keep this function defined alongside that array so it sees the array type, not a pointer.
	return sizeof(Ai_goal_list) / sizeof(ai_goal_list);
}
SCP_vector<SCP_string> Editor::get_docking_list(int model_index) {
	int i;
	polymodel *pm;
	SCP_vector<SCP_string> out;

	pm = model_get(model_index);
	out.reserve(pm->n_docks);

	for (i=0; i<pm->n_docks; i++)
		out.emplace_back(pm->docking_bays[i].name);

	return out;
}

void Editor::exportShieldSysData(SCP_vector<GlobalShieldStatus>& teams, SCP_vector<GlobalShieldStatus>& types) const {
	teams = Shield_sys_teams;
	types = Shield_sys_types;
}

void Editor::importShieldSysData(const SCP_vector<GlobalShieldStatus>& teams, const SCP_vector<GlobalShieldStatus>& types) {
	Assertion(Shield_sys_teams.size() == teams.size(), "Mismatched shield data from global shield dialog!");
	Assertion(Shield_sys_types.size() == types.size(), "Mismatched shield data from global shield dialog!");

	Shield_sys_teams = teams;
	Shield_sys_types = types;

	for (int i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum >= 0) {
			auto z = Shield_sys_teams[Ships[i].team];
			if (Shield_sys_types[Ships[i].ship_info_index] == GlobalShieldStatus::HasShields)
				z = GlobalShieldStatus::HasShields;
			else if (Shield_sys_types[Ships[i].ship_info_index] == GlobalShieldStatus::NoShields)
				z = GlobalShieldStatus::NoShields;

			if (z == GlobalShieldStatus::HasShields)
				Objects[Ships[i].objnum].flags.remove(Object::Object_Flags::No_shields);
			else if (z == GlobalShieldStatus::NoShields)
				Objects[Ships[i].objnum].flags.set(Object::Object_Flags::No_shields);
		}
	}
}

// adapted from shield_sys_dlg OnInitDialog()
void Editor::normalizeShieldSysData() {
	std::vector<int> teams(Iff_info.size(), 0);
	std::vector<int> types(MAX_SHIP_CLASSES, 0);

	for (int i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum >= 0) {
			auto z = (Objects[Ships[i].objnum].flags[Object::Object_Flags::No_shields]) ? GlobalShieldStatus::NoShields : GlobalShieldStatus::HasShields;
			if (!teams[Ships[i].team])
				Shield_sys_teams[Ships[i].team] = z;
			else if (Shield_sys_teams[Ships[i].team] != z)
				Shield_sys_teams[Ships[i].team] = GlobalShieldStatus::MixedShields;

			if (!types[Ships[i].ship_info_index])
				Shield_sys_types[Ships[i].ship_info_index] = z;
			else if (Shield_sys_types[Ships[i].ship_info_index] != z)
				Shield_sys_types[Ships[i].ship_info_index] = GlobalShieldStatus::MixedShields;

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

SCP_string Editor::get_display_name_for_text_box(const SCP_string &orig_name)
{
	auto index = get_index_of_first_hash_symbol(orig_name);
	if (index >= 0)
	{
		SCP_string display_name(orig_name);
		end_string_at_first_hash_symbol(display_name);
		return display_name;
	}
	else
		return "<none>";
}

SCP_vector<int> Editor::getStartingWingLoadoutUseCounts() {
	// update before sending so that we have the most up to date info.
	updateStartingWingLoadoutUseCounts();

	SCP_vector<int> out;

	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		for (auto& entry : _ship_usage[i]) {
			out.push_back(entry);
		}
	}
	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		for (auto& entry : _weapon_usage[i]) {
			out.push_back(entry);
		}
	}

	return out;
}



} // namespace fred
} // namespace fso
