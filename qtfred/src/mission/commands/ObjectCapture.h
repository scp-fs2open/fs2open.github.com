#pragma once

// ---------------------------------------------------------------------------
// CROSS-REFERENCE MAINTENANCE
// ---------------------------------------------------------------------------
// Every field captured / restored here must stay in sync with the three
// other places that read or write per-object mission data:
//
//   code/missioneditor/missionsave.cpp
//       save_objects(), save_common_object_data(),
//       save_warp_params(), save_turret_info(), save_single_dock_instance()
//
//   code/mission/missionparse.cpp
//       parse_object(), parse_common_object_data()
//
// When a new field is added anywhere in those files (or here), update ALL
// three locations so that undo/redo, save, and load remain consistent.
// ---------------------------------------------------------------------------

#include <globalincs/pstypes.h>
#include <math/vecmat.h>
#include <ship/ship.h>
#include <ai/aigoals.h>
#include <ai/ai_flags.h>
#include <object/object_flags.h>
#include <mission/missionparse.h>

namespace fso::fred {

class Editor;

// ===========================================================================
// Sentinel values for SEXP cue ownership inside Captured* structs.
// Non-negative values are live dup'd SEXP node indices owned by the struct.
// Negative values are sentinels — no node is owned.
// ===========================================================================
static constexpr int SHIP_CUE_LOCKED_TRUE  = -1; // restore to Locked_sexp_true
static constexpr int SHIP_CUE_LOCKED_FALSE = -2; // restore to Locked_sexp_false
static constexpr int SHIP_CUE_NONE         = -3; // command holds no dup right now

// Capture one SEXP cue formula: returns a SHIP_CUE_* sentinel, or a dup of the
// chain owned by the caller.
int captureSexpCue(int cue);

// Produce a live formula from a captured cue for assignment into mission data.
// Sentinels map back to the locked sexps / -1; owned dups are copied again so
// the capture stays valid for repeated undo/redo cycles.
int materializeSexpCue(int cue_dup);

// Free an owned dup (no-op for sentinels) and reset to SHIP_CUE_NONE.
void freeSexpCueDup(int& cue_dup);

// An arrival or departure anchor. The anchor is a ship registry index and the
// dialogs' target a Ships[] slot; both change when the target ship is deleted
// and restored, so a ship target is kept by object signature.
struct AnchorRef {
	int target  = -1; // anchor_to_target() value: a Ships[] slot or a special value
	int shipSig = -1; // the target ship, when target is a ship
	bool operator==(const AnchorRef& o) const { return target == o.target && shipSig == o.shipSig; }
	bool operator!=(const AnchorRef& o) const { return !(*this == o); }
	anchor_t toAnchor() const;
};
AnchorRef captureAnchor(anchor_t anchor);

// A Reinforcements[] entry with its position in the list, so undo can put it
// back where it was with its uses, delay and messages.
struct CapturedReinforcement {
	bool           present = false;
	int            index   = -1;
	reinforcements entry;
};
CapturedReinforcement captureReinforcement(const char* name);
// Make Reinforcements[] match: overwrite the entry for name in place, insert it
// at its old position, or erase it. Does not touch the ship or wing flag.
void restoreReinforcement(const char* name, const CapturedReinforcement& r);
// The same for several entries at once. Inserts go in ascending old position,
// so restoring entries one by one in another order can't scramble the list.
void restoreReinforcements(const SCP_vector<std::pair<SCP_string, CapturedReinforcement>>& items);

// Point Ship_registry_map[ship name] at the ship's own registry entry, fixing
// keys that undo left pointing at the wrong entry (a restored ship can take
// a name a wing survivor still had for a moment) or missing.
// preferredEntry: the ship's entry from before it was deleted. FRED keeps that entry
// (ship_delete only drops the key) and other ships' anchors still point at it,
// so a restored ship takes it back rather than the new one create_ship() made.
void syncShipRegistryEntry(int shipIndex, int preferredEntry = -1);
void syncAllShipRegistryEntries();

// Return saved object flags with the editor-state and runtime bits (Marked,
// Hidden, Was_rendered, ...) taken from live instead. Use when restoring a
// captured flagset so undo never brings back an old selection or visibility.
flagset<Object::Object_Flags> withLiveEditorObjectFlags(flagset<Object::Object_Flags> saved,
                                                        const flagset<Object::Object_Flags>& live);

// ===========================================================================
// Ship / Player-Start (OBJ_SHIP / OBJ_START)
// ===========================================================================

// One subsystem override entry — only written when the subsystem deviates from
// the class-table defaults (damaged, has cargo, or is a turret with custom weapons).
struct CapturedSubsys {
	char        name[NAME_LENGTH];      // system_info->subobj_name — match key on restore
	float       current_hits;           // raw hit count as stored in FRED context
	int         cargo_name_idx;         // subsys_cargo_name (index into Cargo_names[])
	char        cargo_title[NAME_LENGTH];
	bool        is_turret = false;
	ship_weapon turret_weapons;         // only valid when is_turret == true
};

// The subsystems of s that deviate from class defaults, and the reverse:
// apply them to a ship whose subsystems are at class defaults.
SCP_vector<CapturedSubsys> captureSubsystemOverrides(const ship& s);
void restoreSubsystemOverrides(ship& s, const SCP_vector<CapturedSubsys>& subsystems);

struct CapturedAltClass {
	int  ship_class           = -1;
	int  variable_index       = -1;
	bool default_to_this_class = false;
};

// Docking relationship captured for one side of the pair.
// Restore is a two-pass operation (both ships must exist); mark with the TODO
// below until that pass is implemented.
// TODO(docking-undo): implement the second restore pass after all ships exist.
struct CapturedDockInstance {
	char docked_with[NAME_LENGTH]  = {};
	char docker_point[NAME_LENGTH] = {}; // dock point on THIS ship
	char dockee_point[NAME_LENGTH] = {}; // dock point on the OTHER ship
};

struct CapturedSpecialExplosion {
	bool enabled           = false;
	int  damage            = 0;
	int  blast             = 0;
	int  inner_radius      = 0;
	int  outer_radius      = 0;
	bool use_shockwave     = false;
	int  shockwave_speed   = 0;
	int  deathroll_time    = 0;
};

struct CapturedShip {
	// ---- Object ----
	int    objType = OBJ_SHIP; // OBJ_SHIP or OBJ_START
	bool   primaryPlayerStart = false; // was Player_start_shipnum; delete repoints it
	int    registryIndex = -1; // Ship_registry entry; kept by FRED after delete, anchors use it
	// Original object signature, reapplied on restore. The pool slot (objNum)
	// changes on recreation, but restoring the signature keeps every other
	// command's obj_get_by_signature() lookups valid across undo cycles.
	int    signature = -1;
	vec3d  pos     = vmd_zero_vector;
	matrix orient  = vmd_identity_matrix;

	// ---- Identity ----
	char       ship_name[NAME_LENGTH]  = {};
	SCP_string display_name;
	int        ship_info_index         = -1;
	int        team                    = 0;
	SCP_string team_name;   // team color override; only used if ship_info.uses_team_colors
	int        alt_type_index          = -1;
	int        callsign_index          = -1;
	SCP_vector<CapturedAltClass> alt_classes;

	// ---- Arrival ----
	ArrivalLocation arrival_location  = ArrivalLocation::AT_LOCATION;
	int             arrival_distance  = 0;
	anchor_t        arrival_anchor    = {};
	int             arrival_path_mask = 0;
	int             arrival_delay     = 0;
	int             arrival_cue_dup   = SHIP_CUE_NONE; // owned dup; see SHIP_CUE_* sentinels

	// ---- Departure ----
	DepartureLocation departure_location  = DepartureLocation::AT_LOCATION;
	anchor_t          departure_anchor    = {};
	int               departure_path_mask = 0;
	int               departure_delay     = 0;
	int               departure_cue_dup   = SHIP_CUE_NONE; // owned dup

	// ---- Warp parameters ----
	// Indices into the global Warp_params[]. The array never shrinks, so stored
	// indices remain valid across undo/redo cycles.
	int warpin_params_index  = -1;
	int warpout_params_index = -1;

	// ---- Initial conditions ----
	// FRED repurposes runtime object-struct fields for these percentage values.
	//   initial_velocity → Objects[].phys_info.speed  (cast to int)
	//   initial_hull     → fl2i(Objects[].hull_strength)
	//   initial_shields  → fl2i(Objects[].shield_quadrant[0])
	int initial_velocity = 0;
	int initial_hull     = 100;
	int initial_shields  = 100;

	// ---- Weapons / AI ----
	ship_weapon weapons     = {};          // includes weapons.ai_class override
	ai_goal     ai_goals[MAX_AI_GOALS] = {};
	int         kamikaze_damage        = 0; // Ai_info[ai_index].kamikaze_damage

	// ---- Cargo ----
	char cargo1                   = 0;     // byte index into Cargo_names[]
	char cargo_title[NAME_LENGTH] = {};

	// ---- Subsystems ----
	SCP_vector<CapturedSubsys> subsystems;

	// ---- Flags ----
	flagset<Ship::Ship_Flags>     flags;
	flagset<Object::Object_Flags> obj_flags; // editor-state bits are ignored on restore
	flagset<AI::AI_Flags>         ai_flags;  // Kamikaze, No_dynamic, ...

	// ---- FRED alt name / callsign ----
	// After load these live only in Fred_alt_names[] / Fred_callsigns[], keyed
	// by Ships[] slot, so they must follow the ship to its new slot.
	char fred_alt_name[NAME_LENGTH + 1] = {};
	char fred_callsign[NAME_LENGTH + 1] = {};

	// ---- Misc ----
	int   respawn_priority    = 0;
	int   escort_priority     = 0;
	int   guardian_threshold  = 0;    // ship.ship_guardian_threshold
	CapturedSpecialExplosion special_explosion;
	int   special_hitpoints   = 0;
	int   special_shield_pts  = 0;    // ship.special_shield
	int   hotkey              = -1;

	// ---- Docking ----
	SCP_vector<CapturedDockInstance> dock_instances;

	// ---- Kill before mission ----
	int   final_death_time = -1; // ship.final_death_time; -1 = not used

	// ---- Orders / group ----
	SCP_set<size_t> orders_accepted;
	int             group = -1;

	// ---- Layer ----
	SCP_string fred_layer;

	// ---- Score ----
	int   score           = 0;
	float assist_score_pct = 0.0f;

	// ---- Persona ----
	int persona_index = -1;

	// ---- Reinforcement ----
	// The ship's Reinforcements[] entry, if any — delete_object() erases it,
	// so undo must re-add it.
	CapturedReinforcement reinforcement; // with its place in the list

	// ---- Texture replacements ----
	// Entries from the global Fred_texture_replacements[] that belong to this ship.
	SCP_vector<texture_replace> texture_replacements;

	// ---- Wing-membership context (used by DeleteObjectsCommand) ----
	int originalShipIndex = -1; // Ships[] instance at capture time; stable, never mutated
	int wingNum           = -1; // -1 if not in a wing

	CapturedShip()                               = default;
	~CapturedShip();
	CapturedShip(const CapturedShip&)            = delete;
	CapturedShip& operator=(const CapturedShip&) = delete;
	CapturedShip(CapturedShip&&) noexcept;
	CapturedShip& operator=(CapturedShip&&) noexcept;
};

// Capture all FRED-editable fields from the ship at objNum.
// Dups the arrival/departure SEXP cue trees; the returned CapturedShip owns them.
CapturedShip captureShip(int objNum);

// Restore a ship from captured data. Transfers SEXP cue ownership (no second
// dup). Returns the new object number, or -1 on failure.
int restoreShip(CapturedShip& data, Editor* editor);

// Re-capture SEXP cues and texture replacements from the live ship at objNum
// before delete_marked() is called in redo(). The cue_dup fields must be
// SHIP_CUE_NONE (already transferred to the live ship by the preceding
// restoreShip() call). Removes the ship's entries from Fred_texture_replacements.
void recaptureShipForRedo(CapturedShip& data, int objNum);

// Remove all Fred_texture_replacements entries that belong to ship_name.
void removeShipTextureReplacements(const char* ship_name);

// Rebuild pmi->texture_replace for ship_index from current Fred_texture_replacements entries.
// Loads texture bitmaps as needed. Pass after updating Fred_texture_replacements so the
// viewport reflects the change immediately. Passing an index with no entries resets the pmi
// to default textures (pmi->texture_replace = nullptr).
void rebuildShipPmiTextures(int shipIndex);

// ===========================================================================
// Waypoint List
// ===========================================================================

struct CapturedWaypointPoint {
	vec3d pos                 = vmd_zero_vector;
	int   indexInPath         = 0;  // position in its path at capture time
	int   signature           = -1; // reapplied on restore; see CapturedShip::signature
};

struct CapturedWaypointList {
	SCP_string                        listName;
	SCP_vector<CapturedWaypointPoint> points;
	bool                              entirePath = false; // true = all waypoints in list deleted
	int                               listIndex  = -1;    // position in Waypoint_lists, for an entire path

	// Visual / organizational properties (absent from the old SavedWaypointGroup)
	bool       no_lines  = false;
	bool       has_color = false;
	int        color_r   = 0;
	int        color_g   = 255;
	int        color_b   = 0;
	SCP_string fred_layer;
};

// Build a CapturedWaypointList for list at Waypoint_lists[listIndex], recording
// only the points whose object numbers appear in deletedObjNums.
// Pass entireList=true when the whole list is being deleted.
CapturedWaypointList captureWaypointList(int listIndex,
                                          const SCP_vector<int>& deletedObjNums,
                                          bool entireList);

// Apply list-level properties back to an already-existing waypoint list.
// Called after the waypoints themselves have been recreated by undo().
void restoreWaypointListProperties(const CapturedWaypointList& data);

// ===========================================================================
// Jump Node
// ===========================================================================

struct CapturedJumpNode {
	int    listIndex                    = -1; // position in Jump_nodes (save order)
	int    signature                    = -1; // reapplied on restore; see CapturedShip::signature
	vec3d  pos                          = vmd_zero_vector;
	char   name[NAME_LENGTH]            = {};
	char   display_name[NAME_LENGTH]    = {}; // empty if HasDisplayName() == false
	char   model_file[MAX_FILENAME_LEN] = {}; // empty if default model
	ubyte  color_r  = 0;
	ubyte  color_g  = 255;
	ubyte  color_b  = 0;
	ubyte  color_a  = 255;
	bool   has_custom_color  = false; // CJumpNode::IsColored()
	bool   hidden            = false; // CJumpNode::IsHidden()
	bool   has_display_name  = false;
	bool   has_special_model = false;
	SCP_string fred_layer;
};

CapturedJumpNode captureJumpNode(int objNum);
int restoreJumpNode(const CapturedJumpNode& data, Editor* editor);

// ===========================================================================
// Prop
// ===========================================================================

struct CapturedProp {
	int        signature       = -1; // reapplied on restore; see CapturedShip::signature
	vec3d      pos             = vmd_zero_vector;
	matrix     orient          = vmd_identity_matrix;
	int        prop_info_index = -1;
	char       prop_name[NAME_LENGTH] = {};
	bool       no_collide      = false; // !Objects[].flags[Object_Flags::Collides]
	SCP_string fred_layer;
};

CapturedProp captureProp(int objNum);
int restoreProp(const CapturedProp& data, Editor* editor);

} // namespace fso::fred
