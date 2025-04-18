#ifndef _PLAYER_HUDPREFS_H
#define _PLAYER_HUDPREFS_H

// Saves the player's current HUD preferences (vis/popup/color for all gauges)
// to <callsign>.hdp
void hud_config_save_player_prefs(const char* callsign);

// Loads the player's current HUD preferences (vis/popup/color for all gauges)
// from <callsign>.hdp
void hud_config_load_player_prefs(const char* callsign);

#endif // _PLAYER_HUDPREFS_H