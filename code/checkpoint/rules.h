#pragma once

// The type and field rules of the checkpoint codec.  Everything that is not plain data, a container or a scanned struct
// needs exactly one rule here; see checkpoint/codec.h for what each extension point means.
//
// Guidelines:
// - is_raw is for trivially copyable types without pointers or resource handles.  Prefer adding a struct to
//   checkpoint_structs.txt, which tolerates changes to the struct between versions.
// - is_transient is for types that are rebuilt on load (resources, caches, links) and never saved.
// - is_skipped is for single fields of that kind whose type is too general to mark, such as an int handle, and for
//   fields that are set from mission or table data when the mission is loaded.

#include "checkpoint/codec.h"

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

template <> struct is_raw<vec3d> : std::true_type {};
template <> struct is_raw<matrix> : std::true_type {};
template <> struct is_raw<angles> : std::true_type {};
template <> struct is_raw<TIMESTAMP> : std::true_type {};
template <> struct is_raw<animation::ModelAnimationSet::RunningAnimationState> : std::true_type {};
template <> struct is_raw<anchor_t> : std::true_type {};
template <> struct is_raw<gamesnd_id> : std::true_type {};
template <> struct is_raw<control_info> : std::true_type {};
template <> struct is_raw<button_info> : std::true_type {};
template <> struct is_raw<beam_info> : std::true_type {};
template <> struct is_raw<ship_spark> : std::true_type {};
template <> struct is_raw<ship_subsys_info> : std::true_type {};
template <> struct is_raw<guard_range_entry> : std::true_type {};
template <> struct is_raw<debris_electrical_arc> : std::true_type {};
template <> struct is_raw<modular_curves_entry_instance> : std::true_type {};
template <> struct is_raw<alt_class> : std::true_type {};
template <> struct is_raw<multi_local_options> : std::true_type {};
template <> struct is_raw<multi_server_options> : std::true_type {};
template <> struct is_raw<gameversion::version> : std::true_type {};
template <> struct is_raw<sound_env> : std::true_type {};
template <> struct is_raw<color> : std::true_type {};

// --- transient types: rebuilt on load, never saved ------------------------------------------------------------------

template <> struct is_transient<sound_handle> : std::true_type {};
template <> struct is_transient<std::unique_ptr<WarpEffect>> : std::true_type {};		// re-created by ship_set_warp_effects
template <> struct is_transient<std::unique_ptr<ship_subsys*[]>> : std::true_type {};	// subsystem index, rebuilt on demand
template <> struct is_transient<std::unique_ptr<sexp_cached_data>> : std::true_type {};	// evaluation cache
template <> struct is_transient<ship_subsys_sentinel> : std::true_type {};				// rebuilt with the subsystem list
template <> struct is_transient<util::event<void, object*>> : std::true_type {};		// Lua callbacks
template <> struct is_transient<util::reset_on_move<p_dock_instance*>> : std::true_type {};	// parse-time docking
template <> struct is_transient<beam_collision> : std::true_type {};					// recomputed every frame
template <> struct is_transient<ship_electrical_arc> : std::true_type {};				// visual effect
template <> struct is_transient<model_electrical_arc> : std::true_type {};				// visual effect
template <> struct is_transient<trail*> : std::true_type {};							// trails are re-created
template <> struct is_transient<submodel_instance*> : std::true_type {};				// set up with the model instance
template <> struct is_transient<mc_info*> : std::true_type {};							// collision scratch data
template <> struct is_transient<htarget_list*> : std::true_type {};					// hotkey list links

// --- skipped fields --------------------------------------------------------------------------------------------------

// intrusive list links, rebuilt when the element is linked in again
template <> struct is_skipped<&object::next> : std::true_type {};
template <> struct is_skipped<&object::prev> : std::true_type {};
template <> struct is_skipped<&ship_subsys::next> : std::true_type {};
template <> struct is_skipped<&ship_subsys::prev> : std::true_type {};
template <> struct is_skipped<&beam::next> : std::true_type {};
template <> struct is_skipped<&beam::prev> : std::true_type {};
template <> struct is_skipped<&shockwave::next> : std::true_type {};
template <> struct is_skipped<&shockwave::prev> : std::true_type {};
template <> struct is_skipped<&p_object::next> : std::true_type {};
template <> struct is_skipped<&p_object::prev> : std::true_type {};

// resources and list memberships that are re-acquired after loading
template <> struct is_skipped<&object::objsnd_num> : std::true_type {};
template <> struct is_skipped<&ship::model_instance_num> : std::true_type {};
template <> struct is_skipped<&ship::cockpit_model_instance> : std::true_type {};
template <> struct is_skipped<&ship::ship_list_index> : std::true_type {};
template <> struct is_skipped<&weapon::model_instance_num> : std::true_type {};
template <> struct is_skipped<&weapon::missile_list_index> : std::true_type {};
template <> struct is_skipped<&debris::model_instance_num> : std::true_type {};
// model numbers differ between sessions; hull debris uses its ship class's model, which the restore takes again
template <> struct is_skipped<&debris::model_num> : std::true_type {};
// positions in the ship's object sound list, which is rebuilt; firing loops restart by themselves
template <> struct is_skipped<&ship_weapon::firing_loop_sounds> : std::true_type {};
template <> struct is_skipped<&asteroid::model_instance_num> : std::true_type {};
template <> struct is_skipped<&prop::model_instance_num> : std::true_type {};
template <> struct is_skipped<&external_weapon_state::model_instance> : std::true_type {};
template <> struct is_skipped<&external_weapon_state::model_instance_weapon> : std::true_type {};

// set up from table data when a shockwave is created, or every frame
template <> struct is_skipped<&shockwave::model_id> : std::true_type {};
template <> struct is_skipped<&shockwave::current_bitmap> : std::true_type {};

// set up from table data when a subsystem is created
template <> struct is_skipped<&ship_subsys::system_info> : std::true_type {};
template <> struct is_skipped<&ship::beam_sys_info> : std::true_type {};

// set from mission data when the mission is loaded
template <> struct is_skipped<&mission::skybox_model_animations> : std::true_type {};

// the operator a node refers to is looked up from its text again; operator indices change as operators are added
template <> struct is_skipped<&sexp_node::op_index> : std::true_type {};

// The pilot's all-time statistics belong to the pilot, not to the mission: they only change when a mission is accepted,
// and a checkpoint must not roll them back.  The mission's own statistics (the m_ members) are saved.
template <> struct is_skipped<&scoring_struct::score> : std::true_type {};
template <> struct is_skipped<&scoring_struct::rank> : std::true_type {};
template <> struct is_skipped<&scoring_struct::medal_counts> : std::true_type {};
template <> struct is_skipped<&scoring_struct::kills> : std::true_type {};
template <> struct is_skipped<&scoring_struct::assists> : std::true_type {};
template <> struct is_skipped<&scoring_struct::kill_count> : std::true_type {};
template <> struct is_skipped<&scoring_struct::kill_count_ok> : std::true_type {};
template <> struct is_skipped<&scoring_struct::p_shots_fired> : std::true_type {};
template <> struct is_skipped<&scoring_struct::s_shots_fired> : std::true_type {};
template <> struct is_skipped<&scoring_struct::p_shots_hit> : std::true_type {};
template <> struct is_skipped<&scoring_struct::s_shots_hit> : std::true_type {};
template <> struct is_skipped<&scoring_struct::p_bonehead_hits> : std::true_type {};
template <> struct is_skipped<&scoring_struct::s_bonehead_hits> : std::true_type {};
template <> struct is_skipped<&scoring_struct::bonehead_kills> : std::true_type {};
template <> struct is_skipped<&scoring_struct::missions_flown> : std::true_type {};
template <> struct is_skipped<&scoring_struct::flight_time> : std::true_type {};
template <> struct is_skipped<&scoring_struct::last_flown> : std::true_type {};
template <> struct is_skipped<&scoring_struct::last_backup> : std::true_type {};

// re-acquired by the jump node's object, from the model name it saves
template <> struct is_skipped<&CJumpNode::m_modelnum> : std::true_type {};
template <> struct is_skipped<&CJumpNode::m_polymodel_instance_num> : std::true_type {};
template <> struct is_skipped<&wing::wing_insignia_texture> : std::true_type {};
template <> struct is_skipped<&p_object::replacement_textures> : std::true_type {};

// the pilot rather than the mission: identity, settings and campaign-persistent data
template <> struct is_skipped<&player::callsign> : std::true_type {};
template <> struct is_skipped<&player::short_callsign> : std::true_type {};
template <> struct is_skipped<&player::short_callsign_width> : std::true_type {};
template <> struct is_skipped<&player::image_filename> : std::true_type {};
template <> struct is_skipped<&player::s_squad_filename> : std::true_type {};
template <> struct is_skipped<&player::s_squad_name> : std::true_type {};
template <> struct is_skipped<&player::m_squad_filename> : std::true_type {};
template <> struct is_skipped<&player::m_squad_name> : std::true_type {};
template <> struct is_skipped<&player::current_campaign> : std::true_type {};
template <> struct is_skipped<&player::readyroom_listing_mode> : std::true_type {};
template <> struct is_skipped<&player::save_flags> : std::true_type {};
template <> struct is_skipped<&player::last_ship_flown_si_index> : std::true_type {};
template <> struct is_skipped<&player::auto_advance> : std::true_type {};
template <> struct is_skipped<&player::m_local_options> : std::true_type {};
template <> struct is_skipped<&player::m_server_options> : std::true_type {};
template <> struct is_skipped<&player::insignia_texture> : std::true_type {};
template <> struct is_skipped<&player::tips> : std::true_type {};
template <> struct is_skipped<&player::failures_this_session> : std::true_type {};
template <> struct is_skipped<&player::show_skip_popup> : std::true_type {};
template <> struct is_skipped<&player::variables> : std::true_type {};
template <> struct is_skipped<&player::containers> : std::true_type {};
template <> struct is_skipped<&player::player_was_multi> : std::true_type {};
template <> struct is_skipped<&player::language> : std::true_type {};
// copies of the controls for scripts, refreshed every frame
template <> struct is_skipped<&player::lua_ci> : std::true_type {};
template <> struct is_skipped<&player::lua_bi> : std::true_type {};
template <> struct is_skipped<&player::lua_bi_full> : std::true_type {};

// handled by Extra<object> and Extra<ai_goal> below
template <> struct is_skipped<&object::dock_list> : std::true_type {};
template <> struct is_skipped<&object::dead_dock_list> : std::true_type {};
template <> struct is_skipped<&ai_goal::target_name> : std::true_type {};
template <> struct is_skipped<&ai_goal::docker> : std::true_type {};
template <> struct is_skipped<&ai_goal::dockee> : std::true_type {};

// --- factories for types without a default constructor ---------------------------------------------------------------

template <>
struct Factory<ship_registry_entry> {
	static ship_registry_entry make() { return ship_registry_entry(""); }
};

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

template <>
struct PoolTraits<beam> {
	static int index_of(const beam* p) { return BEAM_INDEX(p); }
	static beam* at(int index) { return (index >= 0 && index < MAX_BEAMS) ? &Beams[index] : nullptr; }
};

template <>
struct PoolTraits<shockwave> {
	static int index_of(const shockwave* p) { return static_cast<int>(p - Shockwaves); }
	static shockwave* at(int index) { return (index >= 0 && index < MAX_SHOCKWAVES) ? &Shockwaves[index] : nullptr; }
};

template <>
struct PoolTraits<ai_profile_t> {
	static int index_of(const ai_profile_t* p) { return AI_PROFILES_INDEX(p); }
	static ai_profile_t* at(int index) { return (index >= 0 && index < MAX_AI_PROFILES) ? &Ai_profiles[index] : nullptr; }
};

template <>
struct PoolTraits<traitor_override_t> {
	static int index_of(const traitor_override_t* p) { return static_cast<int>(p - Traitor_overrides.data()); }
	static traitor_override_t* at(int index) { return SCP_vector_inbounds(Traitor_overrides, index) ? &Traitor_overrides[index] : nullptr; }
};

template <>
struct PoolTraits<p_object> {
	static int index_of(const p_object* p) { return POBJ_INDEX(p); }
	static p_object* at(int index) { return SCP_vector_inbounds(Parse_objects, index) ? &Parse_objects[index] : nullptr; }
};

// --- custom codecs ---------------------------------------------------------------------------------------------------

/**
 * Subsystems live in a heap pool and are re-allocated on load, so a pointer to one is saved as the object that owns it
 * plus its position in that ship's subsystem list, and resolved once all ships have their subsystems again.
 */
template <>
struct Codec<ship_subsys*> {
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

// Bitmap handles are not stable between sessions, so bitmaps are saved by name and loaded again.
template <>
struct Codec<generic_bitmap> {
	static void save(Writer& w, const generic_bitmap& v) { checkpoint::save(w, SCP_string(v.filename)); }
	static void load(Reader& r, generic_bitmap& v)
	{
		SCP_string filename;
		checkpoint::load(r, filename);
		strcpy_s(v.filename, filename.c_str());
		v.bitmap_id = filename.empty() ? -1 : bm_load(v.filename);
	}
};

// An owned, optional list of objects a countermeasure has already been ignored by.
template <>
struct Codec<SCP_vector<int>*> {
	static void save(Writer& w, SCP_vector<int>* const& v)
	{
		w.value(static_cast<ubyte>(v != nullptr));
		if (v != nullptr)
			checkpoint::save(w, *v);
	}
	static void load(Reader& r, SCP_vector<int>*& v)
	{
		ubyte present = 0;
		r.value(present);
		delete v;
		v = nullptr;
		if (present != 0) {
			v = new SCP_vector<int>();
			checkpoint::load(r, *v);
		}
	}
};

// Volumetric nebulae are mission data and rendering resources; sexps can only switch them on and off.
template <>
struct Codec<volumetric_nebula> {
	static void save(Writer& w, const volumetric_nebula& v) { w.value(static_cast<ubyte>(v.get_enabled())); }
	static void load(Reader& r, volumetric_nebula& v)
	{
		ubyte enabled = 1;
		r.value(enabled);
		v.set_enabled(enabled != 0);
	}
};

template <> struct is_raw<object_ship_wing_point_team> : std::true_type {};

// Lua values can't be saved; orders given by Lua AI sexps are rebuilt from their sexp text once the objects are back.
template <>
struct Codec<ai_lua_parameters> {
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

template <typename T>
struct Codec<reload_pct<T>> {
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
