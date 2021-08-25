#include "tracing/Monitor.h"
#include "tracing/tracing.h"

using namespace tracing;

namespace tracing {

MonitorBase::MonitorBase(const char* name) : _name(name), _tracing_cat(name, false) {}
void MonitorBase::valueChanged(float newVal)
{
	tracing::counter::value(_tracing_cat, newVal);
}

RunningCounter::RunningCounter(Monitor<int>& monitor) : _monitor(monitor)
{
	_monitor += 1;
}
RunningCounter::~RunningCounter()
{
	_monitor -= 1;
}
RunningCounter::RunningCounter(const RunningCounter& other) : _monitor(other._monitor)
{
	_monitor += 1;
}
RunningCounter& RunningCounter::operator=(const RunningCounter&)
{
	return *this;
}
RunningCounter::RunningCounter(RunningCounter&& other) noexcept : _monitor(other._monitor)
{
	_monitor += 1;
}
RunningCounter& RunningCounter::operator=(RunningCounter&&) noexcept
{
	return *this;
}

} // namespace tracing
