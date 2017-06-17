#pragma once

#include "FredRenderer.h"
#include "EditorViewport.h"

#include <functional>
#include <memory>
#include <stdexcept>

#include <globalincs/globals.h>

#include <QObject>
#include <osapi/osapi.h>

namespace fso {
namespace fred {


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

	void unmark_all();

	void createNewMission();

	/*! Load a mission. */
	void loadMission(const std::string& filepath);

	void markObject(int objId);
	void unmarkObject(int objId);

	void selectObject(int objId);

	EditorViewport* createEditorViewport(os::Viewport* renderView);

	/* Schedules updates for all renderes */
	void updateAllViewports();

	int create_player(int num, vec3d *pos, matrix *orient, int type = -1, int init = 1);

	int create_ship(matrix* orient, vec3d* pos, int ship_type);

	bool query_ship_name_duplicate(int ship);

	void fix_ship_name(int ship);

	int getCurrentObject();

	void hideMarkedObjects();
	void showHiddenObjects();

	///! Non-copyable.
	Editor(const Editor&) = delete;
	///! Non-copyable.
	const Editor& operator=(const Editor&) = delete;

public slots:
	/*! Update the game but doesn't render anything. */
	void update();

signals:
	/**
	 * @brief Signal for when a new mission has been loaded
	 * @param filepath The path of the mission file, empty if new mission
	 */
	void missionLoaded(const std::string& filepath);

	/**
	 * @brief A signal emitted when the mission has changed somehow
	 */
	void missionChanged();

 private:
	void clearMission();

	void initialSetup();

	void setupCurrentObjectIndices(int obj);

	SCP_vector<std::unique_ptr<EditorViewport>> _viewports;

	int currentObject = -1;
	int numMarked = 0;

	int Default_player_model = -1;

	int Shield_sys_teams[MAX_IFFS];
	int Shield_sys_types[MAX_SHIP_CLASSES];
};

} // namespace fred
} // namespace fso

extern char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];
extern char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];

