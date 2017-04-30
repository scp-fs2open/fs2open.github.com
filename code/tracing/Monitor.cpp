#include "tracing/Monitor.h"
#include "tracing/tracing.h"

using namespace tracing;

namespace tracing {

MonitorBase::MonitorBase(const char* name) : _name(name), _tracing_cat(name, false) {
}
void MonitorBase::valueChanged(float newVal) {
	tracing::counter::value(_tracing_cat, newVal);
}

}
