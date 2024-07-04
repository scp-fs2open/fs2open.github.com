#ifndef _TRACEEVENTWRITER_H
#define _TRACEEVENTWRITER_H
#pragma once

#include "globalincs/pstypes.h"
#include "tracing/tracing.h"

#include "tracing/ThreadedEventProcessor.h"

#include <fstream>

/** @file
 *  @ingroup tracing
 */

namespace tracing
{
class TraceEventWriter
{
	std::ofstream _out;
	bool _first_line = true;

public:
	TraceEventWriter();
	~TraceEventWriter();

	void processEvent(const trace_event* event);
};

typedef ThreadedEventProcessor<TraceEventWriter> ThreadedTraceEventWriter;
}

#endif // _TRACEEVENTWRITER_H
