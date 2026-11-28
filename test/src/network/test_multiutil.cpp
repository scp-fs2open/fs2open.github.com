#include "network/multi.h"
#include "network/multimsgs.h"
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

class LeavePacketTest : public ::testing::Test {
  protected:
	void SetUp() override
	{
		_saved_net_player = Net_player;
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			_saved_flags[i] = Net_players[i].flags;
			_saved_ids[i] = Net_players[i].player_id;
			Net_players[i].flags = 0;
			Net_players[i].player_id = static_cast<short>(i);
		}
		Net_players[0].flags = NETINFO_FLAG_AM_MASTER | NETINFO_FLAG_CONNECTED;
		Net_players[1].flags = NETINFO_FLAG_CONNECTED;
		Net_players[2].flags = NETINFO_FLAG_CONNECTED;
		Net_player = &Net_players[0];
	}

	void TearDown() override
	{
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			Net_players[i].flags = _saved_flags[i];
			Net_players[i].player_id = _saved_ids[i];
		}
		Net_player = _saved_net_player;
	}

	// returns the bytes the handler consumed, so the caller can check it still skips the whole packet
	static int receive_leave(short deader_id, short sender_id, bool reliable)
	{
		ubyte data[4] = {LEAVE_GAME, 0xff, 0, 0};
		short swap = INTEL_SHORT(deader_id);
		memcpy(data + 2, &swap, sizeof(swap));
		header hinfo{};
		hinfo.id = sender_id;
		process_leave_game_packet(data, &hinfo, reliable);
		return hinfo.bytes_processed;
	}

	net_player* _saved_net_player = nullptr;
	std::array<int, MAX_PLAYERS> _saved_flags{};
	std::array<short, MAX_PLAYERS> _saved_ids{};
};

// a client may only say that it is leaving; the server must not drop someone else on its word
TEST_F(LeavePacketTest, master_ignores_leave_for_another_player)
{
	EXPECT_EQ(receive_leave(1, 2, true), 4);
	EXPECT_TRUE(MULTI_CONNECTED(Net_players[1]));
}

// leave packets are only ever sent reliably, so an unreliable one is ignored even when the ids match
TEST_F(LeavePacketTest, unreliable_leave_is_ignored)
{
	EXPECT_EQ(receive_leave(1, 1, false), 4);
	EXPECT_TRUE(MULTI_CONNECTED(Net_players[1]));
}
