
#include "cmd_brief.h"

namespace scripting {
namespace api {

cmd_brief_stage_h::cmd_brief_stage_h() : cmd_brief(-1), cmd_stage(-1) { }

cmd_brief_stage_h::cmd_brief_stage_h(int brief, int stage) : cmd_brief(brief), cmd_stage(stage) { }

bool cmd_brief_stage_h::IsValid() const {
	return cmd_brief >= 0 && cmd_stage >= 0;
}

cmd_brief_stage* cmd_brief_stage_h::getStage() const {
	return &Cmd_briefs[cmd_brief].stage[cmd_stage];
}

//**********HANDLE: cmd_briefing
ADE_OBJ(l_CmdBriefStage, cmd_brief_stage_h, "cmd_briefing_stage", "Command briefing stage handle");

ADE_VIRTVAR(Text, l_CmdBriefStage, nullptr, "The text of the stage", "string", "The text")
{
	cmd_brief_stage_h stage;
	if (!ade_get_args(L, "o", l_CmdBriefStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage.getStage()->text);
}

ADE_VIRTVAR(AniFilename,
	l_CmdBriefStage,
	nullptr,
	"The filename of the animation to play",
	"string",
	"The file name")
{
	cmd_brief_stage_h stage;
	if (!ade_get_args(L, "o", l_CmdBriefStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage.getStage()->ani_filename);
}

ADE_VIRTVAR(AudioFilename,
	l_CmdBriefStage,
	nullptr,
	"The filename of the audio file to play",
	"string",
	"The file name")
{
	cmd_brief_stage_h stage;
	if (!ade_get_args(L, "o", l_CmdBriefStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage.getStage()->wave_filename);
}

//**********HANDLE: cmd_briefing
ADE_OBJ(l_CmdBrief, int, "cmd_briefing", "Command briefing handle");

ADE_INDEXER(l_CmdBrief,
	"number index",
	"The list of stages in the command briefing.",
	"cmd_briefing_stage",
	"The stage at the specified location.")
{
	int briefIdx;
	int index        = -1;
	if (!ade_get_args(L, "oi", l_CmdBrief.Get(&briefIdx), &index)) {
		return ADE_RETURN_NIL;
	}

	if (briefIdx < 0)
		return ADE_RETURN_NIL;

	cmd_brief* brief = &Cmd_briefs[briefIdx];

	--index;

	if (index < 0 || index >= brief->num_stages) {
		LuaError(L, "Invalid index %d, only have %d entries.", index + 1, brief->num_stages);
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "o", l_CmdBriefStage.Set(cmd_brief_stage_h(briefIdx, index)));
}

ADE_FUNC(__len, l_CmdBrief, nullptr, "The number of stages in the command briefing", "number", "The number of stages.")
{
	int brief;
	if (!ade_get_args(L, "o", l_CmdBrief.Get(&brief))) {
		return ADE_RETURN_NIL;
	}

	if (brief < 0)
		return ADE_RETURN_NIL;

	return ade_set_args(L, "i", Cmd_briefs[brief].num_stages);
}

} // namespace api
} // namespace scripting
