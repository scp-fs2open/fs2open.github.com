#pragma once

#include "globalincs/pstypes.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

#include <Rocket/Core.h>
#include <Rocket/Core/Plugin.h>

#pragma GCC diagnostic pop

#pragma pop_macro("Assert")

namespace scpui {

class SoundPropertyParser : public Rocket::Core::PropertyParser {
	bool ParseValue(Rocket::Core::Property& property, const Rocket::Core::String& value,
	                const Rocket::Core::ParameterMap& parameters) const override;
	void Release() override;
};

class SoundPlugin : public Rocket::Core::Plugin {
	class SoundEventHandler : public Rocket::Core::EventListener {
	  public:
		void ProcessEvent(Rocket::Core::Event& event) override;
	};

	std::unique_ptr<SoundEventHandler> _event_handler;

	float _last_scrollchange_time = -1.0f;

	static SoundPlugin* _instance;
	static SCP_vector<SCP_string> _event_types;

  public:
	SoundPlugin();
	~SoundPlugin() override;

	SoundPlugin(const SoundPlugin&) = delete;
	SoundPlugin& operator=(const SoundPlugin&) = delete;

	SoundPlugin(SoundPlugin&&) noexcept = delete;
	SoundPlugin& operator=(SoundPlugin&&) noexcept = delete;

	void OnShutdown() override;
	int GetEventClasses() override;

	void OnElementCreate(Rocket::Core::Element* element) override;
	void OnElementDestroy(Rocket::Core::Element* element) override;

	/**
	 * @brief Plays a element specific sound
	 *
	 * This can be used for implementing context specific sounds for specific elements where the actual sounds are
	 * defined by the interface designer in the RCSS.
	 *
	 * @param element The element to play the sound for
	 * @param event The event identification
	 * @param state The state for which the sound should be played. May be empty in which case the default sound is
	 * used.
	 * @return @c true if a sound was played, @c false otherwise
	 */
	bool PlayElementSound(Rocket::Core::Element* element, const Rocket::Core::String& event,
	                      const Rocket::Core::String& state = "");

	static const SCP_vector<SCP_string>& getEventTypes();
	static SoundPlugin* instance();
};

} // namespace scpui
