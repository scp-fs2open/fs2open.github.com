#include "checkpoints.h"

#include "checkpoint/checkpoint.h"
#include "scripting/api/libs/mission.h"

namespace scripting {
namespace api {

namespace {

// The strings passed from the given argument on
SCP_vector<SCP_string> string_args(lua_State* L, int first)
{
	SCP_vector<SCP_string> strings;
	for (int i = first; i <= lua_gettop(L); ++i) {
		if (lua_isstring(L, i))
			strings.emplace_back(lua_tostring(L, i));
	}
	return strings;
}

} // namespace

//****SUBLIBRARY: Mission/Checkpoints
ADE_LIB_DERIV(l_Mission_Checkpoints, "Checkpoints", nullptr,
	"Checkpoints of the mission, stored per pilot and mission.  The functions match the savestate-* sexps.",
	l_Mission);

ADE_FUNC(save, l_Mission_Checkpoints, "number index, string... excluded",
	"Saves a checkpoint of the mission under the index, at the end of the frame.  Ships, wings, variables and "
	"containers named after the index are put back to their mission-start state when the checkpoint is loaded.  The "
	"On Checkpoint Save hook runs just before saving.",
	nullptr, nullptr)
{
	int index = 0;
	if (!ade_get_args(L, "i", &index))
		return ADE_RETURN_NIL;
	checkpoint::request_save(index, string_args(L, 2));
	return ADE_RETURN_NIL;
}

ADE_FUNC(load, l_Mission_Checkpoints, "number index",
	"Reloads the mission and restores the checkpoint saved under the index, at the end of the frame.  Requests in the "
	"first frame after a checkpoint was loaded are ignored.  The On Checkpoint Load hook runs once it is restored.",
	nullptr, nullptr)
{
	int index = 0;
	if (!ade_get_args(L, "i", &index))
		return ADE_RETURN_NIL;
	checkpoint::request_load(index);
	return ADE_RETURN_NIL;
}

ADE_FUNC(exists, l_Mission_Checkpoints, "[number index, string mission]",
	"Whether the pilot has any checkpoint of this mission, the checkpoint with this index, or the checkpoint with this "
	"index of another mission.",
	"boolean", "True if the checkpoint exists")
{
	int index = -1;
	const char* mission = nullptr;
	ade_get_args(L, "|is", &index, &mission);
	return ade_set_args(L, "b", checkpoint::has_slot(index, mission));
}

ADE_FUNC(clear, l_Mission_Checkpoints, "[number index]",
	"Removes the checkpoint with this index, or all checkpoints of this mission.  A checkpoint the mission was loaded "
	"from stays in effect.",
	nullptr, nullptr)
{
	int index = -1;
	ade_get_args(L, "|i", &index);
	checkpoint::clear_slot(index);
	return ADE_RETURN_NIL;
}

ADE_FUNC(getLoadedIndex, l_Mission_Checkpoints, nullptr,
	"The index of the checkpoint the mission was loaded from.",
	"number", "The index, or 0 if the mission wasn't loaded from a checkpoint")
{
	return ade_set_args(L, "i", checkpoint::loaded_index());
}

ADE_FUNC(getNextFreeIndex, l_Mission_Checkpoints, nullptr,
	"The lowest index from 1 up without a checkpoint in this mission.",
	"number", "The index")
{
	return ade_set_args(L, "i", checkpoint::next_free_slot());
}

ADE_FUNC(shipStatus, l_Mission_Checkpoints, "string name, [number index]",
	"Whether a ship or wing is in the mission or yet to arrive in the checkpoint the mission was loaded from, or in the "
	"checkpoint with this index, rather than destroyed or departed.",
	"boolean", "False if destroyed or departed; true otherwise, also without a checkpoint or if the name isn't in it")
{
	const char* name = nullptr;
	int index = 0;
	if (!ade_get_args(L, "s|i", &name, &index))
		return ADE_RETURN_TRUE;
	return ade_set_args(L, "b", checkpoint::ship_status(name, index));
}

ADE_FUNC(assist, l_Mission_Checkpoints, "number minHull, number minAmmo, string... ships",
	"When a checkpoint is loaded later in this mission, gives these ships at least the given percentages of their hull "
	"and of the ammunition of their banks and turrets.",
	nullptr, nullptr)
{
	float min_hull = 0.0f;
	float min_ammo = 0.0f;
	if (!ade_get_args(L, "ff", &min_hull, &min_ammo))
		return ADE_RETURN_NIL;
	checkpoint::assist(min_hull, min_ammo, string_args(L, 3));
	return ADE_RETURN_NIL;
}

ADE_FUNC(setData, l_Mission_Checkpoints, "string key, string value",
	"Stores a value that is saved with every checkpoint of this mission and restored when one is loaded, so scripts "
	"can keep their state across checkpoints (e.g. as JSON).  The data starts empty in a mission not loaded from a "
	"checkpoint.",
	nullptr, nullptr)
{
	const char* key = nullptr;
	const char* value = nullptr;
	if (!ade_get_args(L, "ss", &key, &value))
		return ADE_RETURN_NIL;
	checkpoint::set_script_data(key, value);
	return ADE_RETURN_NIL;
}

ADE_FUNC(getData, l_Mission_Checkpoints, "string key",
	"Returns a value stored with setData.",
	"string", "The value, or nil if none is stored under the key")
{
	const char* key = nullptr;
	if (!ade_get_args(L, "s", &key))
		return ADE_RETURN_NIL;
	SCP_string value;
	if (!checkpoint::get_script_data(key, value))
		return ADE_RETURN_NIL;
	return ade_set_args(L, "s", value.c_str());
}

} // namespace api
} // namespace scripting
