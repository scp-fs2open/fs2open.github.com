#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "io/key.h"
#include "io/mouse.h"
#include "lab/renderer/lab_cameras.h"
#include "lab/labv2_internal.h"


LabCamera::~LabCamera() {
	cam_delete(FS_camera);
}

namespace {
bool point_in_rect(int x, int y, int rect_x, int rect_y, int rect_w, int rect_h)
{
	return x >= rect_x && x < rect_x + rect_w && y >= rect_y && y < rect_y + rect_h;
}

struct WidgetFaceProjection {
	OrbitCamera::SnapDirection direction;
	const char* label;
	vec3d center_world;
	vec3d normal_world;
	vec3d corners_world[4];
	int corners_screen_x[4] = {0, 0, 0, 0};
	int corners_screen_y[4] = {0, 0, 0, 0};
	int center_screen_x = 0;
	int center_screen_y = 0;
	float center_depth = 0.0f;
	bool visible = false;
};

const char* snap_direction_label(OrbitCamera::SnapDirection direction)
{
	switch (direction) {
	case OrbitCamera::SnapDirection::Front:
		return "Front";
	case OrbitCamera::SnapDirection::Back:
		return "Back";
	case OrbitCamera::SnapDirection::Top:
		return "Top";
	case OrbitCamera::SnapDirection::Bottom:
		return "Bottom";
	case OrbitCamera::SnapDirection::Left:
		return "Left";
	case OrbitCamera::SnapDirection::Right:
		return "Right";
	default:
		return "";
	}
}

vec3d snap_direction_normal(OrbitCamera::SnapDirection direction)
{
	switch (direction) {
	case OrbitCamera::SnapDirection::Front:
		return vm_vec_new(0.0f, 0.0f, 1.0f);
	case OrbitCamera::SnapDirection::Back:
		return vm_vec_new(0.0f, 0.0f, -1.0f);
	case OrbitCamera::SnapDirection::Top:
		return vm_vec_new(0.0f, 1.0f, 0.0f);
	case OrbitCamera::SnapDirection::Bottom:
		return vm_vec_new(0.0f, -1.0f, 0.0f);
	case OrbitCamera::SnapDirection::Left:
		return vm_vec_new(-1.0f, 0.0f, 0.0f);
	case OrbitCamera::SnapDirection::Right:
		return vm_vec_new(1.0f, 0.0f, 0.0f);
	default:
		return vmd_zero_vector;
	}
}

bool point_in_convex_quad(int x, int y, const int quad_x[4], const int quad_y[4])
{
	int sign = 0;

	for (int i = 0; i < 4; ++i) {
		const int next = (i + 1) % 4;
		const int edge_x = quad_x[next] - quad_x[i];
		const int edge_y = quad_y[next] - quad_y[i];
		const int to_point_x = x - quad_x[i];
		const int to_point_y = y - quad_y[i];
		const int cross = edge_x * to_point_y - edge_y * to_point_x;

		if (cross == 0) {
			continue;
		}

		const int current_sign = (cross > 0) ? 1 : -1;
		if (sign == 0) {
			sign = current_sign;
		} else if (sign != current_sign) {
			return false;
		}
	}

	return true;
}

void get_orbit_view_basis(float phi, float theta, vec3d& forward, vec3d& right, vec3d& up)
{
	vec3d camera_offset;
	camera_offset.xyz.x = sinf(phi) * cosf(theta);
	camera_offset.xyz.y = cosf(phi);
	camera_offset.xyz.z = sinf(phi) * sinf(theta);
	vm_vec_normalize_safe(&camera_offset);

	vm_vec_copy_scale(&forward, &camera_offset, -1.0f);

	vec3d world_up = vmd_y_vector;
	vm_vec_cross(&right, &world_up, &forward);
	if (vm_vec_mag_squared(&right) <= 1e-6f) {
		world_up = vmd_x_vector;
		vm_vec_cross(&right, &world_up, &forward);
	}
	vm_vec_normalize_safe(&right);

	vm_vec_cross(&up, &forward, &right);
	vm_vec_normalize_safe(&up);
}

SCP_vector<WidgetFaceProjection> build_widget_faces(float phi, float theta, int center_x, int center_y, int half_size_px)
{
	const float cube_half = 1.0f;

	SCP_vector<WidgetFaceProjection> faces;
	faces.reserve(6);
	faces.push_back({OrbitCamera::SnapDirection::Front,
		"Front",
		vm_vec_new(0.0f, 0.0f, cube_half),
		vm_vec_new(0.0f, 0.0f, 1.0f),
		{vm_vec_new(-cube_half, cube_half, cube_half),
			vm_vec_new(cube_half, cube_half, cube_half),
			vm_vec_new(cube_half, -cube_half, cube_half),
			vm_vec_new(-cube_half, -cube_half, cube_half)}});
	faces.push_back({OrbitCamera::SnapDirection::Back,
		"Back",
		vm_vec_new(0.0f, 0.0f, -cube_half),
		vm_vec_new(0.0f, 0.0f, -1.0f),
		{vm_vec_new(cube_half, cube_half, -cube_half),
			vm_vec_new(-cube_half, cube_half, -cube_half),
			vm_vec_new(-cube_half, -cube_half, -cube_half),
			vm_vec_new(cube_half, -cube_half, -cube_half)}});
	faces.push_back({OrbitCamera::SnapDirection::Top,
		"Top",
		vm_vec_new(0.0f, cube_half, 0.0f),
		vm_vec_new(0.0f, 1.0f, 0.0f),
		{vm_vec_new(-cube_half, cube_half, -cube_half),
			vm_vec_new(cube_half, cube_half, -cube_half),
			vm_vec_new(cube_half, cube_half, cube_half),
			vm_vec_new(-cube_half, cube_half, cube_half)}});
	faces.push_back({OrbitCamera::SnapDirection::Bottom,
		"Bottom",
		vm_vec_new(0.0f, -cube_half, 0.0f),
		vm_vec_new(0.0f, -1.0f, 0.0f),
		{vm_vec_new(-cube_half, -cube_half, cube_half),
			vm_vec_new(cube_half, -cube_half, cube_half),
			vm_vec_new(cube_half, -cube_half, -cube_half),
			vm_vec_new(-cube_half, -cube_half, -cube_half)}});
	faces.push_back({OrbitCamera::SnapDirection::Left,
		"Left",
		vm_vec_new(-cube_half, 0.0f, 0.0f),
		vm_vec_new(-1.0f, 0.0f, 0.0f),
		{vm_vec_new(-cube_half, cube_half, -cube_half),
			vm_vec_new(-cube_half, cube_half, cube_half),
			vm_vec_new(-cube_half, -cube_half, cube_half),
			vm_vec_new(-cube_half, -cube_half, -cube_half)}});
	faces.push_back({OrbitCamera::SnapDirection::Right,
		"Right",
		vm_vec_new(cube_half, 0.0f, 0.0f),
		vm_vec_new(1.0f, 0.0f, 0.0f),
		{vm_vec_new(cube_half, cube_half, cube_half),
			vm_vec_new(cube_half, cube_half, -cube_half),
			vm_vec_new(cube_half, -cube_half, -cube_half),
			vm_vec_new(cube_half, -cube_half, cube_half)}});

	vec3d forward;
	vec3d right;
	vec3d up;
	get_orbit_view_basis(phi, theta, forward, right, up);
	vec3d camera_dir;
	vm_vec_copy_scale(&camera_dir, &forward, -1.0f);

	for (auto& face : faces) {
		face.visible = vm_vec_dot(&face.normal_world, &camera_dir) > 0.0f;

		vec3d center_view;
		center_view.xyz.x = vm_vec_dot(&face.center_world, &right);
		center_view.xyz.y = vm_vec_dot(&face.center_world, &up);
		center_view.xyz.z = vm_vec_dot(&face.center_world, &camera_dir);
		face.center_screen_x = center_x + fl2i(center_view.xyz.x * half_size_px);
		face.center_screen_y = center_y - fl2i(center_view.xyz.y * half_size_px);
		face.center_depth = center_view.xyz.z;

		for (int i = 0; i < 4; ++i) {
			vec3d view;
			view.xyz.x = vm_vec_dot(&face.corners_world[i], &right);
			view.xyz.y = vm_vec_dot(&face.corners_world[i], &up);
			view.xyz.z = vm_vec_dot(&face.corners_world[i], &camera_dir);
			face.corners_screen_x[i] = center_x + fl2i(view.xyz.x * half_size_px);
			face.corners_screen_y[i] = center_y - fl2i(view.xyz.y * half_size_px);
		}
	}

	std::sort(faces.begin(), faces.end(), [](const WidgetFaceProjection& a, const WidgetFaceProjection& b) {
		return a.center_depth < b.center_depth;
	});

	return faces;
}

OrbitCamera::SnapDirection pick_direction_for_axis(const vec3d& axis, const SCP_vector<OrbitCamera::SnapDirection>& excluded)
{
	const std::array<OrbitCamera::SnapDirection, 6> all_dirs = {OrbitCamera::SnapDirection::Front,
		OrbitCamera::SnapDirection::Back,
		OrbitCamera::SnapDirection::Top,
		OrbitCamera::SnapDirection::Bottom,
		OrbitCamera::SnapDirection::Left,
		OrbitCamera::SnapDirection::Right};

	float best_dot = -FLT_MAX;
	auto best_dir = OrbitCamera::SnapDirection::Front;

	for (auto dir : all_dirs) {
		if (std::find(excluded.begin(), excluded.end(), dir) != excluded.end()) {
			continue;
		}

		const auto normal = snap_direction_normal(dir);
		const float dir_dot = vm_vec_dot(&normal, &axis);
		if (dir_dot > best_dot) {
			best_dot = dir_dot;
			best_dir = dir;
		}
	}

	return best_dir;
}

struct AdjacentLabel {
	OrbitCamera::SnapDirection direction;
	const char* label;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
};

std::array<AdjacentLabel, 4> build_adjacent_labels(float phi, float theta, int center_x, int center_y, int cube_half_px)
{
	vec3d forward;
	vec3d right;
	vec3d up;
	get_orbit_view_basis(phi, theta, forward, right, up);
	vec3d camera_dir;
	vm_vec_copy_scale(&camera_dir, &forward, -1.0f);

	const auto primary = pick_direction_for_axis(camera_dir, {});
	const auto opposite = pick_direction_for_axis(vm_vec_new(-camera_dir.xyz.x, -camera_dir.xyz.y, -camera_dir.xyz.z), {primary});

	const auto up_dir = pick_direction_for_axis(up, {primary, opposite});
	const auto down_dir = pick_direction_for_axis(vm_vec_new(-up.xyz.x, -up.xyz.y, -up.xyz.z), {primary, opposite, up_dir});
	const auto right_dir = pick_direction_for_axis(right, {primary, opposite, up_dir, down_dir});
	const auto left_dir =
		pick_direction_for_axis(vm_vec_new(-right.xyz.x, -right.xyz.y, -right.xyz.z), {primary, opposite, up_dir, down_dir, right_dir});

	static constexpr int padding_x = 5;
	static constexpr int padding_y = 2;
	static constexpr int margin = 6;

	std::array<AdjacentLabel, 4> labels = {{
		{up_dir, snap_direction_label(up_dir)},
		{down_dir, snap_direction_label(down_dir)},
		{left_dir, snap_direction_label(left_dir)},
		{right_dir, snap_direction_label(right_dir)},
	}};

	for (auto& label : labels) {
		int text_w = 0;
		int text_h = 0;
		gr_get_string_size(&text_w, &text_h, label.label);
		label.w = text_w + (padding_x * 2);
		label.h = text_h + (padding_y * 2);
	}

	labels[0].x = center_x - (labels[0].w / 2);
	labels[0].y = center_y - cube_half_px - margin - labels[0].h;

	labels[1].x = center_x - (labels[1].w / 2);
	labels[1].y = center_y + cube_half_px + margin;

	labels[2].x = center_x - cube_half_px - margin - labels[2].w;
	labels[2].y = center_y - (labels[2].h / 2);

	labels[3].x = center_x + cube_half_px + margin;
	labels[3].y = center_y - (labels[3].h / 2);

	return labels;
}
}

void OrbitCamera::handleInput(
	int dx, int dy, int dz, bool, bool lmbPressed, bool rmbDown, int modifierKeys, int mouseX, int mouseY) {
	if (getLabManager()->Renderer->getShowOrientationWidget() && lmbPressed && handleOrientationWidgetClick(mouseX, mouseY)) {
		return;
	}

	if (dx == 0 && dy == 0 && dz == 0)
		return;

	if (dz > 0) {
		for (int i = 0; i < dz; ++i) {
			distance *= 0.9f;
		}
	} else if (dz < 0) {
		for (int i = 0; i < -dz; ++i) {
			distance *= 1.1f;
		}
	}
	CLAMP(distance, 1.0f, 10000000.0f);

	if (rmbDown) {
		if (modifierKeys & KEY_SHIFTED) {
			const float pan_factor = distance / 500.0f;

			vec3d camera_offset;
			camera_offset.xyz.x = sinf(phi) * cosf(theta);
			camera_offset.xyz.y = cosf(phi);
			camera_offset.xyz.z = sinf(phi) * sinf(theta);

			vec3d view_forward;
			vm_vec_copy_scale(&view_forward, &camera_offset, -1.0f);

			vec3d world_up = vmd_y_vector;
			vec3d view_right;
			vm_vec_cross(&view_right, &world_up, &view_forward);

			if (vm_vec_mag_squared(&view_right) <= 1e-6f) {
				world_up = vmd_x_vector;
				vm_vec_cross(&view_right, &world_up, &view_forward);
			}

			vm_vec_normalize_safe(&view_right);

			vec3d view_up;
			vm_vec_cross(&view_up, &view_forward, &view_right);
			vm_vec_normalize_safe(&view_up);

			vm_vec_scale_add2(&pan_offset, &view_right, -dx * pan_factor);
			vm_vec_scale_add2(&pan_offset, &view_up, dy * pan_factor);
		} else {
			theta -= dx / 100.0f;
			phi -= dy / 100.0f;

			CLAMP(phi, 0.01f, PI - 0.01f);
		}
	}

	updateCamera();
}

bool OrbitCamera::handleOrientationWidgetClick(int mouseX, int mouseY)
{
	const int widget_size = WIDGET_CUBE_HALF_SIZE * 4;
	const int widget_left = gr_screen.center_offset_x + gr_screen.center_w - widget_size - WIDGET_MARGIN;
	const int widget_top = gr_screen.center_offset_y + WIDGET_MARGIN;
	const int center_x = widget_left + widget_size / 2;
	const int center_y = widget_top + widget_size / 2;
	const int cube_half = WIDGET_CUBE_HALF_SIZE;

	const auto faces = build_widget_faces(phi, theta, center_x, center_y, cube_half);
	for (auto it = faces.rbegin(); it != faces.rend(); ++it) {
		if (!it->visible) {
			continue;
		}

		if (point_in_convex_quad(mouseX, mouseY, it->corners_screen_x, it->corners_screen_y)) {
			snapToDirection(it->direction);
			return true;
		}
	}

	const auto adjacent_labels = build_adjacent_labels(phi, theta, center_x, center_y, cube_half);
	for (const auto& label : adjacent_labels) {
		if (point_in_rect(mouseX, mouseY, label.x, label.y, label.w, label.h)) {
			snapToDirection(label.direction);
			return true;
		}
	}

	return false;
}

bool OrbitCamera::isOverlayHit(int mouseX, int mouseY) const
{
	const int widget_size = WIDGET_CUBE_HALF_SIZE * 4;
	const int widget_left = gr_screen.center_offset_x + gr_screen.center_w - widget_size - WIDGET_MARGIN;
	const int widget_top = gr_screen.center_offset_y + WIDGET_MARGIN;
	if (point_in_rect(mouseX, mouseY, widget_left, widget_top, widget_size, widget_size)) {
		return true;
	}

	const int center_x = widget_left + widget_size / 2;
	const int center_y = widget_top + widget_size / 2;
	const int cube_half = WIDGET_CUBE_HALF_SIZE;
	const auto adjacent_labels = build_adjacent_labels(phi, theta, center_x, center_y, cube_half);
	for (const auto& label : adjacent_labels) {
		if (point_in_rect(mouseX, mouseY, label.x, label.y, label.w, label.h)) {
			return true;
		}
	}

	return false;
}

void OrbitCamera::snapToDirection(SnapDirection direction)
{
	static constexpr float POLE_EPSILON = 0.01f;

	pan_offset = vmd_zero_vector;
	distance = getObjectFitDistance();

	switch (direction) {
	case SnapDirection::Front:
		phi = PI_2;
		theta = PI_2;
		break;
	case SnapDirection::Back:
		phi = PI_2;
		theta = -PI_2;
		break;
	case SnapDirection::Top:
		phi = POLE_EPSILON;
		break;
	case SnapDirection::Bottom:
		phi = PI - POLE_EPSILON;
		break;
	case SnapDirection::Left:
		phi = PI_2;
		theta = PI;
		break;
	case SnapDirection::Right:
		phi = PI_2;
		theta = 0.0f;
		break;
	}

	updateCamera();
}

float OrbitCamera::getObjectFitDistance() const
{
	static constexpr float distance_multiplier = 1.6f;
	float fit_distance = DEFAULT_DISTANCE;

	if (getLabManager()->CurrentObject != -1) {
		object* obj = &Objects[getLabManager()->CurrentObject];
		// Ships and Missiles use the object radius to get a camera distance
		fit_distance = obj->radius * distance_multiplier;

		// Beams use the muzzle radius
		if (obj->type == OBJ_BEAM) {
			weapon_info* wip = &Weapon_info[Beams[obj->instance].weapon_info_index];
			if (wip != nullptr) {
				fit_distance = wip->b_info.beam_muzzle_radius * distance_multiplier;
			}
		// Lasers use the laser length
		} else if (obj->type == OBJ_WEAPON) {
			weapon_info* wip = &Weapon_info[Weapons[obj->instance].weapon_info_index];
			if (wip != nullptr && wip->render_type == WRT_LASER) {
				fit_distance = wip->laser_length * distance_multiplier;
			}
		}
	}

	return fit_distance;
}

void OrbitCamera::resetView()
{
	phi = DEFAULT_PHI;
	theta = DEFAULT_THETA;
	distance = DEFAULT_DISTANCE;
	pan_offset = vmd_zero_vector;

	displayedObjectChanged();
}

void OrbitCamera::displayedObjectChanged() {
	// Reset camera panning
	pan_offset = vmd_zero_vector;
	distance = getObjectFitDistance();

	updateCamera();
}

void OrbitCamera::updateCamera() {
	auto cam = FS_camera.getCamera();
	vec3d new_position;
	new_position.xyz.x = sinf(phi) * cosf(theta);
	new_position.xyz.y = cosf(phi);
	new_position.xyz.z = sinf(phi) * sinf(theta);

	vm_vec_scale(&new_position, distance);

	object* obj = &Objects[getLabManager()->CurrentObject];
	vec3d target = obj->pos;

	if (obj->type == OBJ_WEAPON) {
		weapon_info* wip = &Weapon_info[Weapons[obj->instance].weapon_info_index];
		if (wip != nullptr && wip->render_type == WRT_LASER) {
			// Offset target by half the laser length forward along the facing
			vec3d forward;
			vm_vec_copy_normalize(&forward, &obj->orient.vec.fvec);
			vm_vec_scale_add2(&target, &forward, wip->laser_length * 0.5f);
		}
	}

	vm_vec_add2(&target, &pan_offset);
	vm_vec_add2(&new_position, &target);

	cam->set_position(&new_position);

	// If these are the same then that's not great so do nothing and use the last facing value
	if (!vm_vec_same(&new_position, &target)) {
		cam->set_rotation_facing(&target);
	}
}

void OrbitCamera::renderOverlay() const
{
	const int widget_size = WIDGET_CUBE_HALF_SIZE * 4;
	const int widget_left = gr_screen.center_offset_x + gr_screen.center_w - widget_size - WIDGET_MARGIN;
	const int widget_top = gr_screen.center_offset_y + WIDGET_MARGIN;
	const int center_x = widget_left + widget_size / 2;
	const int center_y = widget_top + widget_size / 2;
	const int cube_half = WIDGET_CUBE_HALF_SIZE;

	color background;
	gr_init_alphacolor(&background, 24, 24, 24, 96);
	gr_set_color_fast(&background);
	gr_rect(widget_left, widget_top, widget_size, widget_size, GR_RESIZE_NONE);

	int mouse_x = 0;
	int mouse_y = 0;
	mouse_get_pos(&mouse_x, &mouse_y);

	const auto faces = build_widget_faces(phi, theta, center_x, center_y, cube_half);
	for (const auto& face : faces) {
		if (!face.visible) {
			continue;
		}

		const bool hovered = point_in_convex_quad(mouse_x, mouse_y, face.corners_screen_x, face.corners_screen_y);

		color edge_color;
		gr_init_alphacolor(&edge_color, hovered ? 255 : 210, hovered ? 255 : 210, hovered ? 255 : 210, hovered ? 255 : 170);
		gr_set_color_fast(&edge_color);

		for (int i = 0; i < 4; ++i) {
			const int next = (i + 1) % 4;
			gr_line(face.corners_screen_x[i], face.corners_screen_y[i], face.corners_screen_x[next], face.corners_screen_y[next], GR_RESIZE_NONE);
		}

		int text_w = 0;
		int text_h = 0;
		gr_get_string_size(&text_w, &text_h, face.label);
		gr_set_color_fast(hovered ? &Color_white : &Color_grey);
		gr_string(face.center_screen_x - (text_w / 2), face.center_screen_y - (text_h / 2), face.label, GR_RESIZE_NONE);
	}

	const auto adjacent_labels = build_adjacent_labels(phi, theta, center_x, center_y, cube_half);
	for (const auto& label : adjacent_labels) {
		const bool hovered = point_in_rect(mouse_x, mouse_y, label.x, label.y, label.w, label.h);

		color label_bg;
		gr_init_alphacolor(&label_bg, 24, 24, 24, hovered ? 180 : 120);
		gr_set_color_fast(&label_bg);
		gr_rect(label.x, label.y, label.w, label.h, GR_RESIZE_NONE);

		gr_set_color_fast(hovered ? &Color_white : &Color_silver);
		gr_string(label.x + 5, label.y + 2, label.label, GR_RESIZE_NONE);
	}

	const char* subtitle = "Camera orientation";
	int subtitle_w = 0;
	int subtitle_h = 0;
	gr_get_string_size(&subtitle_w, &subtitle_h, subtitle);
	gr_set_color_fast(&Color_silver);
	gr_string(center_x - (subtitle_w / 2), widget_top + widget_size + 8, subtitle, GR_RESIZE_NONE);
}
