#include "network/multi.h"
#include "network/multiutil.h"
#include "physics/physics.h"
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

class NewPlayerIdTest : public ::testing::Test {
  protected:
	void SetUp() override
	{
		_saved_id_num = Multi_id_num;
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			_saved_flags[i] = Net_players[i].flags;
			_saved_ids[i] = Net_players[i].player_id;
			Net_players[i].flags = 0;
		}
	}

	void TearDown() override
	{
		Multi_id_num = _saved_id_num;
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			Net_players[i].flags = _saved_flags[i];
			Net_players[i].player_id = _saved_ids[i];
		}
	}

	static void add(int slot, int flags, short player_id)
	{
		Net_players[slot].flags = flags;
		Net_players[slot].player_id = player_id;
	}

	short _saved_id_num = 0;
	std::array<int, MAX_PLAYERS> _saved_flags{};
	std::array<short, MAX_PLAYERS> _saved_ids{};
};

// the id counter starts over after 20000 while the server and long-connected players still hold low ids
TEST_F(NewPlayerIdTest, skips_ids_of_connected_players_after_wrap)
{
	Multi_id_num = 20000;
	add(0, NETINFO_FLAG_CONNECTED, 0);
	add(3, NETINFO_FLAG_CONNECTED, 1);
	add(4, 0, 2);

	EXPECT_EQ(multi_get_new_id(), 20000);
	EXPECT_EQ(multi_get_new_id(), 2);
}

// ships faster than 128 m/s (common in mods) must reach clients at their real speed
TEST(VelocityPacking, fast_ship_round_trips)
{
	matrix orient = vmd_identity_matrix;
	physics_info sent, received;
	sent.vel = {{{-180.0f, 150.0f, 230.0f}}};

	ubyte data[16] = {};
	int written = multi_pack_unpack_vel(1, data, &orient, &sent);
	int read = multi_pack_unpack_vel(0, data, &orient, &received);
	EXPECT_EQ(written, read);

	EXPECT_NEAR(received.vel.xyz.x, -180.0f, 0.1f);
	EXPECT_NEAR(received.vel.xyz.y, 150.0f, 0.1f);
	EXPECT_NEAR(received.vel.xyz.z, 230.0f, 0.1f);
}

// faster than the fields hold: clamped at the edge, not wrapped around to the other sign
TEST(VelocityPacking, too_fast_clamps)
{
	matrix orient = vmd_identity_matrix;
	physics_info sent, received;

	for (float speed : {300.0f, -300.0f}) {
		sent.vel = {{{speed, speed, speed}}};

		ubyte data[16] = {};
		multi_pack_unpack_vel(1, data, &orient, &sent);
		multi_pack_unpack_vel(0, data, &orient, &received);

		EXPECT_NEAR(received.vel.xyz.x, (speed > 0) ? 255.94f : -256.0f, 0.01f);
		EXPECT_NEAR(received.vel.xyz.y, (speed > 0) ? 255.94f : -256.0f, 0.01f);
		EXPECT_NEAR(received.vel.xyz.z, (speed > 0) ? 255.97f : -256.0f, 0.01f);
	}
}
