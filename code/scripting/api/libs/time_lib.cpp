//
//

#include "time_lib.h"
#include "io/timer.h"
#include "scripting/api/objs/time_obj.h"

namespace scripting {
namespace api {

ADE_LIB(l_Time, "Time", "time", "Real-Time library");

ADE_FUNC(getCurrentTime, l_Time, nullptr, "Gets the current real-time timestamp.", "timestamp", "The current time")
{
	return ade_set_args(L, "o", l_Timestamp.Set(timer_get_nanoseconds()));
}

} // namespace api
} // namespace scripting
