
#include "tracing/tracing.h"
#include "graphics/2d.h"
#include "globalincs/systemvars.h"
#include "parse/parselo.h"
#include "io/timer.h"
#include "cmdline/cmdline.h"

#include "TraceEventWriter.h"
#include "MainFrameTimer.h"
#include "FrameProfiler.h"
#include "options/Option.h"

#include <cinttypes>
#include <fstream>
#include <future>
#include <mutex>

// A function for getting the id of the current thread
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static int64_t query_tid() {
    return (int64_t) GetCurrentThreadId();
}
#elif __LINUX__
#include <sys/syscall.h>
static int64_t query_tid() {
	return (int64_t) syscall(SYS_gettid);
}
#else
#include <pthread.h>

static int64_t query_tid() {
// This is not a reliable way of getting the tid but it's better than nothing
    return (int64_t) pthread_self();
}
#endif

// A function for getting the id of the current process
#ifdef WIN32
static int64_t query_pid() {
    return (int64_t)GetCurrentProcessId();
}
#else
#include <unistd.h>

static int64_t query_pid() {
	return (int64_t) getpid();
}
#endif

// Cached accessors: the thread/process id never changes for the lifetime of a thread, but the
// underlying queries are real syscalls on Linux (SYS_gettid) and glibc (getpid, uncached since
// 2.25). A trace event is emitted for every TRACE_SCOPE, so querying these per event dominated the
// tracing overhead -- cache them in thread-local storage so each thread pays the syscall only once.
static int64_t get_tid() {
	thread_local const int64_t tid = query_tid();
	return tid;
}
static int64_t get_pid() {
	thread_local const int64_t pid = query_pid();
	return pid;
}

namespace {

using namespace tracing;

std::unique_ptr<ThreadedTraceEventWriter> traceEventWriter;
std::unique_ptr<ThreadedMainFrameTimer> mainFrameTimer;
std::unique_ptr<FrameProfiler> frameProfiler;

SCP_vector<int> query_objects;
// Free list for backends where queries are immediately reusable (OpenGL).
// When queries are NOT reusable (Vulkan), the free list is bypassed and
// handles are returned to the backend.
SCP_queue<int> free_query_objects;
bool do_gpu_queries = true;
bool queries_reusable = true;

int get_query_object() {
	if (!free_query_objects.empty()) {
		auto id = free_query_objects.front();
		free_query_objects.pop();
		return id;
	}

	auto id = gr_create_query_object();
	if (queries_reusable) {
		// Track for bulk cleanup at shutdown. When not reusable, the backend
		// owns the lifecycle — handles are returned via gr_delete_query_object
		// and the backend's own shutdown destroys the pool.
		query_objects.push_back(id);
	}
	return id;
}

int get_gpu_timestamp_query() {
	GR_DEBUG_SCOPE("Query tracing timestamp");

	auto query = get_query_object();
	gr_query_value(query, QueryType::Timestamp);

	return query;
}

void free_query_object(int obj) {
	if (queries_reusable) {
		free_query_objects.push(obj);
	} else {
		// Backend manages reset lifecycle internally — hand it back.
		gr_delete_query_object(obj);
	}
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

}

namespace tracing {
bool Profiler_overlay_enabled = false;
}

namespace {

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

	GR_DEBUG_SCOPE("Query GPU timestamps");

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
				UNREACHABLE("Invalid event type %d!", static_cast<int>(first.base_evt.type));
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
	// Seed the runtime profiler-overlay toggle from -profile_frame_time (kept for backward
	// compatibility); this also lazily constructs the FrameProfiler and folds
	// Cmdline_json_profiling/Cmdline_frame_profile into do_trace_events.
	// OR in Profiler_overlay_enabled rather than overwriting it outright: the "Game.ProfilerOverlay"
	// option's loadInitialValues() call (see OptionsManager) runs before tracing::init(), so by the
	// time we get here Profiler_overlay_enabled may already reflect a persisted "on" setting that
	// -profile_frame_time knows nothing about. Overwriting would silently disable the overlay on
	// startup until the option was re-toggled at runtime.
	set_frame_profiling_enabled(Cmdline_frame_profile || Profiler_overlay_enabled);

	do_gpu_queries = gr_is_capable(gr_capability::CAPABILITY_TIMESTAMP_QUERY);
	queries_reusable = gr_is_capable(gr_capability::CAPABILITY_QUERIES_REUSABLE);

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

const frame_overlay_snapshot& get_frame_profiler_overlay_snapshot() {
	Assertion(frameProfiler, "Frame profiling must be enabled for this function!");

	return frameProfiler->getOverlaySnapshot();
}

bool frame_profiling_active() {
	return Profiler_overlay_enabled;
}

void set_frame_profiling_enabled(bool enable) {
	Profiler_overlay_enabled = enable;

	if (enable && !frameProfiler) {
		frameProfiler.reset(new FrameProfiler());
	}

	do_trace_events = Cmdline_json_profiling || Cmdline_frame_profile || Profiler_overlay_enabled;
}

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
static auto ProfilerOverlayOption = options::OptionBuilder<bool>("Game.ProfilerOverlay",
	std::pair<const char*, int>{"Frame Profiler Overlay", 1932},
	std::pair<const char*, int>{"Show an ImGui overlay with a frametime graph and a breakdown of what's taking up frame time", 1933})
	.category(std::make_pair("Graphics", 1825))
	.level(options::ExpertLevel::Advanced)
	.default_func([]() { return Profiler_overlay_enabled; })
	.change_listener([](const bool& val, bool) {
		set_frame_profiling_enabled(val);
		return true;
	})
	.importance(69)
	.finish();

void shutdown() {
	if (queries_reusable) {
		while (!gpu_events.empty()) {
			process_events();

			// Don't do busy waiting...
			os_sleep(5);
		}
	} else {
		// Discard remaining GPU events — no more frames will
		// be submitted, so unsubmitted queries can never become
		// available.
		while (!gpu_events.empty()) {
			auto& first = gpu_events.front();
			gr_delete_query_object(first.gpu_begin_query);
			gr_delete_query_object(first.gpu_end_query);
			gpu_events.pop();
		}
	}

	for (auto query : query_objects) {
		gr_delete_query_object(query);
	}
	query_objects.clear();

	while (!free_query_objects.empty()) {
		free_query_objects.pop();
	}

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
