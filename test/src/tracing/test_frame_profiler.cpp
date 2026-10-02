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
	evt.category_id = category.getId();
	evt.type = type;
	evt.timestamp = timestamp;
	return evt;
}

trace_event make_complete_event(const Category& category,
	uint64_t timestamp,
	uint64_t duration,
	uint64_t event_id,
	uint64_t end_event_id) {
	trace_event evt;
	evt.category = &category;
	evt.category_id = category.getId();
	evt.type = EventType::Complete;
	evt.timestamp = timestamp;
	evt.duration = duration;
	evt.event_id = event_id;
	evt.end_event_id = end_event_id;
	evt.pid = 1;
	evt.tid = 1;
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

// accumulate_self_times must not dereference a Category through the event: FrameProfiler buffers
// events across frames (see processFrame()), so a category made through the Lua API
// tracing_category can die between processEvent() and the processFrame() that finally walks this
// event. Reading evt.category->getId() on that dangling pointer, and indexing self_time_by_id with
// whatever garbage id came back, is the out-of-bounds write that used to corrupt the heap (see
// trace_event::category_id).
TEST(FrameProfilerSelfTime, category_can_die_before_self_time_is_accumulated)
{
	SCP_vector<trace_event> events;
	int id;
	{
		Category temporary("Frame profiler self-time test category", false);
		id = temporary.getId();
		events = {
			make_event(temporary, EventType::Begin, 0),
			make_event(Physics, EventType::Begin, 5),
			make_event(Physics, EventType::End, 8),
			make_event(temporary, EventType::End, 10),
		};
	}
	// temporary is dead here; events still carries its address, but accumulate_self_times must
	// use category_id, not that address.

	self_time_result r(events);

	EXPECT_EQ(r.by_id[static_cast<size_t>(id)], 7u); // [0,5) + [8,10)
	EXPECT_EQ(r.self(Physics), 3u);                  // [5,8)
	EXPECT_EQ(r.total, 10u);
}

// Ids are dense and the registry round-trips them to a name, which is what lets the overlay
// snapshot keep ids alone instead of a parallel id -> name array.
TEST(TracingCategory, id_round_trips_through_registry)
{
	ASSERT_GT(Category::getCount(), 0);

	for (int id = 0; id < Category::getCount(); id++) {
		EXPECT_FALSE(Category::getNameById(id).empty());
	}

	EXPECT_EQ(Category::getNameById(Physics.getId()), SCP_string(Physics.getName()));
	EXPECT_EQ(Category::getNameById(PostMove.getId()), SCP_string(PostMove.getName()));
}

// The overlay snapshot must name its contributors through the registry, not through a Category
// pointer. This drives the full path that crashed: processEvent -> processFrame ->
// build_overlay_snapshot -> getNameById.
TEST(FrameProfilerOverlaySnapshot, contributors_are_named_and_sorted)
{
	// A category made at run time, like the one the Lua API tracing_category makes.
	Category runtime_category("Overlay snapshot test category", false);

	FrameProfiler profiler;

	// The outer scope runs for 10 ns and contains the inner scope, which runs for 3 ns.
	trace_event outer = make_complete_event(runtime_category, 0, 10, 1, 4);
	trace_event inner = make_complete_event(Physics, 2, 3, 2, 3);

	profiler.processEvent(&outer);
	profiler.processEvent(&inner);
	profiler.processFrame();

	const frame_overlay_snapshot& snapshot = profiler.getOverlaySnapshot();

	EXPECT_TRUE(snapshot.valid);
	EXPECT_EQ(snapshot.total_nanosec, 10u);
	ASSERT_EQ(snapshot.top_contributors.size(), 2u);

	// The largest self time comes first.
	EXPECT_EQ(snapshot.top_contributors[0].name, SCP_string("Overlay snapshot test category"));
	EXPECT_EQ(snapshot.top_contributors[0].self_nanosec, 7u);
	EXPECT_EQ(snapshot.top_contributors[1].name, SCP_string(Physics.getName()));
	EXPECT_EQ(snapshot.top_contributors[1].self_nanosec, 3u);
	EXPECT_EQ(snapshot.other_nanosec, 0u);
}

// Not every Category is a global static: the Lua API tracing_category constructs one and copies it
// into a script-owned object, so the constructed object dies while its id stays in the trace
// events. The registry must keep the name alive on its own.
TEST(TracingCategory, name_outlives_the_category_object)
{
	int id = -1;
	{
		Category temporary("Temporary test category", false);
		id = temporary.getId();

		// A copy keeps the id of the original, which is how the Lua object gets a usable id.
		const Category& copy = temporary;
		EXPECT_EQ(copy.getId(), id);
	}

	EXPECT_EQ(Category::getNameById(id), SCP_string("Temporary test category"));
}
