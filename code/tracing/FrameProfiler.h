#pragma once


#include "globalincs/pstypes.h"

#include "tracing.h"

#include <mutex>

/** @file
 *  @ingroup tracing
 */

namespace tracing {

struct profile_sample_history {
	bool valid;
	//char name[256];
	SCP_string name;
	float avg;
	float min;
	float max;
	uint64_t avg_micro_sec;
	uint64_t min_micro_sec;
	uint64_t max_micro_sec;
};

struct profile_sample {
	uint profile_instances;
	int open_profiles;
	//char name[256];
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

	/**
	 * Stores profile data in in the profile history lookup. This is used internally by the profiling code and should
	 * not be called outside of it.
	 * @param name The globally unique name for this profile (see profile_begin()/profile_end())
	 * @param percent How much time the profiled section took to execute (as a percentage of overall frametime)
	 */
	void store_profile_in_history(SCP_string& name, float percent, uint64_t time);

	/**
	 * Gets the min, max and average values for a given profile
	 * @param name The globally unique name for this profile (see profile_begin()/profile_end())
	 * @param avg Pointer to a float in which the average value will be stored (or 0.0 if no value has been saved)
	 * @param min Pointer to a float in which the minimum value will be stored (or 0.0 if no value has been saved)
	 * @param max Pointer to a float in which the maximum value will be stored (or 0.0 if no value has been saved)
	 */
	void get_profile_from_history(SCP_string& name,
								  float* avg,
								  float* min,
								  float* max,
								  uint64_t* avg_micro_sec,
								  uint64_t* min_micro_sec,
								  uint64_t* max_micro_sec);

	void dump_output(SCP_stringstream& out,
					 uint64_t start_profile_time,
					 uint64_t end_profile_time,
					 SCP_vector<profile_sample>& samples);


 public:
	FrameProfiler();
	~FrameProfiler();

	void processEvent(const trace_event* event);

	SCP_string getContent();
};

}
