
#include "briefing.h"
#include "team.h"

namespace scripting {
namespace api {

//**********HANDLE: briefing
ADE_OBJ(l_BriefStage, brief_stage, "briefing_stage", "Briefing stage handle");

ADE_VIRTVAR(Text, l_BriefStage, nullptr, "The text of the stage", "string", "The text")
{
	brief_stage* stage = nullptr;
	if (!ade_get_args(L, "o", l_BriefStage.GetPtr(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage->text);
}

ADE_VIRTVAR(AudioFilename,
	l_BriefStage,
	nullptr,
	"The filename of the audio file to play",
	"string",
	"The file name")
{
	brief_stage* stage = nullptr;
	if (!ade_get_args(L, "o", l_BriefStage.GetPtr(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage->voice);
}

ADE_VIRTVAR(isVisible,
		l_BriefStage,
		nullptr,
		"The result of the stage formula",
		"boolean",
		"true if the stage should be displayed, false otherwise")
	{
		brief_stage* stage = nullptr;
		if (!ade_get_args(L, "o", l_BriefStage.GetPtr(&stage))) {
			return ADE_RETURN_NIL;
		}

		if (ADE_SETTING_VAR) {
			LuaError(L, "This property is read only.");
		}

		if (eval_sexp(stage->formula)) {
			return ADE_RETURN_TRUE;
		} else {
			return ADE_RETURN_FALSE;
		}
	}

//**********HANDLE: briefing
ADE_OBJ(l_Brief, briefing, "briefing", "Briefing handle");

ADE_INDEXER(l_Brief,
	"number index",
	"The list of stages in the briefing.",
	"briefing_stage",
	"The stage at the specified location.")
{
	briefing* brief = nullptr;
	int index = -1;
	if (!ade_get_args(L, "oi", l_Brief.GetPtr(&brief), &index)) {
		return ADE_RETURN_NIL;
	}

	--index;

	if (index < 0 || index >= brief->num_stages) {
		LuaError(L, "Invalid index %d, only have %d entries.", index + 1, brief->num_stages);
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "o", l_BriefStage.Set(brief->stages[index]));
}

ADE_FUNC(__len, l_Brief, nullptr, "The number of stages in the briefing", "number", "The number of stages.")
{
	briefing* brief = nullptr;
	if (!ade_get_args(L, "o", l_Brief.GetPtr(&brief))) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "i", brief->num_stages);
}

//**********HANDLE: mission goals
ADE_OBJ(l_Goals, mission_goal, "mission_goal", "Mission objective handle");

ADE_VIRTVAR(Name, l_Goals, nullptr, "The name of the goal", "string", "The goal name")
{
	mission_goal* current = nullptr;
	if (!ade_get_args(L, "o", l_Goals.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->name);
}

ADE_VIRTVAR(Message, l_Goals, nullptr, "The message of the goal", "string", "The goal message")
{
	mission_goal* current = nullptr;
	if (!ade_get_args(L, "o", l_Goals.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->message);
}

ADE_VIRTVAR(Type, l_Goals, nullptr, "The goal type", "string", "primary, secondary, bonus, or none")
{
	mission_goal* current = nullptr;
	if (!ade_get_args(L, "o", l_Goals.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	SCP_string type;

	int bit = current->type & GOAL_TYPE_MASK;
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
	mission_goal* current = nullptr;
	if (!ade_get_args(L, "o", l_Goals.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "o", l_Team.Set(current->team));
}

ADE_VIRTVAR(isGoalValid, l_Goals, nullptr, "The goal validity", "boolean", "true if valid, false otherwise")
{
	mission_goal* current = nullptr;
	if (!ade_get_args(L, "o", l_Goals.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	bool valid = current->type & INVALID_GOAL;

	return ade_set_args(L, "b", !valid);
}

} // namespace api
} // namespace scripting