
#include "event.h"
#include "scripting/ade_args.h"
#include "scripting/ade.h"
#include "mission/missiongoals.h"

namespace scripting {
namespace api {

//**********HANDLE: event
ADE_OBJ(l_Event, int, "event", "Mission event handle");

ADE_VIRTVAR(Name, l_Event, "string", "Mission event name", "string", NULL)
{
	int idx;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Event.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= (int)Mission_events.size())
		return ade_set_error(L, "s", "");

	mission_event *mep = &Mission_events[idx];

	if (ADE_SETTING_VAR && s != nullptr) {
		mep->name = s;
	}

	return ade_set_args(L, "s", mep->name.c_str());
}

ADE_VIRTVAR(DirectiveText, l_Event, "string", "Directive text", "string", NULL)
{
	int idx;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Event.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= (int)Mission_events.size())
		return ade_set_error(L, "s", "");

	mission_event *mep = &Mission_events[idx];

	if (ADE_SETTING_VAR && s != nullptr) {
		mep->objective_text = s;
	}

	return ade_set_args(L, "s", mep->objective_text.c_str());
}

ADE_VIRTVAR(DirectiveKeypressText, l_Event, "string", "Raw directive keypress text, as seen in FRED.", "string", NULL)
{
	int idx;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Event.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= (int)Mission_events.size())
		return ade_set_error(L, "s", "");

	mission_event *mep = &Mission_events[idx];

	if (ADE_SETTING_VAR && s != nullptr) {
		mep->objective_key_text = s;
	}

	return ade_set_args(L, "s", mep->objective_key_text.c_str());
}

ADE_VIRTVAR(Interval, l_Event, "number", "Time for event to repeat (in seconds)", "number", "Repeat time, or 0 if invalid handle")
{
	int idx;
	int newinterval = 0;
	if (!ade_get_args(L, "o|i", l_Event.Get(&idx), &newinterval))
		return ade_set_error(L, "i", 0);

	if (idx < 0 || idx >= (int)Mission_events.size())
		return ade_set_error(L, "i", 0);

	mission_event *mep = &Mission_events[idx];

	if (ADE_SETTING_VAR) {
		mep->interval = newinterval;
	}

	return ade_set_args(L, "i", mep->interval);
}

ADE_VIRTVAR(ObjectCount, l_Event, "number", "Number of objects left for event", "number", "Repeat count, or 0 if invalid handle")
{
	int idx;
	int newobject = 0;
	if (!ade_get_args(L, "o|i", l_Event.Get(&idx), &newobject))
		return ade_set_error(L, "i", 0);

	if (idx < 0 || idx >= (int)Mission_events.size())
		return ade_set_error(L, "i", 0);

	mission_event *mep = &Mission_events[idx];

	if (ADE_SETTING_VAR) {
		mep->count = newobject;
	}

	return ade_set_args(L, "i", mep->count);
}

ADE_VIRTVAR(RepeatCount, l_Event, "number", "Event repeat count", "number", "Repeat count, or 0 if invalid handle")
{
	int idx;
	int newrepeat = 0;
	if (!ade_get_args(L, "o|i", l_Event.Get(&idx), &newrepeat))
		return ade_set_error(L, "i", 0);

	if (idx < 0 || idx >= (int)Mission_events.size())
		return ade_set_error(L, "i", 0);

	mission_event *mep = &Mission_events[idx];

	if (ADE_SETTING_VAR) {
		mep->repeat_count = newrepeat;
	}

	return ade_set_args(L, "i", mep->repeat_count);
}

ADE_VIRTVAR(Score, l_Event, "number", "Event score", "number", "Event score, or 0 if invalid handle")
{
	int idx;
	int newscore = 0;
	if (!ade_get_args(L, "o|i", l_Event.Get(&idx), &newscore))
		return ade_set_error(L, "i", 0);

	if (idx < 0 || idx >= (int)Mission_events.size())
		return ade_set_error(L, "i", 0);

	mission_event *mep = &Mission_events[idx];

	if (ADE_SETTING_VAR) {
		mep->score = newscore;
	}

	return ade_set_args(L, "i", mep->score);
}

ADE_FUNC(isValid, l_Event, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if (!ade_get_args(L, "o", l_Event.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= (int)Mission_events.size())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}


}
}

