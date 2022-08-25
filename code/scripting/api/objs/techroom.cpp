#include "techroom.h"

namespace scripting {
namespace api {

//**********HANDLE: tech missions
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

//**********HANDLE: tech cutscenes
ADE_OBJ(l_TechRoomCutscene, cutscene_info, "custscene_info", "Tech Room cutscene handle");

ADE_VIRTVAR(Name, l_TechRoomCutscene, nullptr, "The name of the cutscene", "custscene_info", "The cutscene name")
{
	cutscene_info* current = nullptr;
	if (!ade_get_args(L, "o", l_TechRoomCutscene.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->name);
}

ADE_VIRTVAR(Filename, l_TechRoomCutscene, nullptr, "The filename of the cutscene", "custscene_info", "The cutscene filename")
{
	cutscene_info* current = nullptr;
	if (!ade_get_args(L, "o", l_TechRoomCutscene.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->filename);
}

ADE_VIRTVAR(Description, l_TechRoomCutscene, nullptr, "The cutscene description", "custscene_info", "The cutscene description")
{
	cutscene_info* current = nullptr;
	if (!ade_get_args(L, "o", l_TechRoomCutscene.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->description);
}

ADE_VIRTVAR(Visibility,
	l_TechRoomCutscene,
	nullptr,
	"If the cutscene should be visible by default",
	"custscene_info",
	"1 if visible, 0 if not visible")
{
	cutscene_info* current = nullptr;
	if (!ade_get_args(L, "o", l_TechRoomCutscene.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}
	if (current->flags[Cutscene::Cutscene_Flags::Viewable, Cutscene::Cutscene_Flags::Always_viewable] &&
		!current->flags[Cutscene::Cutscene_Flags::Never_viewable]) 
	{
		return ade_set_args(L, "i", 1);
	} else {
		return ade_set_args(L, "i", 0);
	}

}

} // namespace api
} // namespace scripting