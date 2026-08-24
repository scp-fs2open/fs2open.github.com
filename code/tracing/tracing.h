#ifndef _TRACING_H
#define _TRACING_H
#pragma once

#include "globalincs/pstypes.h"
#include "tracing/categories.h"
#include "tracing/scopes.h"

/**
 * @defgroup tracing The Tracing API
 *
 * The tracing system provides functions for gathering tracing data of engine events.
 */

/** @file
 *  @ingroup tracing
 */

namespace tracing {

/**
 * @brief Process id used for GPU events
 */
const std::int64_t GPU_PID = std::numeric_limits<std::int64_t>::min();

/**
 * @brief Possible types of events
 */
enum class EventType {
	Invalid,

	Complete, Begin, End,

	AsyncBegin, AsyncStep, AsyncEnd,

	Counter
};

/**
 * @brief Data of a trace event
 */
struct trace_event {
	const Category* category = nullptr;
	// Category::getId() at record time. A trace_event can outlive the Category it names -- it may
	// sit in a buffer (FrameProfiler) or cross a thread (ThreadedEventProcessor) before it is
	// read, and a category made through the Lua API tracing_category can be gone by then. Code
	// that reads a trace_event after the point where it was recorded must use category_id with
	// Category::getNameById(), never dereference category.
	int category_id = -1;
	const Scope* scope = nullptr;
	EventType type = EventType::Invalid;

	std::uint64_t timestamp = 0;
	std::uint64_t duration = 0;

	std::uint64_t event_id = 0;
	std::uint64_t end_event_id = 0;

	std::int64_t tid = -1;
	std::int64_t pid = -1;

	float value = -1.f;
};

/**
 * A single named contributor to a frame's total traced time, used by the ImGui frame profiler
 * overlay's pie chart.
 */
struct frame_overlay_contributor {
	SCP_string name;
	uint64_t self_nanosec;
};

/**
 * Cap on frame_overlay_snapshot::top_contributors. The single place this is defined, so the
 * producer (FrameProfiler::build_overlay_snapshot) and every consumer (currently the overlay's
 * stacked frame-budget bar) can't drift apart on how many contributors get their own slot before
 * folding into "Other".
 */
constexpr size_t FRAME_OVERLAY_MAX_CONTRIBUTORS = 5;

/**
 * A structured, per-frame snapshot of profiling data meant for the ImGui overlay (as opposed to
 * get_frame_profile_output()'s preformatted text dump). top_contributors holds up to
 * FRAME_OVERLAY_MAX_CONTRIBUTORS categories by self-time, sorted descending; everything else is
 * folded into other_nanosec. total_nanosec is the sum of every sample's self-time
 * (top_contributors + other_nanosec). All times are in nanoseconds, matching
 * trace_event::timestamp/duration (timer_get_nanoseconds()).
 */
struct frame_overlay_snapshot {
	bool valid = false;
	uint64_t total_nanosec = 0;
	SCP_vector<frame_overlay_contributor> top_contributors;
	uint64_t other_nanosec = 0;
};

/**
 * @brief Initializes the tracing subsystem
 */
void init();

/**
 * @brief Should be called regularly to process GPU events
 */
void process_events();

/**
 * @brief Folds this frame's buffered trace events into the overlay snapshot and clears the buffer
 *
 * This is the only thing that drains the profiler's event buffer, so it must run once per
 * presented frame for as long as collection is enabled — gr_flip() drives it (via
 * profiler_overlay_frame()) for exactly that reason. No-op when profiling is off.
 */
void frame_profile_process_frame();

/**
 * @brief Gets the output of the frame profiler.
 * @return The frame profiler output
 */
SCP_string get_frame_profile_output();

/**
 * @brief Gets a structured snapshot of the current frame's profiling data, for the ImGui overlay.
 * @return The frame profiler overlay snapshot
 */
const frame_overlay_snapshot& get_frame_profiler_overlay_snapshot();

/**
 * @brief True if frame profiling data is currently being collected
 *
 * This is the single source of truth for "is the frame profiler on" — every consumer (the
 * overlay, the lab's text dump, osapi's ImGui input forwarding) asks here rather than
 * re-deriving it from the command line or the options system.
 */
bool frame_profiling_active();

/**
 * @brief Enables or disables frame profiling collection at runtime
 *
 * Constructs the profiler on enable and destroys it on disable; that lifetime *is*
 * frame_profiling_active(). Driven by the "Frame Profiler Overlay" option and seeded from
 * -profile_frame_time at startup.
 */
void set_frame_profiling_enabled(bool enable);

/**
 * @brief Deinitializes the tracing subsystem
 */
void shutdown();

namespace complete {
/**
 * @brief Starts a complete event
 * @param category The category this event belongs to
 * @param evt The event which hold the data
 */
void start(const Category& category, trace_event* evt);

/**
 * @brief Ends and submits a complete event
 *
 * @warning Must be called from the same thread as the start function
 *
 * @param evt The event to submit
 */
void end(trace_event* evt);

/**
 * @brief Class for tracing a scope with a complete event
 */
class ScopedCompleteEvent {
	trace_event _evt;

 public:
	explicit ScopedCompleteEvent(const Category& category) {
		start(category, &_evt);
	}
	~ScopedCompleteEvent() {
		end(&_evt);
	}
};
}

namespace async {
/**
 * @brief Begins an asynchronous event
 *
 * @note Events can be submitted from multiple threads
 *
 * @param category The category of the event
 * @param async_scope The scope of the event
 */
void begin(const Category& category, const Scope& async_scope);

/**
 * @brief Steps an asynchronous event
 *
 * @note Events can be submitted from multiple threads
 *
 * @param category The category of the event
 * @param async_scope The scope of the event
 */
void step(const Category& category, const Scope& async_scope);

/**
 * @brief Ends an asynchronous event
 *
 * @note Events can be submitted from multiple threads
 *
 * @param category The category of the event
 * @param async_scope The scope of the event
 */
void end(const Category& category, const Scope& async_scope);
}

namespace counter {

/**
 * @brief Records the new value of a counter event
 * @param category The event category
 * @param value The new value of the category
 */
void value(const Category& category, float value);

}

}

#define TRACE_SCOPE(category) ::tracing::complete::ScopedCompleteEvent SCP_TOKEN_CONCAT(complete_trace_scope, __LINE__)(category)

#endif //_TRACING_H
