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
 * @brief Process if used for GPU events
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
	const Scope* scope = nullptr;
	EventType type = EventType::Invalid;

	std::uint64_t timestamp = 0;
	std::uint64_t duration = 0;

	std::int64_t tid = -1;
	std::int64_t pid = -1;

	float value = -1.f;
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
