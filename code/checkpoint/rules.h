#pragma once

// The type rules of the checkpoint codec.  Everything that is not plain data, a container or a scanned struct needs
// exactly one rule here; see checkpoint/codec_impl.h for what each extension point means.  Only
// checkpoint/checkpoint_fields.cpp includes this file.
//
// Structs and enums are scanned when the line naming them in their definition ends with a marker:
//
//     class ship		// @checkpoint fields
//
// Every member of a scanned struct is saved, unless its type is transient here or the member is marked in its header
// at the end of its line, on its own or after the member's comment:
//
//     int model_instance_num;		// @checkpoint skip: re-acquired after loading
//
// Guidelines:
// - Type rules are only for types with a single meaning.  Generic types (ints, vectors, pointers to scalars, generic
//   wrappers) never get a rule; a member of such a type that must not be saved is skipped at the member.
// - CHECKPOINT_RAW is for trivially copyable types without pointers or resource handles.  Prefer marking a struct's
//   definition with "// @checkpoint fields", which tolerates changes to the struct between versions.
// - is_transient is for types that are rebuilt on load (resources, caches, links) and never saved.
// - A rule must come after the rules of the types it saves, since each macro instantiates its type where it stands.

#include "checkpoint/codec_impl.h"

#include "ai/ai.h"
#include "ai/ai_profiles.h"
#include "ai/aigoals.h"
#include "asteroid/asteroid.h"
#include "bmpman/bmpman.h"
#include "debris/debris.h"
#include "graphics/generic.h"
#include "hud/hudtarget.h"
#include "jumpnode/jumpnode.h"
#include "globalincs/version.h"
#include "mission/missiongoals.h"
#include "mission/missionlog.h"
#include "mission/missionparse.h"
#include "model/animation/modelanimation.h"
#include "model/model.h"
#include "nebula/volumetrics.h"
#include "network/multi_options.h"
#include "object/object.h"
#include "object/objectdock.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"
#include "playerman/player.h"
#include "prop/prop.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "sound/sound.h"
#include "stats/scoring.h"
#include "weapon/beam.h"
#include "weapon/shockwave.h"
#include "weapon/swarm.h"
#include "weapon/trails.h"
#include "weapon/weapon.h"

extern shockwave Shockwaves[];

namespace checkpoint {

// --- raw plain data --------------------------------------------------------------------------------------------------

CHECKPOINT_RAW(vec3d);
CHECKPOINT_RAW(matrix);
CHECKPOINT_RAW(angles);
CHECKPOINT_RAW(TIMESTAMP);
CHECKPOINT_RAW(animation::ModelAnimationSet::RunningAnimationState);
CHECKPOINT_RAW(anchor_t);
CHECKPOINT_RAW(gamesnd_id);
CHECKPOINT_RAW(control_info);
CHECKPOINT_RAW(button_info);
CHECKPOINT_RAW(beam_info);
CHECKPOINT_RAW(ship_spark);
CHECKPOINT_RAW(ship_subsys_info);
CHECKPOINT_RAW(guard_range_entry);
CHECKPOINT_RAW(debris_electrical_arc);
CHECKPOINT_RAW(modular_curves_entry_instance);
CHECKPOINT_RAW(alt_class);
CHECKPOINT_RAW(multi_local_options);
CHECKPOINT_RAW(multi_server_options);
CHECKPOINT_RAW(gameversion::version);
CHECKPOINT_RAW(sound_env);
CHECKPOINT_RAW(color);

// --- transient types: rebuilt on load, never saved ------------------------------------------------------------------

template <> struct is_transient<sound_handle> : std::true_type {};
template <> struct is_transient<std::unique_ptr<WarpEffect>> : std::true_type {};		// re-created by ship_set_warp_effects
template <> struct is_transient<std::unique_ptr<sexp_cached_data>> : std::true_type {};	// evaluation cache
template <> struct is_transient<ship_subsys_sentinel> : std::true_type {};				// rebuilt with the subsystem list
template <> struct is_transient<beam_collision> : std::true_type {};					// recomputed every frame
template <> struct is_transient<ship_electrical_arc> : std::true_type {};				// visual effect
template <> struct is_transient<model_electrical_arc> : std::true_type {};				// visual effect
template <> struct is_transient<trail*> : std::true_type {};							// trails are re-created
template <> struct is_transient<submodel_instance*> : std::true_type {};				// set up with the model instance
template <> struct is_transient<mc_info*> : std::true_type {};							// collision scratch data
template <> struct is_transient<htarget_list*> : std::true_type {};					// hotkey list links

// --- factories for types without a default constructor ---------------------------------------------------------------

template <>
struct Factory<ship_registry_entry> {
	static ship_registry_entry make() { return ship_registry_entry(""); }
};
CHECKPOINT_FACTORY(ship_registry_entry);

// --- pools -----------------------------------------------------------------------------------------------------------

template <>
struct PoolTraits<object> {
	// the used object list is a sentinel for "no object" in places, e.g. weapon::homing_object
	static constexpr int USED_LIST = -2;

	static int index_of(const object* p) { return (p == &obj_used_list) ? USED_LIST : OBJ_INDEX(p); }
	static object* at(int index)
	{
		if (index == USED_LIST)
			return &obj_used_list;
		return (index >= 0 && index < MAX_OBJECTS) ? &Objects[index] : nullptr;
	}
};
CHECKPOINT_POOL(object);

template <>
struct PoolTraits<beam> {
	static int index_of(const beam* p) { return BEAM_INDEX(p); }
	static beam* at(int index) { return (index >= 0 && index < MAX_BEAMS) ? &Beams[index] : nullptr; }
};
CHECKPOINT_POOL(beam);

template <>
struct PoolTraits<shockwave> {
	static int index_of(const shockwave* p) { return static_cast<int>(p - Shockwaves); }
	static shockwave* at(int index) { return (index >= 0 && index < MAX_SHOCKWAVES) ? &Shockwaves[index] : nullptr; }
};
CHECKPOINT_POOL(shockwave);

template <>
struct PoolTraits<ai_profile_t> {
	static int index_of(const ai_profile_t* p) { return AI_PROFILES_INDEX(p); }
	static ai_profile_t* at(int index) { return (index >= 0 && index < MAX_AI_PROFILES) ? &Ai_profiles[index] : nullptr; }
};
CHECKPOINT_POOL(ai_profile_t);

template <>
struct PoolTraits<traitor_override_t> {
	static int index_of(const traitor_override_t* p) { return static_cast<int>(p - Traitor_overrides.data()); }
	static traitor_override_t* at(int index) { return SCP_vector_inbounds(Traitor_overrides, index) ? &Traitor_overrides[index] : nullptr; }
};
CHECKPOINT_POOL(traitor_override_t);

template <>
struct PoolTraits<p_object> {
	// the support ship has its own parse object, outside Parse_objects
	static constexpr int SUPPORT_SHIP = -2;

	static int index_of(const p_object* p) { return (p == &Support_ship_pobj) ? SUPPORT_SHIP : POBJ_INDEX(p); }
	static p_object* at(int index)
	{
		if (index == SUPPORT_SHIP)
			return &Support_ship_pobj;
		return SCP_vector_inbounds(Parse_objects, index) ? &Parse_objects[index] : nullptr;
	}
};
CHECKPOINT_POOL(p_object);

template <>
struct PoolTraits<pnode> {
	// the free pointer of the path points may point just past them
	static int index_of(const pnode* p) { return static_cast<int>(p - Path_points); }
	static pnode* at(int index) { return (index >= 0 && index <= MAX_PATH_POINTS) ? &Path_points[index] : nullptr; }
};
CHECKPOINT_POOL(pnode);

// --- custom codecs ---------------------------------------------------------------------------------------------------

/**
 * Subsystems live in a heap pool and are re-allocated on load, so a pointer to one is saved as the object that owns it
 * plus its position in that ship's subsystem list, and resolved once all ships have their subsystems again.
 */
template <>
struct Rule<ship_subsys*> {
	static void save(Writer& w, ship_subsys* const& v)
	{
		// a pointer may outlive its ship, e.g. in a weapon fired by a destroyed turret; it is saved as null then
		int32_t objnum = -1;
		int32_t index = -1;
		if (v != nullptr && v->parent_objnum >= 0 && v->parent_objnum < MAX_OBJECTS) {
			const auto& parent = Objects[v->parent_objnum];
			if (parent.type == OBJ_SHIP || parent.type == OBJ_GHOST) {
				int position = 0;
				for (auto ss : list_range(&Ships[parent.instance].subsys_list)) {
					if (ss == v) {
						objnum = v->parent_objnum;
						index = position;
						break;
					}
					++position;
				}
			}
		}
		w.value(objnum);
		w.value(index);
	}

	static void load(Reader& r, ship_subsys*& v)
	{
		int32_t objnum = -1;
		int32_t index = -1;
		r.value(objnum);
		r.value(index);
		v = nullptr;
		if (objnum < 0 || index < 0 || r.context() == nullptr)
			return;
		auto target = &v;
		auto context = r.context();
		if (objnum >= MAX_OBJECTS)
			return;
		context->defer([target, objnum, index, context]() {
			const auto& objp = Objects[objnum];
			if (objp.type != OBJ_SHIP && objp.type != OBJ_GHOST) {
				*target = nullptr;
				return;
			}
			// the saved position refers to the saved subsystem list
			int current = index;
			auto it = context->subsystem_positions.find(objnum);
			if (it != context->subsystem_positions.end())
				current = SCP_vector_inbounds(it->second, index) ? it->second[index] : -1;
			*target = (current >= 0) ? ship_get_indexed_subsys(&Ships[objp.instance], current) : nullptr;
		});
	}
};
CHECKPOINT_RULE(ship_subsys*);

// Bitmap handles are not stable between sessions, so bitmaps are saved by name and loaded again.
template <>
struct Rule<generic_bitmap> {
	static void save(Writer& w, const generic_bitmap& v) { checkpoint::save(w, SCP_string(v.filename)); }
	static void load(Reader& r, generic_bitmap& v)
	{
		SCP_string filename;
		checkpoint::load(r, filename);
		strcpy_s(v.filename, filename.c_str());
		v.bitmap_id = filename.empty() ? -1 : bm_load(v.filename);
	}
};
CHECKPOINT_RULE(generic_bitmap);

// Volumetric nebulae are mission data and rendering resources; sexps can only switch them on and off.
template <>
struct Rule<volumetric_nebula> {
	static void save(Writer& w, const volumetric_nebula& v) { w.value(static_cast<ubyte>(v.get_enabled())); }
	static void load(Reader& r, volumetric_nebula& v)
	{
		ubyte enabled = 1;
		r.value(enabled);
		v.set_enabled(enabled != 0);
	}
};
CHECKPOINT_RULE(volumetric_nebula);

// The model instance of an object: the pose of its submodels, replaced textures and running animations.  Null for an
// object without one.  Loading fills the instance the object was just given, whose initial animation states (if any)
// are already set up; if it has none, or its model differs from the saved one, the saved state is dropped.
template <>
struct Rule<polymodel_instance*> {
	static void save(Writer& w, polymodel_instance* const& pmi)
	{
		auto at = w.begin_block();
		if (pmi != nullptr)
			save_state(w, *pmi);
		w.end_block(at);
	}

	static void load(Reader& r, polymodel_instance*& pmi)
	{
		auto block = r.block();
		if (pmi != nullptr && !block.at_end())
			load_state(block, *pmi);
		if (block.failed())
			r.fail();
	}

  private:
	static void save_state(Writer& w, const polymodel_instance& pmi)
	{
		auto pm = model_get(pmi.model_num);

		// the pose is only meaningful for the same model
		checkpoint::save(w, SCP_string(pm->filename));

		w.u32(static_cast<uint32_t>(pm->n_models));
		for (int i = 0; i < pm->n_models; ++i)
			checkpoint::save(w, pmi.submodel[i]);

		// replaced textures are saved by name, since bitmap handles differ between sessions
		SCP_vector<std::pair<int, SCP_string>> textures;
		if (pmi.texture_replace != nullptr) {
			for (int slot = 0; slot < static_cast<int>(pmi.texture_replace->size()); ++slot) {
				int handle = (*pmi.texture_replace)[slot];
				if (handle >= 0)
					textures.emplace_back(slot, bm_get_filename(handle));
			}
		}
		checkpoint::save(w, textures);

		checkpoint::save(w, animation::ModelAnimationSet::getRunningAnimationStates(&pmi));
	}

	static void load_state(Reader& r, polymodel_instance& pmi)
	{
		auto pm = model_get(pmi.model_num);

		SCP_string filename;
		checkpoint::load(r, filename);
		if (stricmp(filename.c_str(), pm->filename) != 0) {
			mprintf(("Checkpoint: the model %s has replaced %s; its saved pose and animations are dropped\n", pm->filename, filename.c_str()));
			return;
		}

		auto submodel_count = r.u32();
		for (uint32_t i = 0; i < submodel_count && !r.failed(); ++i) {
			if (static_cast<int>(i) < pm->n_models) {
				checkpoint::load(r, pmi.submodel[i]);
			} else {
				static submodel_instance discard;
				checkpoint::load(r, discard);
			}
		}

		SCP_vector<std::pair<int, SCP_string>> textures;
		checkpoint::load(r, textures);
		if (!textures.empty()) {
			pmi.texture_replace = std::make_shared<model_texture_replace>();
			for (const auto& texture : textures) {
				if (texture.first >= 0 && texture.first < static_cast<int>(pmi.texture_replace->size()))
					(*pmi.texture_replace)[texture.first] = bm_load_either(texture.second.c_str());
			}
		}

		SCP_vector<animation::ModelAnimationSet::RunningAnimationState> animations;
		checkpoint::load(r, animations);
		animation::ModelAnimationSet::restoreRunningAnimationStates(&pmi, animations);
	}
};
CHECKPOINT_RULE(polymodel_instance*);

CHECKPOINT_RAW(object_ship_wing_point_team);

// Lua values can't be saved; orders given by Lua AI sexps are rebuilt from their sexp text once the objects are back.
template <>
struct Rule<ai_lua_parameters> {
	// operator numbers change as operators are added, so the operator is saved by name
	static void save(Writer& w, const ai_lua_parameters& v)
	{
		int index = (v.sexp_op >= 0) ? find_operator_index(v.sexp_op) : -1;
		checkpoint::save(w, v.target);
		checkpoint::save(w, SCP_vector_inbounds(Operators, index) ? Operators[index].text : SCP_string());
		checkpoint::save(w, v.sexp_arguments);
	}
	static void load(Reader& r, ai_lua_parameters& v)
	{
		SCP_string op;
		checkpoint::load(r, v.target);
		checkpoint::load(r, op);
		checkpoint::load(r, v.sexp_arguments);
		v.sexp_op = op.empty() ? -1 : get_operator_const(op.c_str());
		if (r.context() != nullptr)
			r.context()->defer([&v] { ai_lua_parameters_rebuild(v); });
		else
			ai_lua_parameters_rebuild(v);
	}
};
CHECKPOINT_RULE(ai_lua_parameters);

template <typename T>
struct Rule<reload_pct<T>> {
	static void save(Writer& w, const reload_pct<T>& v)
	{
		checkpoint::save(w, v._points_per_bank);
		checkpoint::save(w, v._value);
		checkpoint::save(w, v._buffer);
	}
	static void load(Reader& r, reload_pct<T>& v)
	{
		checkpoint::load(r, v._points_per_bank);
		checkpoint::load(r, v._value);
		checkpoint::load(r, v._buffer);
	}
};

// Docking is saved from both sides as (my dock point, other object) and re-made once per pair after loading.
template <>
struct Extra<object> {
	static constexpr bool present = true;

	// each dock is saved as (own dock point, other object, other object's dock point)
	static void save(Writer& w, const object& v)
	{
		SCP_vector<std::tuple<int, int, int>> docks;
		for (auto dock = v.dock_list; dock != nullptr; dock = dock->next) {
			auto other = dock->docked_objp;
			docks.emplace_back(dock->dockpoint_used, OBJ_INDEX(other), dock_find_dockpoint_used_by_object(other, const_cast<object*>(&v)));
		}
		checkpoint::save(w, docks);
	}

	static void load(Reader& r, object& v)
	{
		SCP_vector<std::tuple<int, int, int>> docks;
		checkpoint::load(r, docks);
		if (r.context() == nullptr)
			return;
		auto objp = &v;
		for (const auto& [my_point, other, other_point] : docks) {
			if (other <= OBJ_INDEX(objp) || other >= MAX_OBJECTS || my_point < 0 || other_point < 0)
				continue;	// each pair is docked by the object with the lower index
			r.context()->defer([objp, my_point = my_point, other = other, other_point = other_point]() {
				auto other_objp = &Objects[other];
				if (other_objp->type == OBJ_NONE)
					return;
				dock_dock_objects(objp, my_point, other_objp, other_point);
			});
		}
	}
};

// The docker and dockee of a goal are a name or an index depending on the goal flags, and the target name points
// into a shared name table.
template <>
struct Extra<ai_goal> {
	static constexpr bool present = true;

	static void save(Writer& w, const ai_goal& v)
	{
		checkpoint::save(w, SCP_string(v.target_name != nullptr ? v.target_name : ""));
		save_name_or_index(w, v.docker.name, v.docker.index, v.flags[AI::Goal_Flags::Docker_index_valid]);
		save_name_or_index(w, v.dockee.name, v.dockee.index, v.flags[AI::Goal_Flags::Dockee_index_valid]);
	}

	static void load(Reader& r, ai_goal& v)
	{
		SCP_string target_name;
		checkpoint::load(r, target_name);
		v.target_name = target_name.empty() ? nullptr : ai_get_goal_target_name(target_name.c_str(), &v.target_name_index);
		load_name_or_index(r, v.docker.name, v.docker.index, v.flags[AI::Goal_Flags::Docker_index_valid]);
		load_name_or_index(r, v.dockee.name, v.dockee.index, v.flags[AI::Goal_Flags::Dockee_index_valid]);
	}

  private:
	static void save_name_or_index(Writer& w, const char* name, int index, bool is_index)
	{
		if (is_index)
			checkpoint::save(w, index);
		else
			checkpoint::save(w, SCP_string(name != nullptr ? name : ""));
	}

	static void load_name_or_index(Reader& r, const char*& name, int& index, bool is_index)
	{
		if (is_index) {
			checkpoint::load(r, index);
		} else {
			SCP_string loaded;
			checkpoint::load(r, loaded);
			int unused;
			name = loaded.empty() ? nullptr : ai_get_goal_target_name(loaded.c_str(), &unused);
		}
	}
};

} // namespace checkpoint
