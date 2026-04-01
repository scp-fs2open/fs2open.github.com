#include "freespace.h"

#include "actions/Action.h"
#include "camera/camera.h"
#include "camera/photomode.h"
#include "controlconfig/controlsconfig.h"
#include "debugconsole/console.h"
#include "globalincs/alphacolors.h"
#include "graphics/2d.h"
#include "graphics/font.h"
#include "hud/hud.h"
#include "io/keycontrol.h"
#include "localization/localize.h"
#include "mission/missionmessage.h"
#include "object/object.h"
#include "parse/parselo.h"
#include "playerman/player.h"
#include "render/3d.h"
#include "scripting/global_hooks.h"
#include "sound/audiostr.h"

bool Photo_mode_active = false;
camid Photo_mode_id;
fix Photo_mode_saved_time_compression = F1_0;
bool Photo_mode_saved_lock_state = false;
bool Photo_mode_screenshot_queued_this_frame = false;
bool Photo_mode_allowed = true;
bool Photo_mode_audio_paused = false;

const float Photo_mode_turn_rate = PI_2; // radians/sec
constexpr float Photo_mode_move_speed = 90.0f;
constexpr float Photo_mode_boost_multiplier = 6.0f;
constexpr float Photo_mode_time_compression = 0.01f;

struct photo_mode_post_effect_state {
	SCP_string name;
	float intensity = 0.0f;
	vec3d rgb = vmd_zero_vector;
	int effect_idx = -1; // index into Post_processing_manager->getPostEffects()
};

enum class photo_mode_param_type {
	INT_RANGE,  // adjustable integer value with min/max
	BOOL_TOGGLE // on/off toggle
};

enum class photo_mode_param_role {
	NONE,
	GRID_OVERLAY, // bool toggle that enables the rule of thirds grid
	POST_EFFECT   // drives a post processing effect by name
};

struct photo_mode_param {
	const char*           label;    // display name
	int                   xstr_id;  // XSTR localization ID
	photo_mode_param_type type;
	photo_mode_param_role role;

	// For POST_EFFECT role: post processing effect name, range, and values
	const char* effect_name;   // post processing effect to control (nullptr if role != POST_EFFECT)
	int         effect_idx;    // index into Post_processing_manager->getPostEffects() (-1 if unresolved)
	int         min_val;
	int         max_val;
	int         value;
	int         saved_value;

	// For BOOL_TOGGLE
	bool        bool_value;
};

// Parameter definitions
SCP_vector<photo_mode_param> Photo_mode_params = {
	{"Grid",       1906, photo_mode_param_type::BOOL_TOGGLE, photo_mode_param_role::GRID_OVERLAY, nullptr,      -1, 0, 0,   0,   0,   false},
	{"Saturation", 1903, photo_mode_param_type::INT_RANGE,   photo_mode_param_role::POST_EFFECT,  "saturation", -1, 0, 200, 100, 100, false},
	{"Brightness", 1904, photo_mode_param_type::INT_RANGE,   photo_mode_param_role::POST_EFFECT,  "brightness", -1, 0, 200, 100, 100, false},
	{"Contrast",   1905, photo_mode_param_type::INT_RANGE,   photo_mode_param_role::POST_EFFECT,  "contrast",   -1, 0, 200, 100, 100, false},
};

SCP_vector<photo_mode_post_effect_state> Photo_mode_saved_post_effects;
int Photo_mode_selected_parameter = 0;
int Photo_mode_panel_width = 0;
int Photo_mode_panel_padding_w = 0;

void photo_mode_capture_post_effect_state()
{
	Photo_mode_saved_post_effects.clear();

	if (graphics::Post_processing_manager == nullptr) {
		return;
	}

	const auto& post_effects = graphics::Post_processing_manager->getPostEffects();
	Photo_mode_saved_post_effects.reserve(post_effects.size());

	for (int i = 0; i < static_cast<int>(post_effects.size()); ++i) {
		const auto& effect = post_effects[i];
		photo_mode_post_effect_state state;
		state.name = effect.name;
		state.intensity = effect.intensity;
		state.rgb = effect.rgb;
		state.effect_idx = i;
		Photo_mode_saved_post_effects.push_back(state);
	}

	// Resolve effect indices for each param.
	// Done once here rather than on every frame or every key press.
	for (auto& param : Photo_mode_params) {
		if (param.role != photo_mode_param_role::POST_EFFECT) {
			param.effect_idx = -1;
			continue;
		}
		param.effect_idx = -1;
		for (int j = 0; j < static_cast<int>(post_effects.size()); ++j) {
			if (!stricmp(post_effects[j].name.c_str(), param.effect_name)) {
				param.effect_idx = j;
				break;
			}
		}
	}
}

void photo_mode_apply_saved_post_effect_state(bool clear_saved_state)
{
	if (graphics::Post_processing_manager == nullptr) {
		if (clear_saved_state) {
			Photo_mode_saved_post_effects.clear();
		}
		return;
	}

	const auto& post_effects = graphics::Post_processing_manager->getPostEffects();
	for (const auto& saved_state : Photo_mode_saved_post_effects) {
		if (saved_state.effect_idx < 0 || saved_state.effect_idx >= static_cast<int>(post_effects.size())) {
			continue;
		}
		const auto& effect = post_effects[saved_state.effect_idx];
		const int value = static_cast<int>(std::lround((saved_state.intensity - effect.add) * effect.div));
		gr_post_process_set_effect(saved_state.name.c_str(), value, &saved_state.rgb);
	}

	if (clear_saved_state) {
		Photo_mode_saved_post_effects.clear();
	}
}

int photo_mode_get_saved_post_effect_value(int effect_idx)
{
	int value = 100;

	if (effect_idx < 0 || graphics::Post_processing_manager == nullptr) {
		return value;
	}

	const auto& post_effects = graphics::Post_processing_manager->getPostEffects();
	if (effect_idx >= static_cast<int>(post_effects.size())) {
		return value;
	}

	for (const auto& saved_state : Photo_mode_saved_post_effects) {
		if (saved_state.effect_idx != effect_idx) {
			continue;
		}
		const auto& effect = post_effects[effect_idx];
		value = static_cast<int>(std::lround((saved_state.intensity - effect.add) * effect.div));
		break;
	}

	return value;
}

void photo_mode_sync_parameter_values_from_saved_state()
{
	for (auto& param : Photo_mode_params) {
		if (param.role == photo_mode_param_role::POST_EFFECT) {
			param.saved_value = photo_mode_get_saved_post_effect_value(param.effect_idx);
			param.value = param.saved_value;
		} else if (param.type == photo_mode_param_type::BOOL_TOGGLE) {
			param.bool_value = false;
		}
	}

	Photo_mode_selected_parameter = 0;
}

void photo_mode_apply_parameter_values()
{
	for (const auto& param : Photo_mode_params) {
		if (param.role == photo_mode_param_role::POST_EFFECT) {
			gr_post_process_set_effect(param.effect_name, param.value, nullptr);
		}
	}
}

SCP_string format_photo_mode_keybind(int action)
{
	auto primary = Control_config[action].first.textify();
	auto secondary = Control_config[action].second.textify();

	if (primary.empty() && secondary.empty()) {
		return XSTR("Unbound", 1907);
	}
	if (primary.empty()) {
		return secondary;
	}
	if (secondary.empty()) {
		return primary;
	}

	return primary + " / " + secondary;
}

void photo_mode_compute_hud_layout()
{
	const auto old_font = font::get_current_fontnum();
	font::set_font(font::FONT1);

	int max_text_width = 0;
	char buf[512];
	auto measure = [&](const char* text) {
		int w = 0;
		gr_get_string_size(&w, nullptr, text);
		if (w > max_text_width) max_text_width = w;
	};

	// Find the longest string
	measure(XSTR("Photo Mode", 1892));
	measure(XSTR("Controls", 1893));
	measure(XSTR("Status", 1900));
	measure(XSTR("Effects", 1914));
	snprintf(buf, sizeof(buf), XSTR("Toggle: %s", 1894),             format_photo_mode_keybind(TOGGLE_PHOTO_MODE).c_str());
	measure(buf);
	snprintf(buf, sizeof(buf), XSTR("Previous Filter: %s", 1895),    format_photo_mode_keybind(PHOTO_MODE_FILTER_PREV).c_str());
	measure(buf);
	snprintf(buf, sizeof(buf), XSTR("Next Filter: %s", 1896),        format_photo_mode_keybind(PHOTO_MODE_FILTER_NEXT).c_str());
	measure(buf);
	snprintf(buf, sizeof(buf), XSTR("Reset Filters: %s", 1897),      format_photo_mode_keybind(PHOTO_MODE_FILTER_RESET).c_str());
	measure(buf);
	snprintf(buf, sizeof(buf), XSTR("Decrease Parameter: %s", 1898), format_photo_mode_keybind(PHOTO_MODE_PARAM_DECREASE).c_str());
	measure(buf);
	snprintf(buf, sizeof(buf), XSTR("Increase Parameter: %s", 1899), format_photo_mode_keybind(PHOTO_MODE_PARAM_INCREASE).c_str());
	measure(buf);

	// Use worst case placeholder values for the dynamic status strings so the
	// panel width stays stable as these change during a session.
	snprintf(buf, sizeof(buf), XSTR("Time Compression: %.2fx", 1901), 64.0f);                          measure(buf);
	snprintf(buf, sizeof(buf), XSTR("Cam Pos: X %.1f  Y %.1f  Z %.1f", 1902), -99999.9f, -99999.9f, -99999.9f); measure(buf);

	for (const auto& param : Photo_mode_params) {
		if (param.type == photo_mode_param_type::INT_RANGE) {
			snprintf(buf, sizeof(buf), "%s: %d", XSTR(param.label, param.xstr_id), param.max_val);
		} else {
			snprintf(buf, sizeof(buf), "%s: %s", XSTR(param.label, param.xstr_id), XSTR("Off", 1285));
		}
		measure(buf);
	}

	font::set_font(old_font);

	Photo_mode_panel_padding_w = max_text_width / 10;
	Photo_mode_panel_width     = max_text_width + Photo_mode_panel_padding_w * 2;
}

void photo_mode_set_active(bool active)
{
	if (active == Photo_mode_active) {
		return;
	}

	if (active) {
		if (!Photo_mode_allowed) {
			return;
		}

		if ((Game_mode & GM_MULTIPLAYER) != 0 || Player_obj == nullptr || !(Game_mode & GM_IN_MISSION)) {
			return;
		}

		if (Time_compression_locked) {
			return;
		}

		if (Player_obj->type != OBJ_SHIP) {
			return;
		}

		Photo_mode_saved_time_compression = Game_time_compression;
		Photo_mode_saved_lock_state = Time_compression_locked;

		Photo_mode_id = cam_create("Photo Mode", &Eye_position, &Eye_matrix);
		if (!Photo_mode_id.isValid() || !cam_set_camera(Photo_mode_id)) {
			cam_delete(Photo_mode_id);
			Photo_mode_id = camid();
			return;
		}

		set_time_compression(Photo_mode_time_compression, 0.0f);
		lock_time_compression(true);

		photo_mode_capture_post_effect_state();
		photo_mode_sync_parameter_values_from_saved_state();
		photo_mode_apply_parameter_values();
		photo_mode_compute_hud_layout();

		audiostream_pause_all();
		message_pause_all();
		Photo_mode_audio_paused = true;

		Photo_mode_active = true;
		mprintf(("Photo Mode enabled.\n"));

		if (scripting::hooks::OnPhotoModeStarted->isActive()) {
			scripting::hooks::OnPhotoModeStarted->run();
		}
		return;
	}

	cam_reset_camera();
	cam_delete(Photo_mode_id);
	Photo_mode_id = camid();

	set_time_compression(f2fl(Photo_mode_saved_time_compression), 0.0f);
	lock_time_compression(Photo_mode_saved_lock_state);

	if (Photo_mode_audio_paused) {
		audiostream_unpause_all();
		message_resume_all();
		Photo_mode_audio_paused = false;
	}

	photo_mode_apply_saved_post_effect_state(true);
	for (auto& param : Photo_mode_params) {
		if (param.type == photo_mode_param_type::INT_RANGE) {
			param.value = 100;
			param.saved_value = 100;
		} else if (param.type == photo_mode_param_type::BOOL_TOGGLE) {
			param.bool_value = false;
		}
	}
	Photo_mode_selected_parameter = 0;

	Photo_mode_active = false;
	mprintf(("Photo Mode disabled.\n"));

	if (scripting::hooks::OnPhotoModeEnded->isActive()) {
		scripting::hooks::OnPhotoModeEnded->run();
	}
}

void photo_mode_do_frame(float frame_time)
{
	if (!Photo_mode_active || !Photo_mode_id.isValid()) {
		return;
	}

	if (!Photo_mode_allowed) {
		photo_mode_set_active(false);
		return;
	}

	auto photo_mode = Photo_mode_id.getCamera();
	if (photo_mode == nullptr) {
		photo_mode_set_active(false);
		return;
	}

	vec3d cam_pos = vmd_zero_vector;
	matrix cam_orient = vmd_identity_matrix;
	photo_mode->get_info(&cam_pos, &cam_orient);

	angles delta_angles{};
	float pitch = check_control_timef(PITCH_FORWARD) - check_control_timef(PITCH_BACK);
	float heading = check_control_timef(YAW_RIGHT) - check_control_timef(YAW_LEFT);
	float bank = check_control_timef(BANK_LEFT) - check_control_timef(BANK_RIGHT);

	int axis[Action::NUM_VALUES] = {0};
	control_get_axes_readings(axis, flRealframetime);
	pitch += -f2fl(axis[Action::PITCH]);
	heading += f2fl(axis[Action::HEADING]);
	bank -= f2fl(axis[Action::BANK]);

	CLAMP(pitch, -1.0f, 1.0f);
	CLAMP(heading, -1.0f, 1.0f);
	CLAMP(bank, -1.0f, 1.0f);

	delta_angles.p = pitch * Photo_mode_turn_rate * frame_time;
	delta_angles.h = heading * Photo_mode_turn_rate * frame_time;
	delta_angles.b = bank * Photo_mode_turn_rate * frame_time;

	matrix delta_orient = vmd_identity_matrix;
	vm_angles_2_matrix(&delta_orient, &delta_angles);
	vm_matrix_x_matrix(&cam_orient, &cam_orient, &delta_orient);
	vm_fix_matrix(&cam_orient);

	float speed = Photo_mode_move_speed;
	if (check_control(AFTERBURNER)) {
		speed *= Photo_mode_boost_multiplier;
	}

	float forward = check_control_timef(FORWARD_THRUST) - check_control_timef(REVERSE_THRUST);
	float right = check_control_timef(RIGHT_SLIDE_THRUST) - check_control_timef(LEFT_SLIDE_THRUST);
	float up = check_control_timef(UP_SLIDE_THRUST) - check_control_timef(DOWN_SLIDE_THRUST);

	CLAMP(forward, -1.0f, 1.0f);
	CLAMP(right, -1.0f, 1.0f);
	CLAMP(up, -1.0f, 1.0f);

	vm_vec_scale_add2(&cam_pos, &cam_orient.vec.fvec, forward * speed * frame_time);
	vm_vec_scale_add2(&cam_pos, &cam_orient.vec.rvec, right * speed * frame_time);
	vm_vec_scale_add2(&cam_pos, &cam_orient.vec.uvec, up * speed * frame_time);

	photo_mode->set_rotation(&cam_orient);
	photo_mode->set_position(&cam_pos);
}

void photo_mode_maybe_render_hud()
{
	if (!Photo_mode_active || Photo_mode_screenshot_queued_this_frame || gr_is_screenshot_requested()) {
		return;
	}

	if (!Photo_mode_allowed) {
		photo_mode_set_active(false);
		return;
	}

	auto photo_mode = Photo_mode_id.getCamera();
	if (photo_mode == nullptr) {
		return;
	}

	vec3d cam_pos = vmd_zero_vector;
	matrix cam_orient = vmd_identity_matrix;
	photo_mode->get_info(&cam_pos, &cam_orient);

	const auto toggle_keybind = format_photo_mode_keybind(TOGGLE_PHOTO_MODE);
	const auto prev_filter_keybind = format_photo_mode_keybind(PHOTO_MODE_FILTER_PREV);
	const auto next_filter_keybind = format_photo_mode_keybind(PHOTO_MODE_FILTER_NEXT);
	const auto reset_filter_keybind = format_photo_mode_keybind(PHOTO_MODE_FILTER_RESET);
	const auto decrease_param_keybind = format_photo_mode_keybind(PHOTO_MODE_PARAM_DECREASE);
	const auto increase_param_keybind = format_photo_mode_keybind(PHOTO_MODE_PARAM_INCREASE);
	gr_set_color_fast(&Color_silver);
	const auto old_font = font::get_current_fontnum();
	font::set_font(font::FONT1);
	const int line_height     = gr_get_font_height();
	const int num_param_lines = static_cast<int>(Photo_mode_params.size());
	const int panel_padding_h = line_height / 2;
	const int panel_padding_w = Photo_mode_panel_padding_w;
	const int panel_width     = Photo_mode_panel_width;
	const int panel_height    = panel_padding_h * 2 + line_height * (15 + num_param_lines);
	const int panel_x = gr_screen.center_offset_x + (gr_screen.center_w / 4);
	const int panel_y = gr_screen.center_offset_y + gr_screen.center_h - panel_height - (gr_screen.center_h / 6);

	gr_line(panel_x, panel_y, panel_x + panel_width, panel_y, GR_RESIZE_NONE);
	gr_line(panel_x, panel_y + panel_height, panel_x + panel_width, panel_y + panel_height, GR_RESIZE_NONE);
	gr_line(panel_x, panel_y, panel_x, panel_y + panel_height, GR_RESIZE_NONE);
	gr_line(panel_x + panel_width, panel_y, panel_x + panel_width, panel_y + panel_height, GR_RESIZE_NONE);
	gr_line(panel_x,
		panel_y + line_height + panel_padding_h + 1,
		panel_x + panel_width,
		panel_y + line_height + panel_padding_h + 1,
		GR_RESIZE_NONE);

	bool grid_enabled = false;
	for (const auto& param : Photo_mode_params) {
		if (param.role == photo_mode_param_role::GRID_OVERLAY) {
			grid_enabled = param.bool_value;
			break;
		}
	}

	if (grid_enabled) {
		const int x1 = gr_screen.center_offset_x + gr_screen.center_w / 3;
		const int x2 = gr_screen.center_offset_x + (gr_screen.center_w * 2) / 3;
		const int y1 = gr_screen.center_offset_y + gr_screen.center_h / 3;
		const int y2 = gr_screen.center_offset_y + (gr_screen.center_h * 2) / 3;
		gr_line(x1, gr_screen.center_offset_y, x1, gr_screen.center_offset_y + gr_screen.center_h, GR_RESIZE_NONE);
		gr_line(x2, gr_screen.center_offset_y, x2, gr_screen.center_offset_y + gr_screen.center_h, GR_RESIZE_NONE);
		gr_line(gr_screen.center_offset_x, y1, gr_screen.center_offset_x + gr_screen.center_w, y1, GR_RESIZE_NONE);
		gr_line(gr_screen.center_offset_x, y2, gr_screen.center_offset_x + gr_screen.center_w, y2, GR_RESIZE_NONE);
	}

	int line = panel_y + panel_padding_h;
	const int text_x = panel_x + panel_padding_w;
	gr_printf_no_resize(text_x, line, "%s", XSTR("Photo Mode", 1892));
	line += line_height;
	line += line_height;
	gr_printf_no_resize(text_x, line, "%s", XSTR("Controls", 1893));
	line += line_height;
	gr_printf_no_resize(text_x, line, XSTR("Toggle: %s", 1894), toggle_keybind.c_str());
	line += line_height;
	gr_printf_no_resize(text_x, line, XSTR("Previous Filter: %s", 1895), prev_filter_keybind.c_str());
	line += line_height;
	gr_printf_no_resize(text_x, line, XSTR("Next Filter: %s", 1896), next_filter_keybind.c_str());
	line += line_height;
	gr_printf_no_resize(text_x, line, XSTR("Reset Filters: %s", 1897), reset_filter_keybind.c_str());
	line += line_height;
	gr_printf_no_resize(text_x, line, XSTR("Decrease Parameter: %s", 1898), decrease_param_keybind.c_str());
	line += line_height;
	gr_printf_no_resize(text_x, line, XSTR("Increase Parameter: %s", 1899), increase_param_keybind.c_str());
	line += line_height;
	line += line_height;
	gr_printf_no_resize(text_x, line, "%s", XSTR("Status", 1900));
	line += line_height;
	gr_printf_no_resize(text_x, line, XSTR("Time Compression: %.2fx", 1901), f2fl(Game_time_compression));
	line += line_height;
	gr_printf_no_resize(text_x,
		line,
		XSTR("Cam Pos: X %.1f  Y %.1f  Z %.1f", 1902),
		cam_pos.xyz.x,
		cam_pos.xyz.y,
		cam_pos.xyz.z);
	line += line_height;
	line += line_height;
	gr_set_color_fast(&Color_silver);
	gr_printf_no_resize(text_x, line, "%s", XSTR("Effects", 1914));
	line += line_height;
	for (int i = 0; i < static_cast<int>(Photo_mode_params.size()); ++i) {
		const auto& param = Photo_mode_params[i];
		gr_set_color_fast(Photo_mode_selected_parameter == i ? &Color_bright_white : &Color_silver);

		if (param.type == photo_mode_param_type::INT_RANGE) {
			gr_printf_no_resize(text_x, line, "%s: %d", XSTR(param.label, param.xstr_id), param.value);
		} else if (param.type == photo_mode_param_type::BOOL_TOGGLE) {
			gr_printf_no_resize(text_x, line, "%s: %s", XSTR(param.label, param.xstr_id),
				param.bool_value ? XSTR("On", 1285) : XSTR("Off", 1286));
		}

		line += line_height;
	}

	font::set_font(old_font);
}

void photo_mode_clear_screenshot_queued_flag()
{
	Photo_mode_screenshot_queued_this_frame = false;
}

void photo_mode_set_screenshot_queued_flag()
{
	Photo_mode_screenshot_queued_this_frame = true;
}

void game_toggle_photo_mode()
{
	photo_mode_set_active(!Photo_mode_active);
}

void game_set_photo_mode_allowed(bool allowed)
{
	Photo_mode_allowed = allowed && ((Game_mode & GM_MULTIPLAYER) == 0);

	if (!Photo_mode_allowed && Photo_mode_active) {
		photo_mode_set_active(false);
	}
}

bool game_get_photo_mode_allowed()
{
	return Photo_mode_allowed && ((Game_mode & GM_MULTIPLAYER) == 0);
}

bool game_is_photo_mode_active()
{
	return Photo_mode_active;
}

void game_cycle_photo_mode_filter(int direction)
{
	if (!Photo_mode_active) {
		return;
	}

	Photo_mode_selected_parameter = std::clamp(Photo_mode_selected_parameter + direction, 0, static_cast<int>(Photo_mode_params.size()) - 1);
}

void game_reset_photo_mode_filters()
{
	if (!Photo_mode_active) {
		return;
	}

	for (auto& param : Photo_mode_params) {
		if (param.type == photo_mode_param_type::INT_RANGE) {
			param.value = param.saved_value;
		} else if (param.type == photo_mode_param_type::BOOL_TOGGLE) {
			param.bool_value = false;
		}
	}
	photo_mode_apply_parameter_values();
}

void game_adjust_photo_mode_filter_parameter(int delta)
{
	if (!Photo_mode_active) {
		return;
	}

	auto& param = Photo_mode_params[Photo_mode_selected_parameter];

	if (param.type == photo_mode_param_type::BOOL_TOGGLE) {
		param.bool_value = (delta > 0);
		return;
	}

	param.value = std::clamp(param.value + delta, param.min_val, param.max_val);
	photo_mode_apply_parameter_values();
}

