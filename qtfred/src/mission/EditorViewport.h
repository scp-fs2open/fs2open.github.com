#pragma once


#include "FredRenderer.h"
#include "Editor.h"
#include "IDialogProvider.h"

#include <object/object.h>

namespace fso {
namespace fred {

struct ViewSettings {
	bool Universal_heading = false;
	bool Show_stars = true;
	bool Show_horizon = false;
	bool Show_grid = true;
	bool Show_distances = true;
	bool Show_asteroid_field = true;
	bool Aa_gridlines = false;
	bool Show_coordinates = false;
	bool Show_outlines = false;
	bool Show_grid_positions = true;
	bool Show_dock_points = false;
	bool Show_starts = true;
	bool Show_ships = true;
	std::array<bool, MAX_IFFS> Show_iff;
	bool Show_ship_info = true;
	bool Show_ship_models = true;
	bool Show_paths_fred = false;
	bool Lighting_on = false;
	bool FullDetail = false;
	bool Show_waypoints = true;
	bool Show_compass = true;
	bool Move_ships_when_undocking = true;
	bool Highlight_selectable_subsys = false;

	ViewSettings();
};

class EditorViewport {
	std::unique_ptr<FredRenderer> _renderer; //!< Internal, owned pointer

	/**
	* A lot of this stuff doesn't belong here
	* @todo: Move camera stuff into own class
	*/
	int last_x = 0;
	int last_y = 0;

	matrix my_orient = vmd_identity_matrix;
	matrix trackball_orient = vmd_identity_matrix;
	matrix Last_eye_orient = vmd_identity_matrix;
	matrix Last_control_orient = vmd_identity_matrix;
	int Last_cursor_over = -1;

	int Flying_controls_mode = 1;

	fix lasttime = 0;

	bool inc_mission_time();
	void process_system_keys(int key);
	void process_controls(vec3d* pos, matrix* orient, float frametime, int key, int mode = 0);
	void level_object(matrix* orient);

	void initialSetup();
 public:
	enum {
		DUP_DRAG_OF_WING = 2
	};

	EditorViewport(Editor* in_editor, std::unique_ptr<FredRenderer>&& in_renderer);

	void needsUpdate();

	void resetView();

	void select_objects(const Marking_box& box);

	void resetViewPhysics();

	void game_do_frame(const int cur_object_index);

	void move_mouse(int btn, int mdx, int mdy);

	int object_check_collision(object* objp, vec3d* p0, vec3d* p1, vec3d* hitpos);

	int select_object(int cx, int cy);


	// viewpoint -> attach camera to current ship.
	// cur_obj -> ship viewed.
	void level_controlled();
	void verticalize_controlled();

	void drag_rotate_save_backup();

	int create_object_on_grid(int x, int y, int waypoint_instance);

	int	create_object(vec3d *pos, int waypoint_instance = -1);

	int duplicate_marked_objects();
	int drag_objects(int x, int y);

	int drag_rotate_objects(int mouse_dx, int mouse_dy);

	void view_universe(bool just_marked);

	void view_object(int obj_num);

	vec3d Last_eye_pos;

	vec3d eye_pos;
	vec3d Last_control_pos = vmd_zero_vector;
	vec3d my_pos;
	matrix eye_orient;
	control_info view_controls;

	ViewSettings view;

	int Cursor_over = -1;
	CursorMode Editing_mode = CursorMode::Moving;

	matrix view_orient = vmd_identity_matrix;
	vec3d view_pos;
	physics_info view_physics;
	grid* The_grid;

	int physics_speed = 1;
	int physics_rot = 25;

	vec3d Constraint;
	vec3d Anticonstraint;
	bool Single_axis_constraint = false;

	int viewpoint = 0;
	int view_obj = -1;

	int Control_mode = 0;

	bool Selection_lock = false;

	bool button_down = false;
	int on_object = -1;
	int Dup_drag = 0;

	int cur_model_index = 0;

	bool Bg_bitmap_dialog = false;

	object_orient_pos rotation_backup[MAX_OBJECTS];

	vec3d saved_cam_pos = vmd_zero_vector;
	matrix saved_cam_orient;

	vec3d original_pos = vmd_zero_vector;

	bool moved = false;

	int Duped_wing;

	bool Group_rotate = true;
	bool Lookat_mode = false;

	Editor* editor = nullptr;
	FredRenderer* renderer = nullptr;
	IDialogProvider* dialogProvider = nullptr;
};

}
}



