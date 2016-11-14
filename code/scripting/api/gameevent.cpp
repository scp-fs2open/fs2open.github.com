
#include "gameevent.h"


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
	char *n_name = NULL;
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
