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

namespace fso::fred {

const float LOLLIPOP_SIZE = 2.5f;

enum class CursorMode {
	Selecting, Moving, Rotating
};

class Editor;
class EditorViewport;
struct ViewSettings;
struct Marking_box;
struct subsys_to_render;

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
	~FredRenderer() override;

	os::Viewport* getTargetViewport() const { return _targetView; }

	void setViewport(EditorViewport* viewport);

	void resize(int width, int height);

	void render_grid(grid* gridp);
	void display_distances();
	void display_ship_info(int cur_object_index);
	void cancel_display_active_ship_subsystem(subsys_to_render& Render_subsys);
	void display_active_ship_subsystem(subsys_to_render& Render_subsys, int cur_object_index);

	void render_model_x_htl(vec3d* pos, grid* gridp, int col_scheme = 0);
	void render_compass();
	void render_one_model_htl(object* objp, int cur_object_index);
	void render_models(int cur_object_index);
	void render_frame(int cur_object_index,
					  subsys_to_render& Render_subsys,
					  bool box_marking,
					  const Marking_box& marking_box);

 signals:
	void scheduleUpdate();
};

} // namespace fso::fred
