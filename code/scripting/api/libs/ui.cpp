//
//
#include "ui.h"

#include "freespace.h"

#include "globalincs/alphacolors.h"

#include "cmdline/cmdline.h"
#include "cutscene/cutscenes.h"
#include "gamesnd/eventmusic.h"
#include "gamesequence/gamesequence.h"
#include "io/key.h"
#include "menuui/barracks.h"
#include "menuui/credits.h"
#include "menuui/mainhallmenu.h"
#include "menuui/optionsmenu.h"
#include "menuui/playermenu.h"
#include "menuui/readyroom.h"
#include "mission/missionmessage.h"
#include "mission/missionbriefcommon.h"
#include "mission/missionparse.h"
#include "missionui/fictionviewer.h"
#include "missionui/missionbrief.h"
#include "mission/missioncampaign.h"
#include "missionui/missionscreencommon.h"
#include "missionui/missiondebrief.h"
#include "mission/missiongoals.h"
#include "mission/missioncampaign.h"
#include "mission/missionhotkey.h"
#include "missionui/chatbox.h"
#include "missionui/redalert.h"
#include "missionui/missionpause.h"
#include "mod_table/mod_table.h"
#include "network/chat_api.h"
#include "network/multi.h"
#include "network/multiui.h"
#include "network/multi_endgame.h"
#include "network/multiteamselect.h"
#include "network/multi_pause.h"
#include "network/multi_pxo.h"
#include "network/multimsgs.h"
#include "network/multi_ingame.h"
#include "pilotfile/pilotfile.h"
#include "playerman/managepilot.h"
#include "radar/radarsetup.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "scpui/SoundPlugin.h"
#include "scpui/rocket_ui.h"
#include "scripting/api/objs/briefing.h"
#include "scripting/api/objs/cmd_brief.h"
#include "scripting/api/objs/color.h"
#include "scripting/api/objs/control_config.h"
#include "scripting/api/objs/debriefing.h"
#include "scripting/api/objs/enums.h"
#include "scripting/api/objs/fictionviewer.h"
#include "scripting/api/objs/gamehelp.h"
#include "scripting/api/objs/goal.h"
#include "scripting/api/objs/hudconfig.h"
#include "scripting/api/objs/loop_brief.h"
#include "scripting/api/objs/medals.h"
#include "scripting/api/objs/missionhotkey.h"
#include "scripting/api/objs/missionlog.h"
#include "scripting/api/objs/multi_objects.h"
#include "scripting/api/objs/player.h"
#include "scripting/api/objs/rank.h"
#include "scripting/api/objs/redalert.h"
#include "scripting/api/objs/shipwepselect.h"
#include "scripting/api/objs/techroom.h"
#include "scripting/api/objs/texture.h"
#include "scripting/api/objs/vecmath.h"
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

//*************************Global UI stuff*************************
ADE_LIB(l_UserInterface, "UserInterface", "ui", "Functions for managing the \"SCPUI\" user interface system.");

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
	"true if successful")
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
	game_flush();
	Main_hall_poll_key = false;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(disableInput, l_UserInterface, nullptr, "Disables UI input", nullptr, "nothing")
{
	SCP_UNUSED(L);
	
	scpui::disableInput();
	game_flush();
	Main_hall_poll_key = true;

	return ADE_RETURN_NIL;
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

	if (!movie_type.isValid() || movie_type.index < LE_MOVIE_PRE_FICTION || movie_type.index > LE_MOVIE_END_CAMPAIGN)
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

ADE_FUNC(isCutscenePlaying, l_UserInterface, nullptr, "Checks if a cutscene is playing.", "boolean", "Returns true if cutscene is playing, false otherwise")
{
	return ade_set_args(L, "b", Movie_active);
}

ADE_FUNC(launchURL, l_UserInterface, "string url", "Launches the given URL in a web browser", nullptr, nullptr)
{
	const char* url;
	if (!ade_get_args(L, "s", &url))
		return ADE_RETURN_NIL;

	multi_pxo_url(url);

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
              "API for accessing values specific to the Pilot Select UI.",
              l_UserInterface);

ADE_VIRTVAR_DEPRECATED(MAX_PILOTS, l_UserInterface_PilotSelect, nullptr, "Gets the maximum number of possible pilots.", "number",
	"The maximum number of pilots",
	gameversion::version(25, 0, 0, 0),
	"This variable has moved to the GlobalVariabls library.")
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
         "Lists all pilots available for the pilot selection<br>", "table",
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

ADE_FUNC(unloadPilot,
	l_UserInterface_PilotSelect,
	nullptr,
	"Unloads a player file & associated campaign file. Can not be used outside of pilot select!",
	"boolean",
	"Returns true if successful, false otherwise")
{
	if (gameseq_get_state() == GS_STATE_INITIAL_PLAYER_SELECT) {
		Player = nullptr;
		Campaign.filename[0] = '\0';

		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
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
              "API for accessing values specific to the Main Hall UI.",
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

ADE_FUNC(stopMusic, l_UserInterface_MainHall, "boolean Fade=false", "Stops the mainhall music. True to fade, false to stop immediately.", nullptr, "nothing")
{
	bool fade = false;
	if (!ade_get_args(L, "|b", &fade))
		return ADE_RETURN_NIL;
	main_hall_stop_music(fade);
	return ADE_RETURN_NIL;
}

ADE_FUNC(toggleHelp,
	l_UserInterface_MainHall,
	"boolean",
	"Sets the mainhall F1 help overlay to display. True to display, false to hide",
	nullptr,
	"nothing")
{
	bool toggle;
	if (!ade_get_args(L, "b", &toggle))
		return ADE_RETURN_NIL;

	main_hall_toggle_help(toggle);


	return ADE_RETURN_NIL;
}

ADE_FUNC(setMainhall, l_UserInterface_MainHall, "string mainhall, boolean enforce", "The name of the mainhall to try to set. Will immediately change if the player is currently in the mainhall menu. "
	"Use enforce to set this as the mainhall on next mainhall load if setting from outside the mainhall menu. NOTE: If enforce is true then the player will always return back to this mainhall forever. "
	"Call this with a blank string and enforce false to unset enforce without changing the current mainhall that is loaded.", nullptr, "nothing")
{
	const char* name = nullptr;
	bool enforce = false;
	if (!ade_get_args(L, "s|b", &name, &enforce)) {
		return ADE_RETURN_NIL;
	}

	if (enforce) {
		Enforced_main_hall = name;
	} else {
		Enforced_main_hall.clear();
	}

	if (gameseq_get_state() == GS_STATE_MAIN_MENU) {
		if (name[0] != '\0') {
			main_hall_close();
			main_hall_init(name);
		}
	}

	return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/Barracks
ADE_LIB_DERIV(l_UserInterface_Barracks, "Barracks", nullptr,
              "API for accessing values specific to the Barracks UI.",
              l_UserInterface);

ADE_FUNC(listPilotImages, l_UserInterface_Barracks, nullptr, "Lists the names of the available pilot images.",
         "table", "The list of pilot filenames or nil on error")
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
         "table", "The list of squad filenames or nil on error")
{
	pilot_load_squad_pic_list();

	using namespace luacpp;

	LuaTable out = LuaTable::create(L);
	for (auto i = 0; i < Num_pilot_squad_images; ++i) {
		out.addValue(i + 1, Pilot_squad_image_names[i]);
	}
	return ade_set_args(L, "t", &out);
}

ADE_FUNC(acceptPilot, l_UserInterface_Barracks, "player selection, [boolean changeState]", "Accept the given player as the current player. Set second argument to false to prevent returning to the mainhall",
         "boolean", "true on success, false otherwise")
{
	player_h* plh;
	bool changeState = true;
	if (!ade_get_args(L, "o|b", l_Player.GetPtr(&plh), &changeState)) {
		return ADE_RETURN_FALSE;
	}

	barracks_accept_pilot(plh->get(), changeState);
	return ADE_RETURN_TRUE;
}

//**********SUBLIBRARY: UserInterface/OptionsMenu
// This needs a slightly different name since there is already a type called "Options"
ADE_LIB_DERIV(l_UserInterface_Options,
	"OptionsMenu",
	nullptr,
	"API for accessing values specific to the Options UI.",
	l_UserInterface);

ADE_FUNC(playVoiceClip,
	l_UserInterface_Options,
	nullptr,
	"Plays the example voice clip used for checking the voice volume",
	"boolean",
	"true on success, false otherwise")
{
	options_play_voice_clip();
	return ADE_RETURN_TRUE;
}

ADE_FUNC(savePlayerData,
	l_UserInterface_Options,
	nullptr,
	"Saves all player data. This includes the player file and campaign file.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	Pilot.save_player();
	Pilot.save_savefile();

	return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/CampaignMenu
ADE_LIB_DERIV(l_UserInterface_Campaign,
	"CampaignMenu",
	nullptr,
	"API for accessing data related to the Campaign UI.",
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
	"table, table, table",
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
	Pilot.save_player();

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
	"API for accessing data related to the Briefing UI.",
	l_UserInterface);

ADE_FUNC(getBriefingMusicName,
	l_UserInterface_Brief,
	nullptr,
	"Gets the file name of the music file to play for the briefing.",
	"string",
	"The file name or empty if no music")
{
	return ade_set_args(L, "s", common_music_get_filename(SCORE_BRIEFING));
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

ADE_FUNC(commitToMission,
	l_UserInterface_Brief,
	nullptr,
	"Commits to the current mission with current loadout data, and starts the mission. Returns one of the COMMIT_ enums to indicate any errors.",
	"enumeration",
	"the error value")
{
	commit_pressed_status rc;

	if (Game_mode & GM_MULTIPLAYER) {
		rc = multi_ts_commit_pressed();
	} else {
		rc = commit_pressed();
	}

	lua_enum eh_idx = ENUM_INVALID;
	switch (rc) {
	case commit_pressed_status::GENERAL_FAIL:
		eh_idx = LE_COMMIT_FAIL;
		break;
	case commit_pressed_status::PLAYER_NO_WEAPONS:
		eh_idx = LE_COMMIT_PLAYER_NO_WEAPONS;
		break;
	case commit_pressed_status::NO_REQUIRED_WEAPON:
		eh_idx = LE_COMMIT_NO_REQUIRED_WEAPON;
		break;
	case commit_pressed_status::NO_REQUIRED_WEAPON_MULTIPLE:
		eh_idx = LE_COMMIT_NO_REQUIRED_WEAPON_MULTIPLE;
		break;
	case commit_pressed_status::BANK_GAP_ERROR:
		eh_idx = LE_COMMIT_BANK_GAP_ERROR;
		break;
	case commit_pressed_status::PLAYER_NO_SLOT:
		eh_idx = LE_COMMIT_PLAYER_NO_SLOT;
		break;
	case commit_pressed_status::MULTI_PLAYERS_NO_SHIPS:
		eh_idx = LE_COMMIT_MULTI_PLAYERS_NO_SHIPS;
		break;
	case commit_pressed_status::MULTI_NOT_ALL_ASSIGNED:
		eh_idx = LE_COMMIT_MULTI_NOT_ALL_ASSIGNED;
		break;
	case commit_pressed_status::MULTI_NO_PRIMARY:
		eh_idx = LE_COMMIT_MULTI_NO_PRIMARY;
		break;
	case commit_pressed_status::MULTI_NO_SECONDARY:
		eh_idx = LE_COMMIT_MULTI_NO_SECONDARY;
		break;
	case commit_pressed_status::SUCCESS:
	default:
		eh_idx = LE_COMMIT_SUCCESS;
		break;
	}

	return ade_set_args(L, "o", l_Enum.Set(enum_h(eh_idx)));
}

ADE_FUNC(renderBriefingModel,
	l_UserInterface_Brief,
	"string PofName, number CloseupZoom, vector CloseupPos, number X1, number Y1, number X2, number Y2, [number RotationPercent =0, number PitchPercent =0, "
	"number "
	"BankPercent=40, number Zoom=1.3, boolean Lighting=true, boolean Jumpnode=false]",
	"Draws a pof. True for regular lighting, false for flat lighting.",
	"boolean",
	"Whether pof was rendered")
{
	int x1, y1, x2, y2;
	angles rot_angles = {0.0f, 0.0f, 40.0f};
	const char* pof;
	float closeup_zoom;
	vec3d closeup_pos;
	float zoom = 1.3f;
	bool lighting = true;
	bool jumpNode = false;
	if (!ade_get_args(L,
			"sfoiiii|ffffbb",
			&pof,
			&closeup_zoom,
			l_Vector.Get(&closeup_pos),
			&x1,
			&y1,
			&x2,
			&y2,
			&rot_angles.h,
			&rot_angles.p,
			&rot_angles.b,
			&zoom,
			&lighting,
			&jumpNode))
		return ade_set_error(L, "b", false);

	if (x2 < x1 || y2 < y1)
		return ade_set_args(L, "b", false);

	CLAMP(rot_angles.p, 0.0f, 100.0f);
	CLAMP(rot_angles.b, 0.0f, 100.0f);
	CLAMP(rot_angles.h, 0.0f, 100.0f);

	// Handle angles
	matrix orient = vmd_identity_matrix;
	angles view_angles = {-0.6f, 0.0f, 0.0f};
	vm_angles_2_matrix(&orient, &view_angles);

	rot_angles.p = (rot_angles.p * 0.01f) * PI2;
	rot_angles.b = (rot_angles.b * 0.01f) * PI2;
	rot_angles.h = (rot_angles.h * 0.01f) * PI2;
	vm_rotate_matrix_by_angles(&orient, &rot_angles);

	tech_render_type thisType = TECH_POF;

	if (jumpNode) {
		thisType = TECH_JUMP_NODE;
	}

	return ade_set_args(L, "b", render_tech_model(thisType, x1, y1, x2, y2, zoom, lighting, -1, &orient, pof, closeup_zoom, &closeup_pos));
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

ADE_FUNC(checkStageIcons,
	l_UserInterface_Brief,
	"number xPos, number yPos",
	"Sends the mouse position to the brief map rendering functions to properly highlight icons.",
	"string, number, vector, string, number",
	"If an icon is highlighted then this will return the ship name for ships or the pof to render for asteroid, jumpnode, or unknown icons. "
	"also returns the closeup zoom, the closeup position, the closeup label, and the icon id. Otherwise it returns nil")
{
	int x;
	int y;

	if (!ade_get_args(L, "ii", &x, &y)) {
		LuaError(L, "X and Y coordinates not provided!");
		return ADE_RETURN_NIL;
	}

	brief_check_for_anim(true, x, y);

	if (Closeup_icon == nullptr) {
		return ADE_RETURN_NIL;
	} else {
		return ade_set_args(L, "sfosi", Closeup_type_name, Closeup_zoom, l_Vector.Set(Closeup_cam_pos), Closeup_icon->closeup_label, Closeup_icon->id);
	}
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
		return ade_set_args(L, "o", l_Goal.Set(-1));

	return ade_set_args(L, "o", l_Goal.Set(idx));
}

ADE_FUNC(__len, l_Briefing_Goals, nullptr, "The number of goals in the mission", "number", "The number of goals.")
{
	return ade_set_args(L, "i", (int)Mission_goals.size());
}

//**********SUBLIBRARY: UserInterface/CommandBriefing
ADE_LIB_DERIV(l_UserInterface_CmdBrief,
	"CommandBriefing",
	nullptr,
	"API for accessing data related to the Command Briefing UI.",
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
	"API for accessing data related to the Debriefing UI.",
	l_UserInterface);

ADE_FUNC(initDebriefing,
	l_UserInterface_Debrief,
	nullptr,
	"Builds the debriefing, the stats, sets the next campaign mission, and makes all relevant data accessible",
	"number",
	"Returns true when completed")
{
	
	// stop all looping mission sounds
	game_stop_looped_sounds();

	 // fail all incomplete goals before entering debriefing
	mission_goal_fail_incomplete();
	hud_config_as_player();
	debrief_init(true);
	
	return ade_set_args(L, "b", true);
}

ADE_FUNC(getDebriefingMusicName,
	l_UserInterface_Debrief,
	nullptr,
	"Gets the file name of the music file to play for the debriefing.",
	"string",
	"The file name or empty if no music")
{
	return ade_set_args(L, "s", common_music_get_filename(debrief_select_music()));
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
		displayname = get_rank_display_name(&Ranks[Promoted]).c_str();

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

	debrief_close(true);

	if (restart) {
		gameseq_post_event(GS_EVENT_START_GAME);
	}

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

	if (start) {
		debrief_accept(1, true);
	} else {
		debrief_accept(0, true);
	};

	debrief_close(true);

	return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/LoopBrief
ADE_LIB_DERIV(l_UserInterface_LoopBrief,
	"LoopBrief",
	nullptr,
	"API for accessing data related to the Loop Brief UI.",
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
	"API for accessing data related to the Red Alert UI.",
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
	"API for accessing data related to the Fiction Viewer UI.",
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
	return ade_set_args(L, "s", common_music_get_filename(SCORE_FICTION_VIEWER));
}

//**********SUBLIBRARY: UserInterface/ShipWepSelect
ADE_LIB_DERIV(l_UserInterface_ShipWepSelect,
	"ShipWepSelect",
	nullptr,
	"API for accessing data related to the Ship and Weapon Select UIs.",
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
		if (Game_mode & GM_MULTIPLAYER) {
			LuaError(L, "This property may not be modified in multiplayer.");
			return ADE_RETURN_NIL;
		}
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
		if (Game_mode & GM_MULTIPLAYER) {
			LuaError(L, "This property may not be modified in multiplayer.");
			return ADE_RETURN_NIL;
		}
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
	// resetting UI elements. It also resets the weapon pool. - Mjn

	SCP_UNUSED(L); // unused parameter

	//Reset ships pool
	ss_init_pool(&Team_data[Common_team]);
	ss_init_units();

	//Reset weapons pool
	wl_init_pool(&Team_data[Common_team]);

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

ADE_FUNC(sendShipRequestPacket,
	l_UserInterface_ShipWepSelect,
	"number FromType, number ToType, number FromSlotIndex, number ToSlotIndex, number ShipClassIndex",
	"Sends a request to the host to change a ship slot. From/To types are 0 for Ship Slot, 1 for Player Slot, 2 for Pool",
	nullptr,
	nullptr)
{
	int fromType; //2 for pool, 1 for player, 0 for slot
	int toType;  // 2 for pool, 1 for player, 0 for slot
	int fromSlot;
	int toSlot;
	int shipClassIdx;
	if (!ade_get_args(L, "iiiii", &fromType, &toType, &fromSlot, &toSlot, &shipClassIdx))
		return ADE_RETURN_NIL;

	// --revelant points to convert from lua indecies to c
	multi_ts_drop(fromType, --fromSlot, toType, --toSlot,  --shipClassIdx);

	return ADE_RETURN_NIL;
}

ADE_FUNC(sendWeaponRequestPacket,
	l_UserInterface_ShipWepSelect,
	"number FromBank, number ToBank, number fromPoolWepIdx, number toPoolWepIdx, number shipSlot",
	"Sends a request to the host to change a ship slot.",
	nullptr,
	nullptr)
{
	int fromBank;
	int toBank;
	int fromList;
	int toList;
	int shipSlot;
	if (!ade_get_args(L, "iiiii", &fromBank, &toBank, &fromList, &toList, &shipSlot))
		return ADE_RETURN_NIL;

	// --revelant points to convert from lua indecies to c
	wl_drop(--fromBank, --fromList, --toBank, --toList, --shipSlot);

	return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/TechRoom
ADE_LIB_DERIV(l_UserInterface_TechRoom,
	"TechRoom",
	nullptr,
	"API for accessing data related to the Tech Room UIs.",
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
	credits_parse(false);
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
	"cutscene_info",
	"Cutscene handle, or invalid handle if index is invalid")
{
	const char* name;
	if (!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_TechRoomCutscene.Set(cutscene_info_h(-1)));

	// coverity[uninit_use_in_call:FALSE] - name is assigned via ade_get_args
	int idx = get_cutscene_index_by_name(name);

	if (idx < 0) {
		try {
			idx = std::stoi(name);
			idx--; // Lua->FS2
		} catch (const std::exception&) {
			// Not a number
			return ade_set_error(L, "o", l_TechRoomCutscene.Set(cutscene_info_h(-1)));
		}

		if (!SCP_vector_inbounds(Cutscenes, idx)) {
			return ade_set_error(L, "o", l_TechRoomCutscene.Set(cutscene_info_h(-1)));
		}
	}

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

	const char *credits_wavfile_name = nullptr;

	// try substitute music first
	if (*Credits_substitute_music_name)
		credits_wavfile_name = credits_get_music_filename(Credits_substitute_music_name);

	// fall back to regular music
	if (!credits_wavfile_name)
		credits_wavfile_name = credits_get_music_filename(Credits_music_name);

	return ade_set_args(L, "s", credits_wavfile_name);
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

//**********SUBLIBRARY: UserInterface/Medals
ADE_LIB_DERIV(l_UserInterface_Medals,
	"Medals",
	nullptr,
	"API for accessing data related to the Medals UI.",
	l_UserInterface);

ADE_LIB_DERIV(l_Medals, "Medals_List", nullptr, nullptr, l_UserInterface_Medals);
ADE_INDEXER(l_Medals,
	"number Index",
	"Array of Medals",
	"medal",
	"medal handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Medal.Set(medal_h()));
	idx--; // Convert to Lua's 1 based index system

	if ((idx < 0) || (idx >= (int)Medals.size()))
		return ade_set_error(L, "o", l_Medal.Set(medal_h()));

	return ade_set_args(L, "o", l_Medal.Set(medal_h(idx)));
}

ADE_FUNC(__len, l_Medals, nullptr, "The number of valid medals", "number", "The number of valid medals.")
{
	return ade_set_args(L, "i", Medals.size());
}

ADE_LIB_DERIV(l_Ranks, "Ranks_List", nullptr, nullptr, l_UserInterface_Medals);
ADE_INDEXER(l_Ranks, "number Index", "Array of Ranks", "rank", "rank handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Rank.Set(rank_h()));
	idx--; // Convert to Lua's 1 based index system

	if ((idx < 0) || (idx >= (int)Ranks.size()))
		return ade_set_error(L, "o", l_Rank.Set(rank_h()));

	return ade_set_args(L, "o", l_Rank.Set(rank_h(idx)));
}

ADE_FUNC(__len, l_Ranks, nullptr, "The number of valid ranks", "number", "The number of valid ranks.")
{
	return ade_set_args(L, "i", Ranks.size());
}

//**********SUBLIBRARY: UserInterface/Hotkeys
ADE_LIB_DERIV(l_UserInterface_Hotkeys,
	"MissionHotkeys",
	nullptr,
	"API for accessing data related to the Mission Hotkeys UI.",
	l_UserInterface);

ADE_FUNC(initHotkeysList,
	l_UserInterface_Hotkeys,
	nullptr,
	"Initializes the hotkeys list. Must be used before the hotkeys list is accessed.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	reset_hotkeys();
	hotkey_set_selected_line(1);
	hotkey_lines_reset_all();
	hotkey_build_listing();

	// We want to allow the API to handle expanding wings on its own,
	// so lets expand every wing in the list and not try to handle it after that.
	for (int i = 0; i < MAX_LINES; i++) {
		if (Hotkey_lines[i].type == HotkeyLineType::WING) {
			expand_wing(i, true);
		}
	}

	// Reset the selected line back to 1
	hotkey_set_selected_line(1);

	return ADE_RETURN_NIL;
}

ADE_FUNC(resetHotkeys,
	l_UserInterface_Hotkeys,
	nullptr,
	"Resets the hotkeys list to previous setting, removing anything that wasn't saved. Returns nothing.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	reset_hotkeys();

	return ADE_RETURN_NIL;
}

ADE_FUNC(saveHotkeys,
	l_UserInterface_Hotkeys,
	nullptr,
	"Saves changes to the hotkey list. Returns nothing.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	save_hotkeys();

	return ADE_RETURN_NIL;
}

// This is not a retail UI feature, but it's just good design to allow it.
// This will allow SCPUI to create a restore to mission defaults button.
ADE_FUNC(resetHotkeysDefault,
	l_UserInterface_Hotkeys,
	nullptr,
	"Resets the hotkeys list to the default mission setting. Returns nothing.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	// argument to false, because if we're doing this we explicitely do not want
	// to restore player's saved values
	mission_hotkey_set_defaults(false);

	return ADE_RETURN_NIL;
}

ADE_LIB_DERIV(l_Hotkeys, "Hotkeys_List", nullptr, nullptr, l_UserInterface_Hotkeys);
ADE_INDEXER(l_Hotkeys,
	"number Index",
	"Array of Hotkey'd ships",
	"hotkey_ship",
	"hotkey ship handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Hotkey.Set(hotkey_h()));
	idx--; // Convert to Lua's 1 based index system

	if ((idx < 0) || (idx >= MAX_LINES))
		return ade_set_error(L, "o", l_Hotkey.Set(hotkey_h()));

	return ade_set_args(L, "o", l_Hotkey.Set(hotkey_h(idx)));
}

ADE_FUNC(__len, l_Hotkeys, nullptr, "The number of valid hotkey ships", "number", "The number of valid hotkey ships.")
{
	int s = 0;

	// this is dumb, but whatever
	for (int i = 0; i < MAX_LINES; i++) {
		if (Hotkey_lines[i].type == HotkeyLineType::NONE) {
			s = i;
			break;
		}
	}
	
	return ade_set_args(L, "i", s);
}

//**********SUBLIBRARY: UserInterface/GameHelp
ADE_LIB_DERIV(l_UserInterface_GameHelp,
	"GameHelp",
	nullptr,
	"API for accessing data related to the Game Help UI.",
	l_UserInterface);

ADE_FUNC(initGameHelp, l_UserInterface_GameHelp, nullptr, "Initializes the Game Help data. Must be used before Help Sections is accessed.", nullptr, nullptr)
{
	SCP_UNUSED(L);

	Help_text.clear(); // Make sure the vector is empty before we start
	Help_text = gameplay_help_init_text();

	return ADE_RETURN_NIL;
}

ADE_FUNC(closeGameHelp, l_UserInterface_GameHelp, nullptr, "Clears the Game Help data. Should be used when finished accessing Help Sections.", nullptr, nullptr)
{
	SCP_UNUSED(L);

	Help_text.clear();

	return ADE_RETURN_NIL;
}

ADE_LIB_DERIV(l_Help_Sections, "Help_Sections", nullptr, nullptr, l_UserInterface_GameHelp);
ADE_INDEXER(l_Help_Sections,
	"number Index",
	"Array of help sections",
	"help_section",
	"help section handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Help_Section.Set(help_section_h()));
	idx--; //Convert to Lua's 1 based index system

	if ((idx < 0) || (idx >= (int)Help_text.size()))
		return ade_set_error(L, "o", l_Help_Section.Set(help_section_h()));

	return ade_set_args(L, "o", l_Help_Section.Set(help_section_h(idx)));
}

ADE_FUNC(__len, l_Help_Sections, nullptr, "The number of help sections", "number", "The number of help sections.")
{
	return ade_set_args(L, "i", (int)Help_text.size());
}




//**********SUBLIBRARY: UserInterface/MissionLog
ADE_LIB_DERIV(l_UserInterface_MissionLog,
	"MissionLog",
	nullptr,
	"API for accessing data related to the Mission Log UI.",
	l_UserInterface);

ADE_FUNC(initMissionLog, l_UserInterface_MissionLog, nullptr, "Initializes the Mission Log data. Must be used before Mission Log is accessed.", nullptr, nullptr)
{
	SCP_UNUSED(L);

	//explicitly do not split lines!
	mission_log_init_scrollback(0, false);

	return ADE_RETURN_NIL;
}

ADE_FUNC(closeMissionLog, l_UserInterface_MissionLog, nullptr, "Clears the Mission Log data. Should be used when finished accessing Mission Log Entries.", nullptr, nullptr)
{
	SCP_UNUSED(L);

	mission_log_shutdown_scrollback();

	return ADE_RETURN_NIL;
}

ADE_LIB_DERIV(l_Log_Entries, "Log_Entries", nullptr, nullptr, l_UserInterface_MissionLog);
ADE_INDEXER(l_Log_Entries,
	"number Index",
	"Array of mission log entries",
	"log_entry",
	"log entry handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Log_Entry.Set(log_entry_h()));
	idx--; //Convert to Lua's 1 based index system

	if ((idx < 0) || (idx >= mission_log_scrollback_num_lines()))
		return ade_set_error(L, "o", l_Log_Entry.Set(log_entry_h()));

	return ade_set_args(L, "o", l_Log_Entry.Set(log_entry_h(idx)));
}

ADE_FUNC(__len, l_Log_Entries, nullptr, "The number of mission log entries", "number", "The number of log entries.")
{
	return ade_set_args(L, "i", mission_log_scrollback_num_lines());
}

ADE_LIB_DERIV(l_Log_Messages, "Log_Messages", nullptr, nullptr, l_UserInterface_MissionLog);
ADE_INDEXER(l_Log_Messages,
	"number Index",
	"Array of message log entries",
	"message_entry",
	"message entry handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Message_Entry.Set(message_entry_h()));
	idx--; //Convert to Lua's 1 based index system

	if ((idx < 0) || (idx >= (int)Msg_scrollback_vec.size()))
		return ade_set_error(L, "o", l_Message_Entry.Set(message_entry_h()));

	return ade_set_args(L, "o", l_Message_Entry.Set(message_entry_h(idx)));
}

ADE_FUNC(__len, l_Log_Messages, nullptr, "The number of mission message entries", "number", "The number of message entries.")
{
	return ade_set_args(L, "i", (int)Msg_scrollback_vec.size());
}

//**********SUBLIBRARY: UserInterface/ControlConfig
ADE_LIB_DERIV(l_UserInterface_ControlConfig,
	"ControlConfig",
	nullptr,
	"API for accessing data related to the Control Config UI.",
	l_UserInterface);

ADE_FUNC(initControlConfig,
	l_UserInterface_ControlConfig,
	nullptr,
	"Inits the control config UI elements. Must be used before accessing control config elements!",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	control_config_init(true);
	
	return ADE_RETURN_NIL;
}

ADE_FUNC(closeControlConfig,
	l_UserInterface_ControlConfig,
	nullptr,
	"Closes the control config UI elements. Must be used when finished accessing control config elements!",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	control_config_close(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(clearAll,
	l_UserInterface_ControlConfig,
	nullptr,
	"Clears all control bindings.",
	"boolean",
	"Returns true if successful, false otherwise")
{
	SCP_UNUSED(L);

	return ade_set_args(L, "b", control_config_clear_all(true));
}

ADE_FUNC(resetToPreset,
	l_UserInterface_ControlConfig,
	nullptr,
	"Resets all control bindings to the current preset defaults.",
	"boolean",
	"Returns true if successful, false otherwise")
{
	SCP_UNUSED(L);

	return ade_set_args(L, "b", control_config_do_reset(false, true));
}

ADE_FUNC(usePreset,
	l_UserInterface_ControlConfig,
	"string PresetName",
	"Uses a defined preset if it can be found.",
	"boolean",
	"Returns true if successful, false otherwise")
{
	const char* preset = nullptr;
	if (!ade_get_args(L, "s", &preset)) {
		return ADE_RETURN_FALSE;
	}

	SCP_string name = preset;

	return ade_set_args(L, "b", control_config_use_preset_by_name(name));
}

ADE_FUNC(createPreset,
	l_UserInterface_ControlConfig,
	"string Name, [boolean overwrite]",
	"Creates a new preset with the given name. Returns true if successful, false otherwise.",
	"boolean",
	"The return status")
{
	const char* preset;
	bool overwrite = false;
	ade_get_args(L, "s|b", &preset, &overwrite);

	SCP_string name = preset;

	return ade_set_args(L, "b", std::move(control_config_create_new_preset(name, overwrite)));
}

ADE_FUNC(undoLastChange,
	l_UserInterface_ControlConfig,
	nullptr,
	"Reverts the last change to the control bindings",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	control_config_do_undo(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(searchBinds,
	l_UserInterface_ControlConfig,
	nullptr,
	"Waits for a keypress to search for. Returns index into Control Configs if the key matches a bind. Should run On Frame.",
	"number",
	"Control Config index, or 0 if no key was found. Returns -1 if Escape was pressed.")
{
	SCP_UNUSED(L);

	int idx = control_config_search_key_on_frame(true);
	idx++; //convert to lua

	return ade_set_args(L, "i", idx);
}

ADE_FUNC(acceptBinding,
	l_UserInterface_ControlConfig,
	nullptr,
	"Accepts changes to the keybindings. Returns true if successful, false if there are key conflicts or the preset needs to be saved.",
	"boolean",
	"The return status")
{
	SCP_UNUSED(L);

	return ade_set_args(L, "b", control_config_accept(true));
}

ADE_FUNC(cancelBinding,
	l_UserInterface_ControlConfig,
	nullptr,
	"Cancels changes to the keybindings, reverting changes to the state it was when initControlConfig was called.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	control_config_cancel_exit(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(getCurrentPreset,
	l_UserInterface_ControlConfig,
	nullptr,
	"Returns the name of the current controls preset.",
	"string",
	"The name of the preset or nil if the current binds do not match a preset")
{
	SCP_UNUSED(L);

	auto it = control_config_get_current_preset();

	if (it == Control_config_presets.end()) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "s", it->name.c_str());
}

ADE_LIB_DERIV(l_Presets, "ControlPresets", nullptr, nullptr, l_UserInterface_ControlConfig);
ADE_INDEXER(l_Presets,
	"number Index",
	"Array of control presets",
	"preset",
	"control preset handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Preset.Set(preset_h()));
	idx--; // Convert to Lua's 1 based index system

	if ((idx < 0) || (idx >= (int)Control_config.size()))
		return ade_set_error(L, "o", l_Preset.Set(preset_h()));

	return ade_set_args(L, "o", l_Preset.Set(preset_h(idx)));
}

ADE_FUNC(__len, l_Presets, nullptr, "The number of control presets", "number", "The number of control presets.")
{
	return ade_set_args(L, "i", (int)Control_config_presets.size());
}

ADE_LIB_DERIV(l_Controls, "ControlConfigs", nullptr, nullptr, l_UserInterface_ControlConfig);
ADE_INDEXER(l_Controls,
	"number Index",
	"Array of controls",
	"control",
	"control handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Control.Set(control_h()));
	idx--; // Convert from Lua's 1 based index system

	if ((idx < 0) || (idx >= (int)Control_config.size()))
		return ade_set_error(L, "o", l_Control.Set(control_h()));

	return ade_set_args(L, "o", l_Control.Set(control_h(idx)));
}

ADE_FUNC(__len, l_Controls, nullptr, "The number of controls", "number", "The number of controls.")
{
	return ade_set_args(L, "i", (int)Control_config.size());
}

//**********SUBLIBRARY: UserInterface/HUDConfig
ADE_LIB_DERIV(l_UserInterface_HUDConfig,
	"HudConfig",
	nullptr,
	"API for accessing data related to the HUD Config UI.",
	l_UserInterface);

ADE_FUNC(initHudConfig,
	l_UserInterface_HUDConfig,
	"[number X, number Y, number Width, number height]",
	"Initializes the HUD Configuration data. Must be used before HUD Configuration data accessed. "
	"X and Y are the coordinates where the HUD preview will be drawn when drawHudConfig is used. "
	"Width is the pixel width to draw the gauges preview. Height is the pixel height to draw the gauges preview.",
	nullptr,
	nullptr)
{
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	ade_get_args(L, "|iiii", &x, &y, &w, &h);

	hud_config_init(true, x, y, w, h);

	return ADE_RETURN_NIL;
}

ADE_FUNC(closeHudConfig,
	l_UserInterface_HUDConfig,
	"boolean Save",
	"If True then saves the gauge configuration, discards if false. Defaults to false. Then cleans up memory. Should be used when finished accessing HUD Configuration.",
	nullptr,
	nullptr)
{
	bool save = false;
	ade_get_args(L, "|b", &save);

	if (save) {
		Pilot.save_savefile();
	} else {
		hud_config_cancel(false);
	}

	hud_config_close(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawHudConfig,
	l_UserInterface_HUDConfig,
	"[number MouseX, number MouseY]",
	"Draws the HUD for the HUD Config UI. Should be called On Frame.",
	"gauge_config",
	"Returns the gauge currently being hovered over, or empty handle if nothing is hovered")
{
	int mx = 0;
	int my = 0;
	ade_get_args(L, "|ii", &mx, &my);

	hud_config_do_frame(0.0f, true, mx, my);

	if (!HC_gauge_hot.empty()) {
		return ade_set_args(L, "o", l_Gauge_Config.Set(gauge_config_h(HC_gauge_hot)));
	} else {
		return ade_set_error(L, "o", l_Gauge_Config.Set(gauge_config_h()));
	}
}

ADE_FUNC(getCurrentHudName,
	l_UserInterface_HUDConfig,
	nullptr,
	"Returns the name of the current HUD configuration.",
	"string",
	"The name of the current HUD configuration.")
{
	SCP_UNUSED(L);

	if (SCP_vector_inbounds(HC_available_huds, HC_chosen_hud)) {
		return ade_set_args(L, "s", HC_available_huds[HC_chosen_hud].second.c_str());
	} else {
		return ade_set_args(L, "s", "Default Hud");
	}
}

ADE_FUNC(selectAllGauges,
	l_UserInterface_HUDConfig,
	"boolean Toggle",
	"Sets all gauges as selected. True for select all, False to unselect all. Defaults to False.",
	nullptr,
	nullptr)
{
	bool toggle = false;
	ade_get_args(L, "|b", &toggle);

	hud_config_select_all_toggle(toggle, true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(selectNoGauges, l_UserInterface_HUDConfig, nullptr, "Sets no gauges as selected.", nullptr, nullptr)
{
	SCP_UNUSED(L);

	hud_config_select_none();

	return ADE_RETURN_NIL;
}

ADE_FUNC(selectNextHud, l_UserInterface_HUDConfig, nullptr, "Selects the next available HUD", nullptr, nullptr)
{
	SCP_UNUSED(L);

	hud_config_select_hud(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(selectPrevHud, l_UserInterface_HUDConfig, nullptr, "Selects the previous available HUD", nullptr, nullptr)
{
	SCP_UNUSED(L);

	hud_config_select_hud(false);

	return ADE_RETURN_NIL;
}

ADE_FUNC(setToDefault,
	l_UserInterface_HUDConfig,
	"string Filename",
	"Sets all gauges to the defined default. If no filename is provided then 'hud_3.hcf' is used.",
	nullptr,
	nullptr)
{
	const char* filename = HC_default_preset_file.c_str();
	ade_get_args(L, "|s", &filename);

	hud_config_select_all_toggle(0, true);
	hud_set_default_hud_config(Player, filename);
	HUD_init_hud_color_array();

	return ADE_RETURN_NIL;
}

ADE_FUNC(saveToPreset,
	l_UserInterface_HUDConfig,
	"string Filename",
	"Saves all gauges to the file with the name provided. Filename should not include '.hcf' extension and not be longer than 28 characters.",
	nullptr,
	nullptr)
{
	const char* filename;
	ade_get_args(L, "s", &filename);

	SCP_string name = filename;

	// trim filename length to leave room for adding the extension
	if (name.size() > MAX_FILENAME_LEN - 4) {
		name.resize(MAX_FILENAME_LEN - 4);
	}

	// add extension
	name += ".hcf";

	hud_config_color_save(name.c_str());

	// reload the preset list for sorting
	hud_config_preset_init();

	return ADE_RETURN_NIL;
}

ADE_FUNC(usePresetFile,
	l_UserInterface_HUDConfig,
	"string Filename",
	"Sets all gauges to the provided preset file settings.",
	nullptr,
	nullptr)
{
	const char* filename;
	ade_get_args(L, "s", &filename);

	hud_config_color_load(filename);
	HUD_init_hud_color_array();

	return ADE_RETURN_NIL;
}

ADE_LIB_DERIV(l_HUD_Gauges, "GaugeConfigs", nullptr, nullptr, l_UserInterface_HUDConfig);
ADE_INDEXER(l_HUD_Gauges,
	"number Index",
	"Array of built-in gauge configs",
	"gauge_config",
	"gauge_config handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Gauge_Config.Set(gauge_config_h()));
	idx--; // Convert from Lua's 1 based index system

	if ((idx < 0) || (idx >= static_cast<int>(HC_gauge_map.size())))
		return ade_set_error(L, "o", l_Gauge_Config.Set(gauge_config_h()));

	return ade_set_args(L, "o", l_Gauge_Config.Set(gauge_config_h(HC_gauge_map[idx].first)));
}

ADE_FUNC(__len, l_HUD_Gauges, nullptr, "The number of gauge configs", "number", "The number of gauge configs.")
{
	return ade_set_args(L, "i", static_cast<int>(HC_gauge_map.size()));
}

ADE_LIB_DERIV(l_HUD_Presets, "GaugePresets", nullptr, nullptr, l_UserInterface_HUDConfig);
ADE_INDEXER(l_HUD_Presets,
	"number Index",
	"Array of HUD Preset files",
	"hud_preset",
	"hud_preset handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_HUD_Preset.Set(hud_preset_h()));
	idx--; // Convert from Lua's 1 based index system

	if ((idx < 0) || (idx >= (int)HC_preset_filenames.size()))
		return ade_set_error(L, "o", l_HUD_Preset.Set(hud_preset_h()));

	return ade_set_args(L, "o", l_HUD_Preset.Set(hud_preset_h(idx)));
}

ADE_FUNC(__len, l_HUD_Presets, nullptr, "The number of hud presets", "number", "The number of hud presets.")
{
	return ade_set_args(L, "i", HC_preset_filenames.size());
}

ADE_LIB_DERIV(l_HUD_Color_Presets, "GaugeColorPresets", nullptr, nullptr, l_UserInterface_HUDConfig);
ADE_INDEXER(l_HUD_Color_Presets,
	"number Index",
	"Array of HUD Color Presets",
	"hud_color_preset",
	"hud_color_preset handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_HUD_Color_Preset.Set(hud_color_preset_h()));
	idx--; // Convert from Lua's 1 based index system

	if ((idx < 0) || (idx >= NUM_HUD_COLOR_PRESETS))
		return ade_set_error(L, "o", l_HUD_Color_Preset.Set(hud_color_preset_h()));

	return ade_set_args(L, "o", l_HUD_Color_Preset.Set(hud_color_preset_h(idx)));
}

ADE_FUNC(__len, l_HUD_Color_Presets, nullptr, "The number of hud color presets", "number", "The number of hud color presets.")
{
	return ade_set_args(L, "i", NUM_HUD_COLOR_PRESETS);
}

//**********SUBLIBRARY: UserInterface/PauseScreen
ADE_LIB_DERIV(l_UserInterface_PauseScreen,
	"PauseScreen",
	nullptr,
	"API for accessing data related to the Pause Screen UI.",
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
	message_pause_all();

	Paused = true;

	return ADE_RETURN_NIL;
}

ADE_FUNC(closePause, l_UserInterface_PauseScreen, nullptr, "Makes sure everything is done correctly to unpause the game.", nullptr, nullptr)
{
	SCP_UNUSED(L);

	weapon_unpause_sounds();
	audiostream_unpause_all();
	message_resume_all();

	// FSO can run pause_init() before the actual games state change when the game loses focus
	// so this is required to make sure that the saved screen is cleared if SCPUI takes over
	// after the game state change
	gr_free_screen(Pause_saved_screen);
	Pause_saved_screen = -1;

	Paused = false;

	return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/MultiPXO
ADE_LIB_DERIV(l_UserInterface_MultiPXO,
	"MultiPXO",
	nullptr,
	"API for accessing data related to the Multi PXO UI.",
	l_UserInterface);

ADE_FUNC(initPXO, l_UserInterface_MultiPXO, nullptr, "Makes sure everything is done correctly to begin a multi lobby session.", nullptr, nullptr)
{
	SCP_UNUSED(L);

	multi_pxo_init(0, true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(closePXO, l_UserInterface_MultiPXO, nullptr, "Makes sure everything is done correctly to end a multi lobby session.", nullptr, nullptr)
{
	SCP_UNUSED(L);

	multi_pxo_close(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(runNetwork, l_UserInterface_MultiPXO, nullptr, "Runs the network commands to update the lobby lists once.", nullptr, nullptr)
{
	SCP_UNUSED(L);

	multi_pxo_do_normal(true);

	// run api stuff
	if (Multi_pxo_connected) {
		multi_pxo_api_process();
	}
	multi_pxo_process_common();

	return ADE_RETURN_NIL;
}

ADE_FUNC(getPXOLinks,
	l_UserInterface_MultiPXO,
	nullptr,
	"Gets all the various PXO links and returns them as a table of strings",
	"table",
	"A table of URLs")
{
	SCP_UNUSED(L);

	using namespace luacpp;

	LuaTable urls = LuaTable::create(L);

	int i = 1;
	urls.addValue(i++, Multi_options_g.pxo_rank_url);
	urls.addValue(i++, Multi_options_g.pxo_create_url);
	urls.addValue(i++, Multi_options_g.pxo_verify_url);
	urls.addValue(i++, Multi_options_g.pxo_banner_url);

	return ade_set_args(L, "t", urls);
}

ADE_FUNC(getChat,
	l_UserInterface_MultiPXO,
	nullptr,
	"Gets the entire chat as a table of tables each with the following values: "
	"Callsign - the name of the message sender, "
	"Message - the message text, "
	"Mode - the mode where 0 is normal, 1 is private message, 2 is channel switch, 3 is server message, 4 is MOTD",
	"table",
	"A table of chat strings")
{
	SCP_UNUSED(L);

	using namespace luacpp;

	LuaTable chats = LuaTable::create(L);

	int i = 1;
	for (auto& line : Multi_pxo_chat) {
		
		char callsign[CALLSIGN_LEN];
		char text[MAX_CHAT_LINE_LEN];

		char* spacePos = std::strchr(line.text, ' ');

		if (spacePos != nullptr) {
			// Calculate the length of the callsign
			int len = static_cast<int>(spacePos - line.text);

			// Copy the callsign
			std::strncpy(callsign, line.text, len);
			callsign[len] = '\0';

			// Copy the text
			std::strcpy(text, spacePos + 1); // Copy from the character after spacePos
		}

		int mode;
		switch (line.mode) {
		case CHAT_MODE_PRIVATE:
			mode = 1;
			break;
		case CHAT_MODE_CHANNEL_SWITCH:
			mode = 2;
			break;
		case CHAT_MODE_SERVER:
			mode = 3;
			break;
		case CHAT_MODE_MOTD:
			mode = 4;
			break;
		default:
			mode = 0;
			break;
		}

		auto item = luacpp::LuaTable::create(L);

		item.addValue("Callsign", luacpp::LuaValue::createValue(Script_system.GetLuaSession(), callsign));
		item.addValue("Message", luacpp::LuaValue::createValue(Script_system.GetLuaSession(), text));
		item.addValue("Mode", luacpp::LuaValue::createValue(Script_system.GetLuaSession(), mode));

		chats.addValue(i++, item);
	}

	return ade_set_args(L, "t", chats);
}

ADE_FUNC(sendChat, l_UserInterface_MultiPXO, "string", "Sends a string to the current channel's IRC chat", nullptr, nullptr)
{
	const char* msg;
	if (!ade_get_args(L, "s", &msg))
		return ADE_RETURN_NIL;

	multi_pxo_chat_send(msg);

	return ADE_RETURN_NIL;
}

ADE_FUNC(getPlayers, l_UserInterface_MultiPXO, nullptr, "Gets the entire player list as a table of strings", "string[]", "A table of player names")
{
	SCP_UNUSED(L);

	using namespace luacpp;

	LuaTable chats = LuaTable::create(L);

	for (size_t i = 0; i < Multi_pxo_players.size(); i++) {
		chats.addValue(i + 1, Multi_pxo_players[i]);
	}

	return ade_set_args(L, "t", chats);
}

ADE_FUNC(getPlayerChannel,
	l_UserInterface_MultiPXO,
	"string",
	"Searches for a player and returns if they were found and the channel they are on. Channel is an empty string if "
	"channel is private or player is not found.",
	"string, string",
	"The response string and the player's channel")
{
	const char* plr_name;
	if (!ade_get_args(L, "s", &plr_name))
		return ADE_RETURN_NIL;

	if (!Multi_pxo_connected) {
		return ade_set_args(L, "ss", "", "");
	}

	char* channel = nullptr;

	GetChannelByUser(plr_name);

	while (channel == nullptr) {
		os_sleep(10);
		multi_pxo_api_process();
		channel = GetChannelByUser(nullptr);
	}

	SCP_string response;
	SCP_string channel_name;
	if (channel == (char *)-1) {
		response = XSTR("User not found", 964);
		channel_name = "";
	} else {
		if (*channel == '*') {
			response = XSTR("Player is logged in but is not on a channel", 965);
			channel_name = "";
		} else {
			// if they are on a public channel, display which one
			if (*channel == '#') {
				sprintf(response, XSTR("Found %s on :", 966), plr_name);
				response += channel;
				channel_name = channel;

			// if they are on a private channel
			} else if (*channel == '+') {
				sprintf(response, XSTR("Found %s on a private channel", 967), plr_name);
				channel_name = "";
			}
		}
	}

	return ade_set_args(L, "ss", response.c_str(), channel_name.c_str());
}

ADE_FUNC(getPlayerStats, l_UserInterface_MultiPXO, "string", "Gets a handle of the player stats by player name or invalid handle if the name is invalid", "scoring_stats", "Player stats handle")
{
	const char* plr_name;
	if (!ade_get_args(L, "s", &plr_name))
		return ade_set_error(L, "o", l_ScoringStats.Set(scoring_stats_h()));
	
	if (multi_pxo_maybe_get_player(plr_name))
	{
		return ade_set_args(L, "o", l_ScoringStats.Set(scoring_stats_h(Multi_pxo_pinfo_player.stats, &Multi_pxo_pinfo_player)));
	}
	
	return ade_set_args(L, "o", l_ScoringStats.Set(scoring_stats_h()));
}

ADE_VIRTVAR(StatusText, l_UserInterface_MultiPXO, nullptr, "The current status text", "string", "the status text")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}
	
	return ade_set_args(L, "s", Multi_pxo_status_text);
}

ADE_VIRTVAR(MotdText, l_UserInterface_MultiPXO, nullptr, "The current message of the day", "string", "the motd text")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", Pxo_motd);
}

ADE_VIRTVAR(bannerFilename, l_UserInterface_MultiPXO, nullptr, "The current banner filename", "string", "the banner filename")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", Multi_pxo_banner.ban_file);
}

ADE_VIRTVAR(bannerURL, l_UserInterface_MultiPXO, nullptr, "The current banner URL", "string", "the banner url")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", Multi_pxo_banner.ban_url);
}

ADE_LIB_DERIV(l_PXO_Channels, "Channels", nullptr, nullptr, l_UserInterface_MultiPXO);
ADE_INDEXER(l_PXO_Channels,
	"number Index",
	"Array of channels",
	"pxo_channel",
	"channel handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");

	// convert from lua index
	idx--;

	if ((idx < 0) || idx >= static_cast<int>(Multi_pxo_channels.size()))
		return ade_set_args(L, "o", l_Channel.Set(channel_h()));

	return ade_set_args(L, "o", l_Channel.Set(channel_h(idx)));
}

ADE_FUNC(__len, l_PXO_Channels, nullptr, "The number of channels available", "number", "The number of channels.")
{
	return ade_set_args(L, "i", static_cast<int>(Multi_pxo_channels.size()));
}

ADE_FUNC(joinPrivateChannel, l_UserInterface_MultiPXO, "string channel", "Joins the specified private channel", nullptr, nullptr)
{
	const char* channel;
	if (!ade_get_args(L, "s", &channel))
		return ADE_RETURN_NIL;

	SCP_string name = channel;
	name.insert(0, "+");

	pxo_channel priv;
	strcpy_s(priv.name, name.c_str());
	priv.num_users = 0;

	multi_pxo_maybe_join_channel(&priv);

	return ADE_RETURN_NIL;
}

ADE_FUNC(getHelpText, l_UserInterface_MultiPXO, nullptr, "Gets the help text lines as a table of strings", "string[]", "The help lines")
{
	SCP_UNUSED(L);

	multi_pxo_help_load();

	using namespace luacpp;

	LuaTable pages = LuaTable::create(L);

	int count = 1;
	for (auto i = 0; i < MULTI_PXO_MAX_PAGES; i++) {
		for (int idx = 0; idx < Multi_pxo_help_pages[i].num_lines; idx++) {
			SCP_string text = Multi_pxo_help_pages[i].text[idx];
			pages.addValue(count, text.c_str());
			count++;
		}
	}

	multi_pxo_help_free();

	return ade_set_args(L, "t", pages);
}

//**********SUBLIBRARY: UserInterface/MultiGeneral
ADE_LIB_DERIV(l_UserInterface_MultiGeneral,
	"MultiGeneral",
	nullptr,
	"API for accessing general data related to all Multi UIs with the exception of the PXO Lobby.",
	l_UserInterface);

ADE_VIRTVAR(StatusText, l_UserInterface_MultiGeneral, nullptr, "The current status text", "string", "the status text")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", Multi_common_notify_text);
}

ADE_VIRTVAR(InfoText, l_UserInterface_MultiGeneral, nullptr, "The current info text", "string", "the info text")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", Multi_common_all_text);
}

ADE_FUNC(getNetGame,
	l_UserInterface_MultiGeneral,
	nullptr,
	"The handle to the netgame. Note that the netgame will be invalid if a multiplayer game has not been joined or created.",
	"netgame",
	"The netgame handle")
{
	SCP_UNUSED(L);
	return ade_set_args(L, "o", l_NetGame.Set(net_game_h()));
}

ADE_LIB_DERIV(l_Net_Players, "NetPlayers", nullptr, nullptr, l_UserInterface_MultiGeneral);
ADE_INDEXER(l_Net_Players,
	"number Index",
	"Array of net players",
	"net_player",
	"net player handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");

	// convert from lua index
	idx--;

	if ((idx < 0) || (idx >= MAX_PLAYERS))
		return ade_set_args(L, "o", l_NetPlayer.Set(net_player_h()));

	return ade_set_args(L, "o", l_NetPlayer.Set(net_player_h(idx)));
}

ADE_FUNC(__len, l_Net_Players, nullptr, "The number of net players", "number", "The number of players.")
{
	return ade_set_args(L, "i", MAX_PLAYERS);
}

ADE_FUNC(getChat,
	l_UserInterface_MultiGeneral,
	nullptr,
	"Gets the entire chat as a table of tables each with the following values: "
	"Callsign - the name of the message sender, "
	"Message - the message text, "
	"Color - the color the text should be according to the player id",
	"table",
	"A table of chat strings")
{
	SCP_UNUSED(L);

	using namespace luacpp;

	LuaTable chats = LuaTable::create(L);

	int i = 1;
	SCP_string text = "";
	SCP_string callsign = "";
	color* clr = nullptr;

	auto createChatEntry = [L, &i, &chats](const SCP_string& this_text, const SCP_string& this_callsign, const color* this_clr) {
			auto item = luacpp::LuaTable::create(L);
			item.addValue("Callsign", luacpp::LuaValue::createValue(Script_system.GetLuaSession(), this_callsign.c_str()));
			item.addValue("Message", luacpp::LuaValue::createValue(Script_system.GetLuaSession(), this_text.c_str()));
			item.addValue("Color", l_Color.Set(*this_clr));
			chats.addValue(i++, item);
	};


	bool send = false;
	for (auto& chat : Brief_chat) {

		// In API mode we don't need to worry about line splits
		// so reconnect all indented lines into a single chat line and
		// let the API's UI deal with the rest.
		// But first send the previous chat line, if any, to the lua table
		// now that we know we're done concatenating indented lines.
		if (!chat.indent) {
			if (send){
				createChatEntry(text, callsign, clr);

				//Sent, so now clear and start over
				text.clear();
				callsign.clear();
				clr = nullptr;
			}

			text = chat.text;
			callsign = chat.callsign;
			clr = Color_netplayer[chat.player_id];

			send = true;
		} else {
			text += chat.text;
		}
	}

	// We might have one more line to add
	if (send) {
		createChatEntry(text, callsign, clr);
	}

	return ade_set_args(L, "t", chats);
}

ADE_FUNC(sendChat,
	l_UserInterface_MultiGeneral,
	"string",
	"Sends a string to the current game's IRC chat. Limited to 125 characters. Anything after that is dropped.",
	nullptr,
	nullptr)
{
	const char* msg;
	if (!ade_get_args(L, "s", &msg))
		return ADE_RETURN_NIL;

	char temp[CHATBOX_MAX_LEN];
	strncpy(temp, msg, CHATBOX_MAX_LEN);
	temp[CHATBOX_MAX_LEN - 1] = '\0';

	chatbox_send_msg(temp);

	return ADE_RETURN_NIL;
}

ADE_FUNC(quitGame,
	l_UserInterface_MultiGeneral,
	nullptr, "Quits the game for the current player and returns them to the PXO lobby", nullptr, nullptr)
{
	SCP_UNUSED(L);

	if (Game_mode & GM_MULTIPLAYER) {
		multi_quit_game(PROMPT_ALL);
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(setPlayerState, l_UserInterface_MultiGeneral, nullptr, "Sets the current player's network state based on the current game state.", nullptr, nullptr)
{
	SCP_UNUSED(L);

	int state = gameseq_get_state();

	// We really only need to handle this for game states that the lua API
	// completely replaces; Where the state_init() code doesn't actually run
	// but it won't hurt to include others.
	// This may not be an exhaustive list. Feel free to add more.
	switch (state) {
	case GS_STATE_BRIEFING:
		Net_player->state = NETPLAYER_STATE_BRIEFING;
		break;
	case GS_STATE_CMD_BRIEF:
		Net_player->state = NETPLAYER_STATE_CMD_BRIEFING;
		break;
	case GS_STATE_SHIP_SELECT:
		Net_player->state = NETPLAYER_STATE_SHIP_SELECT;
		break;
	case GS_STATE_WEAPON_SELECT:
		Net_player->state = NETPLAYER_STATE_WEAPON_SELECT;
		break;
	case GS_STATE_RED_ALERT:
		Net_player->state = NETPLAYER_STATE_RED_ALERT;
		break;
	case GS_STATE_DEBRIEF:
		Net_player->state = NETPLAYER_STATE_DEBRIEF;
		break;
	case GS_STATE_FICTION_VIEWER:
		Net_player->state = NETPLAYER_STATE_FICTION_VIEWER;
		break;
	case GS_STATE_MULTI_HOST_SETUP:
		Net_player->state = NETPLAYER_STATE_HOST_SETUP;
		break;
	case GS_STATE_MULTI_MISSION_SYNC:
		Net_player->state = NETPLAYER_STATE_MISSION_SYNC;
		break;
	case GS_STATE_TEAM_SELECT:
		Net_player->state = NETPLAYER_STATE_SHIP_SELECT;
		break;
	case GS_STATE_MULTI_CLIENT_SETUP:
		Net_player->state = NETPLAYER_STATE_JOINED;
		break;
	default:
		break;
	}

	return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/MultiJoinGame
ADE_LIB_DERIV(l_UserInterface_MultiJoinGame,
	"MultiJoinGame",
	nullptr,
	"API for accessing data related to the Multi Join Game UI.",
	l_UserInterface);

ADE_FUNC(initMultiJoin,
	l_UserInterface_MultiJoinGame,
	nullptr,
	"Makes sure everything is done correctly to begin a multi join session.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_join_game_init(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(closeMultiJoin,
	l_UserInterface_MultiJoinGame,
	nullptr,
	"Makes sure everything is done correctly to end a multi join session.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_join_game_close(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(runNetwork,
	l_UserInterface_MultiJoinGame,
	nullptr,
	"Runs the network required commands to update the lists once and handle join requests",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_join_game_do_frame(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(refresh, l_UserInterface_MultiJoinGame, nullptr, "Force refreshing the games list", nullptr, nullptr)
{
	SCP_UNUSED(L);

	broadcast_game_query();

	return ADE_RETURN_NIL;
}

ADE_FUNC(createGame,
	l_UserInterface_MultiJoinGame,
	nullptr,
	"Starts creating a new game and moves to the new UI",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_join_create_game();

	return ADE_RETURN_NIL;
}

ADE_FUNC(sendJoinRequest,
	l_UserInterface_MultiJoinGame,
	"[boolean AsObserver]",
	"Sends a join game request",
	"boolean",
	"True if successful, false otherwise")
{

	bool observer = false;
	ade_get_args(L, "|b", &observer);

	if (Active_games.empty()) {
		multi_common_add_notify(XSTR("No games found!", 757));
		return ADE_RETURN_FALSE;
	} else if (Multi_join_selected_item == nullptr) {
		multi_common_add_notify(XSTR("No game selected!", 758));
		return ADE_RETURN_FALSE;
	} else if (Multi_join_sent_stamp.isValid() && !ui_timestamp_elapsed(Multi_join_sent_stamp)) {
		multi_common_add_notify(XSTR("Still waiting on previous join request!", 759));
	} else {
		// otherwise, if he's already played PXO games, warn him

		if (Player->flags & PLAYER_FLAGS_HAS_PLAYED_PXO) {
			if (!multi_join_warn_pxo()) {
				return ADE_RETURN_FALSE;
			}
		}

		// send a join request packet
		multi_join_send_join_request(observer);
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

ADE_LIB_DERIV(l_Active_Games, "ActiveGames", nullptr, nullptr, l_UserInterface_MultiJoinGame);
ADE_INDEXER(l_Active_Games,
	"number Index",
	"Array of active games",
	"active_game",
	"active game handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");

	// convert from lua index
	idx--;

	if ((idx < 0) || idx >= static_cast<int>(Active_games.size()))
		return ade_set_args(L, "o", l_Active_Game.Set(active_game_h()));

	return ade_set_args(L, "o", l_Active_Game.Set(active_game_h(idx)));
}

ADE_FUNC(__len,
	l_Active_Games,
	nullptr,
	"The number of active games available",
	"number",
	"The number of active games.")
{
	return ade_set_args(L, "i", static_cast<int>(Active_games.size()));
}

//**********SUBLIBRARY: UserInterface/MultiStartGame
ADE_LIB_DERIV(l_UserInterface_MultiStartGame,
	"MultiStartGame",
	nullptr,
	"API for accessing data related to the Multi Start Game UI.",
	l_UserInterface);

ADE_FUNC(initMultiStart,
	l_UserInterface_MultiStartGame,
	nullptr,
	"Initializes the Create Game methods and variables",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_start_game_init(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(closeMultiStart,
	l_UserInterface_MultiStartGame,
	"boolean Start_or_Quit",
	"Finalizes the new game settings and moves to the host game UI if true or cancels if false. Defaults to true.",
	nullptr,
	nullptr)
{
	bool choice = true;
	if (!ade_get_args(L, "b", &choice))
		return ADE_RETURN_NIL;

	if (choice) {
		multi_start_game_close(true);
	} else {
		multi_quit_game(PROMPT_NONE);
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(runNetwork,
	l_UserInterface_MultiStartGame,
	nullptr,
	"Runs the network required commands to update the status text",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_start_game_do(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(setName,
	l_UserInterface_MultiStartGame,
	"string Name",
	"Sets the game's name",
	"boolean",
	"True if successful, false otherwise")
{
	const char* name;
	if (!ade_get_args(L, "s", &name))
		return ADE_RETURN_FALSE;

	if (strlen(name) > MAX_GAMENAME_LEN + 1) {
		return ADE_RETURN_FALSE;
	}

	strcpy_s(Multi_sg_netgame->name, name);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(setGameType,
	l_UserInterface_MultiStartGame,
	"enumeration type=MULTI_GAME_TYPE_OPEN, [string | number password_or_rank_index]",
	"Sets the game's type and, optionally, the password or rank index.",
	"boolean",
	"True if successful, false otherwise")
{
	enum_h* game = nullptr;
	const char* password = nullptr;
	int rank_idx = 0;
	if (!ade_get_args(L, "o", l_Enum.GetPtr(&game))) {
		return ADE_RETURN_FALSE;
	}

	switch (game->index) {
	case LE_MULTI_GAME_TYPE_OPEN:
		Multi_sg_netgame->mode = NG_MODE_OPEN;
		break;
	case LE_MULTI_GAME_TYPE_PASSWORD:
		Multi_sg_netgame->mode = NG_MODE_PASSWORD;
		if (!ade_get_args(L, "*s", &password)) {
			return ADE_RETURN_FALSE;
		}
		if ((password == nullptr) || (strlen(password) > MAX_PASSWD_LEN + 1)) {
			return ADE_RETURN_FALSE;
		}
		strcpy_s(Multi_sg_netgame->passwd, password);
		break;
	case LE_MULTI_GAME_TYPE_RANK_ABOVE:
		Multi_sg_netgame->mode = NG_MODE_RANK_ABOVE;
		if (!ade_get_args(L, "*i", &rank_idx)) {
			return ADE_RETURN_FALSE;
		}
		Multi_sg_netgame->rank_base = verify_rank(rank_idx);
		break;
	case LE_MULTI_GAME_TYPE_RANK_BELOW:
		Multi_sg_netgame->mode = NG_MODE_RANK_BELOW;
		if (!ade_get_args(L, "*i", &rank_idx)) {
			return ADE_RETURN_FALSE;
		}
		Multi_sg_netgame->rank_base = verify_rank(rank_idx);
		break;
	default:
		return ADE_RETURN_FALSE;
		break;
	}

	return ADE_RETURN_TRUE;
}

//**********SUBLIBRARY: UserInterface/MultiHostSetup
ADE_LIB_DERIV(l_UserInterface_MultiHostSetup,
	"MultiHostSetup",
	nullptr,
	"API for accessing data related to the Multi Host Setup UI.",
	l_UserInterface);

ADE_FUNC(initMultiHostSetup,
	l_UserInterface_MultiHostSetup,
	nullptr,
	"Makes sure everything is done correctly to begin the host setup ui.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_create_game_init(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(closeMultiHostSetup,
	l_UserInterface_MultiHostSetup,
	"boolean Commit_or_Quit",
	"Closes Multi Host Setup. True to commit, false to quit.",
	nullptr,
	nullptr)
{
	bool choice = true;
	if (!ade_get_args(L, "b", &choice))
		return ADE_RETURN_NIL;

	if (choice) {
		int idx = -1;
		if (Netgame.campaign_mode == MULTI_CREATE_SHOW_MISSIONS)
			for (size_t i = 0; i < Multi_create_mission_list.size(); i++) {
				if (strcmp(Multi_create_mission_list[i].filename, Netgame.mission_name) == 0) {
					idx = static_cast<int>(i);
					break;
				}
			}
		else {
			for (size_t i = 0; i < Multi_create_campaign_list.size(); i++) {
				if (strcmp(Multi_create_campaign_list[i].filename, Netgame.mission_name) == 0) {
					idx = static_cast<int>(i);
					break;
				}
			}
		}
		if (multi_create_ok_to_commit(idx)) {
			//Some of this seems redundant but that's what the retail UI does!
			multi_create_accept_hit(Netgame.campaign_mode, idx);
		}
	} else {
		multi_quit_game(PROMPT_HOST);
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(runNetwork,
	l_UserInterface_MultiHostSetup,
	nullptr,
	"Runs the network required commands to update the lists and run the chat",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_create_game_do(true);

	return ADE_RETURN_NIL;
}

ADE_LIB_DERIV(l_Net_Missions, "NetMissions", nullptr, nullptr, l_UserInterface_MultiHostSetup);
ADE_INDEXER(l_Net_Missions,
	"number Index",
	"Array of net missions",
	"net_mission",
	"net player handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");

	// convert from lua index
	idx--;

	if (!SCP_vector_inbounds(Multi_create_mission_list, idx))
		return ade_set_args(L, "o", l_NetMission.Set(net_mission_h()));

	return ade_set_args(L, "o", l_NetMission.Set(net_mission_h(idx)));
}

ADE_FUNC(__len, l_Net_Missions, nullptr, "The number of net missions", "number", "The number of missions.")
{
	return ade_set_args(L, "i", static_cast<int>(Multi_create_mission_list.size()));
}

ADE_LIB_DERIV(l_Net_Campaigns, "NetCampaigns", nullptr, nullptr, l_UserInterface_MultiHostSetup);
ADE_INDEXER(l_Net_Campaigns,
	"number Index",
	"Array of net campaigns",
	"net_campaign",
	"net player handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");

	// convert from lua index
	idx--;

	if (!SCP_vector_inbounds(Multi_create_campaign_list, idx))
		return ade_set_args(L, "o", l_NetCampaign.Set(net_campaign_h()));

	return ade_set_args(L, "o", l_NetCampaign.Set(net_campaign_h(idx)));
}

ADE_FUNC(__len, l_Net_Campaigns, nullptr, "The number of net campaigns", "number", "The number of campaigns.")
{
	return ade_set_args(L, "i", static_cast<int>(Multi_create_campaign_list.size()));
}

//**********SUBLIBRARY: UserInterface/MultiClientSetup
ADE_LIB_DERIV(l_UserInterface_MultiClientSetup,
	"MultiClientSetup",
	nullptr,
	"API for accessing data related to the Multi Client Setup UI.",
	l_UserInterface);

ADE_FUNC(initMultiClientSetup,
	l_UserInterface_MultiClientSetup,
	nullptr,
	"Makes sure everything is done correctly to begin the client setup ui.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_game_client_setup_init(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(closeMultiClientSetup,
	l_UserInterface_MultiClientSetup,
	nullptr,
	"Cancels Multi Client Setup and leaves the game.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_quit_game(PROMPT_CLIENT);

	return ADE_RETURN_NIL;
}

ADE_FUNC(runNetwork,
	l_UserInterface_MultiClientSetup,
	nullptr,
	"Runs the network required commands to update the lists and run the chat",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_game_client_setup_do_frame(true);

	return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/MultiSync
ADE_LIB_DERIV(l_UserInterface_MultiSync,
	"MultiSync",
	nullptr,
	"API for accessing data related to the Multi Sync UI.",
	l_UserInterface);

ADE_FUNC(initMultiSync,
	l_UserInterface_MultiSync,
	nullptr,
	"Makes sure everything is done correctly to begin the multi sync ui.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_sync_init(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(closeMultiSync,
	l_UserInterface_MultiSync, "boolean QuitGame",
	"Closes MultiSync. If QuitGame is true then it cancels and leaves the game automatically.",
	nullptr,
	nullptr)
{
	bool choice = true;
	if (!ade_get_args(L, "b", &choice))
		return ADE_RETURN_NIL;

	if (!choice) {
		multi_sync_close(true);
	} else {
		multi_quit_game(PROMPT_CLIENT);
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(runNetwork,
	l_UserInterface_MultiSync,
	nullptr,
	"Runs the network required commands to run the chat",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_sync_do(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(startCountdown, l_UserInterface_MultiSync, nullptr,
	"Starts the Launch Mission Countdown that, when finished, will move all players into the mission.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);
	
	multi_sync_start_countdown();

	return ADE_RETURN_NIL;
}

ADE_FUNC(getCountdownTime,
	l_UserInterface_MultiSync,
	nullptr,
	"Gets the current countdown time. Will be -1 before the countdown starts otherwise will be the num seconds until missions starts.",
	"number",
	"The countdown in seconds")
{
	SCP_UNUSED(L);

	return ade_set_args(L, "i", Multi_sync_countdown);
}

//**********SUBLIBRARY: UserInterface/MultiPreJoin
ADE_LIB_DERIV(l_UserInterface_MultiPreJoin,
	"MultiPreJoin",
	nullptr,
	"API for accessing data related to the Pre Join UI.",
	l_UserInterface);

ADE_FUNC(initPreJoin,
	l_UserInterface_MultiPreJoin,
	nullptr,
	"Makes sure everything is done correctly to init the pre join ui.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_ingame_select_init(true);

	return ADE_RETURN_NIL;
}

ADE_LIB_DERIV(l_Join_Ship_Choices, "JoinShipChoices", nullptr, nullptr, l_UserInterface_MultiPreJoin);
ADE_INDEXER(l_Join_Ship_Choices,
	"number Index",
	"Array of ship choices. Ingame Join must be inited first",
	"net_join_choice",
	"net choice handle, or invalid handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");

	// convert from lua index
	idx--;

	if (!SCP_vector_inbounds(Ingame_ship_choices, idx))
		return ade_set_args(L, "o", l_Join_Ship_Choice.Set(join_ship_choices_h()));

	return ade_set_args(L, "o", l_Join_Ship_Choice.Set(join_ship_choices_h(idx)));
}

ADE_FUNC(__len, l_Join_Ship_Choices, nullptr, "The number of ship choices", "number", "The number of ships.")
{
	return ade_set_args(L, "i", static_cast<int>(Ingame_ship_choices.size()));
}

ADE_FUNC(closePreJoin,
	l_UserInterface_MultiPreJoin,
	"[boolean Accept]",
	"Makes sure everything is done correctly to accept or cancel the pre join. True to accept, False to quit",
	"boolean",
	"returns true if successful, false otherwise")
{
	bool accept = false;
	ade_get_args(L, "*b", &accept);

	bool result;
	if (accept) {
		result = multi_ingame_join_accept(true);
	} else {
		multi_quit_game(PROMPT_CLIENT);
		result = true;
	}

	Ingame_ship_choices.clear();
	Ingame_ship_choices.shrink_to_fit();

	return ade_set_args(L, "b", result);
}

ADE_FUNC(runNetwork, l_UserInterface_MultiPreJoin,
	nullptr,
	"Runs the network required commands.", "number", "The seconds left until pre join times out")
{
	SCP_UNUSED(L);

	multi_ingame_join_calc_avail(true);
	int time = multi_ingame_handle_timeout(true);

	return ade_set_args(L, "i", time);
}

//**********SUBLIBRARY: UserInterface/MultiPauseScreen
ADE_LIB_DERIV(l_UserInterface_MultiPauseScreen,
	"MultiPauseScreen",
	nullptr,
	"API for accessing data related to the Pause Screen UI.",
	l_UserInterface);

ADE_VIRTVAR(isPaused,
	l_UserInterface_MultiPauseScreen,
	nullptr,
	"Returns true if the game is paused, false otherwise",
	"boolean",
	"true if paused, false if unpaused")
{
	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "b", Multi_paused);
}

ADE_VIRTVAR(Pauser,
	l_UserInterface_MultiPauseScreen,
	nullptr,
	"The callsign of who paused the game. Empty string if invalid",
	"string",
	"the callsign")
{
	SCP_UNUSED(L);
	
	if ((Multi_pause_pauser != NULL) && (Multi_pause_pauser->m_player != NULL)) {
		return ade_set_args(L, "s", Multi_pause_pauser->m_player->callsign);
	}

	return ade_set_args(L, "s", "");
}

ADE_FUNC(requestUnpause,
	l_UserInterface_MultiPauseScreen,
	nullptr,
	"Sends a request to unpause the game.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_pause_request(0);

	return ADE_RETURN_NIL;
}

ADE_FUNC(initPause,
	l_UserInterface_MultiPauseScreen,
	nullptr,
	"Makes sure everything is done correctly to pause the game.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_pause_init(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(closePause,
	l_UserInterface_MultiPauseScreen,
	"[boolean EndMission]",
	"Makes sure everything is done correctly to unpause the game. If end mission is true then it tries to end the mission.",
	nullptr,
	nullptr)
{
	bool end_mission = false;
	ade_get_args(L, "*b", &end_mission);

	multi_pause_close(end_mission, true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(runNetwork,
	l_UserInterface_MultiPauseScreen,
	nullptr,
	"Runs the network required commands.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_pause_do(true);

	return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/MultiDogfightDebrief
ADE_LIB_DERIV(l_UserInterface_MultiDogfightDebrief,
	"MultiDogfightDebrief",
	nullptr,
	"API for accessing data related to the Dogfight Debrief UI.",
	l_UserInterface);

ADE_FUNC(getDogfightScores,
	l_UserInterface_MultiDogfightDebrief,
	"net_player",
	"The handle to the dogfight scores",
	"dogfight_scores",
	"The dogfight scores handle")
{
	net_player_h player;
	if (!ade_get_args(L, "o", l_NetPlayer.Get(&player)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "o", l_Dogfight_Scores.Set(dogfight_scores_h(player.getIndex())));
}

ADE_FUNC(initDebrief,
	l_UserInterface_MultiDogfightDebrief,
	nullptr,
	"Makes sure everything is done correctly to init the dogfight scores.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_df_debrief_init(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(closeDebrief,
	l_UserInterface_MultiDogfightDebrief,
	"[boolean Accept]",
	"Makes sure everything is done correctly to accept or close the debrief. True to accept, False to quit",
	nullptr,
	nullptr)
{
	bool accept = false;
	ade_get_args(L, "*b", &accept);

	if (accept) {
		multi_debrief_accept_hit();
	} else {
		multi_debrief_esc_hit();
	}
	multi_df_debrief_close(true);

	return ADE_RETURN_NIL;
}

ADE_FUNC(runNetwork,
	l_UserInterface_MultiDogfightDebrief,
	nullptr,
	"Runs the network required commands.",
	nullptr,
	nullptr)
{
	SCP_UNUSED(L);

	multi_df_debrief_do(true);

	return ADE_RETURN_NIL;
}

} // namespace api
} // namespace scripting
