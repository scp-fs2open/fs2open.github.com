#include "techroom.h"
#include "pilotfile/pilotfile.h"

namespace scripting {
namespace api {

sim_mission_h::sim_mission_h() : missionIdx(-1), isCMission(false) {}
sim_mission_h::sim_mission_h(int index, bool cmission) : missionIdx(index), isCMission(cmission) {}

bool sim_mission_h::isValid() const
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

bool cutscene_info_h::isValid() const
{
	return SCP_vector_inbounds(Cutscenes, cutscene);
}

cutscene_info* cutscene_info_h::getScene() const
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
ADE_OBJ(l_TechRoomCutscene, cutscene_info_h, "cutscene_info", "Tech Room cutscene handle");

ADE_VIRTVAR(Name, l_TechRoomCutscene, nullptr, "The name of the cutscene", "string", "The cutscene name")
{
	cutscene_info_h current;
	if (!ade_get_args(L, "o", l_TechRoomCutscene.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getScene()->name);
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

	return ade_set_args(L, "s", current.getScene()->filename);
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

	return ade_set_args(L, "s", current.getScene()->description);
}

ADE_VIRTVAR(isVisible,
	l_TechRoomCutscene,
	"boolean",
	"If the cutscene should be visible by default",
	"boolean",
	"true if visible, false if not visible")
{
	cutscene_info_h current;
	bool visible;
	if (!ade_get_args(L, "o|b", l_TechRoomCutscene.Get(&current), &visible)) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		current.getScene()->flags.set(Cutscene::Cutscene_Flags::Viewable, visible);
		Pilot.save_savefile();
	}
	if (current.getScene()->flags[Cutscene::Cutscene_Flags::Viewable, Cutscene::Cutscene_Flags::Always_viewable] &&
		!current.getScene()->flags[Cutscene::Cutscene_Flags::Never_viewable]) 
	{
		return ade_set_args(L, "b", true);
	} else {
		return ade_set_args(L, "b", false);
	}

}

ADE_VIRTVAR(CustomData, l_TechRoomCutscene, nullptr, "Gets the custom data table for this cutscene", "table", "The cutscene's custom data table") 
{
	cutscene_info_h current;
	if (!ade_get_args(L, "o", l_TechRoomCutscene.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	auto table = luacpp::LuaTable::create(L);

	for (const auto& pair : current.getScene()->custom_data)
	{
		table.addValue(pair.first, pair.second);
	}

	return ade_set_args(L, "t", &table);	
}

ADE_FUNC(hasCustomData, l_TechRoomCutscene, nullptr, "Detects whether the cutscene has any custom data", "boolean", "true if the cutscene's custom_data is not empty, false otherwise") 
{
	cutscene_info_h current;
	if (!ade_get_args(L, "o", l_TechRoomCutscene.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	bool result = !current.getScene()->custom_data.empty();
	return ade_set_args(L, "b", result);
}

ADE_FUNC(isValid, l_TechRoomCutscene, NULL, "Detects whether cutscene is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	cutscene_info_h current;
	if (!ade_get_args(L, "o", l_TechRoomCutscene.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "b", current.isValid());
}

} // namespace api
} // namespace scripting