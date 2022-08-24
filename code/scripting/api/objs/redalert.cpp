#include "redalert.h"

namespace scripting {
namespace api {

//**********HANDLE: loop_briefing
ADE_OBJ(l_RedAlertStage, briefing, "red_alert_stage", "Red Alert stage handle");

ADE_VIRTVAR(Text, l_RedAlertStage, nullptr, "The briefing text of the stage", "red_alert_stage", "The text string")
{
	briefing* current = nullptr;
	if (!ade_get_args(L, "o", l_RedAlertStage.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->stages[0].text.c_str());
}

ADE_VIRTVAR(AudioFilename, l_RedAlertStage, nullptr, "The audio file of the stage", "red_alert_stage", "The  audio file")
{
	briefing* current = nullptr;
	if (!ade_get_args(L, "o", l_RedAlertStage.GetPtr(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current->stages[0].voice);
}

} // namespace api
} // namespace scripting