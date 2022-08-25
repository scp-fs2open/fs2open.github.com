#include "techroom.h"

namespace scripting {
namespace api {

//**********HANDLE: loop_briefing
ADE_OBJ(l_TechRoomMission, sim_mission, "sim_mission", "Tech Room mission handle");

ADE_VIRTVAR(Name, l_TechRoomMission, nullptr, "The name of the mission", "sim_mission", "The name")
{
	sim_mission* current = nullptr;
	if (!ade_get_args(L, "o", l_TechRoomMission.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->name);
}

ADE_VIRTVAR(Filename, l_TechRoomMission, nullptr, "The filename of the mission", "sim_mission", "The filename")
{
	sim_mission* current = nullptr;
	if (!ade_get_args(L, "o", l_TechRoomMission.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->filename);
}

ADE_VIRTVAR(Description, l_TechRoomMission, nullptr, "The mission description", "sim_mission", "The description")
{
	sim_mission* current = nullptr;
	if (!ade_get_args(L, "o", l_TechRoomMission.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->mission_desc);
}

ADE_VIRTVAR(Author, l_TechRoomMission, nullptr, "The mission author", "sim_mission", "The author")
{
	sim_mission* current = nullptr;
	if (!ade_get_args(L, "o", l_TechRoomMission.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->author);
}

ADE_VIRTVAR(Visibility, l_TechRoomMission, nullptr, "If the mission should be visible by default", "sim_mission", "1 if visible, 0 if not visible, returns nil if not a campaign mission")
{
	sim_mission* current = nullptr;
	if (!ade_get_args(L, "o", l_TechRoomMission.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current->visible);
}

} // namespace api
} // namespace scripting