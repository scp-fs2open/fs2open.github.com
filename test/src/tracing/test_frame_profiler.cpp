//
//

#include <gtest/gtest.h>

#include "tracing/FrameProfiler.h"
#include "tracing/categories.h"

using namespace tracing;

namespace {

trace_event make_event(const Category& category, EventType type, uint64_t timestamp) {
	trace_event evt;
	evt.category = &category;
	evt.type = type;
	evt.timestamp = timestamp;
	return evt;
}

// Small helper to run the function under test and look results up by category.
struct self_time_result {
	SCP_vector<uint64_t> by_id;
	uint64_t total = 0;

	explicit self_time_result(const SCP_vector<trace_event>& events) {
		total = accumulate_self_times(events, by_id);
	}

	uint64_t self(const Category& c) const { return by_id[static_cast<size_t>(c.getId())]; }
};

} // namespace

// Parent fully contains a child: the child's interval is exclusive to the child, the rest is the
// parent's self-time.
TEST(FrameProfilerSelfTime, nested_scope)
{
	SCP_vector<trace_event> events{
		make_event(Physics, EventType::Begin, 0),
		make_event(PostMove, EventType::Begin, 5),
		make_event(PostMove, EventType::End, 8),
		make_event(Physics, EventType::End, 10),
	};

	self_time_result r(events);

	EXPECT_EQ(r.self(Physics), 7u);  // [0,5) + [8,10)
	EXPECT_EQ(r.self(PostMove), 3u); // [5,8)
	EXPECT_EQ(r.total, 10u);
}

// Two top-level scopes with an idle gap between them: the gap belongs to no scope and must be
// excluded from both the per-category totals and the frame total.
TEST(FrameProfilerSelfTime, gap_between_top_level_scopes)
{
	SCP_vector<trace_event> events{
		make_event(Physics, EventType::Begin, 0),
		make_event(Physics, EventType::End, 3),
		// gap [3, 10)
		make_event(PostMove, EventType::Begin, 10),
		make_event(PostMove, EventType::End, 15),
	};

	self_time_result r(events);

	EXPECT_EQ(r.self(Physics), 3u);
	EXPECT_EQ(r.self(PostMove), 5u);
	EXPECT_EQ(r.total, 8u); // gap of 7 excluded
}

// The same category nested within itself (recursion). All time is still attributed to that one
// category; the old tree-based path would have tripped its "max 1 open" assert here.
TEST(FrameProfilerSelfTime, recursion_same_category)
{
	SCP_vector<trace_event> events{
		make_event(Physics, EventType::Begin, 0),
		make_event(Physics, EventType::Begin, 3),
		make_event(Physics, EventType::End, 7),
		make_event(Physics, EventType::End, 10),
	};

	self_time_result r(events);

	EXPECT_EQ(r.self(Physics), 10u);
	EXPECT_EQ(r.total, 10u);
}

// A parent with two sibling children. Verifies the parent's self-time correctly excludes both
// children and that the frame total equals the sum of every category's self-time.
TEST(FrameProfilerSelfTime, siblings_sum_to_total)
{
	SCP_vector<trace_event> events{
		make_event(MoveObjects, EventType::Begin, 0),
		make_event(Physics, EventType::Begin, 2),
		make_event(Physics, EventType::End, 7),
		make_event(PostMove, EventType::Begin, 10),
		make_event(PostMove, EventType::End, 14),
		make_event(MoveObjects, EventType::End, 20),
	};

	self_time_result r(events);

	EXPECT_EQ(r.self(Physics), 5u);
	EXPECT_EQ(r.self(PostMove), 4u);
	EXPECT_EQ(r.self(MoveObjects), 11u); // [0,2) + [7,10) + [14,20)

	uint64_t sum = 0;
	for (uint64_t v : r.by_id) {
		sum += v;
	}
	EXPECT_EQ(sum, r.total);
	EXPECT_EQ(r.total, 20u);
}

// Empty input must be handled gracefully.
TEST(FrameProfilerSelfTime, empty_input)
{
	SCP_vector<trace_event> events;
	self_time_result r(events);
	EXPECT_EQ(r.total, 0u);
}

// The output buffer is meant to be reused across frames, so a run must not see the previous run's
// totals. (processFrame() keeps one buffer alive for the life of the profiler.)
TEST(FrameProfilerSelfTime, reused_buffer_is_cleared)
{
	SCP_vector<uint64_t> by_id;

	SCP_vector<trace_event> first{
		make_event(Physics, EventType::Begin, 0),
		make_event(Physics, EventType::End, 10),
	};
	EXPECT_EQ(accumulate_self_times(first, by_id), 10u);
	EXPECT_EQ(by_id[static_cast<size_t>(Physics.getId())], 10u);

	SCP_vector<trace_event> second{
		make_event(Physics, EventType::Begin, 0),
		make_event(Physics, EventType::End, 3),
	};
	EXPECT_EQ(accumulate_self_times(second, by_id), 3u);
	EXPECT_EQ(by_id[static_cast<size_t>(Physics.getId())], 3u);

	// An empty frame must zero it rather than leave the previous frame's numbers behind.
	SCP_vector<trace_event> empty;
	EXPECT_EQ(accumulate_self_times(empty, by_id), 0u);
	EXPECT_EQ(by_id[static_cast<size_t>(Physics.getId())], 0u);
}

// Ids are dense and the registry round-trips them, which is what lets the overlay snapshot drop
// the parallel id -> Category* array.
TEST(TracingCategory, id_round_trips_through_registry)
{
	ASSERT_GT(Category::getCount(), 0);

	for (int id = 0; id < Category::getCount(); id++) {
		EXPECT_EQ(Category::getById(id).getId(), id);
	}

	EXPECT_EQ(&Category::getById(Physics.getId()), &Physics);
	EXPECT_EQ(&Category::getById(PostMove.getId()), &PostMove);
}
