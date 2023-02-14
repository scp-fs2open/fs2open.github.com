//
//
#include "ui.h"

#include "freespace.h"

#include "globalincs/alphacolors.h"

#include "cmdline/cmdline.h"
#include "cutscene/cutscenes.h"
#include "gamesnd/eventmusic.h"
#include "gamesequence/gamesequence.h"
#include "menuui/barracks.h"
#include "menuui/credits.h"
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
#include "missionui/missionscreencommon.h"
#include "missionui/missiondebrief.h"
#include "mission/missiongoals.h"
#include "mission/missioncampaign.h"
#include "missionui/redalert.h"
#include "missionui/missionpause.h"
#include "mod_table/mod_table.h"
#include "network/multi.h"
#include "network/multiteamselect.h"
#include "playerman/managepilot.h"
#include "radar/radarsetup.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "scpui/SoundPlugin.h"
#include "scpui/rocket_ui.h"
#include "scripting/api/objs/techroom.h"
#include "scripting/api/objs/loop_brief.h"
#include "scripting/api/objs/redalert.h"
#include "scripting/api/objs/fictionviewer.h"
#include "scripting/api/objs/cmd_brief.h"
#include "scripting/api/objs/briefing.h"
#include "scripting/api/objs/debriefing.h"
#include "scripting/api/objs/shipwepselect.h"
#include "scripting/api/objs/color.h"
#include "scripting/api/objs/enums.h"
#include "scripting/api/objs/player.h"
#include "scripting/api/objs/texture.h"
#include "scripting/lua/LuaTable.h"
#include "sound/audiostr.h"
#include "stats/medals.h"
#include "stats/stats.h"

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

ADE_FUNC(playCutscene, l_UserInterface, "string Filename, boolean RestartMusic, number ScoreIndex", "Plays a cutscene.  If RestartMusic is true, then the music score at ScoreIndex will be started after the cutscene plays.", nullptr, "Returns nothing")
{
	const char* filename;
	bool restart_music = false;
	int score_index = 0;

	if (!ade_get_args(L, "sbi", &filename, &restart_music, &score_index))
		return ADE_RETURN_NIL;

	common_play_cutscene(filename, restart_music, score_index);
	return ADE_RETURN_NIL;
}

ADE_FUNC(linkTexture, l_UserInterface, "texture texture", "Links a texture directly to librocket.", "string", "The url string for librocket, or an empty string if invalid.")
{
	texture_h* tex;

	if (!ade_get_args(L, "o", l_Texture.GetPtr(&tex)))
		return ade_set_error(L, "s", "");

	if(tex == nullptr || !tex->isValid())
		return ade_set_error(L, "s", "");
	
	return ade_set_args(L, "s", "data:image/bmpman," + std::to_string(tex->handle));
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
	SCP_UNUSED(L);
	main_hall_start_ambient();
	return ADE_RETURN_NIL;
}

ADE_FUNC(stopAmbientSound, l_UserInterface_MainHall, nullptr, "Stops the ambient mainhall sound.", nullptr, "nothing")
{
	SCP_UNUSED(L);
	main_hall_stop_ambient();
	return ADE_RETURN_NIL;
}

ADE_FUNC(startMusic, l_UserInterface_MainHall, nullptr, "Starts the mainhall music.", nullptr, "nothing")
{
	SCP_UNUSED(L);
	main_hall_start_music();
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

ADE_FUNC(initBriefing,
	l_UserInterface_Brief,
	nullptr,
	"Initializes the briefing and prepares the map for drawing.  Also handles various non-UI housekeeping tasks "
	"and compacts the stages to remove those that should not be shown.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	brief_api_init();

	return ADE_RETURN_NIL;
}

ADE_FUNC(closeBriefing,
	l_UserInterface_Brief,
	nullptr,
	"Closes the briefing and pauses the map. Required after using the briefing API!",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);
	brief_api_close();
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
	"Skips the current mission, exits the campaign loop, and loads the next non-loop mission in a campaign. Returns to the main hall if the player is not in a campaign.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	if (!(Game_mode & GM_CAMPAIGN_MODE)) {
		gameseq_post_event(GS_EVENT_MAIN_MENU);
	}

	mission_campaign_exit_loop();

	return ADE_RETURN_NIL;
}

ADE_FUNC(skipMission,
	l_UserInterface_Brief,
	nullptr,
	"Skips the current mission, and loads the next mission in a campaign. Returns to the main hall if the player is not in a campaign.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	if (!(Game_mode & GM_CAMPAIGN_MODE)) {
		gameseq_post_event(GS_EVENT_MAIN_MENU);
	}

	mission_campaign_skip_to_next();

	return ADE_RETURN_NIL;
}

ADE_FUNC(skipTraining,
	l_UserInterface_Brief,
	nullptr,
	"Skips the current training mission, and loads the next mission in a campaign. Returns to the main hall if the player is not in a campaign.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

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

// For now any loadout error checking needs to happen in the script to prevent FSO from
// generating a popup that will not be interactible from Librocket. A later update to this
// method will introduce return values instead of generating popups and those return values
// can be used to handle loadout popups on the script side
ADE_FUNC(commitToMission,
	l_UserInterface_Brief,
	nullptr,
	"Commits to the current mission with current loadout data, and starts the mission. Returns an integer to represent "
	"built-in errors or 0 if successful. 1 = general error, 2 = a player ship has no weapons, 3 = the required weapon was not found "
	"loaded on a ship, 4 = 2 or more required weapons were not found loaded on a ship, 5 = a gap in a ship's weapon banks was discovered "
	"and all empty banks must be at the bottom of the list, 6 = a player has no ship selected",
	"number error",
	"the error value")
{
	return ade_set_args(L, "i", static_cast<int>(commit_pressed(true)));
}

ADE_FUNC(drawBriefingMap,
	l_UserInterface_Brief,
	"number x, number y, [number width = 888, number height = 371]",
	"Draws the briefing map for the current mission at the specified coordinates. Note that the "
	"width and height must be a specific aspect ratio to match retail. If changed then some icons "
	"may be clipped from view unexpectedly. Must be called On Frame.",
	nullptr,
	nullptr)
{

	int x1;
	int y1;
	int x2 = 888;
	int y2 = 371;

	if (!ade_get_args(L, "ii|ii", &x1, &y1, &x2, &y2)) {
		LuaError(L, "X and Y coordinates not provided!");
		return ADE_RETURN_NIL;
	}

	// Saving retail coords here for posterity
	//  GR_640 - 19, 147, 555, 232
	//  GR_1024 - 30, 235, 888, 371

	bscreen.map_x1 = x1;
	bscreen.map_x2 = x1 + x2;
	bscreen.map_y1 = y1;
	bscreen.map_y2 = y1 + y2;
	bscreen.resize = GR_RESIZE_NONE;

	brief_api_do_frame(flRealframetime);

	return ADE_RETURN_NIL;
}

ADE_FUNC(callNextMapStage,
	l_UserInterface_Brief,
	nullptr,
	"Sends the briefing map to the next stage.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);
	brief_do_next_pressed(0);
	return ADE_RETURN_NIL;
}

ADE_FUNC(callPrevMapStage,
	l_UserInterface_Brief,
	nullptr,
	"Sends the briefing map to the previous stage.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);
	brief_do_prev_pressed();
	return ADE_RETURN_NIL;
}

ADE_FUNC(callFirstMapStage,
	l_UserInterface_Brief,
	nullptr,
	"Sends the briefing map to the first stage.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);
	brief_do_start_pressed();
	return ADE_RETURN_NIL;
}

ADE_FUNC(callLastMapStage,
	l_UserInterface_Brief,
	nullptr,
	"Sends the briefing map to the last stage.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);
	brief_do_end_pressed();
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

	// convert from lua index
	idx--;

	if ((idx < 0) || idx >= (int)Mission_goals.size())
		return ade_set_args(L, "o", l_Goals.Set(-1));

	return ade_set_args(L, "o", l_Goals.Set(idx));
}

ADE_FUNC(__len, l_Briefing_Goals, nullptr, "The number of goals in the mission", "number", "The number of goals.")
{
	return ade_set_args(L, "i", (int)Mission_goals.size());
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
	return ade_set_args(L, "o", l_CmdBrief.Set(0));
}

//**********SUBLIBRARY: UserInterface/Debriefing
ADE_LIB_DERIV(l_UserInterface_Debrief,
	"Debriefing",
	nullptr,
	"API for accessing data related to the debriefing UI.<br><b>Warning:</b> This is an internal "
	"API for the new UI system. This should not be used by other code and may be removed in the future!",
	l_UserInterface);

ADE_FUNC(initDebriefing,
	l_UserInterface_Debrief,
	nullptr,
	"Builds the debriefing, the stats, sets the next campaign mission, and makes all relevant data accessible",
	"number",
	"Returns true when completed")
{
	//This is used to skip some UI preloading in debrief init
	API_Access = true;
	
	// stop all looping mission sounds
	game_stop_looped_sounds();

	 // fail all incomplete goals before entering debriefing
	mission_goal_fail_incomplete();
	hud_config_as_player();
	debrief_init();

	API_Access = false;
	
	return ade_set_args(L, "b", true);
}

ADE_FUNC(getDebriefingMusicName,
	l_UserInterface_Debrief,
	nullptr,
	"Gets the file name of the music file to play for the debriefing.",
	"string",
	"The file name or empty if no music")
{
	return ade_set_args(L, "s", common_music_get_filename(debrief_select_music()).c_str());
}

ADE_FUNC(getDebriefing, l_UserInterface_Debrief, nullptr, "Get the debriefing", "debriefing", "The debriefing data")
{
	// get a pointer to the appropriate debriefing structure
	if (MULTI_TEAM) {
		return ade_set_args(L, "o", l_Debrief.Set(Net_player->p_info.team));
	} else {
		return ade_set_args(L, "o", l_Debrief.Set(0));
	}
}

ADE_FUNC(getEarnedMedal,
	l_UserInterface_Debrief,
	nullptr,
	"Get the earned medal name and bitmap",
	"string, string",
	"The name and bitmap or NIL if not earned")
{
	char filename[80];
	SCP_string displayname;

	if (Player->stats.m_medal_earned != -1) {
		debrief_choose_medal_variant(filename,
			Player->stats.m_medal_earned,
			Player->stats.medal_counts[Player->stats.m_medal_earned] - 1);
		displayname = Medals[Player->stats.m_medal_earned].get_display_name();

		return ade_set_args(L, "ss", displayname, filename);
	} else {
		return ADE_RETURN_NIL;
	}
}

ADE_FUNC(getEarnedPromotion,
	l_UserInterface_Debrief,
	nullptr,
	"Get the earned promotion stage, name, and bitmap",
	"debriefing_stage, string, string",
	"The promotion stage, name and bitmap or NIL if not earned")
{
	char filename[80];
	SCP_string displayname;

	if (Player->stats.m_promotion_earned != -1) {
		Promoted = Player->stats.m_promotion_earned;
		debrief_choose_medal_variant(filename, Rank_medal_index, Promoted);
		displayname = Ranks[Promoted].name;

		return ade_set_args(L, "oss", l_DebriefStage.Set(debrief_stage_h(&Promotion_stage)), displayname, filename);
	} else {
		return ADE_RETURN_NIL;
	}
}

ADE_FUNC(getEarnedBadge,
	l_UserInterface_Debrief,
	nullptr,
	"Get the earned badge stage, name, and bitmap",
	"debriefing_stage, string, string",
	"The badge stage, name and bitmap or NIL if not earned")
{
	char filename[80];
	SCP_string displayname;

	if (Player->stats.m_badge_earned.size()) {
		debrief_choose_medal_variant(filename,
			Player->stats.m_badge_earned.back(),
			Player->stats.medal_counts[Player->stats.m_badge_earned.back()] - 1);
		displayname = Medals[Player->stats.m_badge_earned.back()].get_display_name();

		return ade_set_args(L, "oss", l_DebriefStage.Set(debrief_stage_h(&Badge_stage)), displayname, filename);
	} else {
		return ADE_RETURN_NIL;
	}
}

ADE_FUNC(clearMissionStats,
	l_UserInterface_Debrief,
	nullptr,
	"Clears out the player's mission stats.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L); // unused parameter
	Player->flags &= ~PLAYER_FLAGS_PROMOTED;
	scoring_level_init(&Player->stats);
	return ADE_RETURN_NIL;
}

ADE_FUNC(getTraitor,
	l_UserInterface_Debrief,
	nullptr,
	"Get the traitor stage",
	"debriefing_stage",
	"The traitor stage or nil if the player is not traitor. Should be coupled with clearMissionStats if true")
{
	if (Turned_traitor) {
		return ade_set_args(L, "o", l_DebriefStage.Set(debrief_stage_h(&Traitor_debriefing.stages[0])));
	} else {
		return ADE_RETURN_NIL;
	}
}

ADE_FUNC(mustReplay,
	l_UserInterface_Debrief,
	nullptr,
	"Gets whether or not the player must replay the mission. Should be coupled with clearMissionStats if true",
	"boolean",
	"true if must replay, false otherwise")
{

	if (Must_replay_mission) {
		return ade_set_args(L, "b", true);
	} else {
		return ade_set_args(L, "b", false);
	}
}

ADE_FUNC(canSkip,
	l_UserInterface_Debrief,
	nullptr,
	"Gets whether or not the player has failed enough times to trigger a skip dialog",
	"boolean",
	"true if can skip, false otherwise")
{

	if (Player->failures_this_session >= PLAYER_MISSION_FAILURE_LIMIT) {
		return ade_set_args(L, "b", true);
	} else {
		return ade_set_args(L, "b", false);
	}
}

ADE_FUNC(replayMission,
	l_UserInterface_Debrief,
	"[boolean restart = true]",
	"Resets the mission outcome, and optionally restarts the mission at the briefing; "
	"true to restart the mission, false to stay at current UI. Defaults to true.",
	nullptr,
	nullptr)
{

	bool restart = true;

	ade_get_args(L, "|b", &restart);

	// This is used to skip some UI preloading in debrief init
	API_Access = true;

	debrief_close();

	if (restart) {
		gameseq_post_event(GS_EVENT_START_GAME);
	}

	API_Access = false;

	return ADE_RETURN_NIL;
}

ADE_FUNC(acceptMission,
	l_UserInterface_Debrief,
	"[boolean start = true]",
	"Accepts the mission outcome, saves the stats, and optionally begins the next mission if in campaign; "
	"true to start the next mission, false to stay at current UI. Defaults to true.",
	nullptr,
	nullptr)
{
	bool start = true;

	ade_get_args(L, "|b", &start);

	// This is used to skip some UI preloading in debrief init
	API_Access = true;

	if (start) {
		debrief_accept(1);
	} else {
		debrief_accept(0);
	};

	debrief_close();

	API_Access = false;

	return ADE_RETURN_NIL;
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
	return ade_set_args(L, "o", l_LoopBriefStage.Set(cmission_h(Campaign.current_mission)));
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
		return ade_set_args(L, "o", l_RedAlertStage.Set(redalert_stage_h(0,0)));
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
		return ade_set_args(L, "o", l_FictionViewerStage.Set(fiction_viewer_stage_h(Fiction_viewer_active_stage)));
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

//**********SUBLIBRARY: UserInterface/ShipWepSelect
ADE_LIB_DERIV(l_UserInterface_ShipWepSelect,
	"ShipWepSelect",
	nullptr,
	"API for accessing data related to the ship and weapon select UIs.<br><b>Warning:</b> This is an internal "
	"API for the new UI system. This should not be used by other code and may be removed in the future!",
	l_UserInterface);

ADE_FUNC(initSelect,
	l_UserInterface_ShipWepSelect,
	nullptr,
	"Initializes selection data including wing slots, ship and weapon pool, and loadout information. "
	"Must be called before every mission regardless if ship or weapon select is actually used! "
	"Should also be called on initialization of relevant briefing UIs such as briefing and red alert "
	"to ensure that the ships and weapons are properly set for the current mission.",
	nullptr,
	nullptr)
{
	//Note this does all the things from common_select_init() in missionscreencommon.cpp except load UI
	//elements into memory - Mjn
	
	SCP_UNUSED(L); // unused parameter

	Common_team = 0;

	if ((Game_mode & GM_MULTIPLAYER) && IS_MISSION_MULTI_TEAMS)
		Common_team = Net_player->p_info.team;

	common_set_team_pointers(Common_team);

	ship_select_common_init(true);
	weapon_select_common_init(true);

	if ( Game_mode & GM_MULTIPLAYER ) {
		multi_ts_common_init();
	}

	// restore loadout from Player_loadout if this is the same mission as the one previously played
	if ( !(Game_mode & GM_MULTIPLAYER) ) {
		if ( !stricmp(Player_loadout.filename, Game_current_mission_filename) ) {
			wss_maybe_restore_loadout();
			ss_synch_interface();
			wl_synch_interface();
		}
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(saveLoadout,
	l_UserInterface_ShipWepSelect,
	nullptr,
	"Saves the current loadout to the player file. Only should be used when a mission is loaded but has not been started.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L); // unused parameter

	// This could be requested before Common_team has been initialized, so let's check.
	// Freespace.cpp will clear Common_select_inited if the player ever leaves a valid
	// "briefing" game state, so this should be a pretty safe check to avoid the assert
	// contained within. - Mjn
	if (Common_select_inited) {
		wss_save_loadout();
	} else {
		return ADE_RETURN_NIL;
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(get3dShipChoices,
	l_UserInterface_ShipWepSelect,
	nullptr,
	"Gets the 3d select choices from game_settings.tbl relating to ships.",
	"boolean, number, boolean",
	"3d ship select choice(true for on, false for off), default ship select effect(0 = off, 1 = FS1, 2 = FS2), 3d ship "
	"icons choice(true for on, false for off)")
{

	return ade_set_args(L,
		"bib",
		Use_3d_ship_select,
		Default_ship_select_effect,
		Use_3d_ship_icons);
}

ADE_FUNC(get3dWeaponChoices,
	l_UserInterface_ShipWepSelect,
	nullptr,
	"Gets the 3d select choices from game_settings.tbl relating to weapons.",
	"boolean, number, boolean",
	"3d weapon select choice(true for on, false for off), default weapon select effect(0 = off, 1 = FS1, 2 = FS2), 3d weapon "
	"icons choice(true for on, false for off)")
{

	return ade_set_args(L,
		"bib",
		Use_3d_weapon_select,
		Default_weapon_select_effect,
		Use_3d_weapon_icons);
}

ADE_FUNC(get3dOverheadChoices,
	l_UserInterface_ShipWepSelect,
	nullptr,
	"Gets the 3d select choices from game_settings.tbl relating to weapon select overhead view.",
	"boolean, number",
	"3d overhead select choice(true for on, false for off), default overhead style(0 for top view, 1 for rotate)")
{

	return ade_set_args(L,
		"bi",
		Use_3d_overhead_ship,
		(int)Default_overhead_ship_style);
}

ADE_LIB_DERIV(l_Ship_Pool, "Ship_Pool", nullptr, nullptr, l_UserInterface_ShipWepSelect);
ADE_INDEXER(l_Ship_Pool,
	"number Index, number amount",
	"Array of ship amounts available in the pool for selection in the current mission. Index is index into Ship Classes.",
	"number",
	"Amount of the ship that's available")
{
	int idx;
	int amount;
	if (!ade_get_args(L, "*i|i", &idx, &amount))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx > ship_info_size()) {
		return ADE_RETURN_NIL;
	};

	idx--; // Convert to Lua's 1 based index system

	if (ADE_SETTING_VAR) {
		if (amount < 0) {
			Ss_pool[idx] = 0;
		} else {
			Ss_pool[idx] = amount;
		}
	}

	return ade_set_args(L, "i", Ss_pool[idx]);
}

ADE_FUNC(__len, l_Ship_Pool, nullptr, "The number of ship classes in the pool", "number", "The number of ship classes.")
{
	return ade_set_args(L, "i", ship_info_size());
}

ADE_LIB_DERIV(l_Weapon_Pool, "Weapon_Pool", nullptr, nullptr, l_UserInterface_ShipWepSelect);
ADE_INDEXER(l_Weapon_Pool,
	"number Index, number amount",
	"Array of weapon amounts available in the pool for selection in the current mission. Index is index into Weapon Classes.",
	"number",
	"Amount of the weapon that's available")
{
	int idx;
	int amount;
	if (!ade_get_args(L, "*i|i", &idx, &amount))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx > weapon_info_size()) {
		return ADE_RETURN_NIL;
	};

	idx--; // Convert to Lua's 1 based index system

	if (ADE_SETTING_VAR) {
		if (amount < 0) {
			Wl_pool[idx] = 0;
		} else {
			Wl_pool[idx] = amount;
		}
	}

	return ade_set_args(L, "i", Wl_pool[idx]);
}

ADE_FUNC(__len,
	l_Weapon_Pool, nullptr, "The number of weapon classes in the pool", "number", "The number of weapon classes.")
{
	return ade_set_args(L, "i", weapon_info_size());
}

ADE_FUNC(resetSelect,
	l_UserInterface_ShipWepSelect,
	nullptr,
	"Resets selection data to mission defaults including wing slots, ship and weapon pool, and loadout information",
	nullptr,
	nullptr)
{
	// Note this does all the things from ss_reset_to_default() in missionshipchoice.cpp except
	// resetting UI elements - Mjn

	SCP_UNUSED(L); // unused parameter

	ss_init_pool(&Team_data[Common_team]);
	ss_init_units();

	if (!(Game_mode & GM_MULTIPLAYER)) {
		wl_fill_slots();
	}

	return ADE_RETURN_NIL;
}

ADE_LIB_DERIV(l_Loadout_Wings, "Loadout_Wings", nullptr, nullptr, l_UserInterface_ShipWepSelect);
ADE_INDEXER(l_Loadout_Wings,
	"number Index",
	"Array of loadout wing data",
	"loadout_wing",
	"loadout handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Loadout_Wing.Set(ss_wing_info_h()));
	idx--; //Convert to Lua's 1 based index system
	return ade_set_args(L, "o", l_Loadout_Wing.Set(ss_wing_info_h(idx)));
}

ADE_FUNC(__len, l_Loadout_Wings, nullptr, "The number of loadout wings", "number", "The number of loadout wings.")
{
	int count = 0;

	for (int i = 0; i < MAX_STARTING_WINGS; i++) {
		if (Ss_wings[i].ss_slots[0].in_mission)
			count++;
	};

	return ade_set_args(L, "i", count);
}

ADE_LIB_DERIV(l_Loadout_Ships, "Loadout_Ships", nullptr, nullptr, l_UserInterface_ShipWepSelect);
ADE_INDEXER(l_Loadout_Ships,
	"number Index",
	"Array of loadout ship data. Slots are 1-12 where 1-4 is wing 1, 5-8 is wing 2, 9-12 is wing 3. "
	"This is the array that is used to actually build the mission loadout on Commit.",
	"loadout_ship",
	"loadout handle, or nil if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ADE_RETURN_NIL;
	idx--; // Convert to Lua's 1 based index system
	return ade_set_args(L, "o", l_Loadout_Ship.Set(idx));
}

ADE_FUNC(__len, l_Loadout_Ships, nullptr, "The number of loadout ships", "number", "The number of loadout ships.")
{
	return ade_set_args(L, "i", MAX_WING_BLOCKS*MAX_WING_SLOTS);
}

//**********SUBLIBRARY: UserInterface/TechRoom
ADE_LIB_DERIV(l_UserInterface_TechRoom,
	"TechRoom",
	nullptr,
	"API for accessing data related to the tech room UI.<br><b>Warning:</b> This is an internal "
	"API for the new UI system. This should not be used by other code and may be removed in the future!",
	l_UserInterface);

ADE_FUNC(buildMissionList,
	l_UserInterface_TechRoom,
	nullptr,
	"Builds the mission list for display. Must be called before the sim_mission handle will have data",
	"number",
	"Returns 1 when completed")
{
	Sim_Missions.clear();
	Sim_CMissions.clear();

	api_sim_room_build_mission_list(true);

	mprintf(("Building mission lists for scripting API is complete!\n"));

	return ade_set_args(L, "i", 1);
}

ADE_FUNC(buildCredits,
	l_UserInterface_TechRoom,
	nullptr,
	"Builds the credits for display. Must be called before the credits_info handle will have data",
	"number",
	"Returns 1 when completed")
{
	credits_parse();
	credits_scp_position();

	size_t count = Credit_text_parts.size();
	credits_complete.clear();

	for (size_t i = 0; i < count; i++) {
		credits_complete.append(Credit_text_parts[i]);
	}

	//Make sure we clean up after ourselves
	Credit_text_parts.clear();

	mprintf(("Building credits for scripting API is complete!\n"));

	return ade_set_args(L, "i", 1);
}

ADE_LIB_DERIV(l_UserInterface_SingleMissions, "SingleMissions", nullptr, nullptr, l_UserInterface_TechRoom);
ADE_INDEXER(l_UserInterface_SingleMissions, "number Index", "Array of simulator missions", "sim_mission", "Mission handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");
	
	return ade_set_args(L, "o", l_TechRoomMission.Set(sim_mission_h(idx, false)));
}

ADE_FUNC(__len, l_UserInterface_SingleMissions, nullptr, "The number of single missions", "number", "The number of single missions")
{
	return ade_set_args(L, "i", Sim_Missions.size());
}

ADE_LIB_DERIV(l_UserInterface_CampaignMissions, "CampaignMissions", nullptr, nullptr, l_UserInterface_TechRoom);
ADE_INDEXER(l_UserInterface_CampaignMissions, "number Index", "Array of campaign missions", "sim_mission", "Mission handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");
	
	return ade_set_args(L, "o", l_TechRoomMission.Set(sim_mission_h(idx, true)));
}

ADE_FUNC(__len, l_UserInterface_CampaignMissions, nullptr, "The number of campaign missions", "number", "The number of campaign missions")
{
	return ade_set_args(L, "i", Sim_CMissions.size());
}

ADE_LIB_DERIV(l_UserInterface_Cutscenes, "Cutscenes", nullptr, nullptr, l_UserInterface_TechRoom);
ADE_INDEXER(l_UserInterface_Cutscenes,
	"number Index",
	"Array of cutscenes",
	"custscene_info",
	"Cutscene handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "o", l_TechRoomCutscene.Set(cutscene_info_h(idx)));
}

ADE_FUNC(__len, l_UserInterface_Cutscenes, nullptr, "The number of cutscenes", "number", "The number of cutscenes")
{
	return ade_set_args(L, "i", Cutscenes.size());
}

ADE_LIB_DERIV(l_UserInterface_Credits, "Credits", nullptr, nullptr, l_UserInterface_TechRoom);
ADE_VIRTVAR(Music, l_UserInterface_Credits, nullptr, "The credits music filename", "string", "The music filename")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", credits_get_music_filename(Credits_music_name));
}

ADE_VIRTVAR(NumImages, l_UserInterface_Credits, nullptr, "The total number of credits images", "number", "The number of images")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", Credits_num_images);
}

ADE_VIRTVAR(StartIndex, l_UserInterface_Credits, nullptr, "The image index to begin with", "number", "The index")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	int retv = Credits_artwork_index;
	if (retv < 0) {
		retv = Random::next(Credits_num_images);
	}

	return ade_set_args(L, "i", retv);
}

ADE_VIRTVAR(DisplayTime, l_UserInterface_Credits, nullptr, "The display time for each image", "number", "The display time")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "f", Credits_artwork_display_time);
}

ADE_VIRTVAR(FadeTime, l_UserInterface_Credits, nullptr, "The crossfade time for each image", "number", "The fade time")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "f", Credits_artwork_fade_time);
}

ADE_VIRTVAR(ScrollRate, l_UserInterface_Credits, nullptr, "The scroll rate of the text", "number", "The scroll rate")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "f", Credits_scroll_rate);
}

ADE_VIRTVAR(Complete, l_UserInterface_Credits, nullptr, "The complete credits string", "string", "The credits")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", credits_complete);
}
//**********SUBLIBRARY: UserInterface/PauseScreen
ADE_LIB_DERIV(l_UserInterface_PauseScreen,
	"PauseScreen",
	nullptr,
	"API for accessing data related to the pause screen UI.<br><b>Warning:</b> This is an internal "
	"API for the new UI system. This should not be used by other code and may be removed in the future!",
	l_UserInterface);

ADE_VIRTVAR(isPaused, l_UserInterface_PauseScreen, nullptr, "Returns true if the game is paused, false otherwise", "boolean", "true if paused, false if unpaused")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", Paused);
}

ADE_FUNC(initPause, l_UserInterface_PauseScreen, nullptr, "Makes sure everything is done correctly to pause the game.", nullptr, nullptr)
{
	SCP_UNUSED(L);

	weapon_pause_sounds();
	audiostream_pause_all();

	Paused = true;

	return ADE_RETURN_NIL;
}

ADE_FUNC(closePause, l_UserInterface_PauseScreen, nullptr, "Makes sure everything is done correctly to unpause the game.", nullptr, nullptr)
{
	SCP_UNUSED(L);

	weapon_unpause_sounds();
	audiostream_unpause_all();

	// FSO can run pause_init() before the actual games state change when the game loses focus
	// so this is required to make sure that the saved screen is cleared if SCPUI takes over
	// after the game state change
	gr_free_screen(Pause_saved_screen);
	Pause_saved_screen = -1;

	Paused = false;

	return ADE_RETURN_NIL;
}

} // namespace api
} // namespace scripting
