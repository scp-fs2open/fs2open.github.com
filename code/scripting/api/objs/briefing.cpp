
#include "briefing.h"
#include "team.h"

namespace scripting {
namespace api {

brief_stage_h::brief_stage_h() : br_brief(-1), br_stage(-1) {}

brief_stage_h::brief_stage_h(int brief, int stage) : br_brief(brief), br_stage(stage) {}

bool brief_stage_h::IsValid() const
{
	return br_brief >= 0 && br_stage >= 0;
}

brief_stage* brief_stage_h::getStage() const
{
	return &Briefings[br_brief].stages[br_stage];
}

//**********HANDLE: briefing
ADE_OBJ(l_BriefStage, brief_stage_h, "briefing_stage", "Briefing stage handle");

ADE_VIRTVAR(Text, l_BriefStage, nullptr, "The text of the stage", "string", "The text")
{
	brief_stage_h stage;
	if (!ade_get_args(L, "o", l_BriefStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage.getStage()->text);
}

ADE_VIRTVAR(AudioFilename,
	l_BriefStage,
	nullptr,
	"The filename of the audio file to play",
	"string",
	"The file name")
{
	brief_stage_h stage;
	if (!ade_get_args(L, "o", l_BriefStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage.getStage()->voice);
}

ADE_VIRTVAR(hasForwardCut,
	l_BriefStage,
	nullptr,
	"If the stage has a forward cut flag",
	"boolean",
	"true if the stage is set to cut to the next stage, false otherwise")
{
	brief_stage_h stage;
	if (!ade_get_args(L, "o", l_BriefStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (stage.getStage()->flags & BS_FORWARD_CUT) {
		return ADE_RETURN_TRUE;
	} else {
		return ADE_RETURN_FALSE;
	}
}

ADE_VIRTVAR(hasBackwardCut,
	l_BriefStage,
	nullptr,
	"If the stage has a backward cut flag",
	"boolean",
	"true if the stage is set to cut to the previous stage, false otherwise")
{
	brief_stage_h stage;
	if (!ade_get_args(L, "o", l_BriefStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (stage.getStage()->flags & BS_BACKWARD_CUT) {
		return ADE_RETURN_TRUE;
	} else {
		return ADE_RETURN_FALSE;
	}
}

//**********HANDLE: briefing
ADE_OBJ_NO_MULTI(l_Brief, int, "briefing", "Briefing handle");

ADE_INDEXER(l_Brief,
	"number index",
	"The list of stages in the briefing.",
	"briefing_stage",
	"The stage at the specified location.")
{
	int briefIdx;
	int index = -1;
	if (!ade_get_args(L, "oi", l_Brief.Get(&briefIdx), &index)) {
		return ADE_RETURN_NIL;
	}

	if (briefIdx < 0)
		return ADE_RETURN_NIL;

	briefing* brief = &Briefings[briefIdx];

	--index;

	if (index < 0 || index >= brief->num_stages) {
		LuaError(L, "Invalid index %d, only have %d entries.", index + 1, brief->num_stages);
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "o", l_BriefStage.Set(brief_stage_h(briefIdx, index)));
}

ADE_FUNC(__len, l_Brief, nullptr, "The number of stages in the briefing", "number", "The number of stages.")
{
	int brief;
	if (!ade_get_args(L, "o", l_Brief.Get(&brief))) {
		return ADE_RETURN_NIL;
	}

	if (brief < 0)
		return ADE_RETURN_NIL;

	return ade_set_args(L, "i", Briefings[brief].num_stages);
}

//**********HANDLE: mission goals
ADE_OBJ_NO_MULTI(l_Goals, int, "mission_goal", "Mission objective handle");

ADE_VIRTVAR(Name, l_Goals, nullptr, "The name of the goal", "string", "The goal name")
{
	int current;
	if (!ade_get_args(L, "o", l_Goals.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", Mission_goals[current].name);
}

ADE_VIRTVAR(Message, l_Goals, nullptr, "The message of the goal", "string", "The goal message")
{
	int current;
	if (!ade_get_args(L, "o", l_Goals.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", Mission_goals[current].message);
}

ADE_VIRTVAR(Type, l_Goals, nullptr, "The goal type", "string", "primary, secondary, bonus, or none")
{
	int current;
	if (!ade_get_args(L, "o", l_Goals.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	SCP_string type;

	int bit = Mission_goals[current].type & GOAL_TYPE_MASK;
	switch (bit) {
	case PRIMARY_GOAL:
		type = "primary";
		break;

	case SECONDARY_GOAL:
		type = "secondary";
		break;

	case BONUS_GOAL:
		type = "bonus";
		break;

	default:
		type = "none";
		break;
	}

	return ade_set_args(L, "s", type);
}

ADE_VIRTVAR(Team, l_Goals, nullptr, "The goal team", "team", "The goal team")
{
	int current;
	if (!ade_get_args(L, "o", l_Goals.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "o", l_Team.Set(Mission_goals[current].team));
}

ADE_VIRTVAR(isGoalValid, l_Goals, nullptr, "The goal validity", "boolean", "true if valid, false otherwise")
{
	int current;
	if (!ade_get_args(L, "o", l_Goals.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", !(Mission_goals[current].type & INVALID_GOAL));
}

} // namespace api
} // namespace scripting
