//
//
#include "ui.h"

#include "globalincs/alphacolors.h"

#include "cmdline/cmdline.h"
#include "gamesnd/eventmusic.h"
#include "menuui/barracks.h"
#include "menuui/mainhallmenu.h"
#include "menuui/optionsmenu.h"
#include "menuui/playermenu.h"
#include "menuui/readyroom.h"
#include "mission/missionbriefcommon.h"
#include "missionui/fictionviewer.h"
#include "mission/missioncampaign.h"
#include "missionui/missionscreencommon.h"
#include "missionui/redalert.h"
#include "playerman/managepilot.h"
#include "scpui/SoundPlugin.h"
#include "scpui/rocket_ui.h"
#include "scripting/api/objs/redalert.h"
#include "scripting/api/objs/fictionviewer.h"
#include "scripting/api/objs/cmd_brief.h"
#include "scripting/api/objs/color.h"
#include "scripting/api/objs/enums.h"
#include "scripting/api/objs/player.h"
#include "scripting/lua/LuaTable.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#include <Rocket/Core/Lua/LuaType.h>

#pragma pop_macro("Assert")

namespace scripting {
namespace api {

//*************************Testing stuff*************************
// This section is for stuff that's considered experimental.
ADE_LIB(l_UserInterface, "UserInterface", "ui", "Functions for managing the \"scpui\" user interface system.");

ADE_FUNC(setOffset, l_UserInterface, "number x, number y",
         "Sets the offset from the top left corner at which <b>all</b> rocket contexts will be rendered", "boolean",
         "true if the operation was successful, false otherwise")
{

	float x;
	float y;

	if (!ade_get_args(L, "ff", &x, &y)) {
		return ADE_RETURN_FALSE;
	}

	scpui::setOffset(x, y);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(enableInput,
	l_UserInterface,
	"any context /* A libRocket Context value */",
	"Enables input for the specified libRocket context",
	"boolean",
	"true if successfull")
{
	using namespace Rocket::Core;

	// Check parameter number
	if (!ade_get_args(L, "*")) {
		return ADE_RETURN_FALSE;
	}

	auto ctx = Lua::LuaType<Context>::check(L, 1);

	if (ctx == nullptr) {
		LuaError(L, "Parameter 1 is not a valid context handle!");
		return ADE_RETURN_FALSE;
	}

	scpui::enableInput(ctx);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(disableInput, l_UserInterface, "", "Disables UI input", "boolean", "true if successfull")
{
	scpui::disableInput();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(playElementSound,
	l_UserInterface,
	"any element /* A libRocket element */, string event, string state = \"\"",
	"Plays an element specific sound with an optional state for differentiating different UI states.",
	"boolean",
	"true if a sound was played, false otherwise")
{
	using namespace Rocket::Core;
	const char* event;
	const char* state = "";
	if (!ade_get_args(L, "*s|s", &event, &state)) {
		return ADE_RETURN_FALSE;
	}

	auto el = Lua::LuaType<Element>::check(L, 1);

	if (el == nullptr) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", scpui::SoundPlugin::instance()->PlayElementSound(el, event, state));
}

ADE_FUNC(maybePlayCutscene, l_UserInterface, "enumeration MovieType, boolean RestartMusic, number ScoreIndex", "Plays a cutscene, if one exists, for the appropriate state transition.  If RestartMusic is true, then the music score at ScoreIndex will be started after the cutscene plays.", nullptr, "Returns nothing")
{
	enum_h movie_type;
	bool restart_music = false;
	int score_index = 0;

	if (!ade_get_args(L, "obi", l_Enum.Get(&movie_type), &restart_music, &score_index))
		return ADE_RETURN_NIL;

	if (!movie_type.IsValid() || movie_type.index < LE_MOVIE_PRE_FICTION || movie_type.index > LE_MOVIE_END_CAMPAIGN)
	{
		Warning(LOCATION, "Invalid movie type index %d", movie_type.index);
		return ADE_RETURN_NIL;
	}

	common_maybe_play_cutscene(movie_type.index - LE_MOVIE_PRE_FICTION, restart_music, score_index);
	return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/PilotSelect
ADE_LIB_DERIV(l_UserInterface_PilotSelect, "PilotSelect", nullptr,
              "API for accessing values specific to the pilot select screen.<br><b>Warning:</b> This is an internal "
              "API for the new UI system. This should not be used by other code and may be removed in the future!",
              l_UserInterface);

ADE_VIRTVAR(MAX_PILOTS, l_UserInterface_PilotSelect, nullptr, "Gets the maximum number of possible pilots.", "number",
            "The maximum number of pilots")
{
	return ade_set_args(L, "i", MAX_PILOTS);
}
ADE_VIRTVAR(WarningCount, l_UserInterface_PilotSelect, nullptr, "The amount of warnings caused by the mod while loading.", "number",
            "The maximum number of pilots")
{
	return ade_set_args(L, "i", Global_warning_count);
}
ADE_VIRTVAR(ErrorCount, l_UserInterface_PilotSelect, nullptr, "The amount of errors caused by the mod while loading.", "number",
            "The maximum number of pilots")
{
	return ade_set_args(L, "i", Global_error_count);
}

ADE_FUNC(enumeratePilots, l_UserInterface_PilotSelect, nullptr,
         "Lists all pilots available for the pilot selection<br>", "string[]",
         "A table containing the pilots (without a file extension) or nil on error")
{
	using namespace luacpp;

	auto table = LuaTable::create(L);

	auto pilots = player_select_enumerate_pilots();
	for (size_t i = 0; i < pilots.size(); ++i) {
		table.addValue(i + 1, pilots[i]);
	}

	return ade_set_args(L, "t", &table);
}

ADE_FUNC(getLastPilot, l_UserInterface_PilotSelect, nullptr,
         "Reads the last active pilot from the config file and returns some information about it. callsign is the name "
         "of the player and is_multi indicates whether the pilot was last active as a multiplayer pilot.",
         "string", "The pilot name or nil if there was no last pilot")
{
	using namespace luacpp;

	auto callsign = player_get_last_player();
	if (callsign.empty()) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "s", callsign.c_str());
}

ADE_FUNC(checkPilotLanguage, l_UserInterface_PilotSelect, "string callsign",
         "Checks if the pilot with the specified callsign has the right language.", "boolean",
         "true if pilot is valid, false otherwise")
{
	const char* callsign;
	if (!ade_get_args(L, "s", &callsign)) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", valid_pilot(callsign, true));
}

ADE_FUNC(selectPilot, l_UserInterface_PilotSelect, "string callsign, boolean is_multi",
         "Selects the pilot with the specified callsign and advances the game to the main menu.", nullptr, "nothing")
{
	const char* callsign;
	bool is_multi;
	if (!ade_get_args(L, "sb", &callsign, &is_multi)) {
		return ADE_RETURN_NIL;
	}

	player_finish_select(callsign, is_multi);

	return ADE_RETURN_NIL;
}

ADE_FUNC(deletePilot, l_UserInterface_PilotSelect, "string callsign",
         "Deletes the pilot with the specified callsign. This is not reversible!", "boolean",
         "true on success, false otherwise")
{
	const char* callsign;
	if (!ade_get_args(L, "s", &callsign)) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "b", delete_pilot_file(callsign));
}

ADE_FUNC(
    createPilot, l_UserInterface_PilotSelect, "string callsign, boolean is_multi, [string copy_from]",
    "Creates a new pilot in either single or multiplayer mode and optionally copies settings from an existing pilot.",
    "boolean", "true on success, false otherwise")
{
	const char* callsign;
	bool is_multi;
	const char* copy_from = nullptr;
	if (!ade_get_args(L, "sb|s", &callsign, &is_multi, &copy_from)) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "b", player_create_new_pilot(callsign, is_multi, copy_from));
}

ADE_FUNC(isAutoselect, l_UserInterface_PilotSelect, nullptr,
         "Determines if the pilot selection screen should automatically select the default user.", "boolean",
         "true if autoselect is enabled, false otherwise")
{
	return ade_set_args(L, "b", Cmdline_benchmark_mode || Cmdline_pilot);
}

ADE_VIRTVAR(CmdlinePilot, l_UserInterface_PilotSelect, nullptr,
			"The pilot chosen from commandline, if any.", "string",
			"The name if specified, nil otherwise")
{
	if (Cmdline_pilot)
		return ade_set_args(L, "s", Cmdline_pilot);
	else
		return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/MainHall
ADE_LIB_DERIV(l_UserInterface_MainHall, "MainHall", nullptr,
              "API for accessing values specific to the main hall screen.<br><b>Warning:</b> This is an internal "
              "API for the new UI system. This should not be used by other code and may be removed in the future!",
              l_UserInterface);

ADE_FUNC(startAmbientSound, l_UserInterface_MainHall, nullptr, "Starts the ambient mainhall sound.", nullptr, "nothing")
{
	(void)L;
	main_hall_start_ambient();
	return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/Barracks
ADE_LIB_DERIV(l_UserInterface_Barracks, "Barracks", nullptr,
              "API for accessing values specific to the barracks screen.<br><b>Warning:</b> This is an internal "
              "API for the new UI system. This should not be used by other code and may be removed in the future!",
              l_UserInterface);

ADE_FUNC(listPilotImages, l_UserInterface_Barracks, nullptr, "Lists the names of the available pilot images.",
         "string[]", "The list of pilot filenames or nil on error")
{
	pilot_load_pic_list();

	using namespace luacpp;

	LuaTable out = LuaTable::create(L);
	for (auto i = 0; i < Num_pilot_images; ++i) {
		out.addValue(i + 1, Pilot_image_names[i]);
	}
	return ade_set_args(L, "t", &out);
}

ADE_FUNC(listSquadImages, l_UserInterface_Barracks, nullptr, "Lists the names of the available squad images.",
         "string[]", "The list of squad filenames or nil on error")
{
	pilot_load_squad_pic_list();

	using namespace luacpp;

	LuaTable out = LuaTable::create(L);
	for (auto i = 0; i < Num_pilot_squad_images; ++i) {
		out.addValue(i + 1, Pilot_squad_image_names[i]);
	}
	return ade_set_args(L, "t", &out);
}

ADE_FUNC(acceptPilot, l_UserInterface_Barracks, "player selection", "Accept the given player as the current player",
         "boolean", "true on sucess, false otherwise")
{
	player_h* plh;
	if (!ade_get_args(L, "o", l_Player.GetPtr(&plh))) {
		return ADE_RETURN_FALSE;
	}

	barracks_accept_pilot(plh->get());
	return ADE_RETURN_TRUE;
}

//**********SUBLIBRARY: UserInterface/OptionsMenu
// This needs a slightly different name since there is already a type called "Options"
ADE_LIB_DERIV(l_UserInterface_Options,
	"OptionsMenu",
	nullptr,
	"API for accessing values specific to the options screen.<br><b>Warning:</b> This is an internal "
	"API for the new UI system. This should not be used by other code and may be removed in the future!",
	l_UserInterface);

ADE_FUNC(playVoiceClip,
	l_UserInterface_Options,
	nullptr,
	"Plays the example voice clip used for checking the voice volume",
	"boolean",
	"true on sucess, false otherwise")
{
	options_play_voice_clip();
	return ADE_RETURN_TRUE;
}

//**********SUBLIBRARY: UserInterface/CampaignMenu
// This needs a slightly different name since there is already a type called "Options"
ADE_LIB_DERIV(l_UserInterface_Campaign,
	"CampaignMenu",
	nullptr,
	"API for accessing data related to the campaign UI.<br><b>Warning:</b> This is an internal "
	"API for the new UI system. This should not be used by other code and may be removed in the future!",
	l_UserInterface);

ADE_FUNC(loadCampaignList,
	l_UserInterface_Campaign,
	nullptr,
	"Loads the list of available campaigns",
	"boolean",
	// ade_type_info({ade_type_array("string"), ade_type_array("string")}),
	"false if something failed while loading the list, true otherwise")
{
	return ade_set_args(L, "b", !campaign_build_campaign_list());
}

ADE_FUNC(getCampaignList,
	l_UserInterface_Campaign,
	nullptr,
	"Get the campaign name and description lists",
	"string[], string[], string[]",
	"Three tables with the names, file names, and descriptions of the campaigns")
{
	luacpp::LuaTable nameTable        = luacpp::LuaTable::create(L);
	luacpp::LuaTable fileNameTable    = luacpp::LuaTable::create(L);
	luacpp::LuaTable descriptionTable = luacpp::LuaTable::create(L);

	for (int i = 0; i < Num_campaigns; ++i) {
		nameTable.addValue(i + 1, Campaign_names[i]);
		fileNameTable.addValue(i + 1, Campaign_file_names[i]);

		auto description = Campaign_descs[i];
		descriptionTable.addValue(i + 1, description ? description : "");
	}

	// We actually do not need this anymore now so free it immediately
	mission_campaign_free_list();

	return ade_set_args(L, "ttt", nameTable, fileNameTable, descriptionTable);
}

ADE_FUNC(selectCampaign,
	l_UserInterface_Campaign,
	"string campaign_file",
	"Selects the specified campaign file name",
	"boolean",
	"true if successful, false otherwise")
{
	const char* filename = nullptr;
	if (!ade_get_args(L, "s", &filename)) {
		return ADE_RETURN_FALSE;
	}

	campaign_select_campaign(filename);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(resetCampaign,
	l_UserInterface_Campaign,
	"string campaign_file",
	"Resets the campaign with the specified file name",
	"boolean",
	"true if successful, false otherwise")
{
	const char* filename = nullptr;
	if (!ade_get_args(L, "s", &filename)) {
		return ADE_RETURN_FALSE;
	}

	campaign_reset(filename);

	return ADE_RETURN_TRUE;
}

//**********SUBLIBRARY: UserInterface/CommandBriefing
// This needs a slightly different name since there is already a type called "Options"
ADE_LIB_DERIV(l_UserInterface_CmdBrief,
	"CommandBriefing",
	nullptr,
	"API for accessing data related to the command briefing UI.<br><b>Warning:</b> This is an internal "
	"API for the new UI system. This should not be used by other code and may be removed in the future!",
	l_UserInterface);

ADE_VIRTVAR(ColorTags,
	l_UserInterface_CmdBrief,
	nullptr,
	"The available tagged colors",
	"{ string => color ... }",
	"A mapping from tag string to color value")
{
	using namespace luacpp;

	LuaTable mapping = LuaTable::create(L);

	for (const auto& tagged : Tagged_Colors) {
		SCP_string tag;
		tag.resize(1, tagged.first);

		mapping.addValue(tag, l_Color.Set(*tagged.second));
	}

	return ade_set_args(L, "t", mapping);
}

ADE_VIRTVAR(DefaultTextColorTag,
	l_UserInterface_CmdBrief,
	nullptr,
	"Gets the default color tag string for the command briefing. Index into ColorTags.",
	"string",
	"The default color tag")
{
	SCP_string tagStr;

	auto defaultColor = default_command_briefing_color;

	if (defaultColor == '\0' || !brief_verify_color_tag(defaultColor)) {
		defaultColor = Color_Tags[0];
	}
	tagStr.resize(1, defaultColor);

	return ade_set_args(L, "s", tagStr);
}

ADE_FUNC(getBriefing,
	l_UserInterface_CmdBrief,
	nullptr,
	"Get the command briefing.",
	"cmd_briefing",
	"The briefing data")
{
	// The cmd briefing code has support for specifying the team but only sets the index to 0
	return ade_set_args(L, "o", l_CmdBrief.Set(Cmd_briefs[0]));
}

ADE_FUNC(getBriefingMusicName, l_UserInterface_CmdBrief, nullptr, "Gets the file name of the music file to play for the briefing.", "string", "The file name or empty if no music")
{
	return ade_set_args(L, "s", common_music_get_filename(SCORE_BRIEFING).c_str());
}

ADE_FUNC(runBriefingStageHook, l_UserInterface_CmdBrief, "number oldStage, number newStage", "Run $On Briefing Stage: hooks.", nullptr, nullptr)
{
	int oldStage = -1, newStage = -1;
	if (ade_get_args(L, "ii", &oldStage, &newStage) == 2)
	{
		// Subtract 1 to convert from Lua conventions to C conventions
		common_fire_stage_script_hook(oldStage -1, newStage -1);
	}
	else
	{
		LuaError(L, "Bad arguments given to ui.runBriefingStageHook!");
	}
	return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/RedAlert
ADE_LIB_DERIV(l_UserInterface_RedAlert,
	"RedAlert",
	nullptr,
	"API for accessing data related to the Red Alert UI.<br><b>Warning:</b> This is an internal "
	"API for the new UI system. This should not be used by other code and may be removed in the future!",
	l_UserInterface);

 ADE_FUNC(getRedAlert,
	l_UserInterface_RedAlert,
	nullptr,
	"Get the red alert brief.",
	"red_alert_stage",
	"The red-alert data")
{
	
	 if (Briefing[0].num_stages) {
		return ade_set_args(L, "o", l_RedAlertStage.Set(Briefings[0].stages[0]));
	 } else {
		 ADE_RETURN_NIL;
	 }
	 
}

ADE_FUNC(replayPreviousMission,
	l_UserInterface_RedAlert,
	nullptr,
	"Loads the previous mission of the campaign, does nothing if not in campaign",
	"boolean",
	"Returns true if the operation was successful, false otherwise")
{
	if (!mission_campaign_previous_mission()) {
		return ADE_RETURN_FALSE;
	} else {
		return ADE_RETURN_TRUE;
	}

}

//**********SUBLIBRARY: UserInterface/FictionViewer
ADE_LIB_DERIV(l_UserInterface_FictionViewer,
	"FictionViewer",
	nullptr,
	"API for accessing data related to the fiction viewer UI.<br><b>Warning:</b> This is an internal "
	"API for the new UI system. This should not be used by other code and may be removed in the future!",
	l_UserInterface);

ADE_FUNC(getFiction, l_UserInterface_FictionViewer, nullptr, "Get the fiction.", "fiction_viewer_stage", "The fiction data")
{
	return ade_set_args(L, "o", l_FictionViewerStage.Set(Fiction_viewer_stages[Fiction_viewer_active_stage]));
}

ADE_FUNC(getFictionMusicName, l_UserInterface_FictionViewer, nullptr,
	"Gets the file name of the music file to play for the fiction viewer.",
	"string",
	"The file name or empty if no music")
{
	return ade_set_args(L, "s", common_music_get_filename(SCORE_FICTION_VIEWER).c_str());
}

} // namespace api
} // namespace scripting
