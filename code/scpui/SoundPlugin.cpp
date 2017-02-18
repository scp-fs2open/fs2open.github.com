//
//

#include "SoundPlugin.h"
#include "gamesnd/gamesnd.h"
#include "utils/string_utils.h"

using namespace Rocket::Core;

namespace {

struct SoundDefinition {
	interface_snd_id default_id;

	SCP_unordered_map<SCP_string, interface_snd_id> state_ids;
};

SoundDefinition parse_sound_value(const String& value)
{
	SoundDefinition def{};

	bool saw_default = false;
	for (auto& part : util::split_string(SCP_string(value.CString()), ',')) {
		drop_white_space(part);
		if (part.find(')') != SCP_string::npos) {
			auto state_start = part.find('(');
			auto state_end   = part.find(')');

			if (state_start == SCP_string::npos || state_end == SCP_string::npos) {
				throw std::runtime_error("Could not find both '(' and ')' for state sound!");
			}

			state_start += 1; // skip the (

			if (state_start > state_end) {
				throw std::runtime_error("')' appeared before '('!");
			}
			SCP_string state_name = part.substr(state_start, state_end - state_start);
			if (state_name.empty()) {
				throw std::runtime_error("State name is empty!");
			}

			auto sound_name = part.substr(state_end + 1);
			drop_white_space(sound_name);

			auto snd = gamesnd_get_by_iface_name(sound_name.c_str());
			if (!snd.isValid()) {
				SCP_string error("Sound '");
				error += sound_name;
				error += "' for state '";
				error += state_name;
				error += "' is invalid!";
				throw std::runtime_error(error);
			}

			def.state_ids.emplace(state_name, snd);
		} else {
			if (saw_default) {
				throw std::runtime_error("Multiple default sounds specified!");
			}

			def.default_id = gamesnd_get_by_iface_name(part.c_str());
			saw_default    = true;

			if (!def.default_id.isValid()) {
				throw std::runtime_error("Default sound has an invalid name!");
			}
		}
	}

	return def;
}
}

namespace scpui {

bool SoundPropertyParser::ParseValue(Property& property, const String& value, const ParameterMap& /*parameters*/) const
{
	try {
		parse_sound_value(value);
	} catch (const std::exception& e) {
		mprintf(("libRocket: Failure to parse sound CSS property '%s': %s\n", value.CString(), e.what()));
		return false;
	}

	property.value = Variant(value);
	property.unit  = Property::STRING;
	return true;
}
void SoundPropertyParser::Release() { delete this; }

int SoundPlugin::GetEventClasses() { return EVT_BASIC | EVT_ELEMENT; }

SoundPlugin* SoundPlugin::_instance              = nullptr;
SCP_vector<SCP_string> SoundPlugin::_event_types = {"click",   "dblclick",  "mouseover",   "mouseout",
                                                    "mouseup", "mousedown", "mousescroll", "scrollchange"};
const SCP_vector<SCP_string>& SoundPlugin::getEventTypes() { return _event_types; }

void SoundPlugin::OnShutdown() { delete this; }
void SoundPlugin::OnElementCreate(Element* element)
{
	for (auto& type : getEventTypes()) {
		element->AddEventListener(type.c_str(), _event_handler.get());
	}
}
void SoundPlugin::OnElementDestroy(Element* /*element*/) {}
SoundPlugin::SoundPlugin()
{
	_event_handler.reset(new SoundEventHandler());
	_instance = this;
}
SoundPlugin::~SoundPlugin() { _instance = nullptr; }
SoundPlugin* SoundPlugin::instance() { return _instance; }

bool SoundPlugin::PlayElementSound(Element* element, const String& event, const String& state)
{
	auto property_name = event + "-sound";

	const Property* sound_prop = element->GetProperty(property_name);

	if (!(sound_prop->unit & Property::STRING)) {
		// sounds are stored as their original string values which are then parsed whenever necessary
		return false;
	}

	auto sound = sound_prop->Get<String>();

	try {
		auto snds = parse_sound_value(sound);

		interface_snd_id sound_id;

		if (state.Empty()) {
			sound_id = snds.default_id;
		} else {
			auto snd_iter = snds.state_ids.find(SCP_string(state.CString()));

			if (snd_iter == snds.state_ids.end()) {
				return false;
			}

			sound_id = snd_iter->second;
		}

		// The default sound is optional so this might be invalid
		if (!sound_id.isValid()) {
			return false;
		}

		if (event == "scrollchange") {
			if (_last_scrollchange_time >= 0) {
				auto diff = Rocket::Core::GetSystemInterface()->GetElapsedTime() - _last_scrollchange_time;

				if (diff < gamesnd_get_max_duration(gamesnd_get_interface_sound(sound_id)) / 1000.0f * 1.1f) {
					return false;
				}
			}
			_last_scrollchange_time = Rocket::Core::GetSystemInterface()->GetElapsedTime();
		}

		gamesnd_play_iface(sound_id);

		return true;
	} catch (const std::exception& e) {
		mprintf(("libRocket: Failure to parse sound CSS property '%s': %s\n", sound.CString(), e.what()));
		return false;
	}
}

void SoundPlugin::SoundEventHandler::ProcessEvent(Event& event)
{
	SoundPlugin::instance()->PlayElementSound(event.GetCurrentElement(), event.GetType());
}
} // namespace scpui
