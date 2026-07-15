#pragma once


#include "CameraController.h"
#include "FredRenderer.h"
#include "Editor.h"
#include "IDialogProvider.h"

#include <object/object.h>

namespace fso::fred {

struct Marking_box {
	int x1 = 0;
	int y1 = 0;
	int x2 = 0;
	int y2 = 0;
};

enum class CreateKind {
	Ship,
	Prop,
	Other,
};

enum class OtherKind {
	Waypoint,
	JumpNode,
};

struct ViewSettings {
	bool Universal_heading = false;
	bool Show_stars = true;
	bool Show_horizon = false;
	bool Show_grid = true;
	bool Show_distances = true;
	bool Show_coordinates = false;
	bool Show_outlines = false;
	bool Draw_outlines_on_selected_ships = true;
	bool Draw_outline_at_warpin_position = false;
	bool Show_grid_positions = true;
	bool Show_dock_points = false;
	bool Show_bay_paths = false;
	bool Show_starts = true;
	bool Show_ships = true;
	SCP_vector<bool> Show_iff;
	bool Show_ship_info = true;
	bool Show_ship_models = true;
	bool Show_paths_fred = false;
	bool Lighting_on = false;
	bool FullDetail = false;
	bool Show_waypoints = true;
	bool Show_props = true;
	bool Show_jump_nodes = true;
	bool Show_compass = true;
	bool Highlight_selectable_subsys = false;
	int Outline_lod = 1;

	ViewSettings();
};

class EditorViewport {
	std::unique_ptr<FredRenderer> _renderer; //!< Internal, owned pointer

	int Last_cursor_over = -1;

	void process_system_keys();
	void level_object(matrix* orient);

	void initialSetup();

	SCP_vector<SCP_string> _layerNames;
	SCP_vector<bool> _layerVisibility;
	std::unordered_map<int, size_t> _objectLayers;

	size_t getLayerIndex(const SCP_string& name) const;
	size_t getObjectLayerIndex(int objectIndex) const;
	bool isLayerVisible(size_t layerIndex) const;
	void syncMissionLayerNames() const;
	void setObjectLayerByIndex(int objectIndex, size_t layerIndex);

 public:
	class ViewportControlLock {
	  public:
		explicit ViewportControlLock(EditorViewport* viewport);
		~ViewportControlLock();

		ViewportControlLock(const ViewportControlLock&) = delete;
		ViewportControlLock& operator=(const ViewportControlLock&) = delete;

		ViewportControlLock(ViewportControlLock&& other) noexcept;
		ViewportControlLock& operator=(ViewportControlLock&& other) noexcept;

	  private:
		EditorViewport* _viewport = nullptr;
	};

	static const char* DefaultLayerName;

	enum {
		DUP_DRAG_OF_WING = 2,
		// Ctrl+Shift+drag.  Same as a normal Ctrl+drag duplicate, except marked
		// waypoints insert a copy into their source path rather than starting a
		// new path.  Non-waypoint object types behave like a plain duplicate.
		DUP_DRAG_INSERT = 3,
	};

	EditorViewport(Editor* in_editor, std::unique_ptr<FredRenderer>&& in_renderer);

	void needsUpdate();
	bool areControlsLocked() const;
	[[nodiscard]] ViewportControlLock acquireControlLock();

	void reset();

	void select_objects(const Marking_box& box);

	void game_do_frame(const int cur_object_index);

	vec3d orbitCameraGetPivot();

	int object_check_collision(object* objp, vec3d* p0, vec3d* p1, vec3d* hitpos);

	int select_object(int cx, int cy);

	SCP_vector<SCP_string> getLayerNames() const;
	bool addLayer(const SCP_string& name, SCP_string* errorMessage = nullptr);
	bool deleteLayer(const SCP_string& name, SCP_string* errorMessage = nullptr);
	bool setLayerVisibility(const SCP_string& name, bool visible, SCP_string* errorMessage = nullptr);
	bool getLayerVisibility(const SCP_string& name, bool* visible, SCP_string* errorMessage = nullptr) const;
	void showAllLayers();
	int getHiddenLayerCount() const;
	void reloadLayersFromMission();

	SCP_string getObjectLayerName(int objectIndex) const;
	bool moveObjectToLayer(int objectIndex, const SCP_string& layerName, SCP_string* errorMessage = nullptr);
	void moveMarkedObjectsToLayer(const SCP_string& layerName, SCP_string* errorMessage = nullptr);

	// Sync the _objectLayers map for a freshly-created object by reading its
	// per-object fred_layer string (or the parent waypoint list's, for waypoints).
	// Call this after creating a new ship/prop/jump-node/waypoint outside of
	// the normal mission-load path (e.g. when duplicating objects).
	void registerObjectInLayer(int objectIndex);

	bool isObjectVisibleInLayer(const object* objp) const;


	// viewpoint -> attach camera to current ship.
	// cur_obj -> ship viewed.
	void level_controlled();
	void verticalize_controlled();

	void drag_rotate_save_backup();

	int create_object_on_grid(int x, int y, int waypoint_instance);
	int create_object_on_grid(int x, int y, int waypoint_instance, CreateKind kind);

	int	create_object(vec3d *pos, int waypoint_instance = -1, CreateKind kind = CreateKind::Ship);

	vec3d getCreatePosition(int x, int y, float fallbackDist);
	int createShipAtScreenPos(int x, int y, int modelIndex);
	int createPropAtScreenPos(int x, int y, int propIndex);
	int createWaypointAtScreenPos(int x, int y, int waypoint_instance = -1);
	int createJumpNodeAtScreenPos(int x, int y);

	// When `insert_waypoints` is true, marked waypoints get a new waypoint
	// inserted into their source path (right after the source waypoint) instead
	// of being duplicated into a fresh path.  Other object types are duplicated
	// either way.  Triggered from Ctrl+Shift+drag in the viewport.
	int duplicate_marked_objects(bool insert_waypoints = false);
	int drag_objects(int x, int y);

	int drag_rotate_objects(int mouse_dx, int mouse_dy);
	void cancel_drag();

	void view_universe(bool just_marked);

	void view_object(int obj_num);

	CameraController camera;

	ViewSettings view;

	int Cursor_over = -1;
	CursorMode Editing_mode = CursorMode::Moving;

	grid* The_grid;

	vec3d Constraint;
	vec3d Anticonstraint;
	bool Single_axis_constraint = false;

	bool Selection_lock = false;

	bool button_down = false;
	int on_object = -1;
	int Dup_drag = 0;

	int cur_model_index = 0;
	int cur_prop_index = -1;
	OtherKind cur_other_kind = OtherKind::Waypoint;

	object_orient_pos rotation_backup[MAX_OBJECTS];

	vec3d original_pos = vmd_zero_vector;

	bool moved = false;

	int Duped_wing;

	bool Group_rotate = true;
	int  toolbar_icon_size = 24;  ///< Toolbar icon size in pixels (16, 24, or 32)
	int  sexp_number_every_n = 5; ///< Show a numbered badge on every Nth argument in sexp trees (0 = disabled)
	bool Offer_autosave_recovery   = true;
	int  autosave_interval_seconds = 300;  // 5 minutes; 0 = disabled
	bool Create_bak_on_save        = true;
	bool Move_ships_when_undocking = true;
	bool Always_save_display_names = false;
	bool Error_checker_checks_potential_issues = true;
	bool Error_checker_apply_auto_corrections = true;
	// One-shot override: when set, the next auto-run of the error checker shows
	// the dialog and forces potential issues on regardless of the user's saved
	// preference. Consumed (cleared) by autoRunErrorChecker. Not persisted.
	bool Error_checker_force_display_potentials_once = false;

	bool Show_sexp_help_mission_events = true;
	bool Show_sexp_help_mission_goals = true;
	bool Show_sexp_help_mission_cutscenes = true;
	bool Show_sexp_help_ship_editor = false;
	bool Show_sexp_help_wing_editor = false;

	bool Dark_mode = false;

	void saveSettings() const;

	Editor* editor = nullptr;
	FredRenderer* renderer = nullptr;
	IDialogProvider* dialogProvider = nullptr;

private:
	fix _lasttime = 0;
	vec3d Last_control_pos = vmd_zero_vector;
	matrix Last_control_orient = vmd_identity_matrix;
	int _controlLockCount = 0;

	bool incMissionTime();
	void loadSettings();

	void lockControls();
	void unlockControls();
};

} // namespace fso::fred
