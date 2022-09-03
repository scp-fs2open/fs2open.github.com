//
//

#include "time_lib.h"
#include "io/timer.h"
#include "scripting/api/objs/time_obj.h"

namespace scripting {
namespace api {

ADE_LIB(l_Time, "Time", "time", "Real-Time library");

ADE_FUNC(getCurrentTime, l_Time, nullptr, "Gets the current real-time timestamp, i.e. the actual elapsed time, regardless of whether the game has changed time compression or been paused.", "timestamp", "The current time")
{
	return ade_set_args(L, "o", l_Timestamp.Set(timer_get_nanoseconds()));
}

ADE_FUNC(getCurrentMissionTime, l_Time, nullptr, "Gets the current mission-time timestamp, which can be affected by time compression and paused.", "timestamp", "The current mission time")
{
	return ade_set_args(L, "o", l_Timestamp.Set(timestamp_get_mission_time_in_microseconds() * NANOSECONDS_PER_MICROSECOND));
}

} // namespace api
} // namespace scripting
