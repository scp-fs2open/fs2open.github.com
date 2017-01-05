
#include "tracing/tracing.h"
#include "graphics/2d.h"
#include "parse/parselo.h"
#include "io/timer.h"

#include "TraceEventWriter.h"
#include "MainFrameTimer.h"
#include "FrameProfiler.h"

#include <inttypes.h>
#include <fstream>
#include <future>
#include <mutex>

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
	return (int64_t) getpid();
}
#endif

namespace {

using namespace tracing;

std::unique_ptr<ThreadedTraceEventWriter> traceEventWriter;
std::unique_ptr<ThreadedMainFrameTimer> mainFrameTimer;
std::unique_ptr<FrameProfiler> frameProfiler;

SCP_vector<int> query_objects;
// The GPU timestamp queries use an internal free list to reduce the number of graphics API calls
SCP_queue<int> free_query_objects;
bool do_gpu_queries = true;

int get_query_object() {
	if (!free_query_objects.empty()) {
		auto id = free_query_objects.front();
		free_query_objects.pop();
		return id;
	}

	auto id = gr_create_query_object();
	query_objects.push_back(id);
	return id;
}

int get_gpu_timestamp_query() {
	auto query = get_query_object();
	gr_query_value(query, QueryType::Timestamp);

	return query;
}

void free_query_object(int obj) {
	free_query_objects.push(obj);
}

struct gpu_trace_event {
	trace_event base_evt;

	int gpu_begin_query = -1;
	int gpu_end_query = -1;
};

SCP_queue<gpu_trace_event> gpu_events;

bool initialized = false;

bool do_trace_events = false;
bool do_async_events = false;
bool do_counter_events = false;
std::int64_t main_thread_id = -1;

int gpu_start_query = -1;
std::uint64_t gpu_start_time = 0;
std::uint64_t cpu_start_time = 0;

std::uint64_t current_id = 0;

void submit_event(trace_event* evt) {
	if (evt->pid == GPU_PID) {
		evt->timestamp -= gpu_start_time;
	} else {
		evt->timestamp -= cpu_start_time;
	}

	if (traceEventWriter) {
		// Trace event writer receives all events
		traceEventWriter->processEvent(evt);
	}

	if (mainFrameTimer) {
		mainFrameTimer->processEvent(evt);
	}

	if (frameProfiler) {
		frameProfiler->processEvent(evt);
	}
}

void process_gpu_events() {
	Assertion(get_tid() == main_thread_id, "This function must be called from the main thread!");

	if (gpu_start_query >= 0) {
		if (gr_query_value_available(gpu_start_query)) {
			gpu_start_time = gr_get_query_value(gpu_start_query);
			gpu_start_query = -1;
		} else {
			// Wait until query is finished
			return;
		}
	}

	while (!gpu_events.empty()) {
		auto& first = gpu_events.front();
		auto result_available = true;

		if (first.gpu_begin_query != -1) {
			if (gr_query_value_available(first.gpu_begin_query)) {
				first.base_evt.timestamp = gr_get_query_value(first.gpu_begin_query);

				free_query_object(first.gpu_begin_query);
				first.gpu_begin_query = -1;
			} else {
				// Query not processed yet, try again later...
				result_available = false;
			}
		}

		switch (first.base_evt.type) {
			case EventType::Complete:
				// For complete events, check the end query
				if (gr_query_value_available(first.gpu_end_query)) {
					// All queries are finished, get the values and submit the event
					auto finished_evt = first.base_evt;
					auto val = gr_get_query_value(first.gpu_end_query);

					finished_evt.duration = val - finished_evt.timestamp;
					free_query_object(first.gpu_end_query);

					submit_event(&finished_evt);

					gpu_events.pop();
				} else {
					result_available = false;
				}
				break;
			case EventType::Begin:
			case EventType::End:
				if (result_available) {
					submit_event(&first.base_evt);

					gpu_events.pop();
				}
				break;
			default:
				Assertion(false, "Invalid event type!");
				gpu_events.pop();
				break;
		}

		if (!result_available) {
			// GPU result not available, try again next frame
			break;
		}
	}
}

void init_event(const Category& category, trace_event* evt) {
	evt->category = &category;

	evt->timestamp = timer_get_nanoseconds();

	evt->pid = get_pid();
	evt->tid = get_tid();
}
}

namespace tracing {
void init() {
	do_trace_events = false;
	do_async_events = false;
	do_counter_events = false;

	if (Cmdline_json_profiling) {
		traceEventWriter.reset(new ThreadedTraceEventWriter());
		do_trace_events = true;
		do_async_events = true;
		do_counter_events = true;
	}
	if (Cmdline_profile_write_file) {
		mainFrameTimer.reset(new ThreadedMainFrameTimer());
		do_async_events = true;
	}
	if (Cmdline_frame_profile) {
		frameProfiler.reset(new FrameProfiler());
		do_trace_events = true;
	}

	do_gpu_queries = gr_is_capable(CAPABILITY_TIMESTAMP_QUERY);

	if (do_gpu_queries) {
		gpu_start_query = get_gpu_timestamp_query();
	}
	cpu_start_time = timer_get_nanoseconds();

	main_thread_id = get_tid();

	initialized = true;
}

void process_events() {
	if (do_gpu_queries) {
		// Process pending GPU events
		process_gpu_events();
	}
}
void frame_profile_process_frame() {
	Assertion(frameProfiler, "Frame profiling must be enabled for this function!");

	return frameProfiler->processFrame();
}

SCP_string get_frame_profile_output() {
	Assertion(frameProfiler, "Frame profiling must be enabled for this function!");

	return frameProfiler->getContent();
}

void shutdown() {
	while (!gpu_events.empty()) {
		process_events();

		// Don't do busy waiting...
		os_sleep(5);
	}

	for (auto query : query_objects) {
		gr_delete_query_object(query);
	}
	query_objects.clear();

	mainFrameTimer = nullptr;
	traceEventWriter = nullptr;

	initialized = false;
}

namespace complete {

void start(const Category& category, trace_event* evt) {
	if (!do_trace_events) {
		// No one to process the event is here
		return;
	}

	if (!initialized) {
		return;
	}

	init_event(category, evt);

	evt->duration = 0;
	evt->type = EventType::Complete;
	evt->event_id = ++current_id;

	if (do_gpu_queries && category.usesGPUCounter()) {
		Assertion(get_tid() == main_thread_id, "This function must be called from the main thread!");

		gpu_trace_event gpu_event;
		gpu_event.base_evt.category = &category;
		gpu_event.base_evt.tid = 1;
		gpu_event.base_evt.pid = GPU_PID;
		gpu_event.base_evt.type = EventType::Begin;

		gpu_event.gpu_begin_query = get_gpu_timestamp_query();

		// This does not need to be synchronized since GPU queries are only allowed on the main thread.
		gpu_events.push(gpu_event);
	}
}

void end(trace_event* evt) {
	if (!do_trace_events) {
		// No one to process the event is here
		return;
	}

	if (!initialized) {
		return;
	}

	Assertion(evt->pid == get_pid(), "Complete events must be generated from the same process!");
	Assertion(evt->tid == get_tid(), "Complete events must be generated from the same thread!");

	evt->duration = timer_get_nanoseconds() - evt->timestamp;
	evt->end_event_id = ++current_id;

	// Process CPU events
	submit_event(evt);

	// Create GPU events
	if (do_gpu_queries && evt->category->usesGPUCounter()) {
		Assertion(get_tid() == main_thread_id, "This function must be called from the main thread!");

		gpu_trace_event gpu_event;
		gpu_event.base_evt.category = evt->category;
		gpu_event.base_evt.tid = 1;
		gpu_event.base_evt.pid = GPU_PID;
		gpu_event.base_evt.type = EventType::End;

		gpu_event.gpu_begin_query = get_gpu_timestamp_query();

		// This does not need to be synchronized since GPU queries are only allowed on the main thread.
		gpu_events.push(gpu_event);
	}
}

}

namespace async {

void begin(const Category& category, const Scope& async_scope) {
	if (!do_async_events) {
		return;
	}

	trace_event evt;
	init_event(category, &evt);

	evt.type = EventType::AsyncBegin;
	evt.scope = &async_scope;
	evt.event_id = ++current_id;

	submit_event(&evt);
}

void step(const Category& category, const Scope& async_scope) {
	if (!do_async_events) {
		return;
	}

	trace_event evt;
	init_event(category, &evt);

	evt.type = EventType::AsyncStep;
	evt.scope = &async_scope;
	evt.event_id = ++current_id;

	submit_event(&evt);
}

void end(const Category& category, const Scope& async_scope) {
	if (!do_async_events) {
		return;
	}

	trace_event evt;
	init_event(category, &evt);

	evt.type = EventType::AsyncEnd;
	evt.scope = &async_scope;
	evt.event_id = ++current_id;

	submit_event(&evt);
}

}

namespace counter {

void value(const Category& category, float value) {
	if (!do_counter_events) {
		return;
	}

	trace_event evt;
	init_event(category, &evt);
	evt.type = EventType::Counter;
	evt.value = value;
	evt.event_id = ++current_id;

	submit_event(&evt);
}

}

}
