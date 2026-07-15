#include <globalincs/linklist.h>
#include <globalincs/systemvars.h>
#include <io/timer.h>
#include <object/object.h>
#include <render/3d.h>
#include <ship/ship.h>
#include "ui/ControlBindings.h"

#include "object.h"

#include "EditorViewport.h"
#include <QSettings>
#include <math/fvi.h>
#include <jumpnode/jumpnode.h>
#include <mission/missionparse.h>
#include <prop/prop.h>
#include <FredApplication.h>

namespace {

constexpr auto SETTINGS_GROUP = "Preferences";

const float REDUCER = 100.0f;

void align_vector_to_axis(vec3d* v) {
	float x, y, z;

	x = v->xyz.x;
	if (x < 0) {
		x = -x;
	}

	y = v->xyz.y;
	if (y < 0) {
		y = -y;
	}

	z = v->xyz.z;
	if (z < 0) {
		z = -z;
	}

	if ((x > y) && (x > z)) { // x axis
		if (v->xyz.x < 0) // negative x
			vm_vec_make(v, -1.0f, 0.0f, 0.0f);
		else // positive x
			vm_vec_make(v, 1.0f, 0.0f, 0.0f);
	} else if (y > z) { // y axis
		if (v->xyz.y < 0) // negative y
			vm_vec_make(v, 0.0f, -1.0f, 0.0f);
		else // positive y
			vm_vec_make(v, 0.0f, 1.0f, 0.0f);
	} else { // z axis
		if (v->xyz.z < 0) // negative z
			vm_vec_make(v, 0.0f, 0.0f, -1.0f);
		else // positive z
			vm_vec_make(v, 0.0f, 0.0f, 1.0f);
	}
}
void verticalize_object(matrix* orient) {
	align_vector_to_axis(&orient->vec.fvec);
	align_vector_to_axis(&orient->vec.uvec);
	align_vector_to_axis(&orient->vec.rvec);
	vm_fix_matrix(orient); // just in case something odd occurs.
}

}

namespace fso::fred {

const char* EditorViewport::DefaultLayerName = "Default";

EditorViewport::ViewportControlLock::ViewportControlLock(EditorViewport* viewport) : _viewport(viewport)
{
	if (_viewport != nullptr) {
		_viewport->lockControls();
	}
}

EditorViewport::ViewportControlLock::~ViewportControlLock()
{
	if (_viewport != nullptr) {
		_viewport->unlockControls();
	}
}

EditorViewport::ViewportControlLock::ViewportControlLock(ViewportControlLock&& other) noexcept
	: _viewport(other._viewport)
{
	other._viewport = nullptr;
}

EditorViewport::ViewportControlLock& EditorViewport::ViewportControlLock::operator=(
	ViewportControlLock&& other) noexcept
{
	if (this == &other) {
		return *this;
	}

	if (_viewport != nullptr) {
		_viewport->unlockControls();
	}

	_viewport = other._viewport;
	other._viewport = nullptr;
	return *this;
}

EditorViewport::EditorViewport(Editor* in_editor, std::unique_ptr<FredRenderer>&& in_renderer) :
	_renderer(std::move(in_renderer)), editor(in_editor) {
	renderer = _renderer.get();

	_renderer->setViewport(this);

	vm_vec_make(&Constraint, 1.0f, 0.0f, 1.0f);
	vm_vec_make(&Anticonstraint, 0.0f, 1.0f, 0.0f);
	reset();

	_layerNames.emplace_back(DefaultLayerName);
	_layerVisibility.push_back(true);
	syncMissionLayerNames();

	loadSettings();

	fredApp->runAfterInit([this]() { initialSetup(); });
}

void EditorViewport::loadSettings() {
	QSettings settings;
	settings.beginGroup(SETTINGS_GROUP);
	toolbar_icon_size                  = settings.value("toolbar_icon_size",                  toolbar_icon_size).toInt();
	sexp_number_every_n                = settings.value("sexp_number_every_n",                sexp_number_every_n).toInt();
	Offer_autosave_recovery            = settings.value("offer_autosave_recovery",            Offer_autosave_recovery).toBool();
	autosave_interval_seconds         = settings.value("autosave_interval_seconds",          autosave_interval_seconds).toInt();
	Create_bak_on_save                 = settings.value("create_bak_on_save",                 Create_bak_on_save).toBool();
	Move_ships_when_undocking          = settings.value("move_ships_when_undocking",          Move_ships_when_undocking).toBool();
	Always_save_display_names          = settings.value("always_save_display_names",          Always_save_display_names).toBool();
	Error_checker_checks_potential_issues = settings.value("error_checker_checks_potential_issues", Error_checker_checks_potential_issues).toBool();
	Error_checker_apply_auto_corrections  = settings.value("error_checker_apply_auto_corrections",  Error_checker_apply_auto_corrections).toBool();
	Show_sexp_help_mission_events      = settings.value("show_sexp_help_mission_events",      Show_sexp_help_mission_events).toBool();
	Show_sexp_help_mission_goals       = settings.value("show_sexp_help_mission_goals",       Show_sexp_help_mission_goals).toBool();
	Show_sexp_help_mission_cutscenes   = settings.value("show_sexp_help_mission_cutscenes",   Show_sexp_help_mission_cutscenes).toBool();
	Show_sexp_help_ship_editor         = settings.value("show_sexp_help_ship_editor",         Show_sexp_help_ship_editor).toBool();
	Show_sexp_help_wing_editor         = settings.value("show_sexp_help_wing_editor",         Show_sexp_help_wing_editor).toBool();
	Dark_mode                          = settings.value("dark_mode",                          Dark_mode).toBool();

	view.Universal_heading                 = settings.value("view_universal_heading",                 view.Universal_heading).toBool();
	view.Show_stars                        = settings.value("view_show_stars",                        view.Show_stars).toBool();
	view.Show_horizon                      = settings.value("view_show_horizon",                      view.Show_horizon).toBool();
	view.Show_grid                         = settings.value("view_show_grid",                         view.Show_grid).toBool();
	view.Show_distances                    = settings.value("view_show_distances",                    view.Show_distances).toBool();
	view.Show_coordinates                  = settings.value("view_show_coordinates",                  view.Show_coordinates).toBool();
	view.Show_outlines                     = settings.value("view_show_outlines",                     view.Show_outlines).toBool();
	view.Draw_outlines_on_selected_ships   = settings.value("view_draw_outlines_on_selected_ships",   view.Draw_outlines_on_selected_ships).toBool();
	view.Draw_outline_at_warpin_position   = settings.value("view_draw_outline_at_warpin_position",   view.Draw_outline_at_warpin_position).toBool();
	view.Show_grid_positions               = settings.value("view_show_grid_positions",               view.Show_grid_positions).toBool();
	view.Show_dock_points                  = settings.value("view_show_dock_points",                  view.Show_dock_points).toBool();
	view.Show_bay_paths                    = settings.value("view_show_bay_paths",                    view.Show_bay_paths).toBool();
	view.Show_starts                       = settings.value("view_show_starts",                       view.Show_starts).toBool();
	view.Show_ships                        = settings.value("view_show_ships",                        view.Show_ships).toBool();
	view.Show_ship_info                    = settings.value("view_show_ship_info",                    view.Show_ship_info).toBool();
	view.Show_ship_models                  = settings.value("view_show_ship_models",                  view.Show_ship_models).toBool();
	view.Show_paths_fred                   = settings.value("view_show_paths_fred",                   view.Show_paths_fred).toBool();
	view.Lighting_on                       = settings.value("view_lighting_on",                       view.Lighting_on).toBool();
	view.FullDetail                        = settings.value("view_full_detail",                       view.FullDetail).toBool();
	view.Show_waypoints                    = settings.value("view_show_waypoints",                    view.Show_waypoints).toBool();
	view.Show_compass                      = settings.value("view_show_compass",                      view.Show_compass).toBool();
	view.Highlight_selectable_subsys       = settings.value("view_highlight_selectable_subsys",       view.Highlight_selectable_subsys).toBool();
	view.Outline_lod                       = settings.value("view_outline_lod",                       view.Outline_lod).toInt();
	camera.setInvertOrbitX(settings.value("camera_invert_orbit_x", camera.getInvertOrbitX()).toBool());
	camera.setInvertOrbitY(settings.value("camera_invert_orbit_y", camera.getInvertOrbitY()).toBool());
	settings.endGroup();
}

void EditorViewport::saveSettings() const {
	QSettings settings;
	settings.beginGroup(SETTINGS_GROUP);
	settings.setValue("toolbar_icon_size",                   toolbar_icon_size);
	settings.setValue("sexp_number_every_n",                 sexp_number_every_n);
	settings.setValue("offer_autosave_recovery",             Offer_autosave_recovery);
	settings.setValue("autosave_interval_seconds",          autosave_interval_seconds);
	settings.setValue("create_bak_on_save",                  Create_bak_on_save);
	settings.setValue("move_ships_when_undocking",           Move_ships_when_undocking);
	settings.setValue("always_save_display_names",           Always_save_display_names);
	settings.setValue("error_checker_checks_potential_issues", Error_checker_checks_potential_issues);
	settings.setValue("error_checker_apply_auto_corrections",  Error_checker_apply_auto_corrections);
	settings.setValue("show_sexp_help_mission_events",       Show_sexp_help_mission_events);
	settings.setValue("show_sexp_help_mission_goals",        Show_sexp_help_mission_goals);
	settings.setValue("show_sexp_help_mission_cutscenes",    Show_sexp_help_mission_cutscenes);
	settings.setValue("show_sexp_help_ship_editor",          Show_sexp_help_ship_editor);
	settings.setValue("show_sexp_help_wing_editor",          Show_sexp_help_wing_editor);
	settings.setValue("dark_mode",                           Dark_mode);

	settings.setValue("view_universal_heading",                 view.Universal_heading);
	settings.setValue("view_show_stars",                        view.Show_stars);
	settings.setValue("view_show_horizon",                      view.Show_horizon);
	settings.setValue("view_show_grid",                         view.Show_grid);
	settings.setValue("view_show_distances",                    view.Show_distances);
	settings.setValue("view_show_coordinates",                  view.Show_coordinates);
	settings.setValue("view_show_outlines",                     view.Show_outlines);
	settings.setValue("view_draw_outlines_on_selected_ships",   view.Draw_outlines_on_selected_ships);
	settings.setValue("view_draw_outline_at_warpin_position",   view.Draw_outline_at_warpin_position);
	settings.setValue("view_show_grid_positions",               view.Show_grid_positions);
	settings.setValue("view_show_dock_points",                  view.Show_dock_points);
	settings.setValue("view_show_bay_paths",                    view.Show_bay_paths);
	settings.setValue("view_show_starts",                       view.Show_starts);
	settings.setValue("view_show_ships",                        view.Show_ships);
	settings.setValue("view_show_ship_info",                    view.Show_ship_info);
	settings.setValue("view_show_ship_models",                  view.Show_ship_models);
	settings.setValue("view_show_paths_fred",                   view.Show_paths_fred);
	settings.setValue("view_lighting_on",                       view.Lighting_on);
	settings.setValue("view_full_detail",                       view.FullDetail);
	settings.setValue("view_show_waypoints",                    view.Show_waypoints);
	settings.setValue("view_show_compass",                      view.Show_compass);
	settings.setValue("view_highlight_selectable_subsys",       view.Highlight_selectable_subsys);
	settings.setValue("view_outline_lod",                       view.Outline_lod);
	settings.setValue("camera_invert_orbit_x",                  camera.getInvertOrbitX());
	settings.setValue("camera_invert_orbit_y",                  camera.getInvertOrbitY());
	settings.endGroup();
}
void EditorViewport::needsUpdate() {
	_renderer->scheduleUpdate();
}

bool EditorViewport::areControlsLocked() const
{
	return _controlLockCount > 0;
}

EditorViewport::ViewportControlLock EditorViewport::acquireControlLock()
{
	return ViewportControlLock(this);
}

void EditorViewport::lockControls()
{
	++_controlLockCount;
}

void EditorViewport::unlockControls()
{
	Assertion(_controlLockCount > 0, "Mismatched unlock on EditorViewport controls");
	--_controlLockCount;
}

bool EditorViewport::incMissionTime() {
	const fix MAX_FRAMETIME = (F1_0 / 4);
	const fix MIN_FRAMETIME = (F1_0 / 120);

	fix thistime = timer_get_fixed_seconds();
	fix time_diff;
	if (!_lasttime) {
		time_diff = F1_0 / 30;
	} else {
		time_diff = thistime - _lasttime;
	}

	if (time_diff > MAX_FRAMETIME) {
		time_diff = MAX_FRAMETIME;
	} else if (time_diff < MIN_FRAMETIME) {
		return false;
	}

	Frametime = time_diff;
	Missiontime += Frametime;
	_lasttime = thistime;

	return true;
}
void EditorViewport::select_objects(const Marking_box& box) {
	int x, y, valid;
	vertex v;
	object* ptr;

	// Copy this so we can modify it
	auto marking_box = box;

	if (marking_box.x1 > marking_box.x2) {
		x = marking_box.x1;
		marking_box.x1 = marking_box.x2;
		marking_box.x2 = x;
	}

	if (marking_box.y1 > marking_box.y2) {
		y = marking_box.y1;
		marking_box.y1 = marking_box.y2;
		marking_box.y2 = y;
	}

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		valid = 1;
		if (ptr->flags[Object::Object_Flags::Hidden, Object::Object_Flags::Locked_from_editing]) {
			valid = 0;
		}
		if (!isObjectVisibleInLayer(ptr)) {
			valid = 0;
		}

		Assert(ptr->type != OBJ_NONE);
		switch (ptr->type) {
		case OBJ_WAYPOINT:
			if (!Show_waypoints) {
				valid = 0;
			}
			break;

		case OBJ_START:
			if (!view.Show_starts || !view.Show_ships) {
				valid = 0;
			}
			break;

		case OBJ_SHIP:
			if (!view.Show_ships) {
				valid = 0;
			}

			if (!view.Show_iff[Ships[ptr->instance].team]) {
				valid = 0;
			}

			break;

		case OBJ_PROP:
			if (!view.Show_props) {
				valid = 0;
			}
			break;

		case OBJ_JUMP_NODE:
			if (!view.Show_jump_nodes) {
				valid = 0;
			}
			break;
		}

		g3_rotate_vertex(&v, &ptr->pos);
		if (!(v.codes & CC_BEHIND) && valid) {
			if (!(g3_project_vertex(&v) & PF_OVERFLOW)) {
				x = (int) v.screen.xyw.x;
				y = (int) v.screen.xyw.y;

				if (x >= marking_box.x1 && x <= marking_box.x2 && y >= marking_box.y1 && y <= marking_box.y2) {
					if (ptr->flags[Object::Object_Flags::Marked]) {
						editor->unmarkObject(OBJ_INDEX(ptr));
					} else {
						editor->markObject(OBJ_INDEX(ptr));
					}
				}
			}
		}

		ptr = GET_NEXT(ptr);
	}

	needsUpdate();
}

void EditorViewport::reset() {
	camera.resetView();
	camera.resetViewPhysics();
	The_grid = create_default_grid();
	maybe_create_new_grid(The_grid, &camera.view_pos, &camera.view_orient, 1);
}

///////////////////////////////////////////////////
void EditorViewport::process_system_keys() {
	auto& bindings = ControlBindings::instance();
	if (areControlsLocked()) {
		return;
	}
	if (bindings.takeTriggered(ControlAction::ToggleSelectionLock)) {
		Selection_lock = !Selection_lock;
	}

}

void EditorViewport::game_do_frame(const int cur_object_index) {
	int cmode;
	vec3d control_pos;
	object* objp;
	matrix control_orient;

	if (!incMissionTime()) {
		return;
	}

	// sync all timestamps across the entire frame
	timer_start_frame();

	if ((camera.getViewpoint() == 1) && !query_valid_object(camera.getViewObj())) {
		camera.setViewpoint(0);
	}

	process_system_keys();
	const auto controlsLocked = areControlsLocked();
	cmode = camera.getControlMode();
	if ((camera.getViewpoint() == 1) && !cmode) {
		cmode = 2;
	}

	control_pos = Last_control_pos;
	control_orient = Last_control_orient;

	switch (cmode) {
	case 0: //	Control the viewer's location and orientation
		if (!controlsLocked && camera.processControls(&camera.view_pos, &camera.view_orient, f2fl(Frametime), true)) {
			needsUpdate();
		}
		control_pos = camera.view_pos;
		control_orient = camera.view_orient;
		break;

	case 2: // Control viewpoint object
		if (!controlsLocked && !Objects[camera.getViewObj()].flags[Object::Object_Flags::Locked_from_editing]) {
			camera.processControls(&Objects[camera.getViewObj()].pos, &Objects[camera.getViewObj()].orient,
			                       f2fl(Frametime), false);
			object_moved(&Objects[camera.getViewObj()]);
			control_pos = Objects[camera.getViewObj()].pos;
			control_orient = Objects[camera.getViewObj()].orient;
		}
		break;

	case 1: //	Control the current object's location and orientation
		if (!controlsLocked && query_valid_object(cur_object_index) && !Objects[cur_object_index].flags[Object::Object_Flags::Locked_from_editing]) {
			vec3d delta_pos, leader_old_pos;
			matrix leader_orient, leader_transpose, tmp;
			object* leader;

			leader = &Objects[cur_object_index];
			leader_old_pos = leader->pos;
			leader_orient = leader->orient;
			vm_copy_transpose(&leader_transpose, &leader_orient);

			camera.processControls(&leader->pos, &leader->orient, f2fl(Frametime), false);
			vm_vec_sub(&delta_pos, &leader->pos, &leader_old_pos);
			control_pos = leader->pos;
			control_orient = leader->orient;

			objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				Assert(objp->type != OBJ_NONE);
				if ((objp->flags[Object::Object_Flags::Marked]) && (cur_object_index != OBJ_INDEX(objp))) {
					if (Group_rotate) {
						matrix rot_trans;
						vec3d tmpv1, tmpv2;

						vm_copy_transpose(&rot_trans, &camera.getLastRotMat());
						vm_vec_sub(&tmpv1, &objp->pos, &leader_old_pos);
						vm_vec_rotate(&tmpv2, &tmpv1, &leader_orient);
						vm_vec_rotate(&tmpv1, &tmpv2, &rot_trans);
						vm_vec_rotate(&tmpv2, &tmpv1, &leader_transpose);
						vm_vec_add(&objp->pos, &leader->pos, &tmpv2);

						vm_matrix_x_matrix(&tmp, &objp->orient, &camera.getLastRotMat());
						vm_orthogonalize_matrix(&tmp);
						objp->orient = tmp;
					} else {
						vm_vec_add2(&objp->pos, &delta_pos);
						vm_matrix_x_matrix(&tmp, &objp->orient, &camera.getLastRotMat());
						objp->orient = tmp;
					}
				}

				objp = GET_NEXT(objp);
			}

			objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				if (objp->flags[Object::Object_Flags::Marked]) {
					object_moved(objp);
				}

				objp = GET_NEXT(objp);
			}

			editor->missionChanged();
		}

		break;

	default:
		Assert(0);
	}

	if (camera.getLookatMode() && query_valid_object(cur_object_index)) {
		float dist;

		dist = vm_vec_dist(&camera.view_pos, &Objects[cur_object_index].pos);
		vm_vec_scale_add(&camera.view_pos, &Objects[cur_object_index].pos, &camera.view_orient.vec.fvec, -dist);
	}

	switch (camera.getViewpoint()) {
	case 0:
		camera.eye_pos = camera.view_pos;
		camera.eye_orient = camera.view_orient;
		break;

	case 1:
		camera.eye_pos = Objects[camera.getViewObj()].pos;
		camera.eye_orient = Objects[camera.getViewObj()].orient;
		break;

	default:
		Assert(0);
	}

	maybe_create_new_grid(The_grid, &camera.eye_pos, &camera.eye_orient);

	if (Cursor_over != Last_cursor_over) {
		Last_cursor_over = Cursor_over;
		needsUpdate();
	}

	// redraw screen if controlled object moved or rotated
	if (vm_vec_cmp(&control_pos, &Last_control_pos) || vm_matrix_cmp(&control_orient, &Last_control_orient)) {
		needsUpdate();
		Last_control_pos = control_pos;
		Last_control_orient = control_orient;
	}

	// redraw screen if current viewpoint moved or rotated
	if (camera.hasEyeMoved()) {
		needsUpdate();
	}
}

void EditorViewport::level_controlled() {
	int cmode, count = 0;
	object* objp;

	cmode = camera.getControlMode();
	if ((camera.getViewpoint() == 1) && !cmode) {
		cmode = 2;
	}

	switch (cmode) {
	case 0: //	Control the viewer's location and orientation
		level_object(&camera.view_orient);
		break;

	case 2: // Control viewpoint object
		if (!Objects[camera.getViewObj()].flags[Object::Object_Flags::Locked_from_editing]) {
			level_object(&Objects[camera.getViewObj()].orient);
			object_moved(&Objects[camera.getViewObj()]);
			///! \todo Notify.
			editor->missionChanged();
		}
		break;

	case 1: //	Control the current object's location and orientation
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				level_object(&objp->orient);
			}

			objp = GET_NEXT(objp);
		}

		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				object_moved(objp);
				count++;
			}

			objp = GET_NEXT(objp);
		}

		///! \todo Notify.
		if (count) {
			editor->missionChanged();
		}

		break;
	}

	return;
}

void EditorViewport::verticalize_controlled() {
	int cmode, count = 0;
	object* objp;

	cmode = camera.getControlMode();
	if ((camera.getViewpoint() == 1) && !cmode) {
		cmode = 2;
	}

	switch (cmode) {
	case 0: //	Control the viewer's location and orientation
		verticalize_object(&camera.view_orient);
		break;

	case 2: // Control viewpoint object
		if (!Objects[camera.getViewObj()].flags[Object::Object_Flags::Locked_from_editing]) {
			verticalize_object(&Objects[camera.getViewObj()].orient);
			object_moved(&Objects[camera.getViewObj()]);
			///! \todo notify.
			editor->missionChanged();
		}
		break;

	case 1: //	Control the current object's location and orientation
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				verticalize_object(&objp->orient);
			}

			objp = GET_NEXT(objp);
		}

		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				object_moved(objp);
				count++;
			}

			objp = GET_NEXT(objp);
		}

		///! \todo Notify.
		if (count) {
			editor->missionChanged();
		}

		break;
	}

	return;
}

void EditorViewport::level_object(matrix* orient) {
	vec3d u;

	u = orient->vec.uvec = The_grid->gmatrix.vec.uvec;
	if (u.xyz.x) // y-z plane
	{
		orient->vec.fvec.xyz.x = orient->vec.rvec.xyz.x = 0.0f;
	} else if (u.xyz.y) { // x-z plane
		orient->vec.fvec.xyz.y = orient->vec.rvec.xyz.y = 0.0f;
	} else if (u.xyz.z) { // x-y plane
		orient->vec.fvec.xyz.z = orient->vec.rvec.xyz.z = 0.0f;
	}

	vm_fix_matrix(orient);
}

vec3d EditorViewport::orbitCameraGetPivot()
{
	vec3d pivot;

	if (query_valid_object(editor->currentObject)) {
		// Pivot on current object
		pivot = Objects[editor->currentObject].pos;
	} else if (!The_grid) {
		// Pivot on the origin, if no grid
		pivot = ZERO_VECTOR;
	} else {
		// Intersect camera forward ray with the grid plane
		vec3d *grid_normal = &The_grid->gmatrix.vec.uvec;
		float denom = vm_vec_dot(grid_normal, &camera.view_orient.vec.fvec);

		if (fl_abs(denom) > 0.0001f) {
			float t = -(vm_vec_dot(grid_normal, &camera.view_pos) + The_grid->planeD) / denom;
			if (t > 0.0f) {
				vm_vec_scale_add(&pivot, &camera.view_pos, &camera.view_orient.vec.fvec, t);
			} else {
				pivot = The_grid->center;
			}
		} else {
			// Camera is parallel to grid plane; fall back to grid center
			pivot = The_grid->center;
		}
	}
	return pivot;
}

int EditorViewport::object_check_collision(object* objp, vec3d* p0, vec3d* p1, vec3d* hitpos) {
	mc_info mc;

	if (objp->type == OBJ_NONE) {
		return 0;
	}

	if ((objp->type == OBJ_WAYPOINT) && !view.Show_waypoints) {
		return 0;
	}

	if ((objp->type == OBJ_START) && !view.Show_starts) {
		return 0;
	}

	if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
		if (!view.Show_ships) {
			return 0;
		}

		if (!view.Show_iff[Ships[objp->instance].team]) {
			return 0;
		}
	}

	if ((objp->type == OBJ_PROP) && !view.Show_props) {
		return 0;
	}

	if ((objp->type == OBJ_JUMP_NODE) && !view.Show_jump_nodes) {
		return 0;
	}

	if (objp->flags[Object::Object_Flags::Hidden, Object::Object_Flags::Locked_from_editing]) {
		return 0;
	}
	if (!isObjectVisibleInLayer(objp)) {
		return 0;
	}

	mc.model_instance_num = -1;

	if ((view.Show_ship_models || view.Show_outlines) && (objp->type == OBJ_SHIP || objp->type == OBJ_START)) {
		auto& shp = Ships[objp->instance];
		mc.model_num = Ship_info[shp.ship_info_index].model_num;			// Fill in the model to check
		mc.model_instance_num = shp.model_instance_num;
	} else if ((view.Show_ship_models || view.Show_outlines) && (objp->type == OBJ_PROP)) {
		auto& prp = Props[objp->instance].value();
		mc.model_num = Prop_info[prp.prop_info_index].model_num;			// Fill in the model to check
		mc.model_instance_num = prp.model_instance_num;
	} else {
		return fvi_ray_sphere(hitpos, p0, p1, &objp->pos, (objp->radius > 0.1f) ? objp->radius : LOLLIPOP_SIZE);
	}
	mc.orient = &objp->orient; // The object's orient
	mc.pos = &objp->pos; // The object's position
	mc.p0 = p0; // Point 1 of ray to check
	mc.p1 = p1; // Point 2 of ray to check
	mc.flags = MC_CHECK_MODEL | MC_CHECK_RAY; // flags
	model_collide(&mc);
	*hitpos = mc.hit_point_world;
	if (mc.num_hits < 1) {
		// check shield
		mc.orient = &objp->orient; // The object's orient
		mc.pos = &objp->pos; // The object's position
		mc.p0 = p0; // Point 1 of ray to check
		mc.p1 = p1; // Point 2 of ray to check
		mc.flags = MC_CHECK_SHIELD; // flags
		model_collide(&mc);
		*hitpos = mc.hit_point_world;
	}

	return mc.num_hits;
}

int EditorViewport::select_object(int cx, int cy) {
	int best = -1;
	double dist, best_dist = 9e99;
	vec3d p0, p1, v, hitpos;
	vertex vt;

	/*	gr_reset_clip();
	g3_start_frame(0); ////////////////
	g3_set_view_matrix(&eye_pos, &eye_orient, 0.5f);*/

	// Mouse events can arrive when no frame is active (G3_count == 0) or when
	// another renderer, such as the briefing map widget, has altered the frame state
	// In those cases we cannot do a valid screen to world conversion
	if (g3_in_frame() != 1) {
		return -1;
	}

	//	Get 3d vector specified by mouse cursor location.
	g3_point_to_vec(&v, cx, cy);

	//	g3_end_frame();
	if (!v.xyz.x && !v.xyz.y && !v.xyz.z) { // zero vector {
		return -1;
	}

	p0 = camera.view_pos;
	vm_vec_scale_add(&p1, &p0, &v, 100.0f);

	for (auto objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (object_check_collision(objp, &p0, &p1, &hitpos)) {
			hitpos.xyz.x = objp->pos.xyz.x - camera.view_pos.xyz.x;
			hitpos.xyz.y = objp->pos.xyz.y - camera.view_pos.xyz.y;
			hitpos.xyz.z = objp->pos.xyz.z - camera.view_pos.xyz.z;
			dist = hitpos.xyz.x * hitpos.xyz.x + hitpos.xyz.y * hitpos.xyz.y + hitpos.xyz.z * hitpos.xyz.z;
			if (dist < best_dist) {
				best = OBJ_INDEX(objp);
				best_dist = dist;
			}
		}
	}

	if (best >= 0) {
		if ((Selection_lock && !Objects[best].flags[Object::Object_Flags::Marked]) || Objects[best].flags[Object::Object_Flags::Locked_from_editing]) {
			return -1;
		}
		return best;
	}

	for (auto objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (!isObjectVisibleInLayer(objp)) {
			continue;
		}
		g3_rotate_vertex(&vt, &objp->pos);
		if (!(vt.codes & CC_BEHIND)) {
			if (!(g3_project_vertex(&vt) & PF_OVERFLOW)) {
				hitpos.xyz.x = vt.screen.xyw.x - cx;
				hitpos.xyz.y = vt.screen.xyw.y - cy;
				dist = hitpos.xyz.x * hitpos.xyz.x + hitpos.xyz.y * hitpos.xyz.y;
				if ((dist < 8) && (dist < best_dist)) {
					best = OBJ_INDEX(objp);
					best_dist = dist;
				}
			}
		}
	}

	if ((Selection_lock && !Objects[best].flags[Object::Object_Flags::Marked]) || Objects[best].flags[Object::Object_Flags::Locked_from_editing]) {
		return -1;
	}

	return best;
}

size_t EditorViewport::getLayerIndex(const SCP_string& name) const {
	for (size_t i = 0; i < _layerNames.size(); ++i) {
		if (stricmp(_layerNames[i].c_str(), name.c_str()) == 0) {
			return i;
		}
	}
	return static_cast<size_t>(-1);
}

size_t EditorViewport::getObjectLayerIndex(int objectIndex) const {
	const auto found = _objectLayers.find(objectIndex);
	if (found == _objectLayers.end() || found->second >= _layerNames.size()) {
		return 0;
	}
	return found->second;
}

bool EditorViewport::isLayerVisible(size_t layerIndex) const {
	if (layerIndex >= _layerVisibility.size()) {
		return true;
	}
	return _layerVisibility[layerIndex];
}

void EditorViewport::syncMissionLayerNames() const {
	The_mission.fred_layers = _layerNames;
}

void EditorViewport::setObjectLayerByIndex(int objectIndex, size_t layerIndex) {
	_objectLayers[objectIndex] = layerIndex;

	const auto& layerName = _layerNames[layerIndex];
	if (Objects[objectIndex].type == OBJ_SHIP || Objects[objectIndex].type == OBJ_START) {
		Ships[Objects[objectIndex].instance].fred_layer = layerName;
	} else if (Objects[objectIndex].type == OBJ_PROP) {
		auto* prop = prop_id_lookup(Objects[objectIndex].instance);
		if (prop != nullptr) {
			prop->fred_layer = layerName;
		}
	} else if (Objects[objectIndex].type == OBJ_JUMP_NODE) {
		auto* jn = jumpnode_get_by_objnum(objectIndex);
		if (jn != nullptr) {
			jn->SetFredLayer(layerName);
		}
	} else if (Objects[objectIndex].type == OBJ_WAYPOINT) {
		// Layer is tracked at the path level; sync all waypoints in the path to the same layer
		auto* wl = find_waypoint_list_with_instance(Objects[objectIndex].instance, nullptr);
		if (wl != nullptr) {
			wl->set_fred_layer(layerName);
			for (const auto& wpt : wl->get_waypoints()) {
				_objectLayers[wpt.get_objnum()] = layerIndex;
			}
		}
	}
}

SCP_vector<SCP_string> EditorViewport::getLayerNames() const {
	return _layerNames;
}

bool EditorViewport::addLayer(const SCP_string& name, SCP_string* errorMessage) {
	if (name.empty()) {
		if (errorMessage != nullptr) {
			*errorMessage = "Layer name cannot be empty.";
		}
		return false;
	}
	if (getLayerIndex(name) != static_cast<size_t>(-1)) {
		if (errorMessage != nullptr) {
			*errorMessage = "Layer names must be unique.";
		}
		return false;
	}

	_layerNames.push_back(name);
	_layerVisibility.push_back(true);
	syncMissionLayerNames();
	editor->notifyLayerStructureChanged();
	editor->notifyLayerListChanged();
	return true;
}

bool EditorViewport::deleteLayer(const SCP_string& name, SCP_string* errorMessage) {
	const auto layerIndex = getLayerIndex(name);
	if (layerIndex == static_cast<size_t>(-1)) {
		if (errorMessage != nullptr) {
			*errorMessage = "Layer does not exist.";
		}
		return false;
	}
	if (layerIndex == 0) {
		if (errorMessage != nullptr) {
			*errorMessage = "The default layer cannot be deleted.";
		}
		return false;
	}

	_layerNames.erase(_layerNames.begin() + static_cast<SCP_vector<SCP_string>::difference_type>(layerIndex));
	_layerVisibility.erase(_layerVisibility.begin() + static_cast<SCP_vector<bool>::difference_type>(layerIndex));

	std::vector<int> toReassign;
	for (auto& objectLayer : _objectLayers) {
		if (objectLayer.second == layerIndex) {
			toReassign.push_back(objectLayer.first);
		} else if (objectLayer.second > layerIndex) {
			--objectLayer.second;
		}
	}
	for (int objIdx : toReassign) {
		setObjectLayerByIndex(objIdx, 0);
	}
	syncMissionLayerNames();
	editor->notifyLayerStructureChanged();
	editor->notifyLayerListChanged();
	return true;
}

bool EditorViewport::setLayerVisibility(const SCP_string& name, bool visible, SCP_string* errorMessage) {
	const auto layerIndex = getLayerIndex(name);
	if (layerIndex == static_cast<size_t>(-1)) {
		if (errorMessage != nullptr) {
			*errorMessage = "Layer does not exist.";
		}
		return false;
	}

	_layerVisibility[layerIndex] = visible;
	if (!visible) {
		for (auto objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
			if (getObjectLayerIndex(OBJ_INDEX(objp)) == layerIndex && objp->flags[Object::Object_Flags::Marked]) {
				editor->unmarkObject(OBJ_INDEX(objp));
			}
		}
	}

	needsUpdate();
	editor->notifyLayerVisibilityChanged();
	return true;
}

bool EditorViewport::getLayerVisibility(const SCP_string& name, bool* visible, SCP_string* errorMessage) const {
	const auto layerIndex = getLayerIndex(name);
	if (layerIndex == static_cast<size_t>(-1)) {
		if (errorMessage != nullptr) {
			*errorMessage = "Layer does not exist.";
		}
		return false;
	}

	if (visible != nullptr) {
		*visible = isLayerVisible(layerIndex);
	}
	return true;
}

void EditorViewport::showAllLayers() {
	std::fill(_layerVisibility.begin(), _layerVisibility.end(), true);
	needsUpdate();
}

int EditorViewport::getHiddenLayerCount() const {
	return static_cast<int>(std::count(_layerVisibility.begin(), _layerVisibility.end(), false));
}

void EditorViewport::reloadLayersFromMission() {
	_layerNames.clear();
	_layerVisibility.clear();
	_objectLayers.clear();

	if (The_mission.fred_layers.empty()) {
		_layerNames.emplace_back(DefaultLayerName);
	} else {
		_layerNames = The_mission.fred_layers;
	}

	if (_layerNames.empty() || _layerNames.front() != DefaultLayerName) {
		_layerNames.insert(_layerNames.begin(), DefaultLayerName);
	}

	_layerVisibility.resize(_layerNames.size(), true);
	syncMissionLayerNames();
	editor->notifyLayerListChanged();

	for (int objectIndex = 0; objectIndex < MAX_OBJECTS; ++objectIndex) {
		auto* objp = &Objects[objectIndex];
		if (objp->type == OBJ_NONE) {
			continue;
		}

		size_t layerIndex = 0;
		if (objp->type == OBJ_SHIP || objp->type == OBJ_START) {
			const auto found = getLayerIndex(Ships[objp->instance].fred_layer);
			layerIndex = found == static_cast<size_t>(-1) ? 0 : found;
		} else if (objp->type == OBJ_PROP) {
			auto* prop = prop_id_lookup(objp->instance);
			if (prop != nullptr) {
				const auto found = getLayerIndex(prop->fred_layer);
				layerIndex = found == static_cast<size_t>(-1) ? 0 : found;
			}
		} else if (objp->type == OBJ_JUMP_NODE) {
			auto* jn = jumpnode_get_by_objnum(objectIndex);
			if (jn != nullptr) {
				const auto found = getLayerIndex(jn->GetFredLayer());
				layerIndex = found == static_cast<size_t>(-1) ? 0 : found;
			}
		} else if (objp->type == OBJ_WAYPOINT) {
			auto* wl = find_waypoint_list_with_instance(objp->instance, nullptr);
			if (wl != nullptr) {
				const auto found = getLayerIndex(wl->get_fred_layer());
				layerIndex = found == static_cast<size_t>(-1) ? 0 : found;
			}
		}

		setObjectLayerByIndex(objectIndex, layerIndex);
	}

	needsUpdate();
}

void EditorViewport::registerObjectInLayer(int objectIndex) {
	if (objectIndex < 0 || objectIndex >= MAX_OBJECTS) {
		return;
	}
	auto* objp = &Objects[objectIndex];
	if (objp->type == OBJ_NONE) {
		return;
	}

	SCP_string layerName;
	switch (objp->type) {
	case OBJ_SHIP:
	case OBJ_START:
		layerName = Ships[objp->instance].fred_layer;
		break;
	case OBJ_PROP:
		if (auto* p = prop_id_lookup(objp->instance)) {
			layerName = p->fred_layer;
		}
		break;
	case OBJ_JUMP_NODE:
		if (auto* jn = jumpnode_get_by_objnum(objectIndex)) {
			layerName = jn->GetFredLayer();
		}
		break;
	case OBJ_WAYPOINT:
		if (auto* wl = find_waypoint_list_with_instance(objp->instance, nullptr)) {
			layerName = wl->get_fred_layer();
		}
		break;
	default:
		return;
	}

	auto layerIndex = getLayerIndex(layerName);
	if (layerIndex == static_cast<size_t>(-1)) {
		layerIndex = 0;
	}
	_objectLayers[objectIndex] = layerIndex;
}

SCP_string EditorViewport::getObjectLayerName(int objectIndex) const {
	const auto layerIndex = getObjectLayerIndex(objectIndex);
	if (layerIndex >= _layerNames.size()) {
		return DefaultLayerName;
	}
	return _layerNames[layerIndex];
}

bool EditorViewport::moveObjectToLayer(int objectIndex, const SCP_string& layerName, SCP_string* errorMessage) {
	const auto layerIndex = getLayerIndex(layerName);
	if (layerIndex == static_cast<size_t>(-1)) {
		if (errorMessage != nullptr) {
			*errorMessage = "Layer does not exist.";
		}
		return false;
	}

	setObjectLayerByIndex(objectIndex, layerIndex);
	if (!isLayerVisible(layerIndex)) {
		editor->unmarkObject(objectIndex);
	}
	needsUpdate();
	editor->notifyLayerStructureChanged();
	return true;
}

void EditorViewport::moveMarkedObjectsToLayer(const SCP_string& layerName, SCP_string* errorMessage) {
	const auto layerIndex = getLayerIndex(layerName);
	if (layerIndex == static_cast<size_t>(-1)) {
		if (errorMessage != nullptr) {
			*errorMessage = "Layer does not exist.";
		}
		return;
	}

	for (auto objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (objp->flags[Object::Object_Flags::Marked]) {
			setObjectLayerByIndex(OBJ_INDEX(objp), layerIndex);
			if (!isLayerVisible(layerIndex)) {
				editor->unmarkObject(OBJ_INDEX(objp));
			}
		}
	}
	needsUpdate();
	editor->notifyLayerStructureChanged();
}

bool EditorViewport::isObjectVisibleInLayer(const object* objp) const {
	if (objp == nullptr) {
		return true;
	}
	return isLayerVisible(getObjectLayerIndex(OBJ_INDEX(objp)));
}

void EditorViewport::drag_rotate_save_backup() {
	object* objp;

	/*
	if (Cur_bitmap != -1)
		bitmap_matrix_backup = Starfield_bitmaps[Cur_bitmap].m;
		*/

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		Assert(objp->type != OBJ_NONE);
		if (objp->flags[Object::Object_Flags::Marked]) {
			rotation_backup[OBJ_INDEX(objp)].pos = objp->pos;
			rotation_backup[OBJ_INDEX(objp)].orient = objp->orient;
		}

		objp = GET_NEXT(objp);
	}
}

int EditorViewport::create_object_on_grid(int x, int y, int waypoint_instance) {
	return create_object_on_grid(x, y, waypoint_instance, CreateKind::Ship);
}

int EditorViewport::create_object_on_grid(int x, int y, int waypoint_instance, CreateKind kind) {
	float fallbackDist = 200.0f;
	if (kind == CreateKind::Prop) {
		if (cur_prop_index >= 0 && cur_prop_index < prop_info_size()) {
			prop_info* pip = &Prop_info[cur_prop_index];
			if (pip->model_num >= 0) {
				fallbackDist = model_get_radius(pip->model_num) * 1.5f;
			} else if (VALID_FNAME(pip->pof_file)) {
				int modelNum = model_load(pip->pof_file.c_str());
				if (modelNum >= 0) {
					fallbackDist = model_get_radius(modelNum) * 1.5f;
					model_unload(modelNum);
				}
			}
		}
	} else if (kind == CreateKind::Ship && cur_model_index >= 0 && cur_model_index < (int)Ship_info.size() &&
		Ship_info[cur_model_index].model_num >= 0) {
		fallbackDist = model_get_radius(Ship_info[cur_model_index].model_num) * 1.5f;
	}

	vec3d pos = getCreatePosition(x, y, fallbackDist);
	editor->unmark_all();
	int obj = create_object(&pos, waypoint_instance, kind);
	if (obj >= 0) {
		editor->markObject(obj);

		editor->missionChanged();

	} else if (obj == -1) {
		dialogProvider->showButtonDialog(DialogType::Error, "Error", "Maximum ship limit reached.  Can't add any more ships.", { DialogButton::Ok });
	}

	return obj;
}
int EditorViewport::create_object(vec3d* pos, int waypoint_instance, CreateKind kind) {

	int obj, n;
	if (kind == CreateKind::Prop) {
		if (cur_prop_index < 0 || cur_prop_index >= prop_info_size()) {
			return -1;
		}

		obj = prop_create(nullptr, pos, cur_prop_index);
		if (obj == -1) {
			return -1;
		}
	} else if (kind == CreateKind::Other) {
		switch (cur_other_kind) {
		case OtherKind::Waypoint:
			obj = editor->create_waypoint(pos, waypoint_instance);
			break;
		case OtherKind::JumpNode: {
			CJumpNode jnp(pos);
			obj = jnp.GetSCPObjectNumber();
			Jump_nodes.push_back(std::move(jnp));
			break;
		}
		default:
			obj = -1;
			break;
		}
	} else {  // CreateKind::Ship
		if (cur_model_index < 0 || cur_model_index >= (int)Ship_info.size() ||
			Ship_info[cur_model_index].flags[Ship::Info_Flags::No_fred]) {
			obj = -1;
		} else {
			obj = editor->create_ship(nullptr, pos, cur_model_index);
			if (obj == -1)
				return -1;

			n = Objects[obj].instance;
			Ships[n].arrival_cue = alloc_sexp("true", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
			Ships[n].departure_cue = alloc_sexp("false", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
			Ships[n].cargo1 = 0;
		}
	}

	if (obj < 0)
		return obj;

	obj_merge_created_list();

	needsUpdate();
	return obj;
}
vec3d EditorViewport::getCreatePosition(int x, int y, float fallbackDist) {
	vec3d dir, pos;
	g3_point_to_vec_delayed(&dir, x, y);
	if (fvi_ray_plane(&pos, &The_grid->center, &The_grid->gmatrix.vec.uvec, &camera.view_pos, &dir, 0.0f) >= 0.0f) {
		return pos;
	}
	vm_vec_scale_add(&pos, &camera.view_pos, &camera.view_orient.vec.fvec, fallbackDist);
	return pos;
}

int EditorViewport::createShipAtScreenPos(int x, int y, int modelIndex) {
	if (modelIndex < 0 || modelIndex >= (int)Ship_info.size() ||
		Ship_info[modelIndex].flags[Ship::Info_Flags::No_fred]) {
		return -1;
	}
	int savedModelIndex = cur_model_index;
	cur_model_index = modelIndex;
	int obj = create_object_on_grid(x, y, -1, CreateKind::Ship);
	cur_model_index = savedModelIndex;
	return obj;
}

int EditorViewport::createPropAtScreenPos(int x, int y, int propIndex) {
	if (propIndex < 0 || propIndex >= prop_info_size() ||
		Prop_info[propIndex].flags[Prop::Info_Flags::No_fred]) {
		return -1;
	}
	int savedPropIndex = cur_prop_index;
	cur_prop_index = propIndex;
	int obj = create_object_on_grid(x, y, -1, CreateKind::Prop);
	cur_prop_index = savedPropIndex;
	return obj;
}

int EditorViewport::createWaypointAtScreenPos(int x, int y, int waypoint_instance) {
	OtherKind savedKind = cur_other_kind;
	cur_other_kind = OtherKind::Waypoint;
	int obj = create_object_on_grid(x, y, waypoint_instance, CreateKind::Other);
	cur_other_kind = savedKind;
	return obj;
}

int EditorViewport::createJumpNodeAtScreenPos(int x, int y) {
	OtherKind savedKind = cur_other_kind;
	cur_other_kind = OtherKind::JumpNode;
	int obj = create_object_on_grid(x, y, -1, CreateKind::Other);
	cur_other_kind = savedKind;
	return obj;
}

void EditorViewport::initialSetup() {
	cur_model_index = get_default_player_ship_index();
	cur_other_kind = OtherKind::Waypoint;
	for (int i = 0; i < prop_info_size(); ++i) {
		if (!Prop_info[i].flags[Prop::Info_Flags::No_fred]) {
			cur_prop_index = i;
			break;
		}
	}
}

int EditorViewport::duplicate_marked_objects(bool insert_waypoints)
{
	int z, cobj, flag;
	object *objp, *ptr;

	cobj = Duped_wing = -1;
	flag = 0;

	int duping_waypoint_list = -1;

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list))	{
		Assert(objp->type != OBJ_NONE);
		if (objp->flags[Object::Object_Flags::Marked]) {
			if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
				z = Ships[objp->instance].wingnum;
				if (!flag)
					Duped_wing = z;
				else if (Duped_wing != z)
					Duped_wing = -1;

			} else {
				Duped_wing = -1;
			}

			flag = 1;

			if (insert_waypoints && objp->type == OBJ_WAYPOINT) {
				// Insert a new waypoint into the source path right after this one.
				// No new list is created, so no list-property copy is needed.
				z = waypoint_add(&objp->pos, objp->instance, false);
				if (z < 0) {
					cobj = -1;
					break;
				}
				Objects[z].pos = objp->pos;
				Objects[z].orient = objp->orient;
				Objects[z].flags.set(Object::Object_Flags::Temp_marked);
				registerObjectInLayer(z);
				if (editor->currentObject == OBJ_INDEX(objp))
					cobj = z;
			} else {
				// make sure we dup as many waypoint lists as we have
				if (objp->type == OBJ_WAYPOINT) {
					int this_list = calc_waypoint_list_index(objp->instance);
					if (duping_waypoint_list != this_list) {
						editor->dup_object(nullptr);  // reset waypoint list
						duping_waypoint_list = this_list;
					}
				}

				z = editor->dup_object(objp);
				if (z == -1) {
					cobj = -1;
					break;
				}

				if (editor->currentObject == OBJ_INDEX(objp))
					cobj = z;
			}
		}

		objp = GET_NEXT(objp);
	}

	obj_merge_created_list();

	// I think this code is to catch the case where an object wasn't created for whatever reason;
	// in this case just delete the remaining objects we just created
	if (cobj == -1) {
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list))	{
			ptr = GET_NEXT(objp);
			if (objp->flags [Object::Object_Flags::Temp_marked])
				editor->delete_object(OBJ_INDEX(objp));

			objp = ptr;
		}

		button_down = false;
		return -1;
	}

	editor->unmark_all();

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list))	{
		if (objp->flags [Object::Object_Flags::Temp_marked]) {
			objp->flags.remove(Object::Object_Flags::Temp_marked);
			editor->markObject(OBJ_INDEX(objp));
		}

		objp = GET_NEXT(objp);
	}

	editor->selectObject(cobj);
	return 0;
}

//	If cur_object_index references a valid object, drag it from its current
//	location to the new cursor location specified by "point".
//	It is dragged relative to the main grid.  Its y coordinate is not changed.
//	Return value: 0/1 = didn't/did move object all the way to goal.
int EditorViewport::drag_objects(int x, int y)
{
	int rval = 1;
	float r;
	float	distance_moved = 0.0f;
	vec3d cursor_dir, int_pnt;
	vec3d movement_vector;
	vec3d obj;
	vec3d vec1, vec2;
	object *objp;
	// starfield_bitmaps *bmp;

	/*
	if (Bg_bitmap_dialog) {
		if (Cur_bitmap < 0)
			return -1;

		bmp = &Starfield_bitmaps[Cur_bitmap];
		if (Single_axis_constraint && Constraint.z) {
			bmp->dist *= 1.0f + mouse_dx / -800.0f;
			calculate_bitmap_points(bmp, 0.0f);

		} else {
			g3_point_to_vec_delayed(&bmp->m.fvec, marking_box.x2, marking_box.y2);
			vm_orthogonalize_matrix(&bmp->m);
			calculate_bitmap_points(bmp, 0.0f);
		}
		return rval;
	}
	*/

	// Do not move ships that we are currently centered around (Lookat_mode). The vector math will start going haywire and return NAN
	if (!query_valid_object(editor->currentObject) || camera.getLookatMode())
		return -1;

	if (Dup_drag == 1 || Dup_drag == DUP_DRAG_INSERT) {
		const bool insert_waypoints = (Dup_drag == DUP_DRAG_INSERT);
		if (duplicate_marked_objects(insert_waypoints) < 0)
			return -1;

		if (Duped_wing != -1)
			Dup_drag = DUP_DRAG_OF_WING;  // indication for later that we duped objects in a wing
		else
			Dup_drag = 0;

		drag_rotate_save_backup();

		editor->missionChanged();
	}

	objp = &Objects[editor->currentObject];
	Assert(objp->type != OBJ_NONE);
	obj = int_pnt = objp->pos;

	//	Get 3d vector specified by mouse cursor location.
	g3_point_to_vec_delayed(&cursor_dir, x, y);
	if (Single_axis_constraint)	{
//		if (fvi_ray_plane(&int_pnt, &obj, &view_orient.fvec, &view_pos, &cursor_dir, 0.0f) >= 0.0f )	{
//			vm_vec_add(&p1, &obj, &Constraint);
//			find_nearest_point_on_line(&nearest_point, &obj, &p1, &int_pnt);
//			int_pnt = nearest_point;
//			distance_moved = vm_vec_dist(&obj, &int_pnt);
//		}

		vec3d tmpAnticonstraint = Anticonstraint;
		vec3d tmpObject = obj;

		tmpAnticonstraint.xyz.x = 0.0f;
		r = fvi_ray_plane(&int_pnt, &tmpObject, &tmpAnticonstraint, &camera.view_pos, &cursor_dir, 0.0f);

		//	If intersected behind viewer, don't move.  Too confusing, not what user wants.
		vm_vec_sub(&vec1, &int_pnt, &camera.view_pos);
		vm_vec_sub(&vec2, &obj, &camera.view_pos);
		if ((r>=0.0f) && (vm_vec_dot(&vec1, &vec2) >= 0.0f))	{
			vec3d tmp1;
			vm_vec_sub( &tmp1, &int_pnt, &obj );
			tmp1.xyz.x *= Constraint.xyz.x;
			tmp1.xyz.y *= Constraint.xyz.y;
			tmp1.xyz.z *= Constraint.xyz.z;
			vm_vec_add( &int_pnt, &obj, &tmp1 );

			distance_moved = vm_vec_dist(&obj, &int_pnt);
		}


	} else {  // Move in x-z plane, defined by grid.  Preserve height.
		r = fvi_ray_plane(&int_pnt, &obj, &Anticonstraint, &camera.view_pos, &cursor_dir, 0.0f);

		//	If intersected behind viewer, don't move.  Too confusing, not what user wants.
		vm_vec_sub(&vec1, &int_pnt, &camera.view_pos);
		vm_vec_sub(&vec2, &obj, &camera.view_pos);
		if ((r>=0.0f) && (vm_vec_dot(&vec1, &vec2) >= 0.0f))
			distance_moved = vm_vec_dist(&obj, &int_pnt);
	}

	//	If moved too far, then move max distance along vector.
	vm_vec_sub(&movement_vector, &int_pnt, &obj);
/*	if (distance_moved > MAX_MOVE_DISTANCE)	{
		vm_vec_normalize(&movement_vector);
		vm_vec_scale(&movement_vector, MAX_MOVE_DISTANCE);
	} */

	if (distance_moved) {
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list))	{
			Assert(objp->type != OBJ_NONE);
			if (objp->flags[Object::Object_Flags::Marked]) {
				vm_vec_add(&objp->pos, &objp->pos, &movement_vector);
				if (objp->type == OBJ_WAYPOINT) {
					waypoint *wpt = find_waypoint_with_instance(objp->instance);
					Assert(wpt != NULL);
					wpt->set_pos(&objp->pos);
				}
			}

			objp = GET_NEXT(objp);
		}

		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (objp->flags[Object::Object_Flags::Marked])
				object_moved(objp);

			objp = GET_NEXT(objp);
		}
	}

	editor->missionChanged();
	return rval;
}
int EditorViewport::drag_rotate_objects(int mouse_dx, int mouse_dy) {
	int rval = 1;
	vec3d int_pnt, obj;
	angles a;
	matrix leader_orient, leader_transpose, tmp, newmat, rotmat;
	object *leader, *objp;
	// starfield_bitmaps *bmp;

	needsUpdate();
	/*
    if (Bg_bitmap_dialog) {
        if (Cur_bitmap < 0)
            return -1;

        bmp = &Starfield_bitmaps[Cur_bitmap];
        calculate_bitmap_points(bmp, mouse_dx / -300.0f);
        return rval;
    }
    */

	if (!query_valid_object(editor->currentObject)){
		return -1;
	}

	objp = &Objects[editor->currentObject];
	Assert(objp->type != OBJ_NONE);
	obj = int_pnt = objp->pos;

	memset(&a, 0, sizeof(angles));
	if (Single_axis_constraint) {
		if (Constraint.xyz.x)
			a.p = mouse_dy / REDUCER;
		else if (Constraint.xyz.y)
			a.h = mouse_dx / REDUCER;
		else if (Constraint.xyz.z)
			a.b = -mouse_dx / REDUCER;

	} else {
		if (!Constraint.xyz.x) {				// yz
			a.b = -mouse_dx / REDUCER;
			a.h = mouse_dy / REDUCER;
		} else if (!Constraint.xyz.y) {	// xz
			a.p = mouse_dy / REDUCER;
			a.b = -mouse_dx / REDUCER;
		} else if (!Constraint.xyz.z) {	// xy
			a.p = mouse_dy / REDUCER;
			a.h = mouse_dx / REDUCER;
		}
	}

	leader = &Objects[editor->currentObject];
	leader_orient = leader->orient;			// save original orientation
	vm_copy_transpose(&leader_transpose, &leader_orient);

	vm_angles_2_matrix(&rotmat, &a);
	vm_matrix_x_matrix(&newmat, &leader->orient, &rotmat);
	leader->orient = newmat;

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list))			{
		Assert(objp->type != OBJ_NONE);
		if ((objp->flags[Object::Object_Flags::Marked]) && (editor->currentObject != OBJ_INDEX(objp) )) {
			if (Group_rotate) {
				matrix rot_trans;
				vec3d tmpv1, tmpv2;

				// change rotation matrix to rotate in opposite direction.  This rotation
				// matrix is what the leader ship has rotated by.
				vm_copy_transpose(&rot_trans, &rotmat);

				// get point relative to our point of rotation (make POR the origin).
				vm_vec_sub(&tmpv1, &objp->pos, &leader->pos);

				// convert point from real-world coordinates to leader's relative coordinate
				// system (z=forward vec, y=up vec, x=right vec
				vm_vec_rotate(&tmpv2, &tmpv1, &leader_orient);

				// now rotate the point by the transpose from above.
				vm_vec_rotate(&tmpv1, &tmpv2, &rot_trans);

				// convert point back into real-world coordinates
				vm_vec_rotate(&tmpv2, &tmpv1, &leader_transpose);

				// and move origin back to real-world origin.  Object is now at its correct
				// position.
				vm_vec_add(&objp->pos, &leader->pos, &tmpv2);

				// Now fix the object's orientation to what it should be.
				vm_matrix_x_matrix(&tmp, &objp->orient, &rotmat);
				vm_orthogonalize_matrix(&tmp);  // safety check
				objp->orient = tmp;

			} else {
				vm_matrix_x_matrix(&tmp, &objp->orient, &rotmat);
				objp->orient = tmp;
			}
		}

		objp = GET_NEXT(objp);
	}

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if (objp->flags[Object::Object_Flags::Marked])
			object_moved(objp);

		objp = GET_NEXT(objp);
	}

	editor->missionChanged();
	return rval;
}
void EditorViewport::cancel_drag() {
	if (!button_down) {
		return;
	}

	auto objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		Assert(objp->type != OBJ_NONE);
		if (objp->flags[Object::Object_Flags::Marked]) {
			const auto obj_index = OBJ_INDEX(objp);
			if (!IS_VEC_NULL(&rotation_backup[obj_index].orient.vec.rvec) && !IS_VEC_NULL(&rotation_backup[obj_index].orient.vec.uvec)
				&& !IS_VEC_NULL(&rotation_backup[obj_index].orient.vec.fvec)) {
				objp->pos = rotation_backup[obj_index].pos;
				objp->orient = rotation_backup[obj_index].orient;
			}
		}

		objp = GET_NEXT(objp);
	}

	button_down = false;
	moved = false;
	Dup_drag = 0;
	needsUpdate();
}
void EditorViewport::view_universe(bool just_marked) {
	int max = 0;
	float dist, largest = 20.0f;
	vec3d center, p1, p2;
	vertex v;
	object *ptr;

	if (just_marked)
		ptr = &Objects[editor->currentObject];
	else
		ptr = GET_FIRST(&obj_used_list);

	p1.xyz.x = p2.xyz.x = ptr->pos.xyz.x;
	p1.xyz.y = p2.xyz.y = ptr->pos.xyz.y;
	p1.xyz.z = p2.xyz.z = ptr->pos.xyz.z;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (!just_marked || (ptr->flags[Object::Object_Flags::Marked])) {
			center = ptr->pos;
			if (center.xyz.x < p1.xyz.x)
				p1.xyz.x = center.xyz.x;
			if (center.xyz.x > p2.xyz.x)
				p2.xyz.x = center.xyz.x;
			if (center.xyz.y < p1.xyz.y)
				p1.xyz.y = center.xyz.y;
			if (center.xyz.y > p2.xyz.y)
				p2.xyz.y = center.xyz.y;
			if (center.xyz.z < p1.xyz.z)
				p1.xyz.z = center.xyz.z;
			if (center.xyz.z > p2.xyz.z)
				p2.xyz.z = center.xyz.z;
		}

		ptr = GET_NEXT(ptr);
	}

	vm_vec_avg(&center, &p1, &p2);
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (!just_marked || (ptr->flags[Object::Object_Flags::Marked])) {
			dist = vm_vec_dist_squared(&center, &ptr->pos);
			if (dist > largest)
				largest = dist;

			if (OBJ_INDEX(ptr) > max)
				max = OBJ_INDEX(ptr);
		}

		ptr = GET_NEXT(ptr);
	}

	dist = fl_sqrt(largest) + 1.0f;
	vm_vec_scale_add(&camera.view_pos, &center, &camera.view_orient.vec.fvec, -dist);
	g3_set_view_matrix(&camera.view_pos, &camera.view_orient, 0.5f);

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (!just_marked || (ptr->flags[Object::Object_Flags::Marked])) {
			g3_rotate_vertex(&v, &ptr->pos);
			Assert(!(v.codes & CC_BEHIND));
			if (g3_project_vertex(&v) & PF_OVERFLOW)
				Int3();

			while (v.codes & CC_OFF) {
				dist += 5.0f;
				vm_vec_scale_add(&camera.view_pos, &center, &camera.view_orient.vec.fvec, -dist);
				g3_set_view_matrix(&camera.view_pos, &camera.view_orient, 0.5f);
				g3_rotate_vertex(&v, &ptr->pos);
				if (g3_project_vertex(&v) & PF_OVERFLOW)
					Int3();
			}
		}

		ptr = GET_NEXT(ptr);
	}

	dist *= 1.1f;
	vm_vec_scale_add(&camera.view_pos, &center, &camera.view_orient.vec.fvec, -dist);
	g3_set_view_matrix(&camera.view_pos, &camera.view_orient, 0.5f);

	needsUpdate();
}
void EditorViewport::view_object(int obj_num) {
	vm_vec_scale_add(&camera.view_pos, &Objects[obj_num].pos, &camera.view_orient.vec.fvec,
	                 Objects[obj_num].radius * -3.0f);

	needsUpdate();
}

} // namespace fso::fred
