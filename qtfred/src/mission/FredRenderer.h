#pragma once

#include <QtCore/QObject>

#include <physics/physics.h>
#include <mission/missiongrid.h>
#include <osapi/osapi.h>
#include <globalincs/pstypes.h>
#include <globalincs/globals.h>

#include <array>

class object;
class ship_subsys;

///! \fixme This does NOT belong here. Used for porting and testing purposes ONLY!
struct subsys_to_render {
	bool do_render;
	object* ship_obj;
	ship_subsys* cur_subsys;
};

///! \fixme does NOT belong here.
struct Marking_box {
	int x1, y1, x2, y2;
};

namespace fso {
namespace fred {

enum class CursorMode {
	Selecting,
	Moving,
	Rotating
};

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

	ViewSettings();
};

class Editor;

class FredRenderer : public QObject {
	Q_OBJECT

	/**
     * A lot of this stuff doesn't belong here
     * @todo: Move camera stuff into own class
     */
	int last_x = 0;
	int last_y = 0;

	vec3d Last_eye_pos;

	vec3d eye_pos;
	vec3d Last_control_pos = vmd_zero_vector;
	vec3d my_pos;
	matrix eye_orient;
	control_info view_controls;

	SCP_vector<int> rendering_order;
	int Last_cursor_over = -1;
	int Control_mode = 0;
	bool Group_rotate = true;
	bool Lookat_mode = false;
	int Flying_controls_mode = 1;
	int Fred_outline = 0;

	matrix my_orient = vmd_identity_matrix;
	matrix trackball_orient = vmd_identity_matrix;
	matrix Last_eye_orient = vmd_identity_matrix;
	matrix Last_control_orient = vmd_identity_matrix;

	fix lasttime = 0;

	Editor* _editor = nullptr;
	os::Viewport* _targetView = nullptr;

	FredRenderer(const FredRenderer& other) = delete;
	FredRenderer& operator=(const FredRenderer& other) = delete;

 public:
	explicit FredRenderer(Editor* editor, os::Viewport* targetView);
	~FredRenderer();

	void resize(int width, int height);

	void resetView();

	void inc_mission_time();
	void move_mouse(int btn, int mdx, int mdy);
	void process_system_keys(int key);
	void process_controls(vec3d* pos, matrix* orient, float frametime, int key, int mode = 0);
	void game_do_frame(const int view_obj, const int viewpoint, const int cur_object_index);
	void render_grid(grid* gridp);
	void hilight_bitmap();
	void display_distances();
	void
	display_ship_info(int cur_object_index);
	void cancel_display_active_ship_subsystem(subsys_to_render& Render_subsys);
	void display_active_ship_subsystem(subsys_to_render& Render_subsys, int cur_object_index);

	void render_model_x_htl(vec3d* pos, grid* gridp, int col_scheme = 0);
	void render_compass();
	void draw_orient_sphere2(int col, object* obj, int r, int g, int b);
	void draw_orient_sphere(object* obj, int r, int g, int b);
	void render_model_x(vec3d* pos, grid* gridp, int col_scheme = 0);
	void render_one_model_htl(object* objp,
							  int cur_object_index,
							  bool Bg_bitmap_dialog);
	void render_models(int cur_object_index,
					   bool Bg_bitmap_dialog);
	void render_frame(int cur_object_index,
					  subsys_to_render& Render_subsys,
					  bool box_marking,
					  const Marking_box& marking_box,
					  bool Bg_bitmap_dialog);
	int object_check_collision(object* objp,
							   vec3d* p0,
							   vec3d* p1,
							   vec3d* hitpos);
	int select_object(int cx,
					  int cy,
					  bool Selection_lock);
	void level_object(matrix* orient);
	// viewpoint -> attach camera to current ship.
	// cur_obj -> ship viewed.
	void level_controlled(const int viewpoint, const int cur_obj);
	void verticalize_controlled(const int viewpoint, const int cur_obj);

	ViewSettings view;

	int Cursor_over = -1;
	CursorMode Editing_mode = CursorMode::Moving;

	matrix view_orient = vmd_identity_matrix;
	vec3d view_pos;
	physics_info view_physics;
	grid* The_grid;

	vec3d Constraint = {{ 1.0f, 0.0f, 1.0f }};
	vec3d Anticonstraint = {{ 0.0f, 1.0f, 0.0f }};
	bool Single_axis_constraint = false;

signals:
	void scheduleUpdate();
};
}
}
