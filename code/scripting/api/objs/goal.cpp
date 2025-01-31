
#include "goal.h"
#include "scripting/ade_args.h"
#include "scripting/ade.h"
#include "scripting/api/objs/team.h"
#include "mission/missiongoals.h"

namespace scripting::api
{
//**********HANDLE: mission goal
ADE_OBJ(l_Goal, int, "mission_goal", "Mission goal handle");

ADE_VIRTVAR(Name, l_Goal, nullptr, "The name of the goal", "string", "The goal name")
{
	int current;
	if (!ade_get_args(L, "o", l_Goal.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", Mission_goals[current].name.c_str());
}

ADE_VIRTVAR(Message, l_Goal, nullptr, "The message of the goal", "string", "The goal message")
{
	int current;
	if (!ade_get_args(L, "o", l_Goal.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", Mission_goals[current].message.c_str());
}

ADE_VIRTVAR(Type, l_Goal, nullptr, "The goal type", "string", "primary, secondary, bonus, or none")
{
	int current;
	if (!ade_get_args(L, "o", l_Goal.Get(&current))) {
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

ADE_VIRTVAR(Team, l_Goal, nullptr, "The goal team", "team", "The goal team")
{
	int current;
	if (!ade_get_args(L, "o", l_Goal.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "o", l_Team.Set(Mission_goals[current].team));
}

ADE_VIRTVAR(isGoalSatisfied, l_Goal, nullptr, "The status of the goal", "number", "0 if failed, 1 if complete, 2 if incomplete")
{
	int current;
	if (!ade_get_args(L, "o", l_Goal.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", Mission_goals[current].satisfied);
}

ADE_VIRTVAR(Score, l_Goal, nullptr, "The score of the goal", "number", "the score")
{
	int current;
	if (!ade_get_args(L, "o", l_Goal.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", Mission_goals[current].score);
}

ADE_VIRTVAR(isGoalValid, l_Goal, nullptr, "The goal validity", "boolean", "true if valid, false otherwise")
{
	int current;
	if (!ade_get_args(L, "o", l_Goal.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", !(Mission_goals[current].type & INVALID_GOAL));
}

ADE_FUNC(isValid, l_Goal, nullptr, "Detect if the handle is valid", "boolean", "true if valid, false otherwise")
{
	int current = -1;

	if (!ade_get_args(L, "o", l_Goal.Get(&current)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", (current >= 0) && (current < (int)Mission_goals.size()));
}

}
