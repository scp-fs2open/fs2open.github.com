#include "io/timer.h"
#include "network/multi_time_manager.h"
#include "network/multi_turret_manager.h"
#include "object/object.h"
#include "util/FSTestFixture.h"
#include "utils/finally.h"

#include <gtest/gtest.h>

class MultiTurretManagerTest : public test::FSTestFixture {
  public:
	MultiTurretManagerTest() : test::FSTestFixture(INIT_NONE) {}
};

namespace {
constexpr std::int64_t MS_2_31 = 2'147'483'648LL;
constexpr uint64_t JUMP_US = 25ULL * 86'400ULL * MICROSECONDS_PER_SECOND;
constexpr uint64_t STEP_US = 100'000ULL;
constexpr ushort PARENT_SIG = 1234;
constexpr int PARENT_OBJNUM = 0;
constexpr short SUBSYS_INDEX = 0;
} // namespace

TEST_F(MultiTurretManagerTest, packetsApplyPastThirtyTwoBitGameClock)
{
	const ushort old_sig = Objects[PARENT_OBJNUM].net_signature;
	timestamp_adjust_microseconds(JUMP_US, TIMER_DIRECTION::FORWARD);
	int steps = 0;
	auto restore = util::finally([&steps, old_sig] {
		Multi_Turret_Manager.reset();
		Objects[PARENT_OBJNUM].net_signature = old_sig;
		for (int i = 0; i < steps; ++i) {
			timestamp_adjust_microseconds(STEP_US, TIMER_DIRECTION::BACKWARD);
		}
		timestamp_adjust_microseconds(JUMP_US, TIMER_DIRECTION::BACKWARD);
		timer_start_frame();
	});
	timer_start_frame();
	ASSERT_GT(_timestamp().value(), MS_2_31);

	Objects[PARENT_OBJNUM].net_signature = PARENT_SIG;
	Multi_Timing_Info.set_mission_start_time();
	Multi_Timing_Info.update_current_time();
	Multi_Turret_Manager.add_incoming_packet(50, PARENT_OBJNUM, SUBSYS_INDEX, 0, {true, 1.0f}, {false, 0.0f});

	turret_packet packet;
	EXPECT_FALSE(Multi_Turret_Manager.get_latest_packet(PARENT_SIG, SUBSYS_INDEX, &packet));

	timestamp_adjust_microseconds(STEP_US, TIMER_DIRECTION::FORWARD);
	++steps;
	timer_start_frame();
	Multi_Timing_Info.update_current_time();

	ASSERT_TRUE(Multi_Turret_Manager.get_latest_packet(PARENT_SIG, SUBSYS_INDEX, &packet));
	EXPECT_EQ(packet.time.value(), Multi_Timing_Info.get_mission_start_time() + 50);
	EXPECT_TRUE(packet.angle1.first);
	EXPECT_FLOAT_EQ(packet.angle1.second, 1.0f);
}
