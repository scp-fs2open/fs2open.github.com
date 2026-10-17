
#include <gtest/gtest.h>

#include "network/multi_lua.h"

TEST(multi_lua, in_order_packet_is_not_stale)
{
	EXPECT_FALSE(multi_lua_packet_is_stale(100, 200, 100));
}

TEST(multi_lua, same_timestamp_is_not_stale)
{
	EXPECT_FALSE(multi_lua_packet_is_stale(100, 100, 0));
}

TEST(multi_lua, reordered_packet_is_stale)
{
	EXPECT_TRUE(multi_lua_packet_is_stale(200, 100, 5));
}

TEST(multi_lua, wrap_forward_is_not_stale)
{
	EXPECT_FALSE(multi_lua_packet_is_stale(65000, 10, 546));
}

TEST(multi_lua, wrap_backward_is_stale)
{
	EXPECT_TRUE(multi_lua_packet_is_stale(10, 65000, 5));
}

TEST(multi_lua, delayed_previous_packet_does_not_toss_newer_packet)
{
	// sent 1500ms apart, but the previous packet arrived 200ms late
	EXPECT_FALSE(multi_lua_packet_is_stale(1000, 2500, 1300));
}

TEST(multi_lua, long_local_gap_is_not_stale)
{
	// a remote gap of 40s looks negative as a 16-bit difference, but the local gap shows the packet is new
	EXPECT_FALSE(multi_lua_packet_is_stale(0, 40000, 39000));
}

TEST(multi_lua, short_local_gap_with_negative_remote_gap_is_stale)
{
	EXPECT_TRUE(multi_lua_packet_is_stale(0, 40000, 100));
}

TEST(multi_lua, remote_gap_boundary)
{
	EXPECT_FALSE(multi_lua_packet_is_stale(0, 0x7FFF, 0));
	EXPECT_TRUE(multi_lua_packet_is_stale(0, 0x8000, 0));
}

TEST(multi_lua, local_window_boundary)
{
	EXPECT_TRUE(multi_lua_packet_is_stale(200, 100, 16383));
	EXPECT_FALSE(multi_lua_packet_is_stale(200, 100, 16384));
}
