#include "network/multi.h"
#include "network/multiutil.h"
#include "playerman/player.h"

#include <gtest/gtest.h>

#include <array>

class FindPlayerByCallsignTest : public ::testing::Test {
  protected:
	void SetUp() override
	{
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			_saved_flags[i] = Net_players[i].flags;
			_saved_players[i] = Net_players[i].m_player;
			Net_players[i].flags = 0;
		}
	}

	void TearDown() override
	{
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			Net_players[i].flags = _saved_flags[i];
			Net_players[i].m_player = _saved_players[i];
		}
	}

	static void add(int slot, int flags, player* pl, const char* callsign)
	{
		strcpy_s(pl->callsign, callsign);
		Net_players[slot].flags = flags;
		Net_players[slot].m_player = pl;
	}

	std::array<int, MAX_PLAYERS> _saved_flags{};
	std::array<player*, MAX_PLAYERS> _saved_players{};
};

TEST_F(FindPlayerByCallsignTest, skips_standalone_own_entry)
{
	player standalone_pl;
	player pilot_pl;
	add(1, NETINFO_FLAG_AM_MASTER | NETINFO_FLAG_CONNECTED, &standalone_pl, "server");
	add(2, NETINFO_FLAG_CONNECTED, &pilot_pl, "server");

	auto found = multi_find_player_by_callsign("server");
	EXPECT_EQ(found.first, 2);
	EXPECT_EQ(found.second, -1);
}

TEST_F(FindPlayerByCallsignTest, finds_listen_server_host)
{
	player host_pl;
	add(1, NETINFO_FLAG_AM_MASTER | NETINFO_FLAG_GAME_HOST | NETINFO_FLAG_CONNECTED, &host_pl, "Hostpilot");

	auto found = multi_find_player_by_callsign("Hostpilot");
	EXPECT_EQ(found.first, 1);
	EXPECT_EQ(found.second, -1);
}
