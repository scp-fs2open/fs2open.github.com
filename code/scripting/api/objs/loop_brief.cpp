#include "loop_brief.h"

namespace scripting {
namespace api {

//**********HANDLE: loop_briefing
ADE_OBJ(l_LoopBriefStage, cmission, "loop_brief_stage", "Loop Brief stage handle");

ADE_VIRTVAR(Text,
	l_LoopBriefStage,
	nullptr,
	"The text file of the stage",
	"loop_brief_stage",
	"The text file")
{
	cmission* current = nullptr;
	if (!ade_get_args(L, "o", l_LoopBriefStage.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->mission_branch_desc);
}

ADE_VIRTVAR(AniFilename,
	l_LoopBriefStage,
	nullptr,
	"The text file of the stage",
	"loop_brief_stage",
	"The text file")
{
	cmission* current = nullptr;
	if (!ade_get_args(L, "o", l_LoopBriefStage.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->mission_branch_brief_anim);
}

ADE_VIRTVAR(AudioFilename,
	l_LoopBriefStage,
	nullptr,
	"The text file of the stage",
	"loop_brief_stage",
	"The text file")
{
	cmission* current = nullptr;
	if (!ade_get_args(L, "o", l_LoopBriefStage.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->mission_branch_brief_sound);
}

} // namespace api
} // namespace scripting