#include "io/timer.h"
#include "network/multi_time_manager.h"
#include "network/multi_turret_manager.h"
#include "object/object.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

namespace {
constexpr std::int64_t MS_2_31 = 2'147'483'648LL;
constexpr std::int64_t CLOCK_START_MS = 1000;
constexpr uint64_t JUMP_US = 25ULL * 86'400ULL * MICROSECONDS_PER_SECOND;
constexpr uint64_t STEP_US = 300'000ULL;
constexpr int PACKET_AHEAD_MS = 500;
constexpr int PAUSE_SLEEP_MS = 400;
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
		// a mission started in the clock's first two milliseconds gets a never or immediate start time
		timestamp_adjust_microseconds(static_cast<uint64_t>(CLOCK_START_MS) * MICROSECONDS_PER_MILLISECOND,
			TIMER_DIRECTION::FORWARD);
		timer_start_frame();
		// the step is rounded down to whole counter ticks
		ASSERT_GE(_timestamp().value(), CLOCK_START_MS - 1);
	}

	void TearDown() override
	{
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

	static void step_clock()
	{
		timestamp_adjust_microseconds(STEP_US, TIMER_DIRECTION::FORWARD);
		timer_start_frame();
		Multi_Timing_Info.update_current_time();
	}

	// game_stop_time() pauses this way when a multiplayer game enters GS_STATE_MULTI_PAUSED
	static void pause_game()
	{
		timestamp_pause(true);
	}

	static void resume_game()
	{
		timestamp_unpause(true);
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
	std::this_thread::sleep_for(std::chrono::milliseconds(PAUSE_SLEEP_MS));
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
