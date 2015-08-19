/*
 * Copyright (C) Freespace Open 2013.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "cmdline/cmdline.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "io/timer.h"

#include <fstream>

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

uint start_profile_time = 0;
uint end_profile_time = 0;

SCP_string profile_output;
std::ofstream profiling_file;

/**
 * @brief Called once at engine initialization to set the timer
 */
void profile_init()
{
	start_profile_time = timer_get_high_res_microseconds();

	if (Cmdline_profile_write_file)
	{
		profiling_file.open("profiling.csv");

		if (!profiling_file.good())
		{
			mprintf(("Failed to open profiling output file 'profiling.csv'!"));
		}
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
}

/**
 * Used to start profiling a section of code. A section started by profile_begin needs to be closed off by calling
 * profile_end with the same argument.
 * @param name A globally unique string that will be displayed in the HUD readout
 */
void profile_begin(const char* name)
{
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
				samples[i].start_time = timer_get_high_res_microseconds();
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
		new_sample.start_time = timer_get_high_res_microseconds();
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
				uint end_time = timer_get_high_res_microseconds();
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
		end_profile_time = timer_get_high_res_microseconds();

		if (Cmdline_profile_write_file)
		{
			profiling_file << end_profile_time << ";" << (end_profile_time - start_profile_time) << std::endl;
		}

		profile_output.clear();
		profile_output += "  Avg :  Min :  Max :   # : Profile Name\n";
		profile_output += "----------------------------------------\n";

		for(int i = 0; i < (int)samples.size(); i++) {
			uint sample_time;
			float percent_time, avg_time, min_time, max_time;
			uint avg_micro_seconds, min_micro_seconds, max_micro_seconds; 

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
		start_profile_time = timer_get_high_res_microseconds();
	}
}

/**
 * Stores profile data in in the profile history lookup. This is used internally by the profiling code and should
 * not be called outside of it.
 * @param name The globally unique name for this profile (see profile_begin()/profile_end())
 * @param percent How much time the profiled section took to execute (as a percentage of overall frametime)
 */
void store_profile_in_history(SCP_string &name, float percent, uint time)
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
void get_profile_from_history(SCP_string &name, float* avg, float* min, float* max, uint *avg_micro_sec, uint *min_micro_sec, uint *max_micro_sec)
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