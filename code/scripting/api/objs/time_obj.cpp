//
//
#include "time_obj.h"

namespace scripting {
namespace api {

ADE_OBJ(l_Timestamp, uint64_t, "timestamp", "A real time time stamp of unspecified precision and resolution.");

ADE_FUNC(__sub, l_Timestamp, "timestamp other", "Computes the difference between two timestamps", "timespan",
         "The time difference")
{
	uint64_t time  = 0;
	uint64_t other = 0;
	if (!ade_get_args(L, "oo", l_Timestamp.Get(&time), l_Timestamp.Get(&other))) {
		return ade_set_error(L, "o", l_TimeSpan.Set(0));
	}

	auto diff = static_cast<int64_t>(time) - static_cast<int64_t>(other);
	return ade_set_args(L, "o", l_TimeSpan.Set(diff));
}

ADE_OBJ(l_TimeSpan, int64_t, "timespan", "A difference between two time stamps");

ADE_FUNC(getSeconds, l_TimeSpan, nullptr, "Gets the value of this timestamp in seconds", "number",
         "The timespan value in seconds")
{
	int64_t value = 0;
	if (!ade_get_args(L, "o", l_TimeSpan.Get(&value))) {
		return ade_set_error(L, "f", 0.0f);
	}

	// Timestamps and spans are stored in nanoseconds
	return ade_set_args(L, "f", (float)((long double)value / (long double)NANOSECONDS_PER_SECOND));
}

} // namespace api
} // namespace scripting
