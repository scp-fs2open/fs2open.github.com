/*
 * Copyright (C) Freespace Open 2013.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "tracing/tracing.h"
#include "cmdline/cmdline.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "io/timer.h"
#include "parse/parselo.h"
#include "globalincs/systemvars.h"

#if SCP_COMPILER_IS_MSVC
#include <direct.h>
#endif
#include <cinttypes>
#include <fstream>
#include <future>
#include <mutex>

#include <jansson.h>

// A function for getting the id of the current thread
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static int64_t get_tid() {
	return (int64_t) GetCurrentThreadId();
}
#elif __LINUX__
#include <sys/syscall.h>
static int64_t get_tid() {
	return (int64_t) syscall(SYS_gettid);
}
#else
#include <pthread.h>

static int64_t get_tid() {
	// This is not a reliable way of getting the tid but it's better than nothing
	return (int64_t) pthread_self();
}
#endif

// A function for getting the id of the current process
#ifdef WIN32
static int64_t get_pid() {
	return (int64_t)GetCurrentProcessId();
}
#else
#include <unistd.h>

static int64_t get_pid() {
	return (int64_t)getpid();
}
#endif

//======================CODE TO PROFILE PERFORMANCE=====================

/*
 * Usage information:
 * In order to gather profiling data for a single function call, you can use the PROFILE macro as defined in
 * pstypes.h.
 * Example:
 * PROFILE("Render", game_render_frame( cid ));
 * 
 * If you want to profile a block of function calls, you will need to use the profile_begin()/profile_end() calls.
 * Example:
 * profile_begin("Some Code");
 * ...some code...
 * profile_end("Some Code");
 * Note that the parameter passed MUST be globally unique across all instances of profiling invocations.
 *
 * Profiling invocations can be nested as deep as necessary; this will show up in the readout as indentations.
 */

SCP_vector<profile_sample> samples;
SCP_vector<profile_sample_history> history;

std::uint64_t start_profile_time = 0;
std::uint64_t end_profile_time = 0;

SCP_string profile_output;
std::ofstream profiling_file;

static SCP_vector<int> query_objects;
// The GPU timestamp queries use an internal free list to reduce the number of graphics API calls
static SCP_queue<int> free_query_objects;
static bool do_gpu_queries = true;

static int get_query_object() {
	if (!free_query_objects.empty()) {
		auto id = free_query_objects.front();
		free_query_objects.pop();
		return id;
	}

	auto id = gr_create_query_object();
	query_objects.push_back(id);
	return id;
}
static void free_query_object(int obj) {
	free_query_objects.push(obj);
}

struct tracing_data
{
	SCP_string name;
	
	int64_t pid;
	int64_t tid;
	
	uint64_t time;

	int gpu_query;
	uint64_t gpu_time;

	bool enter;
};

struct tracing_frame_data {
	SCP_vector<tracing_data> data;
	std::uint64_t frame_num;
};

static SCP_vector<tracing_frame_data> pending_frame_data;

static void write_json_data_file(const tracing_frame_data& data, const SCP_string& file_name, bool print_gpu_times) {
	std::ofstream out(file_name, std::ios::out | std::ios::binary);
	out << "[";

	auto first = true;
	// Output CPU times
	for (auto& trace : data.data)
	{
		if (!first) {
			out << ",";
		}
		out << "\n{\"tid\": " << trace.tid << ",\"ts\":";

		// Save stream state
		auto flags = out.flags();
		out << std::fixed;

		if (print_gpu_times) {
			out << trace.gpu_time / 1000.;
		} else {
			out << trace.time / 1000.;
		}

		// and now restore it
		out.flags(flags);

		out << ",\"pid\":";
		if (print_gpu_times) {
			out << "\"GPU\"";
		} else {
			out << trace.pid;
		}

		out << ",\"name\":\"" << trace.name << "\",\"ph\":\"" << (trace.enter ? "B" : "E") << "\"}";

		first = false;
	}
	out << "]\n";
}

static void write_json_data(const tracing_frame_data& data)
{
	SCP_string file_path;

	sprintf(file_path, "tracing/cpu_%" PRIu64 ".json", data.frame_num);
	write_json_data_file(data, file_path, false);

	sprintf(file_path, "tracing/gpu_%" PRIu64 ".json", data.frame_num);
	write_json_data_file(data, file_path, true);
}

static void process_pending_data() {
	while (!pending_frame_data.empty()) {
		auto& frame_data = pending_frame_data.front();

		bool finished;
		if (!do_gpu_queries) {
			finished = true;
		} else {
			// Determine if all queries have passed already
			finished = true;
			for (auto& trace_data : frame_data.data) {
				if (trace_data.gpu_query == -1) {
					// Event has been processed before
					continue;
				}

				if (gr_query_value_available(trace_data.gpu_query)) {
					trace_data.gpu_time = gr_get_query_value(trace_data.gpu_query);
					free_query_object(trace_data.gpu_query);
					trace_data.gpu_query = -1;
				} else {
					// If we are here then a query hasn't finished yet. Try again next time...
					finished = false;
					break;
				}
			}
		}

		if (finished) {
			std::thread writer_thread(std::bind(write_json_data, frame_data));
			writer_thread.detach();

			pending_frame_data.erase(pending_frame_data.begin());
		} else {
			// GPU queries always finish in sequence so the later queries can't be finished yet
			break;
		}
	}
}

static SCP_vector<tracing_data> current_frame_data;
static uint64_t json_frame_num = 0;
static std::mutex json_mutex;

/**
 * @brief Called once at engine initialization to set the timer
 */
void profile_init()
{
	start_profile_time = timer_get_microseconds();

	if (Cmdline_profile_write_file)
	{
		profiling_file.open("profiling.csv");

		if (!profiling_file.good())
		{
			mprintf(("Failed to open profiling output file 'profiling.csv'!"));
		}
	}

	if (Cmdline_json_profiling) {
		_mkdir("tracing");
	}
}

void profile_deinit()
{
	if (Cmdline_profile_write_file)
	{
		if (profiling_file.is_open())
		{
			profiling_file.flush();
			profiling_file.close();
		}
	}
	if (Cmdline_json_profiling)
	{
		profile_dump_json_output();
		while (!pending_frame_data.empty()) {
			process_pending_data();
		}
	}

	for (auto obj : query_objects) {
		gr_delete_query_object(obj);
	}
	query_objects.clear();
	SCP_queue<int>().swap(free_query_objects);
}

/**
 * Used to start profiling a section of code. A section started by profile_begin needs to be closed off by calling
 * profile_end with the same argument.
 * @param name A globally unique string that will be displayed in the HUD readout
 */
void profile_begin(const char* name)
{
	if (Cmdline_json_profiling)
	{
		std::lock_guard<std::mutex> guard(json_mutex);

		tracing_data data;

		data.name = name;
		
		data.pid = get_pid();
		data.tid = get_tid();

		data.enter = true;
		data.time = timer_get_nanoseconds();

		if (do_gpu_queries) {
			data.gpu_query = get_query_object();
			gr_query_value(data.gpu_query, QueryType::Timestamp);
		} else {
			data.gpu_query = -1;
		}

		current_frame_data.push_back(data);
	}

	if (Cmdline_frame_profile)
	{
		int parent = -1;
		for (int i = 0; i < (int)samples.size(); i++) {
			if ( !samples[i].open_profiles ) {
				continue;
			}

			samples[i].num_children++;

			if (samples[i].num_children == 1) {
				// this is our direct parent for this new sample
				parent = i;
			}
		}

		for(int i = 0; i < (int)samples.size(); i++) {
			if( !strcmp(samples[i].name.c_str(), name) && samples[i].parent == parent ) {
				// found the profile sample
				samples[i].open_profiles++;
				samples[i].profile_instances++;
				samples[i].start_time = timer_get_microseconds();
				Assert(samples[i].open_profiles == 1); // max 1 open at once
				return;
			}
		}

		// create a new profile sample
		profile_sample new_sample;

		new_sample.name = SCP_string(name);
		new_sample.open_profiles = 1;
		new_sample.profile_instances = 1;
		new_sample.accumulator = 0;
		new_sample.start_time = timer_get_microseconds();
		new_sample.children_sample_time = 0;
		new_sample.num_children = 0;
		new_sample.parent = parent;

		samples.push_back(new_sample);
	}
}

/**
 * Used to end profiling of a section of code. Note that the parameter given MUST match that of the preceding call
 * to profile_begin
 * @param name A globally unique string that will be displayed in the HUD readout
 */
void profile_end(const char* name)
{
	if (Cmdline_json_profiling)
	{
		std::lock_guard<std::mutex> guard(json_mutex);

		tracing_data data;

		data.name = name;

		data.pid = get_pid();
		data.tid = get_tid();

		data.enter = false;
		data.time = timer_get_nanoseconds();

		if (do_gpu_queries) {
			data.gpu_query = get_query_object();
			gr_query_value(data.gpu_query, QueryType::Timestamp);
		} else {
			data.gpu_query = -1;
		}

		current_frame_data.push_back(data);
	}

	if (Cmdline_frame_profile) {
		int num_parents = 0;
		int child_of = -1;

		for ( int i = 0; i < (int)samples.size(); i++ ) {
			if ( samples[i].open_profiles ) {
				if ( samples[i].num_children == 1 ) {
					child_of = i;
				}
			}
		}

		for ( int i = 0; i < (int)samples.size(); i++ ) {
			if ( !strcmp(samples[i].name.c_str(), name) && samples[i].parent == child_of ) {
				int inner = 0;
				int parent = -1;
				std::uint64_t end_time = timer_get_microseconds();
				samples[i].open_profiles--;

				// count all parents and find the immediate parent
				while ( inner < (int)samples.size() ) {
					if ( samples[inner].open_profiles > 0 ) {
						// found a parent (any open profiles are parents)
						num_parents++;

						if (parent < 0) {
							// replace invalid parent (index)
							parent = inner;
						}
						else if (samples[inner].start_time >= samples[parent].start_time) {
							// replace with more immediate parent
							parent = inner;
						}
					}
					inner++;
				}

				// remember the current number of parents of the sample
				samples[i].num_parents = num_parents;

				if ( parent >= 0 ) {
					// record this time in children_sample_time (add it in)
					samples[parent].children_sample_time += end_time - samples[i].start_time;
				}

				// save sample time in accumulator
				samples[i].accumulator += end_time - samples[i].start_time;

				break;
			}
		}

		for (int i = 0; i < (int)samples.size(); i++) {
			if (samples[i].open_profiles) {
				samples[i].num_children--;
				samples[i].num_children = MAX(samples[i].num_children, 0);
			}
		}
	}
}

/**
 * Builds the output text.
 */
void profile_dump_output()
{
	if (Cmdline_frame_profile) {
		end_profile_time = timer_get_microseconds();

		if (Cmdline_profile_write_file)
		{
			profiling_file << end_profile_time << ";" << (end_profile_time - start_profile_time) << std::endl;
		}

		profile_output.clear();
		profile_output += "  Avg :  Min :  Max :   # : Profile Name\n";
		profile_output += "----------------------------------------\n";

		for(int i = 0; i < (int)samples.size(); i++) {
			uint64_t sample_time;
			float percent_time, avg_time, min_time, max_time;
			uint64_t avg_micro_seconds, min_micro_seconds, max_micro_seconds; 

			Assert(samples[i].open_profiles == 0);

			sample_time = samples[i].accumulator - samples[i].children_sample_time;

			if (end_profile_time == start_profile_time) {
				percent_time = 0.0f;
			} else {
				percent_time = (i2fl(sample_time) / i2fl(end_profile_time - start_profile_time)) *100.0f;
			}

			avg_micro_seconds = min_micro_seconds = max_micro_seconds = sample_time;
			avg_time = min_time = max_time = percent_time;

			// add new measurement into the history and get avg, min, and max
			store_profile_in_history(samples[i].name, percent_time, sample_time);
			get_profile_from_history(samples[i].name, &avg_time, &min_time, &max_time, &avg_micro_seconds, &min_micro_seconds, &max_micro_seconds);

			// format the data
			char avg[64], min[64], max[64], num[64];

			sprintf(avg, "%3.1f%% (%3.1fms)", avg_time, i2fl(avg_micro_seconds)*0.001f);
			sprintf(min, "%3.1f%% (%3.1fms)", min_time, i2fl(min_micro_seconds)*0.001f);
			sprintf(max, "%3.1f%% (%3.1fms)", max_time, i2fl(max_micro_seconds)*0.001f);
			sprintf(num, "%3d", samples[i].profile_instances);

			SCP_string indented_name(samples[i].name);

			for(uint indent = 0; indent < samples[i].num_parents; indent++) {
				indented_name = ">" + indented_name;
			}

			char line[256];
			sprintf(line, "%5s : %5s : %5s : %3s : ", avg, min, max, num);

			profile_output += line + indented_name + "\n";
		}

		samples.clear();
		start_profile_time = timer_get_microseconds();
	}
}

/**
 * @brief Writes JSON tracing data to a file if the commandlinfe option is enabled
 */
void profile_dump_json_output() {
	if (Cmdline_json_profiling) {
		std::lock_guard<std::mutex> guard(json_mutex);

		// FIXME: This could be improved by using only a single thread and a synchronized bounded
		// queue. Boost has an implementation of that.
		tracing_frame_data frame_data;
		frame_data.data = current_frame_data;
		frame_data.frame_num = ++json_frame_num;

		pending_frame_data.push_back(std::move(frame_data));

		current_frame_data.clear();

		process_pending_data();
	}
}

/**
 * Stores profile data in in the profile history lookup. This is used internally by the profiling code and should
 * not be called outside of it.
 * @param name The globally unique name for this profile (see profile_begin()/profile_end())
 * @param percent How much time the profiled section took to execute (as a percentage of overall frametime)
 */
void store_profile_in_history(SCP_string &name, float percent, uint64_t time)
{
	float old_ratio;
	float new_ratio = 0.8f * f2fl(Frametime);

	if(new_ratio > 1.0f) {
		new_ratio = 1.0f;
	}

	old_ratio = 1.0f - new_ratio;

	for(int i = 0; i < (int)history.size(); i++) {
		if( history[i].valid && history[i].name == name ) {
			// found the sample
			history[i].avg = (history[i].avg * old_ratio) + (percent * new_ratio);
			history[i].avg_micro_sec = fl2i((history[i].avg_micro_sec * old_ratio) + (time * new_ratio));

			if( percent < history[i].min ) {
				history[i].min = percent;
			} else {
				history[i].min = (history[i].min*old_ratio) + (percent*new_ratio);
			}

			if( time < history[i].min_micro_sec ) {
				history[i].min_micro_sec = time;
			} else {
				history[i].min_micro_sec = fl2i((history[i].min_micro_sec*old_ratio) + (time*new_ratio));
			}

			if( percent > history[i].max) {
				history[i].max = percent;
			} else {
				history[i].max = (history[i].max * old_ratio) + (percent * new_ratio);
			}

			if( time > history[i].max_micro_sec) {
				history[i].max_micro_sec = time;
			} else {
				history[i].max_micro_sec = fl2i((history[i].max_micro_sec * old_ratio) + (time * new_ratio));
			}

			return;
		}
	}

	// add to history
	profile_sample_history new_history;

	new_history.name = name;
	new_history.valid = true;
	new_history.avg = new_history.min = new_history.max = percent;
	new_history.avg_micro_sec = new_history.min_micro_sec = new_history.max_micro_sec = time;

	history.push_back(new_history);
}

/**
 * Gets the min, max and average values for a given profile
 * @param name The globally unique name for this profile (see profile_begin()/profile_end())
 * @param avg Pointer to a float in which the average value will be stored (or 0.0 if no value has been saved)
 * @param min Pointer to a float in which the minimum value will be stored (or 0.0 if no value has been saved)
 * @param max Pointer to a float in which the maximum value will be stored (or 0.0 if no value has been saved)
 */
void get_profile_from_history(SCP_string &name, float* avg, float* min, float* max, uint64_t *avg_micro_sec, uint64_t *min_micro_sec, uint64_t *max_micro_sec)
{
	for ( int i = 0; i < (int)history.size(); i++ ) {
		if ( history[i].name == name ) {
			*avg = history[i].avg;
			*min = history[i].min;
			*max = history[i].max;
			*avg_micro_sec = history[i].avg_micro_sec;
			*min_micro_sec = history[i].min_micro_sec;
			*max_micro_sec = history[i].max_micro_sec;
			return;
		}
	}

	*avg = *min = *max = 0.0f;
}