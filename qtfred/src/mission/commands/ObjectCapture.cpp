// ---------------------------------------------------------------------------
// CROSS-REFERENCE MAINTENANCE
// ---------------------------------------------------------------------------
// Fields captured / restored here must stay in sync with:
//   code/missioneditor/missionsave.cpp  (save_objects, save_common_object_data,
//                                        save_warp_params, save_turret_info,
//                                        save_single_dock_instance)
//   code/mission/missionparse.cpp       (parse_object, parse_common_object_data)
//
// When any of those files gains or removes a field, update this file too.
// ---------------------------------------------------------------------------

#include "ObjectCapture.h"

#include <ai/ai.h>
#include <globalincs/linklist.h>
#include <jumpnode/jumpnode.h>
#include <mission/missionparse.h>
#include <bmpman/bmpman.h>
#include <model/model.h>
#include <object/object.h>
#include <object/objectdock.h>
#include <object/waypoint.h>
#include <parse/sexp.h>
#include <prop/prop.h>
#include <ship/ship.h>
#include <model/modelreplace.h>

#include "../../ui/FredView.h"
#include "../Editor.h"

namespace fso::fred {

// ===========================================================================
// CapturedShip — construction / destruction / move semantics
// ===========================================================================

CapturedShip::~CapturedShip()
{
	auto freeDup = [](int& dup) {
		if (dup >= 0) {
			free_sexp2(dup);
			dup = SHIP_CUE_NONE;
		}
	};
	freeDup(arrival_cue_dup);
	freeDup(departure_cue_dup);
}

CapturedShip& CapturedShip::operator=(CapturedShip&& o) noexcept
{
	if (this == &o) return *this;

	// Release any cues we currently own before overwriting
	if (arrival_cue_dup >= 0)   { free_sexp2(arrival_cue_dup);   arrival_cue_dup   = SHIP_CUE_NONE; }
	if (departure_cue_dup >= 0) { free_sexp2(departure_cue_dup); departure_cue_dup = SHIP_CUE_NONE; }

	objType           = o.objType;
	signature         = o.signature;
	pos               = o.pos;
	orient            = o.orient;
	memcpy(ship_name, o.ship_name, sizeof(ship_name));
	display_name      = std::move(o.display_name);
	ship_info_index   = o.ship_info_index;
	team              = o.team;
	team_name         = std::move(o.team_name);
	alt_type_index    = o.alt_type_index;
	callsign_index    = o.callsign_index;
	alt_classes       = std::move(o.alt_classes);

	arrival_location  = o.arrival_location;
	arrival_distance  = o.arrival_distance;
	arrival_anchor    = o.arrival_anchor;
	arrival_path_mask = o.arrival_path_mask;
	arrival_delay     = o.arrival_delay;
	arrival_cue_dup   = o.arrival_cue_dup;
	o.arrival_cue_dup = SHIP_CUE_NONE;

	departure_location  = o.departure_location;
	departure_anchor    = o.departure_anchor;
	departure_path_mask = o.departure_path_mask;
	departure_delay     = o.departure_delay;
	departure_cue_dup   = o.departure_cue_dup;
	o.departure_cue_dup = SHIP_CUE_NONE;

	warpin_params_index  = o.warpin_params_index;
	warpout_params_index = o.warpout_params_index;

	initial_velocity = o.initial_velocity;
	initial_hull     = o.initial_hull;
	initial_shields  = o.initial_shields;

	weapons          = o.weapons;
	for (int i = 0; i < MAX_AI_GOALS; i++) ai_goals[i] = std::move(o.ai_goals[i]);
	kamikaze_damage  = o.kamikaze_damage;

	cargo1 = o.cargo1;
	memcpy(cargo_title, o.cargo_title, sizeof(cargo_title));

	subsystems = std::move(o.subsystems);
	flags      = o.flags;

	respawn_priority   = o.respawn_priority;
	escort_priority    = o.escort_priority;
	guardian_threshold = o.guardian_threshold;
	special_explosion  = o.special_explosion;
	special_hitpoints  = o.special_hitpoints;
	special_shield_pts = o.special_shield_pts;
	hotkey             = o.hotkey;

	dock_instances   = std::move(o.dock_instances);
	final_death_time = o.final_death_time;
	orders_accepted  = std::move(o.orders_accepted);
	group            = o.group;
	fred_layer       = std::move(o.fred_layer);
	score            = o.score;
	assist_score_pct = o.assist_score_pct;
	persona_index    = o.persona_index;
	texture_replacements = std::move(o.texture_replacements);

	hadReinforcement   = o.hadReinforcement;
	reinforcementEntry = o.reinforcementEntry;

	originalShipIndex = o.originalShipIndex;
	wingNum           = o.wingNum;

	return *this;
}

CapturedShip::CapturedShip(CapturedShip&& o) noexcept
{
	*this = std::move(o);
}

// ===========================================================================
// captureShip / restoreShip
// ===========================================================================

static int captureSexpCue(int cue)
{
	if (cue == Locked_sexp_true)  return SHIP_CUE_LOCKED_TRUE;
	if (cue == Locked_sexp_false) return SHIP_CUE_LOCKED_FALSE;
	if (cue < 0)                  return SHIP_CUE_NONE;
	// TODO(sexp_tree_refactor): dup_sexp_chain shallow-copies SEXP_FLAG_VARIABLE nodes;
	// variable-reference indices become dangling after free_sexp2() on the live tree.
	return dup_sexp_chain(cue);
}

CapturedShip captureShip(int objNum)
{
	const object&   o  = Objects[objNum];
	const ship&     s  = Ships[o.instance];
	const ai_info&  ai = Ai_info[s.ai_index];

	CapturedShip c;

	// Object
	c.objType   = o.type;
	c.signature = o.signature;
	c.pos       = o.pos;
	c.orient    = o.orient;

	// Identity
	strcpy_s(c.ship_name, s.ship_name);
	c.display_name   = s.display_name;
	c.ship_info_index = s.ship_info_index;
	c.team           = s.team;
	c.team_name      = s.team_name;
	c.alt_type_index = s.alt_type_index;
	c.callsign_index = s.callsign_index;

	for (const auto& ac : s.s_alt_classes) {
		CapturedAltClass cac;
		cac.ship_class             = ac.ship_class;
		cac.variable_index         = ac.variable_index;
		cac.default_to_this_class  = ac.default_to_this_class;
		c.alt_classes.push_back(cac);
	}

	// Arrival / departure — dup cues so this struct owns a live copy
	c.arrival_cue_dup   = captureSexpCue(s.arrival_cue);
	c.departure_cue_dup = captureSexpCue(s.departure_cue);

	c.arrival_location   = s.arrival_location;
	c.arrival_distance   = s.arrival_distance;
	c.arrival_anchor     = s.arrival_anchor;
	c.arrival_path_mask  = s.arrival_path_mask;
	c.arrival_delay      = s.arrival_delay;
	c.departure_location  = s.departure_location;
	c.departure_anchor    = s.departure_anchor;
	c.departure_path_mask = s.departure_path_mask;
	c.departure_delay     = s.departure_delay;

	// Warp parameters — indices are stable (Warp_params[] never shrinks)
	c.warpin_params_index  = s.warpin_params_index;
	c.warpout_params_index = s.warpout_params_index;

	// Initial conditions (FRED repurposes runtime object fields for these)
	c.initial_velocity = static_cast<int>(o.phys_info.speed);
	c.initial_hull     = fl2i(o.hull_strength);
	c.initial_shields  = fl2i(o.shield_quadrant[0]);

	// Weapons / AI
	c.weapons         = s.weapons;
	c.kamikaze_damage = ai.kamikaze_damage;
	for (int i = 0; i < MAX_AI_GOALS; i++) c.ai_goals[i] = ai.goals[i];

	// Cargo
	c.cargo1 = s.cargo1;
	strcpy_s(c.cargo_title, s.cargo_title);

	// Subsystems — only record entries that deviate from class-table defaults
	const ship_subsys* ss = GET_FIRST(&s.subsys_list);
	while (ss != END_OF_LIST(&s.subsys_list)) {
		const bool is_turret = (ss->system_info && ss->system_info->type == SUBSYSTEM_TURRET);
		if (ss->current_hits || is_turret || ss->subsys_cargo_name > 0) {
			CapturedSubsys css{};
			strcpy_s(css.name, ss->system_info->subobj_name);
			css.current_hits   = ss->current_hits;
			css.cargo_name_idx = ss->subsys_cargo_name;
			strcpy_s(css.cargo_title, ss->subsys_cargo_title);
			css.is_turret      = is_turret;
			if (is_turret) css.turret_weapons = ss->weapons;
			c.subsystems.push_back(css);
		}
		ss = GET_NEXT(ss);
	}

	// Flags
	c.flags = s.flags;

	// Misc
	c.respawn_priority   = s.respawn_priority;
	c.escort_priority    = s.escort_priority;
	c.guardian_threshold = s.ship_guardian_threshold;

	c.special_explosion.enabled = s.use_special_explosion;
	if (s.use_special_explosion) {
		c.special_explosion.damage         = s.special_exp_damage;
		c.special_explosion.blast          = s.special_exp_blast;
		c.special_explosion.inner_radius   = s.special_exp_inner;
		c.special_explosion.outer_radius   = s.special_exp_outer;
		c.special_explosion.use_shockwave  = s.use_shockwave;
		c.special_explosion.shockwave_speed = s.special_exp_shockwave_speed;
		c.special_explosion.deathroll_time  = s.special_exp_deathroll_time;
	}
	c.special_hitpoints  = s.special_hitpoints;
	c.special_shield_pts = s.special_shield;
	c.hotkey             = s.hotkey;

	// Docking — record both dock-point names for each docked pair
	const int myModelNum = Ship_info[s.ship_info_index].model_num;
	for (const dock_instance* di = o.dock_list; di != nullptr; di = di->next) {
		if (di->docked_objp == nullptr) continue;
		const ship& other      = Ships[di->docked_objp->instance];
		const int otherModelNum = Ship_info[other.ship_info_index].model_num;

		CapturedDockInstance cdi{};
		strcpy_s(cdi.docked_with, other.ship_name);
		strcpy_s(cdi.docker_point, model_get_dock_name(myModelNum,    di->dockpoint_used));

		// Find the matching entry on the other ship to get its dock point index
		for (const dock_instance* odi = di->docked_objp->dock_list; odi; odi = odi->next) {
			if (odi->docked_objp == &o) {
				strcpy_s(cdi.dockee_point, model_get_dock_name(otherModelNum, odi->dockpoint_used));
				break;
			}
		}
		c.dock_instances.push_back(cdi);
	}

	// Kill before mission
	c.final_death_time = s.flags[Ship::Ship_Flags::Kill_before_mission] ? s.final_death_time : -1;

	// Orders / group
	c.orders_accepted = s.orders_accepted;
	c.group           = s.group;

	// Layer
	c.fred_layer = s.fred_layer;

	// Score
	c.score           = s.score;
	c.assist_score_pct = s.assist_score_pct;

	// Persona
	c.persona_index = s.persona_index;

	// Reinforcement entry (erased by delete_object; re-added on restore)
	for (const auto& r : Reinforcements) {
		if (!stricmp(r.name, s.ship_name)) {
			c.hadReinforcement   = true;
			c.reinforcementEntry = r;
			break;
		}
	}

	// Texture replacements — filtered from the global Fred_texture_replacements
	for (const auto& tr : Fred_texture_replacements) {
		if (!stricmp(s.ship_name, tr.ship_name) && !tr.from_table)
			c.texture_replacements.push_back(tr);
	}

	// Wing context
	c.originalShipIndex = o.instance;
	c.wingNum           = s.wingnum;

	return c;
}

static void transferSexpCue(int& cue_dup, int& dest)
{
	if      (cue_dup == SHIP_CUE_LOCKED_TRUE)  dest = Locked_sexp_true;
	else if (cue_dup == SHIP_CUE_LOCKED_FALSE) dest = Locked_sexp_false;
	else if (cue_dup == SHIP_CUE_NONE)         { /* keep create_ship default */ }
	else { dest = cue_dup; cue_dup = SHIP_CUE_NONE; } // transfer ownership
}

int restoreShip(CapturedShip& c, Editor* editor)
{
	matrix orient = c.orient;
	vec3d  pos    = c.pos;
	const int newObj = editor->create_ship(&orient, &pos, c.ship_info_index);
	if (newObj < 0) return -1;

	object&   o  = Objects[newObj];
	ship&     s  = Ships[o.instance];
	ai_info&  ai = Ai_info[s.ai_index];

	// Reapply the original signature so signature-keyed commands stay valid.
	if (c.signature > 0)
		o.signature = c.signature;

	// Identity
	strcpy_s(s.ship_name, c.ship_name);
	s.display_name   = c.display_name;
	s.team           = c.team;
	s.team_name      = c.team_name;
	s.alt_type_index = c.alt_type_index;
	s.callsign_index = c.callsign_index;

	s.s_alt_classes.clear();
	for (const auto& cac : c.alt_classes) {
		alt_class ac;
		ac.ship_class            = cac.ship_class;
		ac.variable_index        = cac.variable_index;
		ac.default_to_this_class = cac.default_to_this_class;
		s.s_alt_classes.push_back(ac);
	}

	// Transfer SEXP cue ownership — no second dup
	transferSexpCue(c.arrival_cue_dup,   s.arrival_cue);
	transferSexpCue(c.departure_cue_dup, s.departure_cue);

	s.arrival_location   = c.arrival_location;
	s.arrival_distance   = c.arrival_distance;
	s.arrival_anchor     = c.arrival_anchor;
	s.arrival_path_mask  = c.arrival_path_mask;
	s.arrival_delay      = c.arrival_delay;
	s.departure_location  = c.departure_location;
	s.departure_anchor    = c.departure_anchor;
	s.departure_path_mask = c.departure_path_mask;
	s.departure_delay     = c.departure_delay;

	// Warp params
	s.warpin_params_index  = c.warpin_params_index;
	s.warpout_params_index = c.warpout_params_index;

	// Initial conditions
	o.phys_info.speed   = static_cast<float>(c.initial_velocity);
	o.hull_strength     = static_cast<float>(c.initial_hull);
	o.shield_quadrant[0] = static_cast<float>(c.initial_shields);

	// Weapons / AI
	s.weapons         = c.weapons;
	ai.kamikaze_damage = c.kamikaze_damage;
	for (int i = 0; i < MAX_AI_GOALS; i++) ai.goals[i] = c.ai_goals[i];

	// Cargo
	s.cargo1 = c.cargo1;
	strcpy_s(s.cargo_title, c.cargo_title);

	// Subsystems — apply stored overrides by matching subobj_name
	for (const auto& css : c.subsystems) {
		ship_subsys* ptr = GET_FIRST(&s.subsys_list);
		while (ptr != END_OF_LIST(&s.subsys_list)) {
			if (ptr->system_info && !stricmp(ptr->system_info->subobj_name, css.name)) {
				ptr->current_hits      = css.current_hits;
				ptr->subsys_cargo_name = css.cargo_name_idx;
				strcpy_s(ptr->subsys_cargo_title, css.cargo_title);
				if (css.is_turret) ptr->weapons = css.turret_weapons;
				break;
			}
			ptr = GET_NEXT(ptr);
		}
	}

	// Flags
	s.flags = c.flags;

	// Misc
	s.respawn_priority        = c.respawn_priority;
	s.escort_priority         = c.escort_priority;
	s.ship_guardian_threshold = c.guardian_threshold;

	s.use_special_explosion = c.special_explosion.enabled;
	if (c.special_explosion.enabled) {
		s.special_exp_damage          = c.special_explosion.damage;
		s.special_exp_blast           = c.special_explosion.blast;
		s.special_exp_inner           = c.special_explosion.inner_radius;
		s.special_exp_outer           = c.special_explosion.outer_radius;
		s.use_shockwave               = c.special_explosion.use_shockwave;
		s.special_exp_shockwave_speed = c.special_explosion.shockwave_speed;
		s.special_exp_deathroll_time  = c.special_explosion.deathroll_time;
	}
	s.special_hitpoints = c.special_hitpoints;
	s.special_shield    = c.special_shield_pts;
	s.hotkey            = c.hotkey;

	// Kill before mission
	if (c.final_death_time >= 0) {
		s.flags.set(Ship::Ship_Flags::Kill_before_mission);
		s.final_death_time = c.final_death_time;
	}

	// Orders / group
	s.orders_accepted = c.orders_accepted;
	s.group           = c.group;

	// Layer
	s.fred_layer = c.fred_layer;

	// Score
	s.score           = c.score;
	s.assist_score_pct = c.assist_score_pct;

	// Persona
	s.persona_index = c.persona_index;

	// Re-add the Reinforcements[] entry that delete_object() erased.
	if (c.hadReinforcement) {
		bool present = false;
		for (const auto& r : Reinforcements) {
			if (!stricmp(r.name, c.ship_name)) {
				present = true;
				break;
			}
		}
		if (!present)
			Reinforcements.push_back(c.reinforcementEntry);
	}

	// Texture replacements — add this ship's entries back to the global list,
	// then rebuild the model instance's textures so the viewport shows them.
	if (!c.texture_replacements.empty()) {
		for (auto tr : c.texture_replacements) {
			strcpy_s(tr.ship_name, c.ship_name);
			Fred_texture_replacements.push_back(tr);
		}
		rebuildShipPmiTextures(o.instance);
	}

	// OBJ_START (player start): create_ship creates OBJ_SHIP; fix the type if needed.
	// This mirrors what missionparse does when OF_Player_start is set.
	if (c.objType == OBJ_START)
		o.type = OBJ_START;

	return newObj;
}

// Remove all Fred_texture_replacements entries belonging to the named ship.
// Called from DeleteObjectsCommand::redo() before re-deleting ships.
void removeShipTextureReplacements(const char* ship_name)
{
	Fred_texture_replacements.erase(
		std::remove_if(Fred_texture_replacements.begin(), Fred_texture_replacements.end(),
			[ship_name](const texture_replace& tr) {
				return !stricmp(tr.ship_name, ship_name) && !tr.from_table;
			}),
		Fred_texture_replacements.end());
}

void rebuildShipPmiTextures(int shipIndex)
{
	ship& shipp  = Ships[shipIndex];
	auto* pmi    = model_get_instance(shipp.model_instance_num);
	auto* pm     = model_get(Ship_info[shipp.ship_info_index].model_num);

	auto load_tex = [](const char* name) -> int {
		if (lcase_equal(name, "invisible"))
			return REPLACE_WITH_INVISIBLE;
		int id = bm_load(name);
		if (id < 0) {
			int nf, fps;
			id = bm_load_animation(name, &nf, &fps, nullptr, nullptr, false, true);
		}
		return id;
	};

	// Collect non-table entries for this ship.
	SCP_vector<const texture_replace*> entries;
	for (const auto& tr : Fred_texture_replacements)
		if (!tr.from_table && !stricmp(tr.ship_name, shipp.ship_name))
			entries.push_back(&tr);

	if (entries.empty()) {
		// No replacements — restore default textures.
		pmi->texture_replace = nullptr;
	} else {
		pmi->texture_replace = std::make_shared<model_texture_replace>();
		for (const auto* tr : entries) {
			int id = load_tex(tr->new_texture);
			if (id == -1)
				continue;
			for (int j = 0; j < pm->n_textures; j++) {
				int tnum = pm->maps[j].FindTexture(tr->old_texture);
				if (tnum >= 0)
					(*pmi->texture_replace)[j * TM_NUM_TYPES + tnum] = id;
			}
		}
	}

	// Re-layer from_table entries (remove_ship_entries leaves them intact, but
	// make_shared above would have wiped them; nullptr case needs them applied too).
	bool hasTable = false;
	for (const auto& tr : Fred_texture_replacements)
		if (tr.from_table && !stricmp(tr.ship_name, shipp.ship_name)) { hasTable = true; break; }

	if (hasTable) {
		if (!pmi->texture_replace)
			pmi->texture_replace = std::make_shared<model_texture_replace>();
		for (const auto& tr : Fred_texture_replacements) {
			if (!tr.from_table || stricmp(tr.ship_name, shipp.ship_name) != 0)
				continue;
			int id = (tr.new_texture_id != -1) ? tr.new_texture_id : load_tex(tr.new_texture);
			if (id == -1)
				continue;
			for (int j = 0; j < pm->n_textures; j++) {
				int tnum = pm->maps[j].FindTexture(tr.old_texture);
				if (tnum >= 0)
					(*pmi->texture_replace)[j * TM_NUM_TYPES + tnum] = id;
			}
		}
	}
}

void recaptureShipForRedo(CapturedShip& data, int objNum)
{
	Assertion(data.arrival_cue_dup == SHIP_CUE_NONE,
	          "recaptureShipForRedo: arrival_cue_dup must be SHIP_CUE_NONE before recapture (was %d)",
	          data.arrival_cue_dup);
	Assertion(data.departure_cue_dup == SHIP_CUE_NONE,
	          "recaptureShipForRedo: departure_cue_dup must be SHIP_CUE_NONE before recapture (was %d)",
	          data.departure_cue_dup);
	const ship& s = Ships[Objects[objNum].instance];
	// Re-dup cues before delete_marked() frees them.
	data.arrival_cue_dup   = captureSexpCue(s.arrival_cue);
	data.departure_cue_dup = captureSexpCue(s.departure_cue);
	// Clean up this ship's texture replacements from the global list; they
	// will be re-added by restoreShip() on the next undo().
	removeShipTextureReplacements(s.ship_name);
}

// ===========================================================================
// captureWaypointList / restoreWaypointListProperties
// ===========================================================================

CapturedWaypointList captureWaypointList(int listIndex,
                                          const SCP_vector<int>& deletedObjNums,
                                          bool entireList)
{
	const waypoint_list& wl = Waypoint_lists[listIndex];
	const auto& wpts = wl.get_waypoints();

	CapturedWaypointList cwl;
	cwl.listName   = wl.get_name();
	cwl.entirePath = entireList;
	cwl.no_lines   = wl.get_no_draw_lines();
	cwl.has_color  = wl.get_has_custom_color();
	cwl.color_r    = wl.get_color_r();
	cwl.color_g    = wl.get_color_g();
	cwl.color_b    = wl.get_color_b();
	cwl.fred_layer = wl.get_fred_layer();

	for (int i = 0; i < (int)wpts.size(); i++) {
		const int objNum = wpts[i].get_objnum();
		if (objNum < 0) continue;
		bool isDeleted = std::find(deletedObjNums.begin(), deletedObjNums.end(), objNum)
		                 != deletedObjNums.end();
		if (!isDeleted) {
			cwl.entirePath = false;
			continue;
		}
		CapturedWaypointPoint pt;
		pt.pos = Objects[objNum].pos;
		pt.predecessorInstance = (i == 0) ? -1 : Objects[wpts[i - 1].get_objnum()].instance;
		pt.signature = Objects[objNum].signature;
		cwl.points.push_back(pt);
	}

	return cwl;
}

void restoreWaypointListProperties(const CapturedWaypointList& data)
{
	waypoint_list* wl = find_matching_waypoint_list(data.listName.c_str());
	if (!wl) return;

	wl->set_no_draw_lines(data.no_lines);
	if (data.has_color)
		wl->set_color(data.color_r, data.color_g, data.color_b);
	else
		wl->clear_color();
	wl->set_fred_layer(data.fred_layer);
}

// ===========================================================================
// captureJumpNode / restoreJumpNode
// ===========================================================================

CapturedJumpNode captureJumpNode(int objNum)
{
	CapturedJumpNode cjn{};
	for (const auto& jn : Jump_nodes) {
		if (jn.GetSCPObjectNumber() != objNum) continue;

		cjn.signature = Objects[objNum].signature;
		cjn.pos = *jn.GetPosition();
		strcpy_s(cjn.name, jn.GetName());

		cjn.has_display_name = jn.HasDisplayName();
		if (cjn.has_display_name)
			strcpy_s(cjn.display_name, jn.GetDisplayName());

		cjn.has_special_model = jn.IsSpecialModel();
		if (cjn.has_special_model) {
			const polymodel* pm = model_get(jn.GetModelNumber());
			if (pm) strcpy_s(cjn.model_file, pm->filename);
		}

		cjn.has_custom_color = jn.IsColored();
		if (cjn.has_custom_color) {
			const color& col = jn.GetColor();
			cjn.color_r = col.red;
			cjn.color_g = col.green;
			cjn.color_b = col.blue;
			cjn.color_a = col.alpha;
		}

		cjn.hidden     = jn.IsHidden();
		cjn.fred_layer = jn.GetFredLayer();
		break;
	}
	return cjn;
}

int restoreJumpNode(const CapturedJumpNode& data, Editor* /*editor*/)
{
	CJumpNode jn(&data.pos);
	jn.SetName(data.name);

	if (data.has_display_name)
		jn.SetDisplayName(data.display_name);

	if (data.has_special_model && data.model_file[0] != '\0')
		jn.SetModel(data.model_file);

	if (data.has_custom_color)
		jn.SetAlphaColor(data.color_r, data.color_g, data.color_b, data.color_a);

	if (data.hidden)
		jn.SetVisibility(false);

	jn.SetFredLayer(data.fred_layer);

	const int newObj = jn.GetSCPObjectNumber();
	Jump_nodes.push_back(std::move(jn));

	// Reapply the original signature so signature-keyed commands stay valid.
	if (newObj >= 0 && data.signature > 0)
		Objects[newObj].signature = data.signature;

	return newObj;
}

// ===========================================================================
// captureProp / restoreProp
// ===========================================================================

CapturedProp captureProp(int objNum)
{
	CapturedProp cp{};
	cp.signature = Objects[objNum].signature;
	cp.pos    = Objects[objNum].pos;
	cp.orient = Objects[objNum].orient;
	cp.no_collide = !Objects[objNum].flags[Object::Object_Flags::Collides];

	const prop* propp = prop_id_lookup(Objects[objNum].instance);
	if (propp) {
		cp.prop_info_index = propp->prop_info_index;
		strcpy_s(cp.prop_name, propp->prop_name);
		cp.fred_layer = propp->fred_layer;
	}
	return cp;
}

int restoreProp(const CapturedProp& data, Editor* /*editor*/)
{
	matrix orient = data.orient;
	vec3d  pos    = data.pos;
	const int newObj = prop_create(&orient, &pos, data.prop_info_index);
	if (newObj < 0) return -1;

	// Reapply the original signature so signature-keyed commands stay valid.
	if (data.signature > 0)
		Objects[newObj].signature = data.signature;

	prop* propp = prop_id_lookup(Objects[newObj].instance);
	if (propp) {
		strcpy_s(propp->prop_name, data.prop_name);
		propp->fred_layer = data.fred_layer;
	}

	if (data.no_collide)
		Objects[newObj].flags.remove(Object::Object_Flags::Collides);
	else
		Objects[newObj].flags.set(Object::Object_Flags::Collides);

	return newObj;
}

} // namespace fso::fred
