#pragma once

#include "FredRenderer.h"
#include "EditorViewport.h"

#include <QObject>

#include <functional>
#include <memory>
#include <stdexcept>

#include <globalincs/globals.h>
#include <osapi/osapi.h>
#include <object/waypoint.h>

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
class Editor: public QObject {
 Q_OBJECT

 public:
	Editor();

	void unmark_all();

	void createNewMission();

	/*! Load a mission. */
	bool loadMission(const std::string& filepath, int flags = 0);

	void markObject(int objId);
	void unmarkObject(int objId);

	void selectObject(int objId);

	EditorViewport* createEditorViewport(os::Viewport* renderView);

	/* Schedules updates for all renderes */
	void updateAllViewports();

	int create_player(int num, vec3d* pos, matrix* orient, int type = -1, int init = 1);

	int create_ship(matrix* orient, vec3d* pos, int ship_type);

	int create_waypoint(vec3d* pos, int waypoint_instance);

	bool query_ship_name_duplicate(int ship);

	void fix_ship_name(int ship);

	int getNumMarked();

	void hideMarkedObjects();
	void showHiddenObjects();

	int dup_object(object* objp);

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

	/**
	 * @brief Signal which is emitted when the currently selected object changed
	 * @param new_obj The newly selected object index, may be -1 if no object is selected
	 */
	void currentObjectChanged(int new_obj);

	/**
	 * @brief A signal which is emitted if the marking status of an object changed
	 * @param obj The object which changed
	 * @param marked @c true if the object is now marked, @c false otherwise
	 */
	void objectMarkingChanged(int obj, bool marked);
 public:

	int Id_select_type_jump_node = 0;
	int Id_select_type_waypoint = 0;

	// object numbers for ships in a wing.
	int wing_objects[MAX_WINGS][MAX_SHIPS_PER_WING];

	int currentObject = -1;
	int cur_wing = -1;
	int cur_ship = -1;

	int cur_wing_index = -1;

	waypoint* cur_waypoint = nullptr;
	waypoint_list* cur_waypoint_list = nullptr;

	// Goober5000
	// This must be done when either the wing name or the custom name is changed.
	// (It's also duplicated in FS2, in post_process_mission, for setting the indexes at mission load.)
	void update_custom_wing_indexes();

	void ai_update_goal_references(int type, const char* old_name, const char* new_name);

	// Goober5000
	void update_texture_replacements(const char* old_name, const char* new_name);


	int set_reinforcement(const char* name, int state);

	/**
	 * @brief Forms a wing from marked objects
	 *
	 * @returns  0 If successful, or
	 * @returns -1 If an error occured
	 */
	int create_wing();

	/**
	 * @brief Mark all ships within this wing
	 *
	 * @param[in] wing Index of the wing to mark
	 */
	void mark_wing(int wing);

	bool query_single_wing_marked();

	/**
	 * @brief Delete a whole wing, leaving ships intact but wingless.
	 *
	 * @param[in] wing_num Index of the wing
	 */
	void remove_wing(int wing_num);

	void delete_marked();

	int delete_wing(int wing_num, int bypass = 0);

 private:
	void clearMission();

	void initialSetup();

	void setupCurrentObjectIndices(int obj);

	SCP_vector<std::unique_ptr<EditorViewport>> _viewports;
	EditorViewport* _lastActiveViewport = nullptr;

	int numMarked = 0;

	int Default_player_model = -1;

	int Shield_sys_teams[MAX_IFFS];
	int Shield_sys_types[MAX_SHIP_CLASSES];

	int delete_flag;

	bool already_deleting_wing = false;

	int common_object_delete(int obj);

	int reference_handler(const char* name, int type, int obj);

	int sexp_reference_handler(int node, int code, const char* msg);
	int orders_reference_handler(int code, char* msg);

	int delete_ship_from_wing(int ship);

	int invalidate_references(const char* name, int type);

	// DA 1/7/99 These ship names are not variables
	int rename_ship(int ship, char* name);

	void delete_reinforcement(int num);

	// changes the currently selected wing.  It is assumed that cur_wing == cur_ship's wing
	// number.  Don't call this if this won't be true, or else you'll screw things up.
	void set_cur_wing(int wing);

	/**
	 * @brief Checks wing dependencies
	 *
	 * @TODO verify
	 */
	int check_wing_dependencies(int wing_num);

	/**
	 * @brief Takes a player out of a wing, deleting wing if that was the only ship in it.
	 */
	void remove_player_from_wing(int player, int min = 1);

	/**
	 * @brief Takes a ship out of a wing, deleting the wing if that was the only ship in it.
	 *
	 * @param[in] ship Index of the ship to remove (Ships[i])
	 * @param[in] min  Minimum number of ships in a wing.
	 *   Pass a 0 to allow a wing to exist without any ships in it, or pass a value >1 to have the wing deleted when it has
	 *   this many members in it
	 */
	void remove_ship_from_wing(int ship, int min = 1);

	/**
	 * @brief Finds a free wing slot (i.e. unused)
	 */
	int find_free_wing();

	void generate_wing_weaponry_usage_list(int* arr, int wing);

	void generate_team_weaponry_usage_list(int team, int* arr);
};

} // namespace fred
} // namespace fso

extern char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];
extern char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];

