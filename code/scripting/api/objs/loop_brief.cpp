#include "loop_brief.h"

namespace scripting {
namespace api {

cmission_h::cmission_h() : l_stage(-1) {}

cmission_h::cmission_h(int stage) : l_stage(stage) {}

bool cmission_h::IsValid() const
{
	return l_stage >= 0;
}

cmission* cmission_h::getStage() const
{
	return &Campaign.missions[l_stage];
}

//**********HANDLE: loop_briefing
ADE_OBJ(l_LoopBriefStage, cmission_h, "loop_brief_stage", "Loop Brief stage handle");

ADE_VIRTVAR(Text,
	l_LoopBriefStage,
	nullptr,
	"The text of the stage",
	"string",
	"The text")
{
	cmission_h current;
	if (!ade_get_args(L, "o", l_LoopBriefStage.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getStage()->mission_branch_desc);
}

ADE_VIRTVAR(AniFilename,
	l_LoopBriefStage,
	nullptr,
	"The ani filename of the stage",
	"string",
	"The ani filename")
{
	cmission_h current;
	if (!ade_get_args(L, "o", l_LoopBriefStage.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getStage()->mission_branch_brief_anim);
}

ADE_VIRTVAR(AudioFilename,
	l_LoopBriefStage,
	nullptr,
	"The audio file of the stage",
	"string",
	"The audio filename")
{
	cmission_h current;
	if (!ade_get_args(L, "o", l_LoopBriefStage.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getStage()->mission_branch_brief_sound);
}

} // namespace api
} // namespace scripting