#include "fictionviewer.h"
#include "localization/localize.h"

namespace scripting {
namespace api {

fiction_viewer_stage_h::fiction_viewer_stage_h() : f_stage(-1) {}

fiction_viewer_stage_h::fiction_viewer_stage_h(int stage) : f_stage(stage) {}

bool fiction_viewer_stage_h::IsValid() const
{
	return f_stage >= 0;
}

fiction_viewer_stage* fiction_viewer_stage_h::getStage() const
{
	return &Fiction_viewer_stages[f_stage];
}

//**********HANDLE: cmd_briefing
ADE_OBJ(l_FictionViewerStage, fiction_viewer_stage_h, "fiction_viewer_stage", "Fiction Viewer stage handle");

ADE_VIRTVAR(TextFile, l_FictionViewerStage, nullptr, "The text file of the stage", "string", "The text filename")
{
	fiction_viewer_stage_h stage;
	if (!ade_get_args(L, "o", l_FictionViewerStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}
	SCP_string localized_filename;
	localized_filename = stage.getStage()->story_filename;

	// check if a localized version of the file exists
	int lang = lcl_get_current_lang_index();
	if (lang > 0) {
		size_t lastindex = localized_filename.find_last_of(".");
		localized_filename = localized_filename.substr(0, lastindex);
		localized_filename = localized_filename + "-" + Lcl_languages[lang].lang_ext + ".txt";
	}

	// if it localized doesn't exist then revert to the base filename
	if (!cf_exists_full(localized_filename.c_str(), CF_TYPE_FICTION))
		localized_filename = stage.getStage()->story_filename;

	return ade_set_args(L, "s", localized_filename);
}

ADE_VIRTVAR(FontFile, l_FictionViewerStage, nullptr, "The font file of the stage", "string", "The font filename")
{
	fiction_viewer_stage_h stage;
	if (!ade_get_args(L, "o", l_FictionViewerStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage.getStage()->font_filename);
}

ADE_VIRTVAR(VoiceFile, l_FictionViewerStage, nullptr, "The voice file of the stage", "string", "The voice filename")
{
	fiction_viewer_stage_h stage;
	if (!ade_get_args(L, "o", l_FictionViewerStage.Get(&stage))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", stage.getStage()->voice_filename);
}

} // namespace api
} // namespace scripting