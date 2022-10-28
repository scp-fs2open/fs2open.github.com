
#include "gameevent.h"

#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"

namespace scripting {
namespace api {
gameevent_h::gameevent_h() {
	edx = -1;
}

gameevent_h::gameevent_h(int n_event) {
	edx = n_event;
}

bool gameevent_h::IsValid() {
	return (edx > -1 && edx < Num_gs_event_text);
}

int gameevent_h::Get() {
	return edx;
}

void gameevent_h::serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size) {
	gameevent_h event;
	value.getValue(l_GameEvent.Get(&event));
	ADD_INT(event.edx);
}

void gameevent_h::deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset) {
	int index;
	GET_INT(index);
	new(data_ptr) gameevent_h(index);
}

ADE_OBJ(l_GameEvent, gameevent_h, "gameevent", "Game event");

ADE_FUNC(__tostring, l_GameEvent, NULL, "Game event name", "string", "Game event name, or empty string if handle is invalid")
{
	gameevent_h *gh = NULL;
	if (!ade_get_args(L, "o", l_GameEvent.GetPtr(&gh)))
		return ade_set_error(L, "s", "");

	if (!gh->IsValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", GS_event_text[gh->Get()]);
}

ADE_VIRTVAR(Name, l_GameEvent, "string", "Game event name", "string", "Game event name, or empty string if handle is invalid")
{
	gameevent_h *gh = NULL;
	const char* n_name = nullptr;
	if (!ade_get_args(L, "o|s", l_GameEvent.GetPtr(&gh), &n_name))
		return ade_set_error(L, "s", "");

	if (!gh->IsValid())
		return ade_set_error(L, "s", "");

	int edx = gh->Get();

	if (ADE_SETTING_VAR)
	{
		Error(LOCATION, "Can't set game event names at this time");
	}

	return ade_set_args(L, "s", GS_event_text[edx]);
}

}
}
