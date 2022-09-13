#include "techroom.h"

namespace scripting {
namespace api {

sim_mission_h::sim_mission_h() : missionIdx(-1), isCMission(false) {}
sim_mission_h::sim_mission_h(int index, bool cmission) : missionIdx(index), isCMission(cmission) {}

bool sim_mission_h::IsValid() const
{
	return missionIdx >= 0;
}

sim_mission* sim_mission_h::getStage() const
{
	if (isCMission)
		return &Sim_CMissions[missionIdx];
	else
		return &Sim_Missions[missionIdx];
};

cutscene_info_h::cutscene_info_h() : cutscene(-1) {}
cutscene_info_h::cutscene_info_h(int scene) : cutscene(scene) {}

bool cutscene_info_h::IsValid() const
{
	return cutscene >= 0;
}

cutscene_info* cutscene_info_h::getStage() const
{
	return &Cutscenes[cutscene];
}

//**********HANDLE: tech missions
ADE_OBJ(l_TechRoomMission, sim_mission_h, "sim_mission", "Tech Room mission handle");

ADE_VIRTVAR(Name, l_TechRoomMission, nullptr, "The name of the mission", "string", "The name")
{
	sim_mission_h current;
	if (!ade_get_args(L, "o", l_TechRoomMission.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getStage()->name);
}

ADE_VIRTVAR(Filename, l_TechRoomMission, nullptr, "The filename of the mission", "string", "The filename")
{
	sim_mission_h current;
	if (!ade_get_args(L, "o", l_TechRoomMission.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getStage()->filename);
}

ADE_VIRTVAR(Description, l_TechRoomMission, nullptr, "The mission description", "string", "The description")
{
	sim_mission_h current;
	if (!ade_get_args(L, "o", l_TechRoomMission.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getStage()->mission_desc);
}

ADE_VIRTVAR(Author, l_TechRoomMission, nullptr, "The mission author", "string", "The author")
{
	sim_mission_h current;
	if (!ade_get_args(L, "o", l_TechRoomMission.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getStage()->author);
}

ADE_VIRTVAR(isVisible, l_TechRoomMission, nullptr, "If the mission should be visible by default", "boolean", "true if visible, false if not visible")
{
	sim_mission_h current;
	if (!ade_get_args(L, "o", l_TechRoomMission.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", !(current.getStage()->visible == 0));
}

ADE_VIRTVAR(isCampaignMission, l_TechRoomMission, nullptr, "If the mission is campaign or single", "boolean", "true if campaign, false if single")
{
	sim_mission_h current;
	if (!ade_get_args(L, "o", l_TechRoomMission.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", current.isCMission);
}

//**********HANDLE: tech cutscenes
ADE_OBJ(l_TechRoomCutscene, cutscene_info_h, "custscene_info", "Tech Room cutscene handle");

ADE_VIRTVAR(Name, l_TechRoomCutscene, nullptr, "The name of the cutscene", "string", "The cutscene name")
{
	cutscene_info_h current;
	if (!ade_get_args(L, "o", l_TechRoomCutscene.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getStage()->name);
}

ADE_VIRTVAR(Filename, l_TechRoomCutscene, nullptr, "The filename of the cutscene", "string", "The cutscene filename")
{
	cutscene_info_h current;
	if (!ade_get_args(L, "o", l_TechRoomCutscene.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getStage()->filename);
}

ADE_VIRTVAR(Description, l_TechRoomCutscene, nullptr, "The cutscene description", "string", "The cutscene description")
{
	cutscene_info_h current;
	if (!ade_get_args(L, "o", l_TechRoomCutscene.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getStage()->description);
}

ADE_VIRTVAR(isVisible,
	l_TechRoomCutscene,
	nullptr,
	"If the cutscene should be visible by default",
	"boolean",
	"true if visible, false if not visible")
{
	cutscene_info_h current;
	if (!ade_get_args(L, "o", l_TechRoomCutscene.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}
	if (current.getStage()->flags[Cutscene::Cutscene_Flags::Viewable, Cutscene::Cutscene_Flags::Always_viewable] &&
		!current.getStage()->flags[Cutscene::Cutscene_Flags::Never_viewable]) 
	{
		return ade_set_args(L, "b", false);
	} else {
		return ade_set_args(L, "b", true);
	}

}

} // namespace api
} // namespace scripting