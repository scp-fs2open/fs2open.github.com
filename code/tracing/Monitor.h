#pragma once

#include <type_traits>

#include "tracing/categories.h"

namespace tracing {

class MonitorBase {
 protected:
	const char* _name;
	Category _tracing_cat;

	void valueChanged(float newVal);

 public:
	MonitorBase(const char* name);

	// Disallow any copy or movement
	MonitorBase(const MonitorBase&) = delete;
	MonitorBase& operator=(const MonitorBase&) = delete;

	MonitorBase(MonitorBase&&) = delete;
	MonitorBase& operator=(MonitorBase&&) = delete;
};

template<typename T>
class Monitor: public MonitorBase {
	T _value;

	static_assert(std::is_convertible<T, float>::value, "Monitor values must be convertible to float!");
 public:
	Monitor(const char* name, const T& defaultVal) : MonitorBase(name), _value(defaultVal) {
	}

	void changeValue(const T& val) {
		_value = val;
		valueChanged(_value);
	}

	Monitor<T>& operator=(const T& val) {
		changeValue(val);
		return *this;
	}

	Monitor<T>& operator+=(const T& val) {
		_value += val;
		valueChanged((float)_value);

		return *this;
	}

	Monitor<T>& operator-=(const T& val) {
		_value -= val;
		valueChanged((float)_value);

		return *this;
	}
};

/**
 * @brief Class that keeps track of how many operations are currently running
 */
class RunningCounter {
	Monitor<int>& _monitor;

  public:
	explicit RunningCounter(Monitor<int>& monitor);
	~RunningCounter();

	RunningCounter(const RunningCounter&);
	RunningCounter& operator=(const RunningCounter&);

	RunningCounter(RunningCounter&&) noexcept;
	RunningCounter& operator=(RunningCounter&&) noexcept;
};

} // namespace tracing

// Creates a monitor variable
#define MONITOR(function_name)				static ::tracing::Monitor<int> mon_##function_name(#function_name, 0);

// Increments a monitor variable
#define MONITOR_INC(function_name, inc)		do { mon_##function_name += (inc); } while(false)


