#pragma once

#include "FredRenderer.h"

#include <functional>
#include <memory>
#include <stdexcept>

#include <globalincs/globals.h>

#include <QObject>

namespace fso {
namespace fred {

enum class SubSystem {
	OS,
	CommandLine,
	Timer,
	CFile,
	Locale,
	Graphics,
	Fonts,
	Keyboard,
	Mouse,
	Particles,
	Iff,
	Objects,
	Species,
	MissionBrief,
	AI,
	AIProfiles,
	Armor,
	Weapon,
	Medals,
	Ships,
	Parse,
	Nebulas,
	Stars,
	View,
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
 * \param[in]   cfilepath   CFile root directory.
 * \param[in]   listener    A callback function called after each initializer.
 */
void initialize(const std::string& cfilepath,
				int argc,
				char* argv[],
				Editor* editor,
				InitializerCallback listener = [](const SubSystem&) {});

void shutdown();

struct mission_load_error: public std::runtime_error {
	mission_load_error(const char* msg) : std::runtime_error{ msg } {
	}
};

/*! Game editor.
 * Handles everything needed to edit the game,
 * without any knowledge of the actual GUI framework stack.
 *
 * Since SCP does not (yet) allow multiple simulations to run
 * simultaneously, this class should be treated as a singleton.
 *
 */
class Editor : public QObject {
	Q_OBJECT

 public:
	Editor();

	/*! Initialize renderer. */
	void initializeRenderer();

	/*! Handle resizing.
     *
     * \param[in]   width   New width.
     * \param[in]   height  New height.
     */
	void resize(int width, int height);

	/*! Update the game an renders a frame. */
	void update();

	void clearMission();

	void unmark_all();

	/*! Load a mission. */
	void loadMission(const std::string& filepath);

	int findFirstObjectUnder(int x, int y);

	void markObject(int objId);

	void selectObject(int objId);

	FredRenderer* renderer() {
		return m_renderer.get();
	}

	///! Non-copyable.
	Editor(const Editor&) = delete;
	///! Non-copyable.
	const Editor& operator=(const Editor&) = delete;

signals:
	/**
	 * @brief Signal for when a new mission has been loaded
	 * @param filepath The path of the mission file, empty if new mission
	 */
	void missionLoaded(const std::string& filepath);

	/**
	 * @brief Signal for when the editor wants to update the render window
	 */
	void scheduleUpdate();

 private:
	void resetPhysics();

	void setupCurrentObjectIndices(int obj);

	std::unique_ptr<FredRenderer> m_renderer;
	subsys_to_render Render_subsys;

	int currentObject = -1;
	int numMarked = 0;
};

} // namespace fred
} // namespace fso

extern char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];
extern char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];

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
