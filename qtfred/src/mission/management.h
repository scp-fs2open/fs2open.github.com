#pragma once

#include <functional>

#include "Editor.h"

namespace fso {
namespace fred {

enum class SubSystem {
	OS,
	CommandLine,
	Timer,
	CFile,
	ModTable,
	Locale,
	Sound,
	Graphics,
	Scripting,
	Fonts,
	Keyboard,
	Mouse,
	Particles,
	Iff,
	Objects,
	Models,
	Species,
	BriefingIcons,
	HudCommOrders,
	AlphaColors,
	GameSound,
	MissionBrief,
	AI,
	AIProfiles,
	Armor,
	Weapon,
	Medals,
	Glowpoints,
	Ships,
	Parse,
	TechroomIntel,
	Nebulas,
	Stars,
	Ssm,
	EventMusic,
	FictionViewer,
	CommandBriefing,
	Cutscenes,
	Mainhalls,
	Ranks,
	Campaign,
	NebulaLightning,
	FFmpeg,
	DynamicSEXPs,
	ScriptingInitHook,
};

typedef std::function<void(const SubSystem&)> InitializerCallback;

class Editor;

/*! Initialize fs2open subsystems needed by the editor.
 *
 * The user may provide a callback function which will be called after
 * each initialization.
 * This enable the developer to provide information about the startup
 * sequence.
 *
 * @param[in]   cfilepath   CFile root directory.
 * @param[in]   listener    A callback function called after each initializer.
 *
 * @return @c true if initialization was successfull, @c false otherwise
 */
bool initialize(const std::string& cfilepath,
				int argc,
				char* argv[],
				Editor* editor,
				const InitializerCallback& listener = [](const SubSystem&) {});

void shutdown();

struct mission_load_error: public std::runtime_error {
	mission_load_error(const char* msg);
};

}
}

// Define hash function for Initialized.
namespace std {
template<>
struct hash<fso::fred::SubSystem> {
	typedef fso::fred::SubSystem argument_type;
	typedef std::size_t result_type;

	result_type operator()(argument_type const& s) const {
		return static_cast<result_type>(s);
	}
};

}
