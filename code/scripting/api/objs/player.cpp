//
//

#include "player.h"

#include "menuui/mainhallmenu.h"
#include "mission/missioncampaign.h"
#include "pilotfile/pilotfile.h"
#include "playerman/player.h"

namespace scripting {
namespace api {

player_h::player_h() = default;
player_h::player_h(const player& plr)
{
	_plr = new player();
	_plr->assign(&plr);
}
bool player_h::isValid() const { return _plr != nullptr; }
player* player_h::get() { return _plr; }
void player_h::cleanup()
{
	delete _plr;
	_plr = nullptr;
}

//**********HANDLE: Player
ADE_OBJ(l_Player, player_h, "player", "Player handle");

ADE_FUNC(__gc, l_Player, nullptr, "Deletes the underlying resources", "nothing", "nothing") {
	player_h* plr;
	if (!ade_get_args(L, "o", l_Player.GetPtr(&plr)))
		return ADE_RETURN_NIL;

	if (!plr->isValid())
		return ADE_RETURN_NIL;

	plr->cleanup();
	return ADE_RETURN_NIL;
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

ADE_FUNC(loadCampaignSavefile, l_Player, "string campaign = <current>", "Loads the specified campaign save file.",
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
}
}

