#include "debriefing.h"

namespace scripting {
namespace api {

debrief_stage_h::debrief_stage_h() {}
debrief_stage_h::debrief_stage_h(debrief_stage* db_stage) : stage(db_stage) {}
debrief_stage* debrief_stage_h::getStage() const
{
	return stage;
};

bool debrief_stage_h::IsValid() const
{
	return stage != nullptr;
}

//**********HANDLE: debriefing
ADE_OBJ(l_DebriefStage, debrief_stage_h, "debriefing_stage", "Debriefing stage handle");

ADE_VIRTVAR(Text, l_DebriefStage, nullptr, "The text of the stage", "string", "The text")
{
	debrief_stage_h stage;
	if (!ade_get_args(L, "o", l_DebriefStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage.getStage()->text);
}

ADE_VIRTVAR(AudioFilename,
	l_DebriefStage,
	nullptr,
	"The filename of the audio file to play",
	"string",
	"The file name")
{
	debrief_stage_h stage;
	if (!ade_get_args(L, "o", l_DebriefStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage.getStage()->voice);
}

ADE_VIRTVAR(Recommendation, l_DebriefStage, nullptr, "The recommendation text of the stage", "string", "The recommendation text")
{
	debrief_stage_h stage;
	if (!ade_get_args(L, "o", l_DebriefStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage.getStage()->recommendation_text);
}

ADE_VIRTVAR(checkVisible,
	l_DebriefStage,
	nullptr,
	"Evaluates the stage formula and returns the result. Could potentially have side effects if the stage formula has a 'perform-actions' or similar operator. "
	"Note that the standard UI evaluates the formula exactly once per stage on briefing initialization.",
	"boolean",
	"true if the stage should be displayed, false otherwise")
{
	debrief_stage_h stage;
	if (!ade_get_args(L, "o", l_DebriefStage.Get(&stage))) {
		return ADE_RETURN_FALSE;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (eval_sexp(stage.getStage()->formula)) {
		return ADE_RETURN_TRUE;
	} else {
		return ADE_RETURN_FALSE;
	}
}

//**********HANDLE: debriefing
ADE_OBJ(l_Debrief, int, "debriefing", "Debriefing handle");

ADE_INDEXER(l_Debrief,
	"number index",
	"The list of stages in the debriefing.",
	"debriefing_stage",
	"The stage at the specified location.")
{
	int debriefIdx;
	int index = -1;
	if (!ade_get_args(L, "oi", l_Debrief.Get(&debriefIdx), &index)) {
		return ADE_RETURN_NIL;
	}

	if (debriefIdx < 0)
		return ADE_RETURN_NIL;

	debriefing* debrief = &Debriefings[debriefIdx];

	--index;

	if (index < 0 || index >= debrief->num_stages) {
		LuaError(L, "Invalid index %d, only have %d entries.", index + 1, debrief->num_stages);
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "o", l_DebriefStage.Set(debrief_stage_h(&Debriefings[debriefIdx].stages[index])));
}

ADE_FUNC(__len, l_Debrief, nullptr, "The number of stages in the debriefing", "number", "The number of stages.")
{
	int debrief;
	if (!ade_get_args(L, "o", l_Debrief.Get(&debrief))) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "i", Debriefings[debrief].num_stages);
}

} // namespace api
} // namespace scripting