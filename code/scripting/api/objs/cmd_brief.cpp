
#include "cmd_brief.h"

namespace scripting {
namespace api {

//**********HANDLE: cmd_briefing
ADE_OBJ(l_CmdBriefStage, cmd_brief_stage, "cmd_briefing_stage", "Command briefing stage handle");

ADE_VIRTVAR(Text, l_CmdBriefStage, nullptr, "The text of the stage", "cmd_briefing_stage", "The text")
{
	cmd_brief_stage* stage = nullptr;
	if (!ade_get_args(L, "o", l_CmdBriefStage.GetPtr(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage->text);
}

ADE_VIRTVAR(AniFilename,
	l_CmdBriefStage,
	nullptr,
	"The filename of the animation to play",
	"cmd_briefing_stage",
	"The file name")
{
	cmd_brief_stage* stage = nullptr;
	if (!ade_get_args(L, "o", l_CmdBriefStage.GetPtr(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage->ani_filename);
}

ADE_VIRTVAR(AudioFilename,
	l_CmdBriefStage,
	nullptr,
	"The filename of the audio file to play",
	"cmd_briefing_stage",
	"The file name")
{
	cmd_brief_stage* stage = nullptr;
	if (!ade_get_args(L, "o", l_CmdBriefStage.GetPtr(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage->wave_filename);
}

//**********HANDLE: cmd_briefing
ADE_OBJ(l_CmdBrief, cmd_brief, "cmd_briefing", "Command briefing handle");

ADE_INDEXER(l_CmdBrief,
	"number index",
	"The list of stages in the command briefing.",
	"cmd_briefing_stage",
	"The stage at the specified location.")
{
	cmd_brief* brief = nullptr;
	int index        = -1;
	if (!ade_get_args(L, "oi", l_CmdBrief.GetPtr(&brief), &index)) {
		return ADE_RETURN_NIL;
	}

	--index;

	if (index < 0 || index >= brief->num_stages) {
		LuaError(L, "Invalid index %d, only have %d entries.", index + 1, brief->num_stages);
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "o", l_CmdBriefStage.Set(brief->stage[index]));
}

ADE_FUNC(__len, l_CmdBrief, nullptr, "The number of stages in the command briefing", "number", "The number of stages.")
{
	cmd_brief* brief = nullptr;
	if (!ade_get_args(L, "o", l_CmdBrief.GetPtr(&brief))) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "i", brief->num_stages);
}

} // namespace api
} // namespace scripting
