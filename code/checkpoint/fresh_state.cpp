#include "checkpoint/fresh_state.h"

#include "checkpoint/serialize.h"

#include "ai/ai.h"
#include "globalincs/linklist.h"
#include "globalincs/systemvars.h"
#include "mission/missiongoals.h"
#include "mission/missionparse.h"
#include "object/object.h"
#include "object/objectdock.h"
#include "object/parseobjectdock.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"
#include "playerman/player.h"
#include "ship/ship.h"

extern int parse_object_on_arrival_list(p_object* pobjp);
extern void clear_cache(int node);

namespace checkpoint {

namespace {

template <typename T>
Buffer encode(const T& value)
{
	Buffer buffer;
	Writer w(buffer);
	save(w, value);
	return buffer;
}

template <typename T>
void decode(const Buffer& buffer, T& value)
{
	Reader r(buffer);
	load(r, value);
}

// A sexp tree as it is at mission start, to be copied into new nodes.  The nodes it had at mission start may since have
// been freed and used for other trees, so the copy never goes back into them.
struct Tree {
	struct Node {
		SCP_string text;
		int type;
		int subtype;
		int first;	// index into nodes, or -1
		int rest;
	};
	SCP_vector<Node> nodes;
	int root = -1;	// the original node, if it is one that isn't copied: -1 or a locked node
};

int add_tree_node(Tree& tree, int node)
{
	if (node < 0)
		return -1;
	int local = static_cast<int>(tree.nodes.size());
	tree.nodes.push_back({Sexp_nodes[node].text, Sexp_nodes[node].type, Sexp_nodes[node].subtype, -1, -1});
	int first = add_tree_node(tree, Sexp_nodes[node].first);
	int rest = add_tree_node(tree, Sexp_nodes[node].rest);
	tree.nodes[local].first = first;
	tree.nodes[local].rest = rest;
	return local;
}

Tree capture_tree(int root)
{
	Tree tree;
	if (root < 0 || root == Locked_sexp_true || root == Locked_sexp_false || Sexp_nodes[root].type == SEXP_NOT_USED)
		tree.root = root;
	else
		add_tree_node(tree, root);
	return tree;
}

int allocate_tree_node(const Tree& tree, int local)
{
	if (local < 0)
		return -1;
	const auto& node = tree.nodes[local];
	int first = allocate_tree_node(tree, node.first);
	int rest = allocate_tree_node(tree, node.rest);
	return alloc_sexp(node.text.c_str(), node.type, node.subtype, first, rest);
}

// Returns the root of a new copy of the tree.
int allocate_tree(const Tree& tree)
{
	return tree.nodes.empty() ? tree.root : allocate_tree_node(tree, 0);
}

struct ShipReset {
	int pobj_index = -1;
	Buffer p_object;
	int registry_index = -1;
	Buffer registry_entry;
	Tree arrival_cue, departure_cue, ai_goals;
	bool present_at_start = false;
	bool on_arrival_list = false;
	Buffer goals;	// the AI goals of the ship created at mission start
};

struct WingReset {
	int wingnum = -1;
	Buffer wing;
	Tree arrival_cue, departure_cue;
	bool present_at_start = false;
	SCP_vector<ShipReset> members;
};

struct EventReset {
	int index = -1;
	Buffer event;
	SCP_vector<std::pair<int, Buffer>> nodes;
};

struct FreshState {
	SCP_vector<std::pair<int, Buffer>> variables;
	SCP_vector<std::pair<SCP_string, Buffer>> containers;
	SCP_vector<EventReset> events;
	SCP_vector<WingReset> wings;
	SCP_vector<ShipReset> ships;
};

FreshState Fresh;

int find_parse_object(const SCP_string& name)
{
	for (size_t i = 0; i < Parse_objects.size(); ++i) {
		if (!stricmp(Parse_objects[i].name, name.c_str()))
			return static_cast<int>(i);
	}
	return -1;
}

ShipReset capture_ship(int pobj_index)
{
	auto& p_objp = Parse_objects[pobj_index];
	ShipReset reset;
	reset.pobj_index = pobj_index;
	reset.p_object = encode(p_objp);
	reset.registry_index = ship_registry_get_index(p_objp.name);
	if (reset.registry_index >= 0)
		reset.registry_entry = encode(Ship_registry[reset.registry_index]);
	reset.arrival_cue = capture_tree(p_objp.arrival_cue);
	reset.departure_cue = capture_tree(p_objp.departure_cue);
	reset.ai_goals = capture_tree(p_objp.ai_goals);
	reset.on_arrival_list = parse_object_on_arrival_list(&p_objp) != 0;

	// ships created at mission start no longer have their goals in the parse object
	auto objp = p_objp.created_object;
	reset.present_at_start = (objp != nullptr && objp->type == OBJ_SHIP);
	if (reset.present_at_start)
		reset.goals = encode(Ai_info[Ships[objp->instance].ai_index].goals);
	return reset;
}

// The parse objects docked to this one at mission start, directly or indirectly
void collect_dock_group(int pobj_index, SCP_vector<int>& group)
{
	if (std::find(group.begin(), group.end(), pobj_index) != group.end())
		return;
	group.push_back(pobj_index);
	for (p_dock_instance* dock = Parse_objects[pobj_index].dock_list; dock != nullptr; dock = dock->next) {
		auto other = static_cast<int>(dock->docked_objp - Parse_objects.data());
		collect_dock_group(other, group);
	}
}

bool is_player_start(int pobj_index)
{
	return Parse_objects[pobj_index].flags[Mission::Parse_Object_Flags::OF_Player_start];
}

void capture_wing(int wingnum)
{
	for (const auto& existing : Fresh.wings) {
		if (existing.wingnum == wingnum)
			return;
	}
	for (size_t i = 0; i < Parse_objects.size(); ++i) {
		if (Parse_objects[i].wingnum == wingnum && is_player_start(static_cast<int>(i))) {
			mprintf(("Checkpoint: wing %s holds the player's ship and can't be excluded\n", Wings[wingnum].name));
			return;
		}
	}

	WingReset reset;
	reset.wingnum = wingnum;
	reset.wing = encode(Wings[wingnum]);
	reset.arrival_cue = capture_tree(Wings[wingnum].arrival_cue);
	reset.departure_cue = capture_tree(Wings[wingnum].departure_cue);
	reset.present_at_start = Wings[wingnum].current_wave > 0;
	for (size_t i = 0; i < Parse_objects.size(); ++i) {
		if (Parse_objects[i].wingnum == wingnum)
			reset.members.push_back(capture_ship(static_cast<int>(i)));
	}
	Fresh.wings.push_back(std::move(reset));
}

void capture_name(const SCP_string& name)
{
	int variable = get_index_sexp_variable_name(name.c_str());
	if (variable >= 0) {
		Fresh.variables.emplace_back(variable, encode(Sexp_variables[variable]));
		return;
	}
	if (auto container = get_sexp_container(name.c_str())) {
		Fresh.containers.emplace_back(name, encode(*container));
		return;
	}
	int wingnum = wing_name_lookup(name.c_str(), 1);
	if (wingnum >= 0) {
		capture_wing(wingnum);
		return;
	}

	int pobj_index = find_parse_object(name);
	if (pobj_index < 0) {
		mprintf(("Checkpoint: excluded name '%s' is no ship, wing, variable or container of this mission\n", name.c_str()));
		return;
	}
	if (Parse_objects[pobj_index].wingnum >= 0) {
		mprintf(("Checkpoint: ship %s is in a wing, so its whole wing is reset\n", name.c_str()));
		capture_wing(Parse_objects[pobj_index].wingnum);
		return;
	}

	SCP_vector<int> group;
	collect_dock_group(pobj_index, group);
	for (int member : group) {
		if (is_player_start(member) || Parse_objects[member].wingnum >= 0) {
			mprintf(("Checkpoint: ship %s is docked to the player or a wing at mission start and can't be excluded\n", name.c_str()));
			return;
		}
	}
	if (group.size() > 1)
		mprintf(("Checkpoint: ship %s is docked to other ships at mission start, so their whole group is reset\n", name.c_str()));
	for (int member : group) {
		bool captured = std::any_of(Fresh.ships.begin(), Fresh.ships.end(), [member](const ShipReset& s) { return s.pobj_index == member; });
		if (!captured)
			Fresh.ships.push_back(capture_ship(member));
	}
}

void capture_events()
{
	for (int i = 0; i < static_cast<int>(Mission_events.size()); ++i) {
		const auto& event = Mission_events[i];
		if (!(event.flags & MEF_NOT_CHECKPOINTED))
			continue;

		EventReset reset;
		reset.index = i;
		reset.event = encode(event);
		// formulas stay in place for the whole mission, so their nodes are put back where they are
		SCP_vector<int> pending = {event.formula};
		while (!pending.empty()) {
			int node = pending.back();
			pending.pop_back();
			if (node < 0 || node == Locked_sexp_true || node == Locked_sexp_false)
				continue;
			reset.nodes.emplace_back(node, encode(Sexp_nodes[node]));
			pending.push_back(Sexp_nodes[node].first);
			pending.push_back(Sexp_nodes[node].rest);
		}
		Fresh.events.push_back(std::move(reset));
	}
}

// Takes a restored ship out of the mission without a trace: no log entry, no departure.
void remove_ship(int shipnum)
{
	auto objp = &Objects[Ships[shipnum].objnum];
	// the cues are shared with the parse object, which gets its own copy back
	Ships[shipnum].arrival_cue = Locked_sexp_false;
	Ships[shipnum].departure_cue = Locked_sexp_false;
	dock_undock_all(objp);
	ship_actually_depart(shipnum, SHIP_VANISHED);
	obj_delete_all_that_should_be_dead();
}

void remove_ship(const char* name)
{
	int shipnum = ship_name_lookup(name, 1);
	if (shipnum >= 0)
		remove_ship(shipnum);
}

// Puts a parse object, its registry entry and its trees back to mission start, before the ship is created.
void restore_parse_object(const ShipReset& reset)
{
	auto& p_objp = Parse_objects[reset.pobj_index];
	decode(reset.p_object, p_objp);
	p_objp.created_object = nullptr;
	p_objp.arrival_cue = allocate_tree(reset.arrival_cue);
	p_objp.departure_cue = allocate_tree(reset.departure_cue);
	p_objp.ai_goals = allocate_tree(reset.ai_goals);

	if (reset.registry_index >= 0) {
		auto& entry = Ship_registry[reset.registry_index];
		decode(reset.registry_entry, entry);
		entry.status = ShipStatus::NOT_YET_PRESENT;
		entry.objnum = -1;
		entry.shipnum = -1;
	}

	if ((reset.on_arrival_list || reset.present_at_start) && !parse_object_on_arrival_list(&p_objp))
		list_append(&Ship_arrival_list, &p_objp);
}

// The ship created at mission start had goals that its parse object no longer holds.
void restore_goals(const ShipReset& reset)
{
	if (!reset.present_at_start)
		return;
	int shipnum = ship_name_lookup(Parse_objects[reset.pobj_index].name, 1);
	if (shipnum >= 0)
		decode(reset.goals, Ai_info[Ships[shipnum].ai_index].goals);
}

void apply_wing(const WingReset& reset)
{
	auto wingp = &Wings[reset.wingnum];

	// every ship of the wing that is in the mission now leaves it
	SCP_vector<int> members;
	for (auto so : list_range(&Ship_obj_list)) {
		auto objp = &Objects[so->objnum];
		if (objp->type == OBJ_SHIP && Ships[objp->instance].wingnum == reset.wingnum)
			members.push_back(objp->instance);
	}
	for (int shipnum : members)
		remove_ship(shipnum);

	decode(reset.wing, *wingp);
	wingp->arrival_cue = allocate_tree(reset.arrival_cue);
	wingp->departure_cue = allocate_tree(reset.departure_cue);
	for (const auto& member : reset.members)
		restore_parse_object(member);

	if (!reset.present_at_start)
		return;

	// the wing as it was before its first wave was created at mission start
	wingp->current_wave = 0;
	wingp->current_count = 0;
	wingp->total_arrived_count = 0;
	wingp->red_alert_skipped_ships = 0;
	wingp->total_destroyed = 0;
	wingp->total_departed = 0;
	wingp->total_vanished = 0;
	for (int& index : wingp->ship_index)
		index = -1;
	wingp->flags.remove(Ship::Wing_Flags::Gone);

	Game_restoring = 1;
	parse_wing_create_ships(wingp, wingp->wave_count, true);
	Game_restoring = 0;
	for (const auto& member : reset.members)
		restore_goals(member);
}

void apply_ships()
{
	for (const auto& reset : Fresh.ships)
		remove_ship(Parse_objects[reset.pobj_index].name);
	for (const auto& reset : Fresh.ships)
		restore_parse_object(reset);

	// a docked group is created through its leader
	Game_restoring = 1;
	for (const auto& reset : Fresh.ships) {
		auto& p_objp = Parse_objects[reset.pobj_index];
		if (!reset.present_at_start || p_objp.created_object != nullptr)
			continue;
		if (object_is_docked(&p_objp) && !p_objp.flags[Mission::Parse_Object_Flags::SF_Dock_leader])
			continue;
		parse_create_object(&p_objp);
	}
	Game_restoring = 0;
	for (const auto& reset : Fresh.ships)
		restore_goals(reset);
}

} // namespace

void capture_fresh_state(const SCP_vector<SCP_string>& excluded)
{
	Fresh = FreshState();
	capture_events();
	for (const auto& name : excluded)
		capture_name(name);
}

void apply_fresh_state()
{
	for (const auto& variable : Fresh.variables)
		decode(variable.second, Sexp_variables[variable.first]);
	for (const auto& container : Fresh.containers) {
		if (auto current = get_sexp_container(container.first.c_str()))
			decode(container.second, *current);
	}

	for (const auto& event : Fresh.events) {
		decode(event.event, Mission_events[event.index]);
		for (const auto& node : event.nodes) {
			clear_cache(node.first);
			decode(node.second, Sexp_nodes[node.first]);
		}
	}

	for (const auto& wing : Fresh.wings)
		apply_wing(wing);
	apply_ships();

	// the player's target may have been one of the removed ships
	if (Player_ai != nullptr && Player_ai->target_objnum >= 0 && Objects[Player_ai->target_objnum].type == OBJ_NONE)
		Player_ai->target_objnum = -1;

	Fresh = FreshState();
}

} // namespace checkpoint
