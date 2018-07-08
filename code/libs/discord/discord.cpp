#include "discord.h"
#include "freespace.h"
#include "events/events.h"
#include "gamesequence/gamesequence.h"
#include "io/timer.h"
#include "mission/missioncampaign.h"
#include "mission/missionparse.h"
#include "parse/parselo.h"
#include "playerman/player.h"

#include "discord_rpc.h"

namespace {
struct PresenceInfo {
	SCP_string state;
	int64_t timestamp;
};

bool initialized        = false;
bool discord_ready      = false;
int next_mission_update = -1;

PresenceInfo current_info;

const char* APPLICATION_ID = "465270111440470016";

SCP_unordered_map<SCP_string, SCP_string> campaign_name_cache;

SCP_string get_campaign_name(const char* filename)
{
	SCP_string name = filename;

	auto iter = campaign_name_cache.find(name);
	if (iter != campaign_name_cache.end()) {
		return iter->second;
	}

	auto campaign_name        = mission_campaign_get_name(filename);
	campaign_name_cache[name] = campaign_name;

	return campaign_name;
}

SCP_string get_current_campaign_name()
{
	if (Game_mode & GM_CAMPAIGN_MODE) {
		return Campaign.name;
	} else {
		return get_campaign_name(Player->current_campaign);
	}
}

SCP_string get_details()
{
	SCP_string res;

	auto has_mission  = strlen(Game_current_mission_filename) > 0;
	auto has_campaign = strlen(Player->current_campaign) > 0;
	if (has_mission && !(Game_mode & GM_CAMPAIGN_MODE)) {
		// This is a standalone mission so we don't actually have a campaign
		has_campaign = false;
	}

	if (has_campaign && has_mission) {
		sprintf(res, "%s: %s", get_current_campaign_name().c_str(), The_mission.name);
	} else if (has_campaign) {
		sprintf(res, "Campaign %s", get_current_campaign_name().c_str());
	} else if (has_mission) {
		res = The_mission.name;
	} else {
		res = "In game";
	}

	return res;
}

void update_presence() {
	auto details = get_details();

	DiscordRichPresence presence;
	memset(&presence, 0, sizeof(presence));
	presence.details        = details.c_str();
	presence.state          = current_info.state.c_str();
	presence.startTimestamp = current_info.timestamp;

	Discord_UpdatePresence(&presence);
}

void set_presence(const SCP_string& state, int64_t timestamp = 0) {
	if (current_info.state == state && current_info.timestamp == timestamp) {
		// No changes
		return;
	}

	current_info.state = state;
	current_info.timestamp = timestamp;

	if (discord_ready) {
		update_presence();
	}
}

void set_game_play_presence()
{
	SCP_string state;

	sprintf(state, "In mission (%d %s)", Player->stats.m_kill_count_ok,
	        Player->stats.m_kill_count_ok == 1 ? "kill" : "kills");

	set_presence(state, (int64_t)time(nullptr) - f2i(Missiontime));

	// Update this every 20 seconds since Discord already has a 15 second rate-limit
	// This will update the "elapsed" time even if time compression is active but the clock will jump forward
	next_mission_update = timestamp(20000);
}

void update_discord()
{
	if (!initialized)
		return;

	Discord_RunCallbacks();

	if (gameseq_get_state() == GS_STATE_GAME_PLAY && timestamp_elapsed(next_mission_update)) {
		set_game_play_presence();
	}
}
void shutdown_discord()
{
	if (!initialized)
		return;

	Discord_Shutdown();
	initialized = false;
	discord_ready = false;
}

void handleEnterState(int /*old_state*/, int new_state)
{
	if (new_state == GS_STATE_GAME_PLAY) {
		// Update immediately if we enter the game play state again
		set_game_play_presence();
		return;
	}

	if (gameseq_get_state_idx(GS_STATE_GAME_PLAY) >= 0) {
		// Special case for when we come out of game play
		switch (new_state) {
		case GS_STATE_OPTIONS_MENU:
		case GS_STATE_HOTKEY_SCREEN:
		case GS_STATE_GAMEPLAY_HELP:
		case GS_STATE_MISSION_LOG_SCROLLBACK:
			set_presence("In game menu");
			return;
		default:
			break;
		}
	}

	switch (new_state) {
	case GS_STATE_MAIN_MENU:
		set_presence("In Mainhall");
		break;
	case GS_STATE_BRIEFING:
		set_presence("In mission briefing");
		break;
	case GS_STATE_CMD_BRIEF:
		set_presence("In command briefing");
		break;
	case GS_EVENT_DEBRIEF:
		set_presence("In debriefing");
		break;
	case GS_STATE_GAME_PAUSED:
		set_presence("Paused");
		break;
	case GS_STATE_RED_ALERT:
		set_presence("Red alert briefing");
		break;
	case GS_STATE_INITIAL_PLAYER_SELECT:
		set_presence("Selecting player");
		break;
	case GS_STATE_FICTION_VIEWER:
		set_presence("In fiction viewer");
		break;
	default:
		break;
	}
}

void handleLeaveState(int old_state, int /*new_state*/)
{
	if (old_state == GS_STATE_GAME_PAUSED) {
		// Reset the game play state immediately
		set_game_play_presence();
	}
}

void handleMissionLoad(const char* /*filename*/) { set_presence("Loading mission"); }

} // namespace

namespace libs {
namespace discord {

void init()
{
	Assertion(!initialized, "Discord integration can only be initialized once!");

	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = [](const DiscordUser* connectedUser) {
		mprintf(("Discord: connected to user %s#%s - %s\n", connectedUser->username, connectedUser->discriminator,
		         connectedUser->userId));
		discord_ready = true;
		update_presence();
	};
	handlers.errored = [](int errcode, const char* message) {
		mprintf(("Discord: error (%d: %s)\n", errcode, message));
		discord_ready = false;
	};
	handlers.disconnected = [](int errcode, const char* message) {
		mprintf(("Discord: disconnected (%d: %s)\n", errcode, message));
		discord_ready = false;
	};

	Discord_Initialize(APPLICATION_ID, &handlers, 0, nullptr);

	events::EngineUpdate.add(update_discord);
	events::EngineShutdown.add(shutdown_discord);
	events::GameEnterState.add(handleEnterState);
	events::GameLeaveState.add(handleLeaveState);
	events::GameMissionLoad.add(handleMissionLoad);

	initialized = true;
}

} // namespace discord
} // namespace libs
