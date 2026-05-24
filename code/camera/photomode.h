#pragma once

void photo_mode_set_active(bool active);
void photo_mode_do_frame(float frame_time);
void photo_mode_maybe_render_hud();
void photo_mode_clear_screenshot_queued_flag();
void photo_mode_set_screenshot_queued_flag();

// mission-level permission to allow/disallow Photo Mode
void game_toggle_photo_mode();
void game_set_photo_mode_allowed(bool allowed);
bool game_get_photo_mode_allowed();
bool game_is_photo_mode_active();
void game_cycle_photo_mode_filter(int direction);
void game_reset_photo_mode_filters();
void game_adjust_photo_mode_filter_parameter(int delta);