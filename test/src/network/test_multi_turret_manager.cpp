#include "io/timer.h"
#include "network/multi_time_manager.h"
#include "network/multi_turret_manager.h"
#include "object/object.h"
#include "util/FSTestFixture.h"
#include "utils/finally.h"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

namespace {
constexpr std::int64_t MS_2_31 = 2'147'483'648LL;
constexpr uint64_t JUMP_US = 25ULL * 86'400ULL * MICROSECONDS_PER_SECOND;
constexpr uint64_t STEP_US = 30'000ULL;
constexpr int PACKET_AHEAD_MS = 50;
constexpr ushort PARENT_SIG = 1234;
constexpr int PARENT_OBJNUM = 0;
constexpr short SUBSYS_INDEX = 0;
} // namespace

class MultiTurretManagerTest : public test::FSTestFixture {
  public:
	MultiTurretManagerTest() : test::FSTestFixture(INIT_NONE) {}

  protected:
	void SetUp() override
	{
		FSTestFixture::SetUp();
		m_saved_timing = Multi_Timing_Info;
		m_saved_sig = Objects[PARENT_OBJNUM].net_signature;
		Objects[PARENT_OBJNUM].net_signature = PARENT_SIG;
	}

	void TearDown() override
	{
		if (timestamp_is_paused()) {
			resume_game();
		}
		for (int i = 0; i < m_steps; ++i) {
			timestamp_adjust_microseconds(STEP_US, TIMER_DIRECTION::BACKWARD);
		}
		// the unpause added the paused time to the counter offset, which timer_init keeps; give it back
		timestamp_adjust_microseconds(m_paused_us, TIMER_DIRECTION::FORWARD);
		timer_start_frame();
		Multi_Turret_Manager.reset();
		Objects[PARENT_OBJNUM].net_signature = m_saved_sig;
		Multi_Timing_Info = m_saved_timing;
		FSTestFixture::TearDown();
	}

	static void start_mission()
	{
		Multi_Timing_Info.set_mission_start_time();
		Multi_Timing_Info.update_current_time();
	}

	void step_clock()
	{
		timestamp_adjust_microseconds(STEP_US, TIMER_DIRECTION::FORWARD);
		++m_steps;
		timer_start_frame();
		Multi_Timing_Info.update_current_time();
	}

	// game_stop_time() pauses this way when a multiplayer game enters GS_STATE_MULTI_PAUSED
	void pause_game()
	{
		m_pause_started_us = timer_get_microseconds();
		timestamp_pause(true);
	}

	void resume_game()
	{
		timestamp_unpause(true);
		m_paused_us += timer_get_microseconds() - m_pause_started_us;
	}

	static void add_packet(int sent_ms, float angle)
	{
		Multi_Turret_Manager.add_incoming_packet(sent_ms, PARENT_OBJNUM, SUBSYS_INDEX, 0, {true, angle}, {false, 0.0f});
	}

	static bool packet_applied(turret_packet* packet)
	{
		return Multi_Turret_Manager.get_latest_packet(PARENT_SIG, SUBSYS_INDEX, packet);
	}

  private:
	multiplayer_timing_info m_saved_timing;
	ushort m_saved_sig = 0;
	int m_steps = 0;
	uint64_t m_pause_started_us = 0;
	uint64_t m_paused_us = 0;
};

TEST_F(MultiTurretManagerTest, packetAppliesOnceMissionTimeReachesSendTime)
{
	timer_start_frame();
	start_mission();
	add_packet(PACKET_AHEAD_MS, 1.0f);

	turret_packet packet;
	step_clock();
	EXPECT_FALSE(packet_applied(&packet));

	step_clock();
	ASSERT_TRUE(packet_applied(&packet));
	EXPECT_EQ(packet.time.value(), Multi_Timing_Info.get_mission_start_time() + PACKET_AHEAD_MS);
}

TEST_F(MultiTurretManagerTest, packetsApplyPastThirtyTwoBitGameClock)
{
	timestamp_adjust_microseconds(JUMP_US, TIMER_DIRECTION::FORWARD);
	auto restore_clock = util::finally([] {
		timestamp_adjust_microseconds(JUMP_US, TIMER_DIRECTION::BACKWARD);
		timer_start_frame();
	});
	timer_start_frame();
	ASSERT_GT(_timestamp().value(), MS_2_31);

	start_mission();
	add_packet(PACKET_AHEAD_MS, 1.0f);

	turret_packet packet;
	EXPECT_FALSE(packet_applied(&packet));
	step_clock();
	EXPECT_FALSE(packet_applied(&packet));

	step_clock();
	ASSERT_TRUE(packet_applied(&packet));
	EXPECT_EQ(packet.time.value(), Multi_Timing_Info.get_mission_start_time() + PACKET_AHEAD_MS);
	EXPECT_TRUE(packet.angle1.first);
	EXPECT_FLOAT_EQ(packet.angle1.second, 1.0f);
}

TEST_F(MultiTurretManagerTest, inGameJoinerUsesItsSkippedMissionTime)
{
	constexpr int JOIN_MS = 10'000;
	timer_start_frame();
	Multi_Timing_Info.set_mission_start_time();
	Multi_Timing_Info.in_game_set_skip_time(static_cast<float>(JOIN_MS) / MILLISECONDS_PER_SECOND);
	Multi_Timing_Info.update_current_time();
	ASSERT_EQ(Multi_Timing_Info.get_current_time(), JOIN_MS);

	add_packet(JOIN_MS - 1000, 1.0f);
	add_packet(JOIN_MS + PACKET_AHEAD_MS, 2.0f);

	turret_packet packet;
	ASSERT_TRUE(packet_applied(&packet));
	EXPECT_FLOAT_EQ(packet.angle1.second, 1.0f);

	step_clock();
	EXPECT_FALSE(packet_applied(&packet));

	step_clock();
	ASSERT_TRUE(packet_applied(&packet));
	EXPECT_FLOAT_EQ(packet.angle1.second, 2.0f);
}

TEST_F(MultiTurretManagerTest, packetsWaitWhileTheGameIsPaused)
{
	timer_start_frame();
	start_mission();
	add_packet(PACKET_AHEAD_MS, 1.0f);

	turret_packet packet;
	step_clock();
	EXPECT_FALSE(packet_applied(&packet));

	pause_game();
	std::this_thread::sleep_for(std::chrono::milliseconds(2 * PACKET_AHEAD_MS));
	timer_start_frame();
	Multi_Timing_Info.update_current_time();
	EXPECT_FALSE(packet_applied(&packet));

	resume_game();
	timer_start_frame();
	Multi_Timing_Info.update_current_time();
	EXPECT_FALSE(packet_applied(&packet));

	step_clock();
	EXPECT_TRUE(packet_applied(&packet));
}
