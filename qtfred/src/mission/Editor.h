#pragma once

#include "FredRenderer.h"
#include "EditorViewport.h"

#include <QObject>

#include <functional>
#include <memory>
#include <stdexcept>

#include <globalincs/globals.h>

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

	int create_waypoint(vec3d *pos, int waypoint_instance);

	bool query_ship_name_duplicate(int ship);

	void fix_ship_name(int ship);

	int getCurrentObject();

	int getNumMarked();

	void hideMarkedObjects();
	void showHiddenObjects();

	int	dup_object(object *objp);

	int delete_object(int obj);

	void setActiveViewport(EditorViewport* viewport);

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
 public:

	int Id_select_type_jump_node = 0;
	int Id_select_type_waypoint = 0;

	// object numbers for ships in a wing.
	int wing_objects[MAX_WINGS][MAX_SHIPS_PER_WING];

 private:
	void clearMission();

	void initialSetup();

	void setupCurrentObjectIndices(int obj);

	SCP_vector<std::unique_ptr<EditorViewport>> _viewports;
	EditorViewport* _lastActiveViewport = nullptr;

	int currentObject = -1;
	int numMarked = 0;

	int Default_player_model = -1;

	int Shield_sys_teams[MAX_IFFS];
	int Shield_sys_types[MAX_SHIP_CLASSES];

	int delete_flag;

	int cur_wing = -1;

	bool already_deleting_wing = false;

	int common_object_delete(int obj);

	int	reference_handler(const char *name, int type, int obj);

	int sexp_reference_handler(int node, int code, const char *msg);
	int orders_reference_handler(int code, char* msg);

	int delete_ship_from_wing(int ship);

	int invalidate_references(const char *name, int type);

	void ai_update_goal_references(int type, const char *old_name, const char *new_name);

	// Goober5000
	void update_texture_replacements(const char *old_name, const char *new_name);

	// DA 1/7/99 These ship names are not variables
	int rename_ship(int ship, char *name);

	void delete_reinforcement(int num);

	int delete_wing(int wing_num, int bypass);

	// changes the currently selected wing.  It is assumed that cur_wing == cur_ship's wing
	// number.  Don't call this if this won't be true, or else you'll screw things up.
	void set_cur_wing(int wing);

	/**
	 * @brief Checks wing dependencies
	 *
	 * @TODO verify
	 */
	int check_wing_dependencies(int wing_num);

	// Goober5000
// This must be done when either the wing name or the custom name is changed.
// (It's also duplicated in FS2, in post_process_mission, for setting the indexes at mission load.)
	void update_custom_wing_indexes();
};

} // namespace fred
} // namespace fso

extern char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];
extern char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];

