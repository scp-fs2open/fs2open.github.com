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
	int x1 = 0;
	int y1 = 0;
	int x2 = 0;
	int y2 = 0;
};

namespace fso {
namespace fred {

const float LOLLIPOP_SIZE = 2.5f;

enum class CursorMode {
	Selecting, Moving, Rotating
};

class Editor;
class EditorViewport;
struct ViewSettings;

class FredRenderer: public QObject {
 Q_OBJECT

	SCP_vector<int> rendering_order;
	int Fred_outline = 0;

	EditorViewport* _viewport = nullptr;
	os::Viewport* _targetView = nullptr;

	FredRenderer(const FredRenderer& other) = delete;
	FredRenderer& operator=(const FredRenderer& other) = delete;

	ViewSettings& view();

 public:
	explicit FredRenderer(os::Viewport* targetView);
	~FredRenderer();

	void setViewport(EditorViewport* viewport);

	void resize(int width, int height);

	void render_grid(grid* gridp);
	void hilight_bitmap();
	void display_distances();
	void display_ship_info(int cur_object_index);
	void cancel_display_active_ship_subsystem(subsys_to_render& Render_subsys);
	void display_active_ship_subsystem(subsys_to_render& Render_subsys, int cur_object_index);

	void render_model_x_htl(vec3d* pos, grid* gridp, int col_scheme = 0);
	void render_compass();
	void draw_orient_sphere2(int col, object* obj, int r, int g, int b);
	void draw_orient_sphere(object* obj, int r, int g, int b);
	void render_model_x(vec3d* pos, grid* gridp, int col_scheme = 0);
	void render_one_model_htl(object* objp, int cur_object_index, bool Bg_bitmap_dialog);
	void render_models(int cur_object_index, bool Bg_bitmap_dialog);
	void render_frame(int cur_object_index,
					  subsys_to_render& Render_subsys,
					  bool box_marking,
					  const Marking_box& marking_box,
					  bool Bg_bitmap_dialog);

 signals:
	void scheduleUpdate();
};
}
}
