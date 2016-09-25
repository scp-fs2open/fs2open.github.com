#pragma once

#include "globalincs/pstypes.h"
#include "tracing/tracing.h"

#include "utils/boost/syncboundedqueue.h"

#include <thread>


/** @file
 *  @ingroup tracing
 */

namespace tracing {

/**
 * @brief A multi-threaded event processor
 *
 * This is a utility class that can be used to implement an event processor that uses a different thread to process the
 * events. To use this, declare your class with a method with the signature
 *
 * @code{.cpp}
 * void processEvent(const trace_event* event)
 * @endcode
 *
 * This function will be called in a background-thread whenever a new event arrives.
 *
 * @tparam Processor Your processor implementation
 * @tparam QUEUE_SIZE The maximum size of the internal event buffer
 */
template<class Processor, size_t QUEUE_SIZE = 200>
class ThreadedEventProcessor {
	sync_bounded_queue<trace_event> _event_queue;

	std::thread _worker_thread;

	Processor _processor;

	void workerThread() {
		while (!_event_queue.closed()) {
			try {
				trace_event evt;
				auto status = _event_queue.wait_pull_front(evt);

				if (status != success) {
					break;
				}

				_processor.processEvent(&evt);
			}
			catch (const sync_queue_is_closed&) {
				// We are done here
				break;
			}
		}
	}
 public:
	template<typename... Params>
	explicit ThreadedEventProcessor(Params&& ... params)
		: _event_queue(QUEUE_SIZE), _worker_thread(&ThreadedEventProcessor<Processor>::workerThread, this),
		  _processor(std::forward<Params>(params)...) {}
	~ThreadedEventProcessor() {
		_event_queue.close();
		_worker_thread.join();
	}

	void processEvent(const trace_event* event) {
		try {
			_event_queue.wait_push_back(std::move(*event));
		} catch (const sync_queue_is_closed&) {
			mprintf(("Stream queue was closed in processEvent! This should not be possible..."));
		}
	}
};

}
