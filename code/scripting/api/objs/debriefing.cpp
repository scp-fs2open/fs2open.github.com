#include "debriefing.h"

namespace scripting {
namespace api {

//**********HANDLE: debriefing
ADE_OBJ(l_DebriefStage, debrief_stage, "debriefing_stage", "Debriefing stage handle");

ADE_VIRTVAR(Text, l_DebriefStage, nullptr, "The text of the stage", "debriefing_stage", "The text")
{
	debrief_stage* stage = nullptr;
	if (!ade_get_args(L, "o", l_DebriefStage.GetPtr(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage->text);
}

ADE_VIRTVAR(AudioFilename,
	l_DebriefStage,
	nullptr,
	"The filename of the audio file to play",
	"debriefing_stage",
	"The file name")
{
	debrief_stage* stage = nullptr;
	if (!ade_get_args(L, "o", l_DebriefStage.GetPtr(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage->voice);
}

ADE_VIRTVAR(Recommendation, l_DebriefStage, nullptr, "The recommendation text of the stage", "debriefing_stage", "The recommendation text")
{
	debrief_stage* stage = nullptr;
	if (!ade_get_args(L, "o", l_DebriefStage.GetPtr(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage->recommendation_text);
}

ADE_VIRTVAR(isVisible,
	l_DebriefStage,
	nullptr,
	"The result of the stage formula",
	"debriefing_stage",
	"true if the stage should be displayed, false otherwise")
{
	debrief_stage* stage = nullptr;
	if (!ade_get_args(L, "o", l_DebriefStage.GetPtr(&stage))) {
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

//**********HANDLE: debriefing
ADE_OBJ(l_Debrief, debriefing, "debriefing", "Debriefing handle");

ADE_INDEXER(l_Debrief,
	"number index",
	"The list of stages in the debriefing.",
	"debriefing_stage",
	"The stage at the specified location.")
{
	debriefing* debrief = nullptr;
	int index = -1;
	if (!ade_get_args(L, "oi", l_Debrief.GetPtr(&debrief), &index)) {
		return ADE_RETURN_NIL;
	}

	--index;

	if (index < 0 || index >= debrief->num_stages) {
		LuaError(L, "Invalid index %d, only have %d entries.", index + 1, debrief->num_stages);
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "o", l_DebriefStage.Set(debrief->stages[index]));
}

ADE_FUNC(__len, l_Debrief, nullptr, "The number of stages in the debriefing", "number", "The number of stages.")
{
	debriefing* debrief = nullptr;
	if (!ade_get_args(L, "o", l_Debrief.GetPtr(&debrief))) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "i", debrief->num_stages);
}

} // namespace api
} // namespace scripting