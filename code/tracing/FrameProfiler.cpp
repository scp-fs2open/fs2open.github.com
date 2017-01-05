//
//

#include "FrameProfiler.h"

#include "globalincs/systemvars.h"

using namespace tracing;

namespace {


bool event_sorter(const trace_event& left, const trace_event& right) {
	return left.event_id < right.event_id;
}

void process_begin(SCP_vector<profile_sample>& samples, const trace_event& evt) {
	int parent = -1;
	for (int i = 0; i < (int) samples.size(); i++) {
		if (!samples[i].open_profiles) {
			continue;
		}

		samples[i].num_children++;

		if (samples[i].num_children == 1) {
			// this is our direct parent for this new sample
			parent = i;
		}
	}

	for (int i = 0; i < (int) samples.size(); i++) {
		if (!strcmp(samples[i].name.c_str(), evt.category->getName()) && samples[i].parent == parent) {
			// found the profile sample
			samples[i].open_profiles++;
			samples[i].profile_instances++;
			samples[i].start_time = evt.timestamp;
			Assert(samples[i].open_profiles == 1); // max 1 open at once
			return;
		}
	}

	// create a new profile sample
	profile_sample new_sample;

	new_sample.name = SCP_string(evt.category->getName());
	new_sample.open_profiles = 1;
	new_sample.profile_instances = 1;
	new_sample.accumulator = 0;
	new_sample.start_time = evt.timestamp;
	new_sample.children_sample_time = 0;
	new_sample.num_children = 0;
	new_sample.parent = parent;

	samples.push_back(new_sample);
}

void process_end(SCP_vector<profile_sample>& samples, const trace_event& evt) {
	uint num_parents = 0;
	int child_of = -1;

	for (int i = 0; i < (int) samples.size(); i++) {
		if (samples[i].open_profiles) {
			if (samples[i].num_children == 1) {
				child_of = i;
			}
		}
	}

	for (int i = 0; i < (int) samples.size(); i++) {
		if (!strcmp(samples[i].name.c_str(), evt.category->getName()) && samples[i].parent == child_of) {
			int inner = 0;
			int parent = -1;
			uint64_t end_time = evt.timestamp;
			samples[i].open_profiles--;

			// count all parents and find the immediate parent
			while (inner < (int) samples.size()) {
				if (samples[inner].open_profiles > 0) {
					// found a parent (any open profiles are parents)
					num_parents++;

					if (parent < 0) {
						// replace invalid parent (index)
						parent = inner;
					} else if (samples[inner].start_time >= samples[parent].start_time) {
						// replace with more immediate parent
						parent = inner;
					}
				}
				inner++;
			}

			// remember the current number of parents of the sample
			samples[i].num_parents = num_parents;

			if (parent >= 0) {
				// record this time in children_sample_time (add it in)
				samples[parent].children_sample_time += end_time - samples[i].start_time;
			}

			// save sample time in accumulator
			samples[i].accumulator += end_time - samples[i].start_time;

			break;
		}
	}

	for (int i = 0; i < (int) samples.size(); i++) {
		if (samples[i].open_profiles) {
			samples[i].num_children--;
			samples[i].num_children = MAX(samples[i].num_children, 0);
		}
	}
}


}

namespace tracing {

FrameProfiler::FrameProfiler() {

}
FrameProfiler::~FrameProfiler() {

}
void FrameProfiler::processEvent(const trace_event* event) {
	if (event->type != EventType::Complete) {
		// Only process complete events
		return;
	}

	if (event->pid == GPU_PID) {
		// Ignore GPU events since we can't display them properly
		return;
	}

	if (event->duration == 0) {
		// Discard events with no duration
		return;
	}

	if (event->category == &MainFrame) {
		// The main frame category doesn't work right since the output is generated while we are still in that category
		return;
	}

	trace_event begin = *event;
	begin.type = EventType::Begin;
	begin.event_id = event->event_id;
	begin.duration = event->duration;

	trace_event end = *event;
	end.type = EventType::End;
	begin.event_id = event->end_event_id;
	end.timestamp = event->timestamp + event->duration;
	end.duration = event->duration;

	std::lock_guard<std::mutex> vectorGuard(_eventsMutex);
	_bufferedEvents.push_back(begin);
	_bufferedEvents.push_back(end);
}

void FrameProfiler::get_profile_from_history(SCP_string& name,
											 uint64_t* avg_micro_sec,
											 uint64_t* min_micro_sec,
											 uint64_t* max_micro_sec) {
	for (int i = 0; i < (int) history.size(); i++) {
		if (history[i].name == name) {
			*avg_micro_sec = history[i].avg_micro_sec;
			*min_micro_sec = history[i].min_micro_sec;
			*max_micro_sec = history[i].max_micro_sec;
			return;
		}
	}
}
void FrameProfiler::store_profile_in_history(SCP_string& name,
											 uint64_t time) {
	float old_ratio;
	float new_ratio = 0.8f * f2fl(Frametime);

	if (new_ratio > 1.0f) {
		new_ratio = 1.0f;
	}

	old_ratio = 1.0f - new_ratio;

	for (int i = 0; i < (int) history.size(); i++) {
		if (history[i].valid && history[i].name == name) {
			// found the sample
			history[i].avg_micro_sec = fl2i((history[i].avg_micro_sec * old_ratio) + (time * new_ratio));


			if (time < history[i].min_micro_sec) {
				history[i].min_micro_sec = time;
			} else {
				history[i].min_micro_sec = fl2i((history[i].min_micro_sec * old_ratio) + (time * new_ratio));
			}

			if (time > history[i].max_micro_sec) {
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
	new_history.avg_micro_sec = new_history.min_micro_sec = new_history.max_micro_sec = time;

	history.push_back(new_history);
}
void FrameProfiler::dump_output(SCP_stringstream& out,
								uint64_t start_profile_time,
								uint64_t end_profile_time,
								SCP_vector<profile_sample>& samples) {
	out << "  Avg :  Min :  Max :   # : Profile Name\n";
	out << "----------------------------------------\n";

	for (int i = 0; i < (int) samples.size(); i++) {
		uint64_t sample_time;
		uint64_t avg_micro_seconds, min_micro_seconds, max_micro_seconds;

		Assert(samples[i].open_profiles == 0);

		sample_time = samples[i].accumulator - samples[i].children_sample_time;


		avg_micro_seconds = min_micro_seconds = max_micro_seconds = sample_time;

		// add new measurement into the history and get avg, min, and max
		store_profile_in_history(samples[i].name, sample_time);
		get_profile_from_history(samples[i].name,
								 &avg_micro_seconds,
								 &min_micro_seconds,
								 &max_micro_seconds);

		// format the data
		char avg[64], min[64], max[64], num[64];

		sprintf(avg, "%3.1fms", i2fl(avg_micro_seconds) * 0.000001f);
		sprintf(min, "%3.1fms", i2fl(min_micro_seconds) * 0.000001f);
		sprintf(max, "%3.1fms", i2fl(max_micro_seconds) * 0.000001f);
		sprintf(num, "%3d", samples[i].profile_instances);

		SCP_string indented_name(samples[i].name);

		for (uint indent = 0; indent < samples[i].num_parents; indent++) {
			indented_name = ">" + indented_name;
		}

		char line[256];
		sprintf(line, "%5s : %5s : %5s : %3s : ", avg, min, max, num);

		out << line + indented_name + "\n";
	}
}

SCP_string FrameProfiler::getContent() {
	return content;
}
void FrameProfiler::processFrame() {

	std::lock_guard<std::mutex> vectorGuard(_eventsMutex);

	std::sort(_bufferedEvents.begin(), _bufferedEvents.end(), event_sorter);

	SCP_stringstream stream;

	SCP_vector<profile_sample> samples;

	bool start_found = false;
	bool end_found = false;
	uint64_t start_profile_time = 0;
	uint64_t end_profile_time = 0;

	for (auto& event : _bufferedEvents) {
		if (!start_found) {
			start_profile_time = event.timestamp;
			start_found = true;
		}
		if (!end_found) {
			end_profile_time = event.timestamp;
			end_found = true;
		}

		switch (event.type) {
			case EventType::Begin:
				process_begin(samples, event);
				break;
			case EventType::End:
				process_end(samples, event);
				break;
			default:
				break;
		}
	}
	_bufferedEvents.clear();

	dump_output(stream, start_profile_time, end_profile_time, samples);

	content = stream.str();
}

}
