
#include "MainFrameTimer.h"

namespace tracing {

MainFrameTimer::MainFrameTimer() : _out("profiling.csv") {
}
MainFrameTimer::~MainFrameTimer() {
	_out.close();
}
void MainFrameTimer::processEvent(const trace_event* event) {
	if (event->scope != &MainFrameScope || event->category != &MainFrame) {
		return;
	}

	switch(event->type) {
		case EventType::AsyncBegin:
			_begin_time = event->timestamp;
			break;
		case EventType::AsyncEnd:
		{
			auto end = event->timestamp;
			auto duration = event->timestamp - _begin_time;

			_out << end << ";" << duration << "\n";
			break;
		}
		default:
			// Ignore everything else
			return;
	}
}

}
