#include "checkpoint/checkpoint.h"

#include "checkpoint/registry.h"
#include "checkpoint/serialize.h"
#include "checkpoint/fresh_state.h"
#include "checkpoint/store.h"

#include "ai/ai_profiles.h"
#include "asteroid/asteroid.h"

#include "debugconsole/console.h"
#include "freespace.h"
#include "gamesequence/gamesequence.h"
#include "hud/hud.h"
#include "hud/hudparse.h"
#include "hud/hudsquadmsg.h"
#include "hud/hudtarget.h"
#include "iff_defs/iff_defs.h"
#include "io/timer.h"
#include "jumpnode/jumpnode.h"
#include "localization/localize.h"
#include "math/curve.h"
#include "mission/missioncampaign.h"
#include "mission/missiongoals.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "object/objcollide.h"
#include "object/waypoint.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"
#include "playerman/player.h"
#include "prop/prop.h"
#include "scripting/global_hooks.h"
#include "scripting/scripting.h"
#include "ship/ship.h"
#include "sound/sound.h"
#include "species_defs/species_defs.h"
#include "stats/medals.h"
#include "stats/scoring.h"
#include "utils/Random.h"
#include "weapon/beam.h"
#include "weapon/weapon.h"

extern SCP_vector<game_snd> Snds;

namespace checkpoint {

namespace {

constexpr uint32_t MAGIC = 0x50435346; // "FSCP"
constexpr uint32_t VERSION = 2;

// --- registries ------------------------------------------------------------------------------------------------------

SCP_vector<GlobalHandler>& global_handlers()
{
	static SCP_vector<GlobalHandler> handlers;
	return handlers;
}

SCP_vector<std::pair<const char*, std::function<void()>>>& after_globals_functions()
{
	static SCP_vector<std::pair<const char*, std::function<void()>>> functions;
	return functions;
}

SCP_vector<std::pair<const char*, std::function<void()>>>& after_restore_functions()
{
	static SCP_vector<std::pair<const char*, std::function<void()>>> functions;
	return functions;
}

SCP_vector<ObjectTypeHandler>& object_type_handlers()
{
	static SCP_vector<ObjectTypeHandler> handlers;
	return handlers;
}

const ObjectTypeHandler* find_object_type_handler(int type)
{
	for (const auto& handler : object_type_handlers()) {
		if (handler.type == type)
			return &handler;
	}
	return nullptr;
}

// --- requests --------------------------------------------------------------------------------------------------------

enum class RequestType { None, Save, Load, Verify };

struct Request {
	RequestType type = RequestType::None;
	int slot = 0;
	int frames = 0;
	SCP_vector<SCP_string> excluded;
	int replay_event = -1;	// a load: the event to run again once the checkpoint is loaded, see request_load()
	SCP_string name;		// a save: the checkpoint's name, or empty to use the pending one
};

Request Pending_request;

// State of the current mission instance.  It survives the reload a checkpoint load does, and is reset when a mission
// starts without one.
int Loaded_index = 0;							// the checkpoint this mission was resumed from, or 0
int Pending_index = 0;							// the checkpoint a load in progress restores
int Pending_replay_event = -1;					// the event a load in progress runs again once it is restored
bool Replaying_event = false;
bool First_frame_after_restore = false;			// loads requested in that frame are ignored, see request_load()
SCP_unordered_map<SCP_string, bool, SCP_string_lcase_hash, SCP_string_lcase_equal_to> Loaded_status;	// ships and wings: still in the mission or yet to arrive

struct AssistSettings {
	float min_hull = 0.0f;	// percent
	float min_ammo = 0.0f;
	SCP_vector<SCP_string> ships;
};
AssistSettings Assist;

// data scripts keep in checkpoints
SCP_map<SCP_string, SCP_string> Script_data;
CHECKPOINT_GLOBAL(Script_data);

// the name the next checkpoint saved is given, see set_pending_name()
SCP_string Pending_name;

// a load in progress: set when the mission reload is started, cleared when the restore is done
std::optional<Buffer> Pending_restore;
bool Loading = false;

// While a checkpoint reloads and restores the mission, the mission isn't starting or ending for scripts, so no hooks
// run until it is restored.
void set_loading(bool loading)
{
	Loading = loading;
	Scripting_hooks_suppressed = loading;
}

// verification in progress
enum class VerifyStage { None, RunningOriginal, AwaitingRestore, RunningRestored };

struct VerifyRun {
	VerifyStage stage = VerifyStage::None;
	int frames = 0;
	int frames_left = 0;
	Buffer start;		// the checkpoint the second run starts from
	Buffer original;	// the state to compare against
	Trace original_trace;
};

VerifyRun Verify;

// fixed frame stepping while verifying
bool Stepping = false;
uint64_t Step_clock = 0;
constexpr fix STEP_FRAMETIME = F1_0 / 30;

// --- sections --------------------------------------------------------------------------------------------------------

void begin_section(Writer& w, const char* name, size_t& at)
{
	w.u32(name_hash(name));
	at = w.begin_block();
}

SCP_unordered_map<uint32_t, Reader> read_sections(Reader& r)
{
	SCP_unordered_map<uint32_t, Reader> sections;
	while (!r.at_end() && !r.failed()) {
		auto hash = r.u32();
		sections.emplace(hash, r.block());
	}
	return sections;
}

// --- compatibility ---------------------------------------------------------------------------------------------------
//
// A checkpoint loads into a build, tables and mission that differ from the ones it was saved with, as long as
// everything its data refers to is still there:
// - The schema fingerprint covers the layout of every saved struct and the registered globals.  It is generated, so it
//   needs no maintenance.  If it differs, loading is tolerant: unknown fields are skipped, missing ones keep their
//   current value, and the differences are logged.
// - Saved data refers to table entries by index, so the names of the entries of those tables must be unchanged, though
//   new entries may be appended.
// - The mission may have changed only if its structure is the same: its objects, wings, events and so on have the same
//   names.  The checkpoint's state, including its mission logic, then replaces the mission's.

using NameLists = SCP_vector<std::pair<SCP_string, SCP_vector<SCP_string>>>;

void mix(uint32_t& hash, const void* data, size_t size)
{
	auto bytes = static_cast<const ubyte*>(data);
	for (size_t i = 0; i < size; ++i) {
		hash ^= bytes[i];
		hash *= 16777619u;
	}
}

void mix(uint32_t& hash, const char* text)
{
	mix(hash, text, strlen(text) + 1);
}

template <typename T>
struct FingerprintVisitor {
	uint32_t& hash;

	template <auto Member>
	void field(const char* name)
	{
		using F = std::remove_cv_t<std::remove_reference_t<decltype(std::declval<T&>().*Member)>>;
		uint32_t size = sizeof(F);
		mix(hash, name);
		mix(hash, &size, sizeof(size));
	}
};

uint32_t schema_fingerprint()
{
	static const uint32_t fingerprint = [] {
		uint32_t hash = 2166136261u;
#define CHECKPOINT_FINGERPRINT_STRUCT(S)                                                                               \
		{                                                                                                              \
			mix(hash, #S);                                                                                             \
			FingerprintVisitor<S> visitor{hash};                                                                       \
			Fields<S>::each(visitor);                                                                                  \
		}
		CHECKPOINT_SCANNED_STRUCTS(CHECKPOINT_FINGERPRINT_STRUCT)
#undef CHECKPOINT_FINGERPRINT_STRUCT
		for (const auto& handler : global_handlers())
			mix(hash, handler.name);
		return hash;
	}();
	return fingerprint;
}

template <typename Container, typename GetName>
SCP_vector<SCP_string> names_of(const Container& container, GetName get_name)
{
	SCP_vector<SCP_string> names;
	for (const auto& entry : container)
		names.emplace_back(get_name(entry));
	return names;
}

// The tables whose entries saved data refers to by index.  This list is kept by hand: a table that saved data
// starts to index must be added here.
NameLists table_names()
{
	NameLists tables;
	tables.emplace_back("Ship_info", names_of(Ship_info, [](const ship_info& e) { return e.name; }));
	tables.emplace_back("Weapon_info", names_of(Weapon_info, [](const weapon_info& e) { return e.name; }));
	tables.emplace_back("Ai_classes", names_of(Ai_class_names, [](const char* e) { return e; }));
	tables.emplace_back("Species_info", names_of(Species_info, [](const species_info& e) { return e.species_name; }));
	tables.emplace_back("Iff_info", names_of(Iff_info, [](const iff_info& e) { return e.iff_name; }));
	tables.emplace_back("Armor_types", names_of(Armor_types, [](ArmorType e) { return SCP_string(e.GetNamePtr()); }));
	tables.emplace_back("Damage_types", names_of(Damage_types, [](const DamageTypeStruct& e) { return e.name; }));
	tables.emplace_back("Asteroid_info", names_of(Asteroid_info, [](const asteroid_info& e) { return e.name; }));
	tables.emplace_back("Personas", names_of(Personas, [](const Persona& e) { return e.name; }));
	tables.emplace_back("Ai_profiles", SCP_vector<SCP_string>());
	for (int i = 0; i < Num_ai_profiles; ++i)
		tables.back().second.emplace_back(Ai_profiles[i].profile_name);
	tables.emplace_back("Prop_info", names_of(Prop_info, [](const prop_info& e) { return e.name; }));
	tables.emplace_back("Ship_types", names_of(Ship_types, [](const ship_type_info& e) { return e.name; }));
	tables.emplace_back("Medals", names_of(Medals, [](const medal_stuff& e) { return e.name; }));
	tables.emplace_back("Ranks", names_of(Ranks, [](const rank_stuff& e) { return e.name; }));
	tables.emplace_back("Player_orders", names_of(Player_orders, [](const player_order& e) { return e.parse_name; }));
	tables.emplace_back("Snds", names_of(Snds, [](const game_snd& e) { return e.name; }));
	tables.emplace_back("Curves", names_of(Curves, [](const Curve& e) { return e.name; }));
	tables.emplace_back("Wing_formations", names_of(Wing_formations, [](const wing_formation& e) { return e.name; }));
	tables.emplace_back("Traitor_overrides", names_of(Traitor_overrides, [](const traitor_override_t& e) { return e.name; }));
	tables.emplace_back("HUD gauges", names_of(default_hud_gauges, [](const SCP_polymorphic<HudGauge>& e) { return std::to_string(e->getObjectType()); }));
	return tables;
}

// The structure of the mission as it was parsed, before anything ran
NameLists Mission_structure;

NameLists mission_structure()
{
	NameLists lists;
	lists.emplace_back("Parse_objects", names_of(Parse_objects, [](const p_object& e) { return e.name; }));
	lists.emplace_back("Wings", SCP_vector<SCP_string>());
	for (int i = 0; i < Num_wings; ++i)
		lists.back().second.emplace_back(Wings[i].name);
	lists.emplace_back("Mission_events", names_of(Mission_events, [](const mission_event& e) { return e.name; }));
	lists.emplace_back("Mission_goals", names_of(Mission_goals, [](const mission_goal& e) { return e.name; }));
	lists.emplace_back("Sexp_variables", SCP_vector<SCP_string>());
	for (const auto& variable : Sexp_variables) {
		if (!(variable.type & SEXP_VARIABLE_NOT_USED))
			lists.back().second.emplace_back(variable.variable_name);
	}
	lists.emplace_back("Sexp_containers", names_of(get_all_sexp_containers(), [](const sexp_container& e) { return e.container_name; }));
	lists.emplace_back("Waypoint_lists", names_of(Waypoint_lists, [](const waypoint_list& e) {
		return SCP_string(e.get_name()) + "#" + std::to_string(e.get_waypoints().size());
	}));
	lists.emplace_back("Messages", SCP_vector<SCP_string>());
	for (int i = Num_builtin_messages; i < Num_messages; ++i)
		lists.back().second.emplace_back(Messages[i].name);
	lists.emplace_back("Jump_nodes", names_of(Jump_nodes, [](const CJumpNode& e) { return e.GetName(); }));
	lists.emplace_back("Reinforcements", names_of(Reinforcements, [](const reinforcements& e) { return e.name; }));

	// Formulas are saved with the checkpoint, but some holders of node indices aren't (debriefing formulas, for
	// example), so the shape of the node trees must be the same; texts and numbers in them may differ.
	uint32_t layout = 2166136261u;
	for (int i = 0; i < Num_sexp_nodes; ++i) {
		const auto& node = Sexp_nodes[i];
		if (node.type == SEXP_NOT_USED)
			continue;
		int shape[] = {i, node.type, node.subtype, node.first, node.rest};
		mix(layout, shape, sizeof(shape));
	}
	lists.emplace_back("Sexp node layout", SCP_vector<SCP_string>{std::to_string(layout)});

	// initial subsystem states are referred to by index from the parse objects
	lists.emplace_back("Subsys_status", SCP_vector<SCP_string>{std::to_string(Subsys_index)});

	// campaign formulas share the node array, so the campaign must be the same one
	lists.emplace_back("Campaign", SCP_vector<SCP_string>{(Game_mode & GM_CAMPAIGN_MODE) ? Campaign.filename : ""});
	return lists;
}

struct Header {
	SCP_string mission;
	ushort checksum = 0;
	uint32_t schema = 0;
	NameLists tables;
	NameLists structure;
};

// The data after the first 16 bytes (magic, version, length, hash) is covered by its length and an FNV-1a hash, so
// truncated or damaged checkpoints are refused before anything is changed.
constexpr size_t INTEGRITY_END = 16;

uint32_t payload_hash(const Buffer& buffer)
{
	uint32_t hash = 2166136261u;
	if (buffer.size() > INTEGRITY_END)
		mix(hash, buffer.data() + INTEGRITY_END, buffer.size() - INTEGRITY_END);
	return hash;
}

void seal(Buffer& buffer)
{
	uint32_t length = static_cast<uint32_t>(buffer.size() - INTEGRITY_END);
	uint32_t hash = payload_hash(buffer);
	memcpy(buffer.data() + 8, &length, sizeof(length));
	memcpy(buffer.data() + 12, &hash, sizeof(hash));
}

bool is_intact(const Buffer& buffer)
{
	if (buffer.size() < INTEGRITY_END)
		return false;
	uint32_t length, hash;
	memcpy(&length, buffer.data() + 8, sizeof(length));
	memcpy(&hash, buffer.data() + 12, sizeof(hash));
	return length == buffer.size() - INTEGRITY_END && hash == payload_hash(buffer);
}

void save_header(Writer& w)
{
	w.u32(MAGIC);
	w.u32(VERSION);
	w.u32(0);	// length and hash, filled in by seal()
	w.u32(0);
	save(w, SCP_string(Game_current_mission_filename));
	w.value(Current_file_checksum);
	w.u32(schema_fingerprint());
	save(w, table_names());
	save(w, Mission_structure);
}

bool read_header(Reader& r, Header& header)
{
	if (r.u32() != MAGIC || r.u32() != VERSION)
		return false;
	r.u32();	// length and hash, see is_intact()
	r.u32();
	load(r, header.mission);
	r.value(header.checksum);
	header.schema = r.u32();
	load(r, header.tables);
	load(r, header.structure);
	return !r.failed();
}

// Returns an empty string if the checkpoint fits the current mission and tables, or why not.
SCP_string check_header(const Buffer& buffer)
{
	Reader r(buffer);
	Header header;
	if (!read_header(r, header))
		return "the data is not a checkpoint of this version";
	if (!is_intact(buffer))
		return "the data is incomplete or damaged";
	if (stricmp(header.mission.c_str(), Game_current_mission_filename) != 0)
		return "it was saved in mission '" + header.mission + "'";

	// tables may have grown, but the entries saved data refers to must be where they were
	auto current_tables = table_names();
	for (const auto& saved : header.tables) {
		auto it = std::find_if(current_tables.begin(), current_tables.end(), [&saved](const auto& t) { return t.first == saved.first; });
		if (it == current_tables.end())
			continue;
		const auto& names = it->second;
		if (saved.second.size() > names.size() || !std::equal(saved.second.begin(), saved.second.end(), names.begin()))
			return "the entries of " + saved.first + " have changed since it was saved";
	}

	if (header.checksum != Current_file_checksum) {
		if (header.structure != Mission_structure)
			return "the mission has changed since it was saved";
		mprintf(("Checkpoint: the mission file has changed since the checkpoint was saved, but its structure is the same; "
				 "the checkpoint's state replaces the mission's\n"));
	}
	return {};
}

// Logs what a tolerant load could not restore exactly.
void report_load(const Header& header, const LoadContext& context)
{
	bool same_schema = (header.schema == schema_fingerprint());
	if (same_schema && context.unknown_fields == 0 && context.mismatched_fields == 0 && context.unknown_enumerators == 0)
		return;

	if (!same_schema)
		mprintf(("Checkpoint: saved by a build with a different data layout; loading it as far as it fits\n"));
	if (context.unknown_fields > 0)
		mprintf(("Checkpoint: %d saved fields are unknown to this build and were skipped\n", context.unknown_fields));
	if (context.mismatched_fields > 0)
		mprintf(("Checkpoint: %d saved fields changed size and kept their current value\n", context.mismatched_fields));
	if (context.unknown_enumerators > 0)
		mprintf(("Checkpoint: %d saved enumerator names are unknown to this build and were dropped\n", context.unknown_enumerators));
	if (!context.missing_fields.empty()) {
		SCP_string names;
		for (const auto& name : context.missing_fields)
			names += (names.empty() ? "" : ", ") + name;
		mprintf(("Checkpoint: these fields were not in the saved data and kept their current value: %s\n", names.c_str()));
	}
}

// --- saving ----------------------------------------------------------------------------------------------------------

Buffer save_checkpoint(Trace* trace, const SCP_vector<SCP_string>& excluded = {}, const SCP_string& name = {})
{
	// objects created this frame are still on the create list; they are saved like all others
	obj_merge_created_list();

	Buffer buffer;
	Writer w(buffer, trace);
	size_t at;

	save_header(w);

	begin_section(w, "clock", at);
	w.value(timestamp_get_checkpoint_state());
	w.end_block(at);

	begin_section(w, "globals", at);
	{
		TraceScope scope(w, "globals");
		for (const auto& handler : global_handlers()) {
			TraceScope global_scope(w, handler.name);
			w.u32(name_hash(handler.name));
			auto global_at = w.begin_block();
			handler.save(w);
			if (trace != nullptr)
				trace->entries.push_back({trace->current_path(), global_at + sizeof(uint32_t), w.size()});
			w.end_block(global_at);
		}
	}
	w.end_block(at);

	begin_section(w, "objects", at);
	{
		SCP_vector<object*> saved;
		for (auto objp : list_range(&obj_used_list)) {
			auto handler = find_object_type_handler(objp->type);
			if (handler != nullptr && (handler->is_saved == nullptr || handler->is_saved(*objp)))
				saved.push_back(objp);
		}
		w.u32(static_cast<uint32_t>(saved.size()));
		for (auto objp : saved) {
			TraceScope scope(w, "objects[" + std::to_string(OBJ_INDEX(objp)) + "]");
			w.value(static_cast<int32_t>(OBJ_INDEX(objp)));
			w.value(static_cast<int32_t>(objp->type));
			save(w, *objp);
			auto instance_at = w.begin_block();
			find_object_type_handler(objp->type)->save(w, *objp);
			w.end_block(instance_at);
		}

		// the order of free slots decides which slots objects created after loading get
		SCP_vector<int> free_slots;
		for (auto objp : list_range(&obj_free_list))
			free_slots.push_back(OBJ_INDEX(objp));
		save(w, free_slots);
	}
	w.end_block(at);

	// the random number generator goes last, so nothing that is saved after it can use it
	// the names to put back to their mission-start state when loading
	begin_section(w, "excluded", at);
	save(w, excluded);
	w.end_block(at);

	// read without restoring anything, to list checkpoints; see read_name()
	begin_section(w, "name", at);
	save(w, name);
	w.end_block(at);

	begin_section(w, "random", at);
	save(w, SCP_string(util::Random::save_state()));
	w.end_block(at);

	seal(buffer);
	return buffer;
}

// --- restoring -------------------------------------------------------------------------------------------------------

// Removes everything the mission reload created, so the saved objects can take their slots.
void purge_objects()
{
	// the player ship would otherwise be turned into a ghost instead of being deleted
	if (Player_ship != nullptr)
		ship_close_cockpit_displays(Player_ship);
	Player_obj = nullptr;
	Player_ship = nullptr;
	Player_ai = nullptr;

	// beams refer to the ships firing them, so they go first and ships go last
	beam_delete_all();

	for (int pass = 0; pass < 2; ++pass) {
		bool ships = (pass == 1);
		for (int objnum = 0; objnum < MAX_OBJECTS; ++objnum) {
			auto& objp = Objects[objnum];
			if (objp.type == OBJ_NONE || ((objp.type == OBJ_SHIP) != ships))
				continue;
			obj_delete(objnum);
		}
	}

	obj_reset_colliders();
}

void restore_free_slot_order(const SCP_vector<int>& free_slots)
{
	for (int objnum : free_slots) {
		if (objnum < 0 || objnum >= MAX_OBJECTS || Objects[objnum].type != OBJ_NONE)
			continue;
		list_remove(&obj_free_list, &Objects[objnum]);
		list_append(&obj_free_list, &Objects[objnum]);
	}
}

void restore_player_pointers()
{
	if (Player == nullptr || Player->objnum < 0 || Player->objnum >= MAX_OBJECTS)
		return;

	Player_obj = &Objects[Player->objnum];
	if (Player_obj->type == OBJ_SHIP || Player_obj->type == OBJ_GHOST) {
		Player_ship = &Ships[Player_obj->instance];
		Player_ai = &Ai_info[Player_ship->ai_index];

		// entering the mission set these up for the player ship the reload created, whose class may have differed
		set_current_hud();
		ship_init_cockpit_displays(Player_ship);
		hud_target_clear_display_list();
	}
}

// Restores a checkpoint into the freshly loaded mission.  Sets mission_changed once it starts changing the mission, so a
// failure after that point can be told from one that left the mission untouched.
// Checks the saved object table before anything is changed: every object has a slot of its own, a type that can be
// restored and an instance in range.
SCP_string check_objects(Reader objects)
{
	SCP_vector<bool> taken(MAX_OBJECTS, false);
	auto count = objects.u32();
	for (uint32_t i = 0; i < count && !objects.failed(); ++i) {
		int32_t objnum = -1;
		int32_t type = OBJ_NONE;
		objects.value(objnum);
		objects.value(type);
		auto handler = find_object_type_handler(type);
		if (handler == nullptr)
			return "object " + std::to_string(objnum) + " has a type that can't be restored";
		if (objnum < 0 || objnum >= MAX_OBJECTS || taken[objnum])
			return "object slot " + std::to_string(objnum) + " is invalid";
		taken[objnum] = true;

		static object scratch;
		scratch.clear();
		load(objects, scratch);
		if (handler->max_instances > 0 && (scratch.instance < 0 || scratch.instance >= handler->max_instances))
			return "object " + std::to_string(objnum) + " has an invalid instance";
		objects.block();
	}
	if (objects.failed())
		return "the object table is damaged";
	return {};
}

bool restore_checkpoint(const Buffer& buffer, bool& mission_changed)
{
	LoadContext context;
	Reader r(buffer, &context);

	Header header;
	if (!read_header(r, header))
		return false;
	auto sections = read_sections(r);
	if (r.failed())
		return false;

	auto section = [&sections](const char* name) -> Reader* {
		auto it = sections.find(name_hash(name));
		return (it == sections.end()) ? nullptr : &it->second;
	};
	auto clock = section("clock");
	auto globals = section("globals");
	auto objects = section("objects");
	auto random = section("random");
	if (clock == nullptr || globals == nullptr || objects == nullptr || random == nullptr)
		return false;

	auto problem = check_objects(*objects);
	if (!problem.empty()) {
		Warning(LOCATION, "The checkpoint cannot be loaded: %s.", problem.c_str());
		return false;
	}

	// what is excluded or not checkpointed is taken from the freshly loaded mission
	SCP_vector<SCP_string> excluded;
	if (auto excluded_section = section("excluded"))
		load(*excluded_section, excluded);
	capture_fresh_state(excluded);

	mission_changed = true;
	purge_objects();

	// timestamps are restored as they were, so the clock must be set before anything reads it
	timestamp_checkpoint_state clock_state;
	clock->value(clock_state);
	timestamp_restore_checkpoint_state(clock_state);
	Missiontime = timestamp_get_mission_time();

	// globals
	{
		SCP_unordered_map<uint32_t, Reader> saved_globals;
		while (!globals->at_end() && !globals->failed()) {
			auto hash = globals->u32();
			saved_globals.emplace(hash, globals->block());
		}
		for (const auto& handler : global_handlers()) {
			auto it = saved_globals.find(name_hash(handler.name));
			if (it != saved_globals.end())
				handler.load(it->second);
			else
				mprintf(("Checkpoint: no saved state for %s, it keeps its mission start value\n", handler.name));
		}
		if (saved_globals.size() > global_handlers().size())
			mprintf(("Checkpoint: %d saved globals are unknown to this build\n", static_cast<int>(saved_globals.size() - global_handlers().size())));
		for (const auto& function : after_globals_functions())
			function.second();
	}

	// objects: each one is put back into its slot, then its type loads the rest and re-acquires its resources
	SCP_vector<int> restored;
	{
		auto count = objects->u32();
		for (uint32_t i = 0; i < count && !objects->failed(); ++i) {
			int32_t objnum = -1;
			int32_t type = OBJ_NONE;
			objects->value(objnum);
			objects->value(type);

			auto handler = find_object_type_handler(type);
			if (handler == nullptr || objnum < 0 || objnum >= MAX_OBJECTS || !obj_allocate_at(objnum)) {
				Warning(LOCATION, "Checkpoint: object %d of type %d cannot be restored.", objnum, type);
				return false;
			}

			auto& objp = Objects[objnum];
			objp.clear();
			load(*objects, objp);
			// collision pairs are rebuilt when the object joins the used list
			objp.flags.set(Object::Object_Flags::Not_in_coll);

			auto instance = objects->block();
			handler->restore(instance, objp);
			restored.push_back(objnum);
		}

		SCP_vector<int> free_slots;
		load(*objects, free_slots);
		restore_free_slot_order(free_slots);
	}
	if (objects->failed())
		return false;

	context.run_fixups();
	obj_merge_created_list();
	restore_player_pointers();
	apply_fresh_state();

	for (int objnum : restored) {
		auto handler = find_object_type_handler(Objects[objnum].type);
		if (handler != nullptr && handler->after_restore != nullptr)
			handler->after_restore(Objects[objnum]);
	}

	for (const auto& function : after_restore_functions())
		function.second();

	report_load(header, context);

	// restoring used random numbers, so the generator is restored last
	SCP_string random_state;
	load(*random, random_state);
	util::Random::load_state(random_state);

	return !random->failed();
}

// --- verification ----------------------------------------------------------------------------------------------------

void report(const SCP_string& text)
{
	mprintf(("Checkpoint verify: %s\n", text.c_str()));
	dc_printf("Checkpoint verify: %s\n", text.c_str());
}

void compare_saves(const Buffer& a, const Trace& a_trace, const Buffer& b, const Trace& b_trace)
{
	SCP_unordered_map<SCP_string, const Trace::Entry*> b_entries;
	for (const auto& entry : b_trace.entries)
		b_entries.emplace(entry.path, &entry);

	int differences = 0;
	int missing = 0;
	constexpr int MAX_REPORTED = 50;

	for (const auto& entry : a_trace.entries) {
		auto it = b_entries.find(entry.path);
		if (it == b_entries.end()) {
			if (missing++ < MAX_REPORTED)
				report("only in the first save: " + entry.path);
			continue;
		}
		auto other = it->second;
		bool same = (entry.end - entry.begin == other->end - other->begin)
			&& memcmp(&a[entry.begin], &b[other->begin], entry.end - entry.begin) == 0;
		if (!same && differences++ < MAX_REPORTED)
			report("differs: " + entry.path);
		b_entries.erase(it);
	}
	for (const auto& entry : b_entries) {
		if (missing++ < MAX_REPORTED)
			report("only in the second save: " + entry.first);
	}

	report(std::to_string(differences) + " fields differ, " + std::to_string(missing) + " fields exist in only one save, "
		+ std::to_string(a_trace.entries.size()) + " fields compared");
}

void start_stepping()
{
	Stepping = true;
	Step_clock = timestamp_get_checkpoint_state().microseconds;
}

// Reads one registered global from a saved checkpoint.
// The name a checkpoint was saved with; empty for one without a name, or one saved before names existed.
SCP_string read_name(const Buffer& buffer)
{
	Reader r(buffer);
	Header header;
	if (!read_header(r, header))
		return {};
	auto sections = read_sections(r);
	auto it = sections.find(name_hash("name"));
	if (r.failed() || it == sections.end())
		return {};
	SCP_string name;
	load(it->second, name);
	return it->second.failed() ? SCP_string() : name;
}

template <typename T>
bool read_saved_global(const Buffer& buffer, const char* name, T& value)
{
	Reader r(buffer);
	Header header;
	if (!read_header(r, header))
		return false;
	auto sections = read_sections(r);
	auto it = sections.find(name_hash("globals"));
	if (r.failed() || it == sections.end())
		return false;
	auto& globals = it->second;
	while (!globals.at_end() && !globals.failed()) {
		auto hash = globals.u32();
		auto block = globals.block();
		if (hash == name_hash(name)) {
			load(block, value);
			return !block.failed();
		}
	}
	return false;
}

// Which ships and wings are still in the mission or yet to arrive, by name
template <typename Registry, typename WingArray>
void collect_status(const Registry& registry, const WingArray& wings, int num_wings,
	SCP_unordered_map<SCP_string, bool, SCP_string_lcase_hash, SCP_string_lcase_equal_to>& status)
{
	status.clear();
	for (const auto& entry : registry)
		status[entry.name] = (entry.status != ShipStatus::EXITED);
	for (int i = 0; i < num_wings; ++i)
		status[wings[i].name] = !wings[i].flags[Ship::Wing_Flags::Gone];
}

void apply_assist()
{
	auto raise = [](int& ammo, int capacity) {
		ammo = MAX(ammo, fl2i(ceilf(capacity * Assist.min_ammo / 100.0f)));
	};
	auto raise_banks = [&raise](ship_weapon& swp) {
		for (int i = 0; i < swp.num_primary_banks; ++i) {
			int weapon = swp.primary_bank_weapons[i];
			if (weapon >= 0 && Weapon_info[weapon].wi_flags[Weapon::Info_Flags::Ballistic])
				raise(swp.primary_bank_ammo[i], swp.primary_bank_start_ammo[i]);
		}
		for (int i = 0; i < swp.num_secondary_banks; ++i) {
			if (swp.secondary_bank_weapons[i] >= 0)
				raise(swp.secondary_bank_ammo[i], swp.secondary_bank_start_ammo[i]);
		}
	};

	for (const auto& name : Assist.ships) {
		int shipnum = ship_name_lookup(name.c_str(), 1);
		if (shipnum < 0)
			continue;
		auto shipp = &Ships[shipnum];
		auto objp = &Objects[shipp->objnum];
		objp->hull_strength = MAX(objp->hull_strength, shipp->ship_max_hull_strength * Assist.min_hull / 100.0f);
		raise_banks(shipp->weapons);
		for (auto ss : list_range(&shipp->subsys_list)) {
			if (ss->system_info != nullptr && ss->system_info->type == SUBSYSTEM_TURRET)
				raise_banks(ss->weapons);
		}
	}
}

void start_load(Buffer buffer)
{
	Pending_restore = std::move(buffer);
	set_loading(true);
	gameseq_post_event(GS_EVENT_START_GAME_QUICK);
}

void handle_request(const Request& request)
{
	// Requests come from sexps and scripts too, so nothing here may use dc_printf: the console's buffer is empty until
	// the console has been opened once.
	if (Game_mode & GM_MULTIPLAYER) {
		mprintf(("Checkpoint: checkpoints are not available in multiplayer\n"));
		return;
	}

	switch (request.type) {
	case RequestType::Save: {
		// scripts store what they need in the checkpoint now
		if (scripting::hooks::OnCheckpointSave->isActive())
			scripting::hooks::OnCheckpointSave->run(scripting::hook_param_list(scripting::hook_param("Index", 'i', request.slot)));
		auto name = request.name.empty() ? Pending_name : request.name;
		Pending_name.clear();
		auto buffer = save_checkpoint(nullptr, request.excluded, name);
		if (store_save(request.slot, buffer))
			mprintf(("Checkpoint: saved checkpoint %d (%d bytes)\n", request.slot, static_cast<int>(buffer.size())));
		else
			Warning(LOCATION, "Checkpoint %d could not be written to disk.", request.slot);
		break;
	}

	case RequestType::Load: {
		Buffer buffer;
		if (!store_load(request.slot, buffer)) {
			mprintf(("Checkpoint: there is no checkpoint %d in this mission\n", request.slot));
			return;
		}
		auto problem = check_header(buffer);
		if (!problem.empty()) {
			Warning(LOCATION, "Checkpoint %d cannot be loaded: %s.", request.slot, problem.c_str());
			return;
		}
		Pending_index = request.slot;
		Pending_replay_event = request.replay_event;
		start_load(std::move(buffer));
		break;
	}

	case RequestType::Verify:
		Pending_index = Loaded_index;	// verifying doesn't change which checkpoint the mission was resumed from
		Pending_replay_event = -1;
		Verify = VerifyRun();
		Verify.frames = request.frames;
		if (request.frames <= 0) {
			Verify.original = save_checkpoint(&Verify.original_trace);
			Verify.stage = VerifyStage::AwaitingRestore;
			start_load(Verify.original);
		} else {
			Verify.start = save_checkpoint(nullptr);
			Verify.frames_left = request.frames;
			Verify.stage = VerifyStage::RunningOriginal;
			start_stepping();
		}
		break;

	case RequestType::None:
		break;
	}
}

} // namespace

// --- registration ----------------------------------------------------------------------------------------------------

void register_global(GlobalHandler handler)
{
	auto& handlers = global_handlers();
	for (const auto& existing : handlers) {
		Assertion(name_hash(existing.name) != name_hash(handler.name), "Two checkpoint globals are registered as '%s' and '%s'; give one of them a different name.", existing.name, handler.name);
	}

	// keep the handlers sorted, so saves don't depend on the order in which files register their globals
	auto it = std::lower_bound(handlers.begin(), handlers.end(), handler, [](const GlobalHandler& a, const GlobalHandler& b) {
		return strcmp(a.name, b.name) < 0;
	});
	handlers.insert(it, std::move(handler));
}

namespace {

// functions run in the order of their names, so the order doesn't depend on the order in which files register them
void insert_sorted(SCP_vector<std::pair<const char*, std::function<void()>>>& functions, const char* name, std::function<void()> function)
{
	auto it = std::lower_bound(functions.begin(), functions.end(), name, [](const std::pair<const char*, std::function<void()>>& a, const char* b) {
		return strcmp(a.first, b) < 0;
	});
	functions.emplace(it, name, std::move(function));
}

} // namespace

void register_after_globals(const char* name, std::function<void()> function)
{
	insert_sorted(after_globals_functions(), name, std::move(function));
}

void register_after_restore(const char* name, std::function<void()> function)
{
	insert_sorted(after_restore_functions(), name, std::move(function));
}

void register_object_type(ObjectTypeHandler handler)
{
	Assertion(find_object_type_handler(handler.type) == nullptr, "Object type %d is registered for checkpoints twice.", handler.type);
	object_type_handlers().push_back(handler);
}

// --- public interface ------------------------------------------------------------------------------------------------

void request_save(int index, SCP_vector<SCP_string> excluded, SCP_string name)
{
	Pending_request = {RequestType::Save, index, 0, std::move(excluded), -1, std::move(name)};
}

void set_pending_name(const SCP_string& name)
{
	Pending_name = name;
}

SCP_string slot_name(int index)
{
	Buffer buffer;
	if (!store_load(index, buffer))
		return {};
	return read_name(buffer);
}

SCP_vector<std::pair<int, SCP_string>> list_checkpoints(int max)
{
	SCP_vector<std::pair<int, SCP_string>> list;
	SCP_map<int, Buffer> slots;
	store_load_all(slots);
	for (const auto& slot : slots) {
		if (static_cast<int>(list.size()) >= max)
			break;
		// a checkpoint that can't be loaded any more, e.g. after the mission was changed, isn't offered
		if (slot.first < 1 || !check_header(slot.second).empty())
			continue;
		auto name = read_name(slot.second);
		if (name.empty()) {
			char buf[NAME_LENGTH + 32];
			sprintf(buf, XSTR("Checkpoint %d", -1), slot.first);
			name = buf;
		}
		list.emplace_back(slot.first, std::move(name));
	}
	return list;
}

bool request_load(int index, int replay_event)
{
	// An event that loads a checkpoint as soon as one exists, e.g. when savestate-check 1 -> savestate-load 1, may
	// still have been waiting when the checkpoint was saved, and would load it again right away, endlessly.
	if (First_frame_after_restore) {
		Warning(LOCATION, "A checkpoint load was requested in the first frame after loading checkpoint %d, and is ignored. "
			"The event requesting it probably fires again after each load; flag it \"not checkpointed\" or check the loaded index.",
			Loaded_index);
		return false;
	}
	if (!store_has(index, nullptr)) {
		mprintf(("Checkpoint: there is no checkpoint %d in this mission\n", index));
		return false;
	}
	Pending_request = {RequestType::Load, index, 0, {}, replay_event, {}};
	return true;
}

bool is_replaying_event()
{
	return Replaying_event;
}

void request_verify(int frames)
{
	Pending_request = {RequestType::Verify, 0, frames, {}, -1, {}};
}

bool has_slot(int index, const char* mission)
{
	return store_has(index, mission);
}

void clear_slot(int index)
{
	store_clear(index);
}

int next_free_slot()
{
	return store_next_free_index();
}

int loaded_index()
{
	return Loaded_index;
}

bool ship_status(const char* name, int index)
{
	// without an index, the checkpoint the mission was resumed from
	if (index <= 0) {
		if (Loaded_index == 0)
			return true;
		auto it = Loaded_status.find(name);
		return (it == Loaded_status.end()) ? true : it->second;
	}

	Buffer buffer;
	if (!store_load(index, buffer))
		return true;
	SCP_vector<ship_registry_entry> registry;
	static wing wings[MAX_WINGS];
	int num_wings = 0;
	read_saved_global(buffer, "Ship_registry", registry);
	read_saved_global(buffer, "Wings", wings);
	read_saved_global(buffer, "Num_wings", num_wings);
	SCP_unordered_map<SCP_string, bool, SCP_string_lcase_hash, SCP_string_lcase_equal_to> status;
	collect_status(registry, wings, MIN(num_wings, MAX_WINGS), status);
	auto it = status.find(name);
	return (it == status.end()) ? true : it->second;
}

void assist(float min_hull, float min_ammo, const SCP_vector<SCP_string>& ships)
{
	Assist.min_hull = min_hull;
	Assist.min_ammo = min_ammo;
	for (const auto& name : ships) {
		bool known = std::any_of(Assist.ships.begin(), Assist.ships.end(), [&name](const SCP_string& s) { return !stricmp(s.c_str(), name.c_str()); });
		if (!known)
			Assist.ships.push_back(name);
	}
}

void set_script_data(const SCP_string& key, const SCP_string& value)
{
	Script_data[key] = value;
}

bool get_script_data(const SCP_string& key, SCP_string& value)
{
	auto it = Script_data.find(key);
	if (it == Script_data.end())
		return false;
	value = it->second;
	return true;
}

void process_requests()
{
	// the first frame after a restore is over: its events have run
	First_frame_after_restore = false;

	if (Verify.stage == VerifyStage::RunningOriginal || Verify.stage == VerifyStage::RunningRestored) {
		if (--Verify.frames_left <= 0) {
			Stepping = false;
			if (Verify.stage == VerifyStage::RunningOriginal) {
				Verify.original = save_checkpoint(&Verify.original_trace);
				Verify.stage = VerifyStage::AwaitingRestore;
				start_load(Verify.start);
			} else {
				Trace trace;
				auto current = save_checkpoint(&trace);
				compare_saves(Verify.original, Verify.original_trace, current, trace);
				Verify = VerifyRun();
			}
		}
		return;
	}

	if (Pending_request.type == RequestType::None || Loading)
		return;

	auto request = Pending_request;
	Pending_request = Request();
	handle_request(request);
}

void apply_pending_restore()
{
	if (!Pending_restore)
		return;

	auto buffer = std::move(*Pending_restore);
	Pending_restore.reset();

	bool restored = false;
	bool mission_changed = false;
	auto problem = check_header(buffer);
	if (problem.empty())
		restored = restore_checkpoint(buffer, mission_changed);
	else
		Warning(LOCATION, "The checkpoint cannot be loaded: %s.", problem.c_str());

	if (!restored) {
		// The data was checked before the mission was cleared, so failing after that point means the checkpoint
		// doesn't match what this build expects, and the mission can't be put back together.
		if (mission_changed)
			Error(LOCATION, "Restoring the checkpoint failed after the mission was cleared; the game cannot continue.");
		Verify = VerifyRun();
		Pending_replay_event = -1;
		set_loading(false);
		Warning(LOCATION, "Restoring the checkpoint failed; the mission continues from its start.");
		return;
	}
	set_loading(false);

	Loaded_index = Pending_index;
	First_frame_after_restore = true;
	Pending_name.clear();
	collect_status(Ship_registry, Wings, Num_wings, Loaded_status);
	apply_assist();
	if (scripting::hooks::OnCheckpointLoad->isActive())
		scripting::hooks::OnCheckpointLoad->run(scripting::hook_param_list(scripting::hook_param("Index", 'i', Loaded_index)));

	// A "not checkpointed" event that loaded the checkpoint was reset with the other such events; it runs again on the
	// loaded mission, as it did when SaveLoadX applied a save state, and then counts as having fired.
	int replay_event = Pending_replay_event;
	Pending_replay_event = -1;
	if (SCP_vector_inbounds(Mission_events, replay_event)) {
		Replaying_event = true;
		sexp_replay_event_actions(replay_event);
		mission_event_set_fired(replay_event);
		Replaying_event = false;
	}

	if (Verify.stage == VerifyStage::AwaitingRestore) {
		if (Verify.frames <= 0) {
			Trace trace;
			auto current = save_checkpoint(&trace);
			compare_saves(Verify.original, Verify.original_trace, current, trace);
			Verify = VerifyRun();
		} else {
			Verify.frames_left = Verify.frames;
			Verify.stage = VerifyStage::RunningRestored;
			start_stepping();
		}
	}
}

void record_mission_structure()
{
	Mission_structure = mission_structure();

	// a mission started without a checkpoint has nothing loaded, nothing to assist and no script data
	if (!Loading) {
		Loaded_index = 0;
		First_frame_after_restore = false;
		Loaded_status.clear();
		Assist = AssistSettings();
		Script_data.clear();
		Pending_name.clear();
	}
}

bool is_loading()
{
	return Loading;
}

bool override_frametime(fix& frametime)
{
	if (!Stepping)
		return false;

	frametime = STEP_FRAMETIME;
	Step_clock += static_cast<uint64_t>(f2fl(STEP_FRAMETIME) * MICROSECONDS_PER_SECOND);

	auto state = timestamp_get_checkpoint_state();
	state.microseconds = Step_clock;
	timestamp_restore_checkpoint_state(state);
	return true;
}

} // namespace checkpoint

// --- console commands ------------------------------------------------------------------------------------------------

DCF(checkpoint_save, "Saves a checkpoint of the running mission (usage: checkpoint_save <slot>)")
{
	int slot = 0;
	dc_maybe_stuff_int(&slot);
	checkpoint::request_save(slot);
}

DCF(checkpoint_load, "Loads a checkpoint saved in this mission (usage: checkpoint_load <slot>)")
{
	int slot = 0;
	dc_maybe_stuff_int(&slot);
	checkpoint::request_load(slot);
}

DCF(checkpoint_verify, "Checks that saving and loading a checkpoint restores everything (usage: checkpoint_verify [frames])")
{
	int frames = 0;
	dc_maybe_stuff_int(&frames);
	checkpoint::request_verify(frames);
}
