#include "redalert.h"

namespace scripting {
namespace api {

redalert_stage_h::redalert_stage_h() : ra_brief(-1), ra_stage(-1) {}

redalert_stage_h::redalert_stage_h(int brief, int stage) : ra_brief(brief), ra_stage(stage) {}

bool redalert_stage_h::IsValid() const
{
	return ra_brief >= 0 && ra_stage >= 0;
}

brief_stage* redalert_stage_h::getStage() const
{
	return &Briefings[ra_brief].stages[ra_stage];
}

//**********HANDLE: red alert
ADE_OBJ(l_RedAlertStage, redalert_stage_h, "red_alert_stage", "Red Alert stage handle");

ADE_VIRTVAR(Text, l_RedAlertStage, nullptr, "The briefing text of the stage", "string", "The text string")
{
	redalert_stage_h current;
	if (!ade_get_args(L, "o", l_RedAlertStage.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getStage()->text.c_str());
}

ADE_VIRTVAR(AudioFilename, l_RedAlertStage, nullptr, "The audio file of the stage", "string", "The audio file")
{
	redalert_stage_h current;
	if (!ade_get_args(L, "o", l_RedAlertStage.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getStage()->voice);
}

} // namespace api
} // namespace scripting