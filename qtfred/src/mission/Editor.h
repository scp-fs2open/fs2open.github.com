#pragma once

#include "EditorViewport.h"
#include "FredRenderer.h"

#include <globalincs/globals.h>
#include <jumpnode/jumpnode.h>
#include <object/waypoint.h>
#include <osapi/osapi.h>
#include <ship/ship.h>

#include <QObject>
#include <QString>
#include <QTimer>
#include <QUndoStack>
#include <functional>
#include <memory>
#include <stdexcept>

namespace fso::fred {

struct subsys_to_render {
	bool do_render = false;
	object* ship_obj = nullptr;
	ship_subsys* cur_subsys = nullptr;
};

enum class WingNameError {
	None,
	Empty,
	TooLong,
	DuplicateWing,
	DuplicateShip,
	DuplicateTargetPriority,
	DuplicateWaypointList,
	DuplicateJumpNode,
};

struct WingNameCheck {
	bool ok;
	WingNameError error;
	std::string message; // human-readable for dialogs
};

enum class GlobalShieldStatus {
	HasShields,
	NoShields,
	MixedShields
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

	void clean_up_selections();

	void unmark_all();

	void createNewMission();

	void maybeUseAutosave(std::string& filepath);

	void startAutosaveTimer(int intervalSeconds);
	void stopAutosaveTimer();
	void setCurrentMissionPath(const QString& path);
	const QString& autosaveDirectory() const { return _autosaveDirectory; }

	void setUndoStack(QUndoStack* stack) { _undoStack = stack; }
	QUndoStack* undoStack() const { return _undoStack; }

	/*! Load a mission. */
	bool loadMission(const std::string& filepath, int flags = 0);

	void markObject(int objId);
	void unmarkObject(int objId);

	void selectObject(int objId);

	EditorViewport* createEditorViewport(os::Viewport* renderView);

	/* Schedules updates for all renderes */
	void updateAllViewports();

	int create_player(vec3d* pos, matrix* orient, int type = -1);

	int create_ship(matrix* orient, vec3d* pos, int ship_type);

	int create_waypoint(vec3d* pos, int waypoint_instance);

	bool query_ship_name_duplicate(int ship);

	void fix_ship_name(int ship);

	int getNumMarked();

	void hideMarkedObjects();
	void showHiddenObjects();

	void lockMarkedObjects();
	void unlockAllObjects();

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

	/*! Emit layerVisibilityChanged — called by EditorViewport after toggling a layer. */
	void notifyLayerVisibilityChanged() { layerVisibilityChanged(); }

	/*! Emit layerStructureChanged — called by EditorViewport when layers are added/removed or objects move between layers. */
	void notifyLayerStructureChanged() { layerStructureChanged(); }

	/*! Emit layerListChanged — called by EditorViewport when layer names are added/removed/reloaded. */
	void notifyLayerListChanged() { layerListChanged(); }

  signals:
	/**
	 * @brief Emitted when the autosave timer fires; receiver performs the actual file save.
	 * @param savePath Absolute path for the autosave file
	 */
	void autosaveDue(const QString& savePath);

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

	/**
	 * @brief A signal emitted when a layer's visibility has been toggled
	 */
	void layerVisibilityChanged();

	/**
	 * @brief A signal emitted when the layer list changes (add/remove) or an object moves between layers
	 */
	void layerStructureChanged();

	/**
	 * @brief A signal emitted when the layer name list itself changes (add/remove/reload)
	 */
	void layerListChanged();

  public:
	// object numbers for ships in a wing.
	int wing_objects[MAX_WINGS][MAX_SHIPS_PER_WING];

	int currentObject = -1;
	int cur_wing = -1;
	int cur_ship = -1;

	// When set, reference_handler() proceeds as if the user confirmed "delete
	// it anyway" instead of prompting. Set (via AutoConfirmReferences in
	// FredCommands.cpp) while replaying delete commands in undo/redo: the user
	// already consented at the original action, and a cancelable modal inside
	// QUndoStack::undo()/redo() would desync the stack from the mission state.
	bool auto_confirm_reference_deletion = false;

	waypoint* cur_waypoint = nullptr;
	waypoint_list* cur_waypoint_list = nullptr;

	subsys_to_render Render_subsys;

	void ai_update_goal_references(sexp_ref_type type, const char* old_name, const char* new_name);

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

	bool wing_is_player_wing(int);

	static bool wing_contains_player_start(int);

	static WingNameCheck validate_wing_name(const SCP_string& new_name, int ignore_wing = -1);

	bool rename_wing(int wing, const SCP_string& new_name, bool rename_members = true);

	// DA 1/7/99 These ship names are not variables
	int rename_ship(int ship, const char* name);

	void rename_jump_node(int objNum, const char* name);
	void rename_prop(int objNum, const char* name);
	void rename_waypoint_list(const char* old_name, const char* new_name);

	/**
	 * @brief Delete a whole wing, leaving ships intact but wingless.
	 *
	 * @param[in] wing_num Index of the wing
	 *
	 * @return 0 on success, nonzero if the user canceled at the reference check
	 */
	int disband_wing(int wing_num);

	void delete_marked();

	int delete_wing(int wing_num, int bypass = 0);

	void select_next_subsystem();
	void select_previous_subsystem();
	void cancel_select_subsystem();

	void select_next_object();
	void select_previous_object();

	static SCP_vector<SCP_string> get_docking_list(int model_index);

	void exportShieldSysData(SCP_vector<GlobalShieldStatus>& teams, SCP_vector<GlobalShieldStatus>& types) const;
	void importShieldSysData(const SCP_vector<GlobalShieldStatus>& teams, const SCP_vector<GlobalShieldStatus>& types);
	void normalizeShieldSysData();

	static void strip_quotation_marks(SCP_string& str);
	static void pad_with_newline(SCP_string& str, size_t max_size);
	static SCP_string get_display_name_for_text_box(const SCP_string &orig_name);

	SCP_vector<int> getStartingWingLoadoutUseCounts();

	static const ai_goal_list* getAi_goal_list();
	static int getAigoal_list_size();

	void generate_team_weaponry_usage_list(int team, int* arr);

  private slots:
	void performTimedAutosave();

  private: // NOLINT(readability-redundant-access-specifiers)
	QTimer*      _autosaveTimer    = nullptr;
	QUndoStack*  _undoStack        = nullptr;
	QString  _autosaveDirectory;
	QString  _currentMissionPath;

	void clearMission(bool fast_reload = false);

	void initialSetup();

	void setupCurrentObjectIndices(int obj);

	SCP_vector<std::unique_ptr<EditorViewport>> _viewports;
	EditorViewport* _lastActiveViewport = nullptr;

	int numMarked = 0;

	SCP_vector<GlobalShieldStatus> Shield_sys_teams;
	SCP_vector<GlobalShieldStatus> Shield_sys_types;

	bool already_deleting_wing = false;

	// ship and weapon usage pools
	int _ship_usage[MAX_TVT_TEAMS][MAX_SHIP_CLASSES];
	int _weapon_usage[MAX_TVT_TEAMS][MAX_WEAPON_TYPES];

	int common_object_delete(int obj);

	int reference_handler(const char* name, sexp_ref_type type, int obj);

	int sexp_reference_handler(int node, sexp_src source, int source_index, const char* msg);
	int orders_reference_handler(sexp_src source, int source_index, char* msg);

	int delete_ship_from_wing(int ship);

	int invalidate_references(const char* name, sexp_ref_type type);

	void delete_reinforcement(const char* name);

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
	 *   Pass a 0 to allow a wing to exist without any ships in it, or pass a value >1 to have the wing deleted when it
	 * has this many members in it
	 */
	void remove_ship_from_wing(int ship, int min = 1);

	/**
	 * @brief Finds a free wing slot (i.e. unused)
	 */
	static int find_free_wing();

	void generate_wing_weaponry_usage_list(int* arr, int wing);

	void generate_ship_usage_list(int* arr, int wing);

	int get_visible_sub_system_count(ship* shipp);

	int get_next_visible_subsys(ship* shipp, ship_subsys** next_subsys);

	int get_prev_visible_subsys(ship* shipp, ship_subsys** prev_subsys);

	void updateStartingWingLoadoutUseCounts();
};

} // namespace fso::fred

extern char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];
extern char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];
