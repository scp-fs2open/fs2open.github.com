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
};

enum photo_mode_parameter {
	PHOTO_MODE_PARAM_SATURATION = 0,
	PHOTO_MODE_PARAM_BRIGHTNESS,
	PHOTO_MODE_PARAM_CONTRAST,
	PHOTO_MODE_PARAM_COUNT
};

SCP_vector<photo_mode_post_effect_state> Photo_mode_saved_post_effects;
std::array<int, PHOTO_MODE_PARAM_COUNT> Photo_mode_saved_parameter_values = {100, 100, 100};
std::array<int, PHOTO_MODE_PARAM_COUNT> Photo_mode_parameter_values = {100, 100, 100};
int Photo_mode_selected_parameter = PHOTO_MODE_PARAM_SATURATION;
bool Photo_mode_grid_enabled = false;

const char* photo_mode_get_parameter_effect_name(int index)
{
	switch (index) {
	case PHOTO_MODE_PARAM_SATURATION:
		return "saturation";
	case PHOTO_MODE_PARAM_BRIGHTNESS:
		return "brightness";
	case PHOTO_MODE_PARAM_CONTRAST:
		return "contrast";
	default:
		return nullptr;
	}
}

void photo_mode_capture_post_effect_state()
{
	Photo_mode_saved_post_effects.clear();

	if (graphics::Post_processing_manager == nullptr) {
		return;
	}

	const auto& post_effects = graphics::Post_processing_manager->getPostEffects();
	Photo_mode_saved_post_effects.reserve(post_effects.size());

	for (const auto& effect : post_effects) {
		photo_mode_post_effect_state state;
		state.name = effect.name;
		state.intensity = effect.intensity;
		state.rgb = effect.rgb;
		Photo_mode_saved_post_effects.push_back(state);
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
		for (const auto& effect : post_effects) {
			if (!stricmp(effect.name.c_str(), saved_state.name.c_str())) {
				const int value = static_cast<int>(std::lround((saved_state.intensity - effect.add) * effect.div));
				gr_post_process_set_effect(saved_state.name.c_str(), value, &saved_state.rgb);
				break;
			}
		}
	}

	if (clear_saved_state) {
		Photo_mode_saved_post_effects.clear();
	}
}

int photo_mode_get_saved_post_effect_value(const char* effect_name)
{
	int value = 100;

	if (effect_name == nullptr || graphics::Post_processing_manager == nullptr) {
		return value;
	}

	for (const auto& saved_state : Photo_mode_saved_post_effects) {
		if (stricmp(saved_state.name.c_str(), effect_name) != 0) {
			continue;
		}

		for (const auto& effect : graphics::Post_processing_manager->getPostEffects()) {
			if (stricmp(effect.name.c_str(), effect_name) == 0) {
				value = static_cast<int>(std::lround((saved_state.intensity - effect.add) * effect.div));
				break;
			}
		}

		break;
	}

	return value;
}

void photo_mode_sync_parameter_values_from_saved_state()
{
	for (int i = 0; i < PHOTO_MODE_PARAM_COUNT; ++i) {
		const auto effect_name = photo_mode_get_parameter_effect_name(i);
		Photo_mode_saved_parameter_values[i] = photo_mode_get_saved_post_effect_value(effect_name);
		Photo_mode_parameter_values[i] = Photo_mode_saved_parameter_values[i];
	}

	Photo_mode_selected_parameter = PHOTO_MODE_PARAM_SATURATION;
}

void photo_mode_apply_parameter_values()
{
	for (int i = 0; i < PHOTO_MODE_PARAM_COUNT; ++i) {
		const auto effect_name = photo_mode_get_parameter_effect_name(i);
		if (effect_name == nullptr) {
			continue;
		}

		gr_post_process_set_effect(effect_name, Photo_mode_parameter_values[i], nullptr);
	}
}

SCP_string format_photo_mode_keybind(int action)
{
	auto primary = Control_config[action].first.textify();
	auto secondary = Control_config[action].second.textify();

	if (primary.empty() && secondary.empty()) {
		return SCP_string(XSTR("Unbound", 1909));
	}
	if (primary.empty()) {
		return secondary;
	}
	if (secondary.empty()) {
		return primary;
	}

	return primary + " / " + secondary;
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
	Photo_mode_saved_parameter_values = {100, 100, 100};
	Photo_mode_parameter_values = {100, 100, 100};
	Photo_mode_selected_parameter = PHOTO_MODE_PARAM_SATURATION;
	Photo_mode_grid_enabled = false;

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

	const float forward = check_control_timef(FORWARD_THRUST) - check_control_timef(REVERSE_THRUST);
	const float right = check_control_timef(RIGHT_SLIDE_THRUST) - check_control_timef(LEFT_SLIDE_THRUST);
	const float up = check_control_timef(UP_SLIDE_THRUST) - check_control_timef(DOWN_SLIDE_THRUST);

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
	const auto grid_keybind = format_photo_mode_keybind(PHOTO_MODE_TOGGLE_GRID);

	gr_set_color_fast(&Color_silver);
	const auto old_font = font::get_current_fontnum();
	font::set_font(font::FONT1);
	const int line_height = gr_get_font_height();
	const int panel_padding = 8;
	const int panel_width = 530;
	const int panel_height = panel_padding * 2 + line_height * 18;
	const int panel_x = gr_screen.center_offset_x + (gr_screen.center_w / 4);
	const int panel_y = gr_screen.center_offset_y + gr_screen.center_h - panel_height - (gr_screen.center_h / 6);

	gr_line(panel_x, panel_y, panel_x + panel_width, panel_y, GR_RESIZE_NONE);
	gr_line(panel_x, panel_y + panel_height, panel_x + panel_width, panel_y + panel_height, GR_RESIZE_NONE);
	gr_line(panel_x, panel_y, panel_x, panel_y + panel_height, GR_RESIZE_NONE);
	gr_line(panel_x + panel_width, panel_y, panel_x + panel_width, panel_y + panel_height, GR_RESIZE_NONE);
	gr_line(panel_x,
		panel_y + line_height + panel_padding + 1,
		panel_x + panel_width,
		panel_y + line_height + panel_padding + 1,
		GR_RESIZE_NONE);

	if (Photo_mode_grid_enabled) {
		const int x1 = gr_screen.center_offset_x + gr_screen.center_w / 3;
		const int x2 = gr_screen.center_offset_x + (gr_screen.center_w * 2) / 3;
		const int y1 = gr_screen.center_offset_y + gr_screen.center_h / 3;
		const int y2 = gr_screen.center_offset_y + (gr_screen.center_h * 2) / 3;
		gr_line(x1, gr_screen.center_offset_y, x1, gr_screen.center_offset_y + gr_screen.center_h, GR_RESIZE_NONE);
		gr_line(x2, gr_screen.center_offset_y, x2, gr_screen.center_offset_y + gr_screen.center_h, GR_RESIZE_NONE);
		gr_line(gr_screen.center_offset_x, y1, gr_screen.center_offset_x + gr_screen.center_w, y1, GR_RESIZE_NONE);
		gr_line(gr_screen.center_offset_x, y2, gr_screen.center_offset_x + gr_screen.center_w, y2, GR_RESIZE_NONE);
	}

	int line = panel_y + panel_padding;
	const int text_x = panel_x + panel_padding;
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
	gr_printf_no_resize(text_x, line, XSTR("Decrease Parameter: %s", 1899), decrease_param_keybind.c_str());
	line += line_height;
	gr_printf_no_resize(text_x, line, XSTR("Increase Parameter: %s", 1900), increase_param_keybind.c_str());
	line += line_height;
	gr_printf_no_resize(text_x, line, XSTR("Toggle Thirds Grid: %s", 1901), grid_keybind.c_str());
	line += line_height;
	line += line_height;
	gr_printf_no_resize(text_x, line, "%s", XSTR("Status", 1902));
	line += line_height;
	gr_printf_no_resize(text_x, line, XSTR("Time Compression: %.2fx", 1903), f2fl(Game_time_compression));
	line += line_height;
	gr_printf_no_resize(text_x,
		line,
		XSTR("Cam Pos: X %.1f  Y %.1f  Z %.1f", 1904),
		cam_pos.xyz.x,
		cam_pos.xyz.y,
		cam_pos.xyz.z);
	line += line_height;
	gr_set_color_fast(
		Photo_mode_selected_parameter == PHOTO_MODE_PARAM_SATURATION ? &Color_bright_white : &Color_silver);
	gr_printf_no_resize(text_x,
		line,
		XSTR("Saturation: %d", 1905),
		Photo_mode_parameter_values[PHOTO_MODE_PARAM_SATURATION]);
	line += line_height;
	gr_set_color_fast(
		Photo_mode_selected_parameter == PHOTO_MODE_PARAM_BRIGHTNESS ? &Color_bright_white : &Color_silver);
	gr_printf_no_resize(text_x,
		line,
		XSTR("Brightness: %d", 1906),
		Photo_mode_parameter_values[PHOTO_MODE_PARAM_BRIGHTNESS]);
	line += line_height;
	gr_set_color_fast(Photo_mode_selected_parameter == PHOTO_MODE_PARAM_CONTRAST ? &Color_bright_white : &Color_silver);
	gr_printf_no_resize(text_x, line, XSTR("Contrast: %d", 1907), Photo_mode_parameter_values[PHOTO_MODE_PARAM_CONTRAST]);
	line += line_height;
	gr_set_color_fast(&Color_silver);
	gr_printf_no_resize(text_x, line, XSTR("Grid: %s", 1908), Photo_mode_grid_enabled ? XSTR("On", 1285) : XSTR("Off", 1286));

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

	Photo_mode_selected_parameter += direction;
	while (Photo_mode_selected_parameter < 0) {
		Photo_mode_selected_parameter += PHOTO_MODE_PARAM_COUNT;
	}
	while (Photo_mode_selected_parameter >= PHOTO_MODE_PARAM_COUNT) {
		Photo_mode_selected_parameter -= PHOTO_MODE_PARAM_COUNT;
	}
}

void game_reset_photo_mode_filters()
{
	if (!Photo_mode_active) {
		return;
	}

	Photo_mode_parameter_values = Photo_mode_saved_parameter_values;
	photo_mode_apply_parameter_values();
}

void game_adjust_photo_mode_filter_parameter(int delta)
{
	if (!Photo_mode_active) {
		return;
	}

	Photo_mode_parameter_values[Photo_mode_selected_parameter] =
		std::clamp(Photo_mode_parameter_values[Photo_mode_selected_parameter] + delta, 0, 200);
	photo_mode_apply_parameter_values();
}

void game_toggle_photo_mode_grid()
{
	if (!Photo_mode_active) {
		return;
	}

	Photo_mode_grid_enabled = !Photo_mode_grid_enabled;
}
