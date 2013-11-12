/*
 * Copyright (C) Freespace Open 2013.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "io/timer.h"
#include "cmdline/cmdline.h"

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

float start_profile_time = 0.0f;
float end_profile_time = 0.0f;

char profile_output[2048] = "";

/**
 * @brief Called once at engine initialization to set the timer
 */
void profile_init()
{
	start_profile_time = f2fl(timer_get_fixed_seconds());
}

/**
 * Used to start profiling a section of code. A section started by profile_begin needs to be closed off by calling
 * profile_end with the same argument.
 * @param name A globally unique string that will be displayed in the HUD readout
 */
void profile_begin(char* name)
{
	if (Cmdline_frame_profile) 
	{
		for(int i = 0; i < (int)samples.size(); i++) {
			if(samples[i].valid && !strcmp(samples[i].name, name) ) {
				// found the profile sample
				samples[i].open_profiles++;
				samples[i].profile_instances++;
				samples[i].start_time = f2fl(timer_get_fixed_seconds());
				Assert(samples[i].open_profiles == 1); // max 1 open at once
				return;
			}
		}

		// create a new profile sample
		profile_sample new_sample;

		strcpy_s(new_sample.name, name);
		new_sample.valid = true;
		new_sample.open_profiles = 1;
		new_sample.profile_instances = 1;
		new_sample.accumulator = 0.0f;
		new_sample.start_time = f2fl(timer_get_fixed_seconds());
		new_sample.children_sample_time = 0.0f;

		samples.push_back(new_sample);
	}
}

/**
 * Used to end profiling of a section of code. Note that the parameter given MUST match that of the preceding call
 * to profile_begin
 * @param name A globally unique string that will be displayed in the HUD readout
 */
void profile_end(char* name)
{
	if (Cmdline_frame_profile) {
		int num_parents = 0;

		for(int i = 0; i < (int)samples.size(); i++) {
			if(samples[i].valid && !strcmp(samples[i].name, name) ) {
				int inner = 0;
				int parent = -1;
				float end_time = f2fl(timer_get_fixed_seconds());
				samples[i].open_profiles--;

				// count all parents and find the immediate parent
				while(samples[inner].valid && inner < (int)samples.size()) {
					if(samples[inner].open_profiles > 0) {
						// found a parent (any open profiles are parents)
						num_parents++;

						if(parent < 0) {
							// replace invalid parent (index)
							parent = inner;
						} else if(samples[inner].start_time >= samples[parent].start_time) {
							// replace with more immediate parent
							parent = inner;
						}
					}
					inner++;
				}

				// remember the current number of parents of the sample
				samples[i].num_parents = num_parents;

				if(parent >= 0) {
					// record this time in children_sample_time (add it in)
					samples[parent].children_sample_time += end_time - samples[i].start_time;
				}

				// save sample time in accumulator
				samples[i].accumulator += end_time - samples[i].start_time;
			
				return;
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
		end_profile_time = f2fl(timer_get_fixed_seconds());

		strcpy_s(profile_output, "");
		strcat(profile_output, "  Avg :  Min :  Max :   # : Profile Name\n");
		strcat(profile_output, "----------------------------------------\n");

		for(int i = 0; i < (int)samples.size(); i++) {
			float sample_time, percent_time, avg_time, min_time, max_time;
			char line[256], name[256], indented_name[256];
			char avg[16], min[16], max[16], num[16];

			Assert(samples[i].open_profiles == 0);

			sample_time = samples[i].accumulator - samples[i].children_sample_time;

			if (end_profile_time == start_profile_time) {
				percent_time = 0.0f;
			} else {
				percent_time = (sample_time / (end_profile_time - start_profile_time)) *100.0f;
			}

			avg_time = min_time = max_time = percent_time;

			// add new measurement into the history and get avg, min, and max
			store_profile_in_history(samples[i].name, percent_time);
			get_profile_from_history(samples[i].name, &avg_time, &min_time, &max_time);
			// format the data
			sprintf(avg, "%3.1f", avg_time);
			sprintf(min, "%3.1f", min_time);
			sprintf(max, "%3.1f", max_time);
			sprintf(num, "%3d", samples[i].profile_instances);

			strcpy(indented_name, samples[i].name);
			for(uint indent = 0; indent < samples[i].num_parents; indent++) {
				sprintf(name, "   %s", indented_name);
				strcpy_s(indented_name, name);
			}

			sprintf(line, "%5s : %5s : %5s : %3s : %s\n", avg, min, max, num, indented_name);
			strcat(profile_output, line);
		}

		samples.clear();
		start_profile_time = f2fl(timer_get_fixed_seconds());
	}
}

/**
 * Stores profile data in in the profile history lookup. This is used internally by the profiling code and should
 * not be called outside of it.
 * @param name The globally unique name for this profile (see profile_begin()/profile_end())
 * @param percent How much time the profiled section took to execute (as a percentage of overall frametime)
 */
void store_profile_in_history(char* name, float percent)
{
	float old_ratio;
	float new_ratio = 0.8f * f2fl(Frametime);

	if(new_ratio > 1.0f) {
		new_ratio = 1.0f;
	}

	old_ratio = 1.0f - new_ratio;

	for(int i = 0; i < (int)history.size(); i++) {
		if( history[i].valid && !strcmp(history[i].name, name) ) {
			// found the sample
			history[i].avg = (history[i].avg * old_ratio) + (percent * new_ratio);

			if( percent < history[i].min ) {
				history[i].min = percent;
			} else {
				history[i].min = (history[i].min*old_ratio) + (percent*new_ratio);
			}

			if( percent > history[i].max) {
				history[i].max = percent;
			} else {
				history[i].max = (history[i].max * old_ratio) + (percent * new_ratio);
			}
			return;
		}
	}

	// add to history
	profile_sample_history new_history;

	strcpy_s(new_history.name, name);
	new_history.valid = true;
	new_history.avg = new_history.min = new_history.max = percent;

	history.push_back(new_history);
}

/**
 * Gets the min, max and average values for a given profile
 * @param name The globally unique name for this profile (see profile_begin()/profile_end())
 * @param avg Pointer to a float in which the average value will be stored (or 0.0 if no value has been saved)
 * @param min Pointer to a float in which the minimum value will be stored (or 0.0 if no value has been saved)
 * @param max Pointer to a float in which the maximum value will be stored (or 0.0 if no value has been saved)
 */
void get_profile_from_history(char* name, float* avg, float* min, float* max)
{
	for(int i = 0; i < (int)history.size(); i++) {
		if(!strcmp(history[i].name, name)) {
			*avg = history[i].avg;
			*min = history[i].min;
			*max = history[i].max;
			return;
		}
	}
	*avg = *min = *max = 0.0f;
}