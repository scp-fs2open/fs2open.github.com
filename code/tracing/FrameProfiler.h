#pragma once


#include "globalincs/pstypes.h"

#include "tracing.h"

#include <mutex>

/** @file
 *  @ingroup tracing
 */

namespace tracing {

/**
 * Computes per-category exclusive (self) time for a frame's trace events in a single pass.
 *
 * @c events must already be sorted into transition order (by @c event_id, i.e. call order), as a
 * stream of Begin/End events. A stack of currently-open scopes is maintained; the time between two
 * consecutive transitions is attributed to the innermost open scope.
 *
 * @c self_time_by_id is resized and zeroed to @c Category::getCount() entries and filled with each
 * category's self time, indexed by @c Category::getId() (recover the category with
 * @c Category::getById()). Pass the same vector every frame to reuse its allocation.
 *
 * @return the sum of all self-times, i.e. the total traced frame time.
 *
 * This is O(events) with no tree building or string comparisons. Declared here (rather than kept
 * file-local) so it can be unit-tested directly.
 */
uint64_t accumulate_self_times(const SCP_vector<trace_event>& events, SCP_vector<uint64_t>& self_time_by_id);

struct profile_sample_history {
	bool valid;
	//char name[256];
	SCP_string name;
	uint64_t avg_micro_sec;
	uint64_t min_micro_sec;
	uint64_t max_micro_sec;
};

struct profile_sample {
	uint profile_instances;
	int open_profiles;
	SCP_string name;
	uint64_t start_time;    // in microseconds
	uint64_t accumulator;
	uint64_t children_sample_time;
	uint num_parents;
	uint num_children;
	int parent;
};

class FrameProfiler {
	std::mutex _eventsMutex;
	SCP_vector<trace_event> _bufferedEvents;

	SCP_vector<profile_sample_history> history;

	frame_overlay_snapshot overlaySnapshot;

	// Reused across frames by accumulate_self_times() so the per-frame walk allocates nothing.
	SCP_vector<uint64_t> _selfTimeScratch;

	std::int64_t _mainThreadID = -1;

	SCP_string content;

	/**
	 * Stores profile data in in the profile history lookup. This is used internally by the profiling code and should
	 * not be called outside of it.
	 * @param name The globally unique name for this profile (see profile_begin()/profile_end())
	 * @param percent How much time the profiled section took to execute (as a percentage of overall frametime)
	 */
	void store_profile_in_history(SCP_string& name, uint64_t time);

	/**
	 * Gets the min, max and average values for a given profile
	 * @param name The globally unique name for this profile (see profile_begin()/profile_end())
	 * @param avg Pointer to a float in which the average value will be stored (or 0.0 if no value has been saved)
	 * @param min Pointer to a float in which the minimum value will be stored (or 0.0 if no value has been saved)
	 * @param max Pointer to a float in which the maximum value will be stored (or 0.0 if no value has been saved)
	 */
	void get_profile_from_history(SCP_string& name,
								  uint64_t* avg_micro_sec,
								  uint64_t* min_micro_sec,
								  uint64_t* max_micro_sec);

	void dump_output(SCP_stringstream& out,
					 uint64_t start_profile_time,
					 uint64_t end_profile_time,
					 SCP_vector<profile_sample>& samples);

	/**
	 * Builds the structured overlay snapshot (see frame_overlay_snapshot) from this frame's
	 * per-category self-time. self_time_by_id is indexed by Category::getId(); total is the sum of
	 * all self-times (i.e. total traced frame time). Called once per processFrame().
	 */
	void build_overlay_snapshot(const SCP_vector<uint64_t>& self_time_by_id, uint64_t total);

 public:
	FrameProfiler();
	~FrameProfiler();

	void processEvent(const trace_event* event);

	void processFrame();

	SCP_string getContent();

	const frame_overlay_snapshot& getOverlaySnapshot() const { return overlaySnapshot; }
};

}
