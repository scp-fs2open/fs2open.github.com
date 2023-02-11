//
//

#include "player.h"

#include "gamesequence/gamesequence.h"
#include "menuui/mainhallmenu.h"
#include "menuui/readyroom.h"
#include "menuui/techmenu.h"
#include "mission/missioncampaign.h"
#include "pilotfile/pilotfile.h"
#include "playerman/player.h"
#include "scripting/api/objs/shipclass.h"
#include "scripting/lua/LuaTable.h"
#include "ship/ship.h"

namespace scripting {
namespace api {

player_h::player_h() = default;
player_h::player_h(player* plr)
{
	_plr = plr;
	_owned = false;
}
player_h::player_h(const player& plr)
{
	_plr = new player();
	_plr->assign(&plr);
	_owned = true;
}
bool player_h::isValid() const { return _plr != nullptr; }
player* player_h::get() { return _plr; }
player_h::~player_h()
{
	if (_owned)
	{
		delete _plr;
	}
	_plr = nullptr;
}
player_h::player_h(player_h&& other) noexcept { *this = std::move(other); }
player_h& player_h::operator=(player_h&& other) noexcept
{
	std::swap(_plr, other._plr);
	return *this;
}

//**********HANDLE: Player
ADE_OBJ(l_Player, player_h, "player", "Player handle");


ADE_VIRTVAR(Stats, l_Player, "scoring_stats stats", "The scoring stats of this player (read-only)", "scoring_stats", "The player stats or invalid handle") {
	player_h* plr;
	if (!ade_get_args(L, "o", l_Player.GetPtr(&plr)))
		return ade_set_error(L, "o", l_ScoringStats.Set(scoring_stats_h()));

	if (!plr->isValid())
		return ade_set_error(L, "o", l_ScoringStats.Set(scoring_stats_h()));

	return ade_set_args(L, "o", l_ScoringStats.Set(scoring_stats_h(plr->get()->stats)));
}

ADE_VIRTVAR(ImageFilename, l_Player, "string name", "The image filename of this pilot", "string",
            "Player image filename, or empty string if handle is invalid")
{
	player_h* plr;
	const char* filename = nullptr;
	if (!ade_get_args(L, "o|s", l_Player.GetPtr(&plr), &filename))
		return ade_set_error(L, "s", "");

	if (!plr->isValid())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR && filename != nullptr) {
		strcpy_s(plr->get()->image_filename, filename);
	}

	return ade_set_args(L, "s", plr->get()->image_filename);
}

ADE_VIRTVAR(SingleSquadFilename, l_Player, "string name", "The singleplayer squad filename of this pilot", "string",
            "singleplayer squad image filename, or empty string if handle is invalid")
{
	player_h* plr;
	const char* filename = nullptr;
	if (!ade_get_args(L, "o|s", l_Player.GetPtr(&plr), &filename))
		return ade_set_error(L, "s", "");

	if (!plr->isValid())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR && filename != nullptr) {
		strcpy_s(plr->get()->s_squad_filename, filename);
	}

	return ade_set_args(L, "s", plr->get()->s_squad_filename);
}

ADE_VIRTVAR(MultiSquadFilename, l_Player, "string name", "The multiplayer squad filename of this pilot", "string",
            "Multiplayer squad image filename, or empty string if handle is invalid")
{
	player_h* plr;
	const char* filename = nullptr;
	if (!ade_get_args(L, "o|s", l_Player.GetPtr(&plr), &filename))
		return ade_set_error(L, "s", "");

	if (!plr->isValid())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR && filename != nullptr) {
		strcpy_s(plr->get()->m_squad_filename, filename);
	}

	return ade_set_args(L, "s", plr->get()->m_squad_filename);
}

ADE_VIRTVAR(IsMultiplayer, l_Player, "boolean value", "Determines if this player is currently configured for multiplayer.", "boolean", "true if this is a multiplayer pilot, false otherwise or if the handle is invalid") {
	player_h* plr;
	bool value = false;
	if (!ade_get_args(L, "o|b", l_Player.GetPtr(&plr), &value))
		return ADE_RETURN_FALSE;

	if (!plr->isValid())
		return ADE_RETURN_FALSE;

	if (ADE_SETTING_VAR) {
		if (value) {
			plr->get()->flags |= PLAYER_FLAGS_IS_MULTI;
		} else {
			plr->get()->flags &= ~PLAYER_FLAGS_IS_MULTI;
		}
	}

	return ade_set_args(L, "b", (plr->get()->flags & PLAYER_FLAGS_IS_MULTI) != 0);
}

ADE_VIRTVAR(WasMultiplayer, l_Player, "boolean value", "Determines if this player is currently configured for multiplayer.", "boolean", "true if this is a multiplayer pilot, false otherwise or if the handle is invalid") {
	player_h* plr;
	bool value = false;
	if (!ade_get_args(L, "o|b", l_Player.GetPtr(&plr), &value))
		return ADE_RETURN_FALSE;

	if (!plr->isValid())
		return ADE_RETURN_FALSE;

	if (ADE_SETTING_VAR) {
		plr->get()->player_was_multi = value ? 1 : 0;
	}

	return ade_set_args(L, "b", plr->get()->player_was_multi != 0);
}

ADE_VIRTVAR(AutoAdvance,
	l_Player,
	"boolean value",
	"Determines if briefing stages should be auto advanced.",
	"boolean",
	"true if auto advance is enabled, false otherwise or if the handle is invalid")
{
	player_h* plr;
	bool value = false;
	if (!ade_get_args(L, "o|b", l_Player.GetPtr(&plr), &value))
		return ADE_RETURN_FALSE;

	if (!plr->isValid())
		return ADE_RETURN_FALSE;

	if (ADE_SETTING_VAR) {
		plr->get()->auto_advance = value ? 1 : 0;
	}

	return ade_set_args(L, "b", plr->get()->auto_advance != 0);
}

ADE_VIRTVAR(ShowSkipPopup,
	l_Player,
	"boolean value",
	"Determines if the skip mission popup is shown for the current mission.",
	"boolean",
	"true if it should be shown, false otherwise or if the handle is invalid")
{
	player_h* plr;
	bool value = false;
	if (!ade_get_args(L, "o|b", l_Player.GetPtr(&plr), &value))
		return ADE_RETURN_FALSE;

	if (!plr->isValid())
		return ADE_RETURN_FALSE;

	if (ADE_SETTING_VAR) {
		plr->get()->show_skip_popup = value ? 1 : 0;
	}

	return ade_set_args(L, "b", plr->get()->show_skip_popup != 0);
}

ADE_FUNC(isValid, l_Player, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	player_h* plr;
	if (!ade_get_args(L, "o", l_Player.GetPtr(&plr)))
		return ADE_RETURN_NIL;

	if (!plr->isValid())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getName, l_Player, NULL, "Gets current player name", "string", "Player name, or empty string if handle is invalid")
{
	player_h* plr;
	if (!ade_get_args(L, "o", l_Player.GetPtr(&plr)))
		return ade_set_error(L, "s", "");

	if (!plr->isValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", plr->get()->callsign);
}

ADE_FUNC(getCampaignFilename, l_Player, NULL, "Gets current player campaign filename", "string", "Campaign name, or empty string if handle is invalid")
{
	player_h* plr;
	if (!ade_get_args(L, "o", l_Player.GetPtr(&plr)))
		return ade_set_error(L, "s", "");

	if (!plr->isValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", plr->get()->current_campaign);
}

ADE_FUNC(getImageFilename, l_Player, NULL, "Gets current player image filename", "string", "Player image filename, or empty string if handle is invalid")
{
	player_h* plr;
	if (!ade_get_args(L, "o", l_Player.GetPtr(&plr)))
		return ade_set_error(L, "s", "");

	if (!plr->isValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", plr->get()->image_filename);
}

ADE_FUNC(getMainHallName, l_Player, NULL, "Gets player's current main hall name", "string", "Main hall name, or name of first mainhall in campaign if something goes wrong")
{
	SCP_string hallname;
	// FS2-->Lua
	if (Campaign.next_mission == -1) {
		hallname = Campaign.missions[0].main_hall;
	} else {
		hallname = Campaign.missions[Campaign.next_mission].main_hall;
	}

	return ade_set_args(L, "i", hallname.c_str());
}

// use getMainHallName if at all possible.
ADE_FUNC(getMainHallIndex, l_Player, NULL, "Gets player's current main hall number", "number", "Main hall index, or index of first mainhall in campaign if something goes wrong")
{
	int hallnum = 0;
	//FS2-->Lua
	if (Campaign.next_mission == -1) {
		hallnum = main_hall_get_index(Campaign.missions[0].main_hall);
	} else {
		hallnum = main_hall_get_index(Campaign.missions[Campaign.next_mission].main_hall);
	}

	return ade_set_args(L, "i", hallnum);
}

ADE_FUNC(getSquadronName, l_Player, NULL, "Gets current player squad name", "string", "Squadron name, or empty string if handle is invalid")
{
	player_h* plr;
	if (!ade_get_args(L, "o", l_Player.GetPtr(&plr)))
		return ade_set_error(L, "s", "");

	if (!plr->isValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", plr->get()->s_squad_name);
}

ADE_FUNC(getMultiSquadronName, l_Player, NULL, "Gets current player multi squad name", "string", "Squadron name, or empty string if handle is invalid")
{
	player_h* plr;
	if (!ade_get_args(L, "o", l_Player.GetPtr(&plr)))
		return ade_set_error(L, "s", "");

	if (!plr->isValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", plr->get()->m_squad_name);
}

ADE_FUNC(loadCampaignSavefile, l_Player, "string campaign = \"<current>\"", "Loads the specified campaign save file.",
         "boolean", "true on success, false otherwise")
{
	player_h* plh;
	const char* savefile = nullptr;
	if (!ade_get_args(L, "o|s", l_Player.GetPtr(&plh), &savefile)) {
		return ADE_RETURN_FALSE;
	}

	if (!plh->isValid()) {
		return ADE_RETURN_FALSE;
	}

	if (savefile == nullptr) {
		savefile = plh->get()->current_campaign;
	}

	pilotfile loader;
	if (!loader.load_savefile(plh->get(), savefile)) {
		return ADE_RETURN_FALSE;
	}

	return ADE_RETURN_TRUE;
}

ADE_FUNC(loadCampaign, l_Player, "string campaign", "Loads the specified campaign file and return to it's mainhall.", "boolean", "true on success, false otherwise")
{
	player_h* plh;
	const char* filename = nullptr;
	if (!ade_get_args(L, "os", l_Player.GetPtr(&plh), &filename)) {
		return ADE_RETURN_FALSE;
	}

	if (!plh->isValid()) {
		return ADE_RETURN_FALSE;
	}

	player* pl = plh->get();

	strcpy_s(pl->current_campaign, filename); // track new campaign for player

	// attempt to load the campaign
	const int load_status = mission_campaign_load(filename, nullptr, pl);

	// see if we successfully loaded this campaign and it's at the beginning
	if (load_status == 0 && Campaign.prev_mission < 0) {
		// Goober5000 - reinitialize tech database if needed
		if ((Campaign.flags & CF_CUSTOM_TECH_DATABASE) || !stricmp(Campaign.filename, "freespace2")) {
			// reset tech database to what's in the tables
			tech_reset_to_default();

			// write the savefile so that we don't later load a stale techroom
			Pilot.save_savefile();
		}

		if (OnCampaignBeginHook->isActive()) {
			OnCampaignBeginHook->run(scripting::hook_param_list(scripting::hook_param("Campaign", 's', Campaign.filename)));
		}
	}

	// that's all we need to do for now; the campaign loading status will be checked again when we try to load the
	// campaign in the ready room

	gameseq_post_event(GS_EVENT_MAIN_MENU);

	return ADE_RETURN_TRUE;
}

scoring_stats_h::scoring_stats_h() = default;
scoring_stats_h::scoring_stats_h(const scoring_struct& stats)
{
	_score.assign(stats);
	_valid = true;
}
bool scoring_stats_h::isValid() const { return _valid; }
scoring_struct* scoring_stats_h::get() { return &_score; }

ADE_OBJ(l_ScoringStats, scoring_stats_h, "scoring_stats", "Player related scoring stats.");

ADE_VIRTVAR(Score, l_ScoringStats, "number", "The current score.", "number", "The score value")
{
	scoring_stats_h* ssh;
	if (!ade_get_args(L, "o", l_ScoringStats.GetPtr(&ssh))) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	return ade_set_args(L, "i", ssh->get()->score);
}

ADE_VIRTVAR(PrimaryShotsFired, l_ScoringStats, "number", "The number of primary shots that have been fired.", "number",
            "The score value")
{
	scoring_stats_h* ssh;
	if (!ade_get_args(L, "o", l_ScoringStats.GetPtr(&ssh))) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	return ade_set_args(L, "i", ssh->get()->p_shots_fired);
}

ADE_VIRTVAR(PrimaryShotsHit, l_ScoringStats, "number", "The number of primary shots that have hit.", "number",
            "The score value")
{
	scoring_stats_h* ssh;
	if (!ade_get_args(L, "o", l_ScoringStats.GetPtr(&ssh))) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	return ade_set_args(L, "i", ssh->get()->p_shots_hit);
}

ADE_VIRTVAR(PrimaryFriendlyHit, l_ScoringStats, "number", "The number of primary friendly fire hits.", "number",
            "The score value")
{
	scoring_stats_h* ssh;
	if (!ade_get_args(L, "o", l_ScoringStats.GetPtr(&ssh))) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	return ade_set_args(L, "i", ssh->get()->p_bonehead_hits);
}

ADE_VIRTVAR(SecondaryShotsFired, l_ScoringStats, "number", "The number of secondary shots that have been fired.",
            "number", "The score value")
{
	scoring_stats_h* ssh;
	if (!ade_get_args(L, "o", l_ScoringStats.GetPtr(&ssh))) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	return ade_set_args(L, "i", ssh->get()->s_shots_fired);
}

ADE_VIRTVAR(SecondaryShotsHit, l_ScoringStats, "number", "The number of secondary shots that have hit.", "number",
            "The score value")
{
	scoring_stats_h* ssh;
	if (!ade_get_args(L, "o", l_ScoringStats.GetPtr(&ssh))) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	return ade_set_args(L, "i", ssh->get()->s_shots_hit);
}

ADE_VIRTVAR(SecondaryFriendlyHit, l_ScoringStats, "number", "The number of secondary friendly fire hits.", "number",
            "The score value")
{
	scoring_stats_h* ssh;
	if (!ade_get_args(L, "o", l_ScoringStats.GetPtr(&ssh))) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	return ade_set_args(L, "i", ssh->get()->s_bonehead_hits);
}

ADE_VIRTVAR(TotalKills, l_ScoringStats, "number", "The total number of kills.", "number", "The score value")
{
	scoring_stats_h* ssh;
	if (!ade_get_args(L, "o", l_ScoringStats.GetPtr(&ssh))) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	return ade_set_args(L, "i", ssh->get()->kill_count_ok);
}

ADE_VIRTVAR(Assists, l_ScoringStats, "number", "The total number of assists.", "number", "The score value")
{
	scoring_stats_h* ssh;
	if (!ade_get_args(L, "o", l_ScoringStats.GetPtr(&ssh))) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	return ade_set_args(L, "i", ssh->get()->assists);
}

ADE_FUNC(getShipclassKills, l_ScoringStats, "shipclass class",
         "Returns the number of kills of a specific ship class recorded in this statistics structure.", "number",
         "The kills for that specific ship class")
{
	using namespace luacpp;

	scoring_stats_h* ssh;
	int ship_idx;
	if (!ade_get_args(L, "oo", l_ScoringStats.GetPtr(&ssh), l_Shipclass.Get(&ship_idx))) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	if (ship_idx < 0 || ship_idx >= (int)Ship_info.size()) {
		return ade_set_error(L, "i", -1);
	}

	return ade_set_args(L, "i", ssh->get()->kills[ship_idx]);
}

//This is per mission stats
ADE_VIRTVAR(MissionPrimaryShotsFired,
	l_ScoringStats,
	"number",
	"The number of primary shots that have been fired in the current mission.",
	"number",
	"The score value")
{
	scoring_stats_h* ssh;
	int setv;
	if (!ade_get_args(L, "o|i", l_ScoringStats.GetPtr(&ssh), &setv)) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		if (setv < 0) {
			setv = 0;
		}
		ssh->get()->mp_shots_fired = setv;
	}

	return ade_set_args(L, "i", ssh->get()->mp_shots_fired);
}

ADE_VIRTVAR(MissionPrimaryShotsHit,
	l_ScoringStats,
	"number",
	"The number of primary shots that have hit in the current mission.",
	"number",
	"The score value")
{
	scoring_stats_h* ssh;
	int setv;
	if (!ade_get_args(L, "o|i", l_ScoringStats.GetPtr(&ssh), &setv)) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		if (setv < 0) {
			setv = 0;
		}
		ssh->get()->mp_shots_hit = setv;
	}

	return ade_set_args(L, "i", ssh->get()->mp_shots_hit);
}

ADE_VIRTVAR(MissionPrimaryFriendlyHit,
	l_ScoringStats,
	"number",
	"The number of primary friendly fire hits in the current mission.",
	"number",
	"The score value")
{
	scoring_stats_h* ssh;
	int setv;
	if (!ade_get_args(L, "o|i", l_ScoringStats.GetPtr(&ssh), &setv)) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		if (setv < 0) {
			setv = 0;
		}
		ssh->get()->mp_bonehead_hits = setv;
	}

	return ade_set_args(L, "i", ssh->get()->mp_bonehead_hits);
}

ADE_VIRTVAR(MissionSecondaryShotsFired,
	l_ScoringStats,
	"number",
	"The number of secondary shots that have been fired in the current mission.",
	"number",
	"The score value")
{
	scoring_stats_h* ssh;
	int setv;
	if (!ade_get_args(L, "o|i", l_ScoringStats.GetPtr(&ssh), &setv)) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		if (setv < 0) {
			setv = 0;
		}
		ssh->get()->ms_shots_fired = setv;
	}

	return ade_set_args(L, "i", ssh->get()->ms_shots_fired);
}

ADE_VIRTVAR(MissionSecondaryShotsHit,
	l_ScoringStats,
	"number",
	"The number of secondary shots that have hit in the current mission.",
	"number",
	"The score value")
{
	scoring_stats_h* ssh;
	int setv;
	if (!ade_get_args(L, "o|i", l_ScoringStats.GetPtr(&ssh), &setv)) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		if (setv < 0) {
			setv = 0;
		}
		ssh->get()->ms_shots_hit = setv;
	}

	return ade_set_args(L, "i", ssh->get()->ms_shots_hit);
}

ADE_VIRTVAR(MissionSecondaryFriendlyHit,
	l_ScoringStats,
	"number",
	"The number of secondary friendly fire hits in the current mission.",
	"number",
	"The score value")
{
	scoring_stats_h* ssh;
	int setv;
	if (!ade_get_args(L, "o|i", l_ScoringStats.GetPtr(&ssh), &setv)) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		if (setv < 0) {
			setv = 0;
		}
		ssh->get()->ms_bonehead_hits = setv;
	}

	return ade_set_args(L, "i", ssh->get()->ms_bonehead_hits);
}

ADE_VIRTVAR(MissionTotalKills, l_ScoringStats, "number", "The total number of kills in the current mission.", "number", "The score value")
{
	scoring_stats_h* ssh;
	int setv;
	if (!ade_get_args(L, "o|i", l_ScoringStats.GetPtr(&ssh), &setv)) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		if (setv < 0) {
			setv = 0;
		}
		ssh->get()->m_kill_count_ok = setv;
	}

	return ade_set_args(L, "i", ssh->get()->m_kill_count_ok);
}

ADE_VIRTVAR(MissionAssists, l_ScoringStats, "number", "The total number of assists in the current mission.", "number", "The score value")
{
	scoring_stats_h* ssh;
	int setv;
	if (!ade_get_args(L, "o|i", l_ScoringStats.GetPtr(&ssh), &setv)) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		if (setv < 0) {
			setv = 0;
		}
		ssh->get()->m_assists = setv;
	}

	return ade_set_args(L, "i", ssh->get()->m_assists);
}

ADE_FUNC(getMissionShipclassKills,
	l_ScoringStats,
	"shipclass class",
	"Returns the number of kills of a specific ship class recorded in this statistics structure for the current mission.",
	"number",
	"The kills for that specific ship class")
{
	using namespace luacpp;

	scoring_stats_h* ssh;
	int ship_idx;
	if (!ade_get_args(L, "oo|i", l_ScoringStats.GetPtr(&ssh), l_Shipclass.Get(&ship_idx))) {
		return ade_set_error(L, "i", -1);
	}

	if (!ssh->isValid()) {
		return ade_set_error(L, "i", -1);
	}

	if (ship_idx < 0 || ship_idx >= (int)Ship_info.size()) {
		return ade_set_error(L, "i", -1);
	}

	return ade_set_args(L, "i", ssh->get()->m_kills[ship_idx]);
}

ADE_FUNC(setMissionShipclassKills,
	l_ScoringStats,
	"shipclass class, number kills",
	"Sets the number of kills of a specific ship class recorded in this statistics structure for the current "
	"mission. Returns true if successful.",
	"boolean",
	"True if successful")
{
	using namespace luacpp;

	scoring_stats_h* ssh;
	int ship_idx;
	int setv;
	if (!ade_get_args(L, "ooi", l_ScoringStats.GetPtr(&ssh), l_Shipclass.Get(&ship_idx), &setv)) {
		return ADE_RETURN_FALSE;
	}

	if (!ssh->isValid()) {
		return ADE_RETURN_FALSE;
	}

	if (ship_idx < 0 || ship_idx >= (int)Ship_info.size()) {
		return ADE_RETURN_FALSE;
	}

	if (setv < 0) {
		setv = 0;
	}
	ssh->get()->m_kills[ship_idx] = setv;

	return ADE_RETURN_TRUE;
}
}
}

