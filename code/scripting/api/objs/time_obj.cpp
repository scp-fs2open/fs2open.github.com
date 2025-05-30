//
//
#include "time_obj.h"
#include "globalincs/pstypes.h"

namespace scripting {
namespace api {

ADE_OBJ(l_Timestamp, uint64_t, "timestamp", "A real-time timestamp of unspecified precision and resolution.");

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

ADE_FUNC(__tostring, l_Timestamp, nullptr, "Converts a timestamp to a string", "string", "Timestamp as string, or empty string if handle is invalid")
{
	uint64_t value = 0;
	if (!ade_get_args(L, "o", l_Timestamp.Get(&value)))
		return ade_set_error(L, "s", "");

	char buf[32];
	sprintf(buf, UINT64_T_ARG, value);

	return ade_set_args(L, "s", buf);
}

ADE_FUNC(getSeconds, l_Timestamp, nullptr, "Gets the value of this timestamp in seconds", "number", "The timestamp value in seconds")
{
	uint64_t value = 0;
	if (!ade_get_args(L, "o", l_Timestamp.Get(&value)))
		return ade_set_error(L, "f", 0.0f);

	// Timestamps and spans are stored in nanoseconds
	return ade_set_args(L, "f", static_cast<float>(static_cast<long double>(value) / static_cast<long double>(NANOSECONDS_PER_SECOND)));
}


ADE_OBJ(l_TimeSpan, int64_t, "timespan", "A difference between two time stamps");

ADE_FUNC(__tostring, l_TimeSpan, nullptr, "Converts a timespan to a string", "string", "Timespan as string, or empty string if handle is invalid")
{
	int64_t value = 0;
	if (!ade_get_args(L, "o", l_TimeSpan.Get(&value)))
		return ade_set_error(L, "s", "");

	char buf[32];
	sprintf(buf, INT64_T_ARG, value);

	return ade_set_args(L, "s", buf);
}

ADE_FUNC(getSeconds, l_TimeSpan, nullptr, "Gets the value of this timespan in seconds", "number",
         "The timespan value in seconds")
{
	int64_t value = 0;
	if (!ade_get_args(L, "o", l_TimeSpan.Get(&value))) {
		return ade_set_error(L, "f", 0.0f);
	}

	// Timestamps and spans are stored in nanoseconds
	return ade_set_args(L, "f", static_cast<float>(static_cast<long double>(value) / static_cast<long double>(NANOSECONDS_PER_SECOND)));
}

} // namespace api
} // namespace scripting
