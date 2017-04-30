
#include "tracing/TraceEventWriter.h"

#include <iomanip>

namespace
{
using namespace tracing;

const char* getTypeStr(tracing::EventType type) {
	switch(type) {
		case tracing::EventType::Complete:
			return "X";
		case tracing::EventType::Begin:
			return "B";
		case tracing::EventType::End:
			return "E";
		case EventType::AsyncBegin:
			return "b";
		case EventType::AsyncStep:
			return "n";
		case EventType::AsyncEnd:
			return "e";
		case EventType::Counter:
			return "C";
		default: 
			Assertion(false, "Invalid enum value!");
			return "";
	}
}

void writeTime(std::ofstream& out, std::uint64_t time) {
	// Save stream state
	auto flags = out.flags();
	out << std::fixed << std::setprecision(3);

	out << (time / 1000.);

	// and now restore it
	out.flags(flags);
}

void writeCompleteEvent(std::ofstream& out_str, const trace_event* evt) {
	Assertion(evt->type == EventType::Complete, "Event must be a complete event!");

	out_str << ",\"dur\":";
	writeTime(out_str, evt->duration);
}
}

namespace tracing
{

TraceEventWriter::TraceEventWriter() : _out("tracing/trace.json") {
	_out << "[";
}

TraceEventWriter::~TraceEventWriter() {
	_out << "]\n";
	_out.close();
}

void TraceEventWriter::processEvent(const trace_event* event) {
	if (event->type == EventType::Complete) {
		if (event->duration < 1000) {
			// Discard events that are less than a millisecond long
			return;
		}
	}

	if (!_first_line) {
		_out << ",";
	}
	_out << "\n{\"tid\": " << event->tid << ",\"ts\":";

	writeTime(_out, event->timestamp);

	_out << ",\"pid\":";
	if (event->pid == GPU_PID) {
		_out << "\"GPU\"";
	}
	else {
		_out << event->pid;
	}

	if (event->scope != nullptr) {
		_out << ",\"cat\":\"" << event->scope->getName() << "\"";
		_out << ",\"id\":\"" << reinterpret_cast<const void*>(event->scope) << "\"";
	}

	_out << ",\"name\":\"" << event->category->getName() << "\",\"ph\":\"" << getTypeStr(event->type) << "\"";

	switch (event->type) {
		case EventType::Complete:
			writeCompleteEvent(_out, event);
			break;
		case EventType::Begin:
		case EventType::End:
			// Nothing to do here
			break;
		case EventType::AsyncBegin:
		case EventType::AsyncStep:
		case EventType::AsyncEnd:
			// Nothing to do here...
			break;
		case EventType::Counter: {
			auto flags = _out.flags();
			_out << std::fixed;

			_out << ",\"args\": {\"value\": " << event->value << "}";

			// and now restore it
			_out.flags(flags);
			break;
		}
		default:
			Assertion(false, "Unhandled enum value! This function should not have been called with this value!");
			break;
	}

	_out << "}";

	_first_line = false;
}
}
