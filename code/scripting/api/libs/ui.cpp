//
//
#include "ui.h"

#include "globalincs/alphacolors.h"

#include "cmdline/cmdline.h"
#include "gamesnd/eventmusic.h"
#include "gamesequence/gamesequence.h"
#include "menuui/barracks.h"
#include "menuui/mainhallmenu.h"
#include "menuui/optionsmenu.h"
#include "menuui/playermenu.h"
#include "menuui/readyroom.h"
#include "mission/missionmessage.h"
#include "mission/missiongoals.h"
#include "mission/missionbriefcommon.h"
#include "mission/missionparse.h"
#include "missionui/fictionviewer.h"
#include "missionui/missionbrief.h"
#include "mission/missioncampaign.h"
#include "mission/missionbriefcommon.h"
#include "missionui/missionscreencommon.h"
#include "mission/missioncampaign.h"
#include "missionui/redalert.h"
#include "network/multi.h"
#include "playerman/managepilot.h"
#include "scpui/SoundPlugin.h"
#include "scpui/rocket_ui.h"
#include "scripting/api/objs/loop_brief.h"
#include "scripting/api/objs/redalert.h"
#include "scripting/api/objs/fictionviewer.h"
#include "scripting/api/objs/cmd_brief.h"
#include "scripting/api/objs/briefing.h"
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

ADE_VIRTVAR(ColorTags,
	l_UserInterface,
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

ADE_FUNC(DefaultTextColorTag,
	l_UserInterface,
	"number UiScreen",
	"Gets the default color tag string for the specified state. 1 for Briefing, 2 for CBriefing, 3 for Debriefing, "
	"4 for Fiction Viewer, 5 for Red Alert, 6 for Loop Briefing, 7 for Recommendation text. Defaults to 1. Index into ColorTags.",
	"string",
	"The default color tag")
{
	int UiScreen;

	if (!ade_get_args(L, "i", &UiScreen)) {
		UiScreen = 1;
	}
	
	SCP_string tagStr;
	char defaultColor = default_briefing_color;

	switch (UiScreen) {
		case 1:
			defaultColor = default_briefing_color;
			break;
		case 2:
			defaultColor = default_command_briefing_color;
			break;
		case 3:
			defaultColor = default_debriefing_color;
			break;
		case 4:
			defaultColor = default_fiction_viewer_color;
			break;
		case 5:
			defaultColor = default_redalert_briefing_color;
			break;
		case 6:
			defaultColor = default_loop_briefing_color;
			break;
		case 7:
			defaultColor = default_recommendation_color;
			break;
	}

	if (defaultColor == '\0' || !brief_verify_color_tag(defaultColor)) {
		defaultColor = Color_Tags[0];
	}
	tagStr.resize(1, defaultColor);

	return ade_set_args(L, "s", tagStr);
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

//**********SUBLIBRARY: UserInterface/Briefing
ADE_LIB_DERIV(l_UserInterface_Brief,
	"Briefing",
	nullptr,
	"API for accessing data related to the briefing UI.<br><b>Warning:</b> This is an internal "
	"API for the new UI system. This should not be used by other code and may be removed in the future!",
	l_UserInterface);

ADE_FUNC(getBriefingMusicName,
	l_UserInterface_Brief,
	nullptr,
	"Gets the file name of the music file to play for the briefing.",
	"string",
	"The file name or empty if no music")
{
	return ade_set_args(L, "s", common_music_get_filename(SCORE_BRIEFING).c_str());
}

ADE_FUNC(runBriefingStageHook,
	l_UserInterface_Brief,
	"number oldStage, number newStage",
	"Run $On Briefing Stage: hooks.",
	nullptr,
	nullptr)
{
	int oldStage = -1, newStage = -1;
	if (ade_get_args(L, "ii", &oldStage, &newStage) == 2) {
		// Subtract 1 to convert from Lua conventions to C conventions
		common_fire_stage_script_hook(oldStage - 1, newStage - 1);
	} else {
		LuaError(L, "Bad arguments given to ui.runBriefingStageHook!");
	}
	return ADE_RETURN_NIL;
}

ADE_FUNC(getBriefing,
	l_UserInterface_Brief,
	nullptr,
	"Get the briefing",
	"briefing",
	"The briefing data")
{
	// get a pointer to the appropriate briefing structure
	if (MULTI_TEAM) {
		return ade_set_args(L, "o", l_Brief.Set(Net_player->p_info.team));
	} else {
		return ade_set_args(L, "o", l_Brief.Set(0));
	}

}

ADE_FUNC(exitLoop,
	l_UserInterface_Brief,
	nullptr,
	"Skips the current mission, exits the campaign loop, and loads the next non-loop mission in a campaign",
	nullptr,
	"nothing")
{
	(void)L;
	mission_campaign_exit_loop();
	return ADE_RETURN_NIL;
}

ADE_FUNC(skipMission,
	l_UserInterface_Brief,
	nullptr,
	"Skips the current mission, and loads the next mission in a campaign",
	nullptr,
	"nothing")
{
	(void)L;
	mission_campaign_skip_to_next();
	return ADE_RETURN_NIL;
}

ADE_FUNC(skipTraining,
	l_UserInterface_Brief,
	nullptr,
	"Skips the current training mission, and loads the next mission in a campaign",
	nullptr,
	"nothing")
{
	(void)L;
	// page out mission messages
	message_mission_shutdown();

	if (!(Game_mode & GM_CAMPAIGN_MODE)) {
		gameseq_post_event(GS_EVENT_MAIN_MENU);
	}

	// tricky part.  Need to move to the next mission in the campaign.
	mission_goal_mark_objectives_complete();
	mission_goal_fail_incomplete();
	mission_campaign_store_goals_and_events_and_variables();

	mission_campaign_eval_next_mission();
	mission_campaign_mission_over();

	if (Campaign.next_mission == -1 || (The_mission.flags[Mission::Mission_Flags::End_to_mainhall])) {
		gameseq_post_event(GS_EVENT_MAIN_MENU);
	} else {
		gameseq_post_event(GS_EVENT_START_GAME);
	}
	return ADE_RETURN_NIL;
}

ADE_FUNC(startBriefingMap,
	l_UserInterface_Brief,
	nullptr,
	"Starts the briefing map for the current mission. Doesn't currently do anything.",
	nullptr,
	"nothing")
{
	(void)L;
	return ADE_RETURN_NIL;
}

ADE_LIB_DERIV(l_Briefing_Goals, "Objectives", nullptr, nullptr, l_UserInterface_Brief);
ADE_INDEXER(l_Briefing_Goals,
	"number Index",
	"Array of goals",
	"mission_goal",
	"goal handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "o", l_Goals.Set(idx));
}

ADE_FUNC(__len, l_Briefing_Goals, nullptr, "The number of goals in the mission", "number", "The number of goals.")
{
	return ade_set_args(L, "i", Num_goals);
}

//**********SUBLIBRARY: UserInterface/CommandBriefing
ADE_LIB_DERIV(l_UserInterface_CmdBrief,
	"CommandBriefing",
	nullptr,
	"API for accessing data related to the command briefing UI.<br><b>Warning:</b> This is an internal "
	"API for the new UI system. This should not be used by other code and may be removed in the future!",
	l_UserInterface);

ADE_FUNC(getCmdBriefing,
	l_UserInterface_CmdBrief,
	nullptr,
	"Get the command briefing.",
	"cmd_briefing",
	"The briefing data")
{
	// The cmd briefing code has support for specifying the team but only sets the index to 0
	return ade_set_args(L, "o", l_CmdBrief.Set(Cmd_briefs[0]));
}

//**********SUBLIBRARY: UserInterface/LoopBrief
ADE_LIB_DERIV(l_UserInterface_LoopBrief,
	"LoopBrief",
	nullptr,
	"API for accessing data related to the loop brief UI.<br><b>Warning:</b> This is an internal "
	"API for the new UI system. This should not be used by other code and may be removed in the future!",
	l_UserInterface);

ADE_FUNC(getLoopBrief,
	l_UserInterface_LoopBrief,
	nullptr,
	"Get the loop brief.",
	"loop_brief_stage",
	"The loop brief data")
{
	return ade_set_args(L, "o", l_LoopBriefStage.Set(Campaign.missions[Campaign.current_mission]));
}

ADE_FUNC(setLoopChoice, l_UserInterface_LoopBrief, "boolean", "Accepts mission outcome and then True to go to loop, False to skip", nullptr, nullptr)
{
	bool choice = false;
	ade_get_args(L, "|b", &choice);

	if (choice) {
		// select the loop mission
		Campaign.loop_enabled = 1;
		Campaign.loop_reentry = Campaign.next_mission; // save reentry pt, so we can break out of loop
		Campaign.next_mission = Campaign.loop_mission;

		mission_campaign_mission_over();
	} else {
		mission_campaign_mission_over();
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

	 if (Briefings[0].num_stages) {
		return ade_set_args(L, "o", l_RedAlertStage.Set(Briefings[0].stages[0]));
	 } else {
		 return ADE_RETURN_NIL;
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
	if (Fiction_viewer_active_stage >= 0) {
		return ade_set_args(L, "o", l_FictionViewerStage.Set(Fiction_viewer_stages[Fiction_viewer_active_stage]));
	} else {
		return ADE_RETURN_NIL;
	}
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
