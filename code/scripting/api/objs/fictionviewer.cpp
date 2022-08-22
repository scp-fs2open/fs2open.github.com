#include "fictionviewer.h"

namespace scripting {
namespace api {

//**********HANDLE: cmd_briefing
ADE_OBJ(l_FictionViewerStage, fiction_viewer_stage, "fiction_viewer_stage", "Fiction Viewer stage handle");

ADE_VIRTVAR(TextFile, l_FictionViewerStage, nullptr, "The text file of the stage", "fiction_viewer_stage", "The text file")
{
	fiction_viewer_stage* stage = nullptr;
	if (!ade_get_args(L, "o", l_FictionViewerStage.GetPtr(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage->story_filename);
}

ADE_VIRTVAR(FontFile, l_FictionViewerStage, nullptr, "The text file of the stage", "fiction_viewer_stage", "The text file")
{
	fiction_viewer_stage* stage = nullptr;
	if (!ade_get_args(L, "o", l_FictionViewerStage.GetPtr(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage->font_filename);
}

ADE_VIRTVAR(VoiceFile, l_FictionViewerStage, nullptr, "The text file of the stage", "fiction_viewer_stage", "The text file")
{
	fiction_viewer_stage* stage = nullptr;
	if (!ade_get_args(L, "o", l_FictionViewerStage.GetPtr(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage->voice_filename);
}

} // namespace api
} // namespace scripting