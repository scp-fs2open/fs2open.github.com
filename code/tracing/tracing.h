//
//

#ifndef _TRACING_H
#define _TRACING_H
#pragma once

#include "globalincs/pstypes.h"

typedef struct profile_sample {
	uint profile_instances;
	int open_profiles;
	//char name[256];
	SCP_string name;
	std::uint64_t start_time;	// in microseconds
	std::uint64_t accumulator;
	std::uint64_t children_sample_time;
	uint num_parents;
	uint num_children;
	int parent;
} profile_sample;

typedef struct profile_sample_history {
	bool valid;
	//char name[256];
	SCP_string name;
	float avg;
	float min;
	float max;
	std::uint64_t avg_micro_sec;
	std::uint64_t min_micro_sec;
	std::uint64_t max_micro_sec;
} profile_sample_history;

extern SCP_string profile_output;

void profile_init();
void profile_deinit();
void profile_begin(const char* name);
void profile_begin(SCP_string &output_handle, const char* name);
void profile_end(const char* name);
void profile_dump_output();
void profile_dump_json_output();
void store_profile_in_history(SCP_string &name, float percent, uint64_t time);
void get_profile_from_history(SCP_string &name, float* avg, float* min, float* max, uint64_t *avg_micro_sec, uint64_t *min_micro_sec, uint64_t *max_micro_sec);

class profile_auto
{
	SCP_string name;
 public:
	profile_auto(const char* profile_name): name(profile_name)
	{
		profile_begin(profile_name);
	}

	~profile_auto()
	{
		profile_end(name.c_str());
	}
};

// Helper macro to encapsulate a single function call in a profile_begin()/profile_end() pair.
#define PROFILE(name, function) { profile_begin(name); function; profile_end(name); }

#endif //_TRACING_H
