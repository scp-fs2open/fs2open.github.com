/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _HUDCONFIG_H
#define _HUDCONFIG_H

#include "hud/hud.h"
#include "hud/hudshield.h"
#include "ui/ui.h"

class player;
class ship;
struct ai_info;

#define HUD_COLOR_PRESET_1		0
#define HUD_COLOR_PRESET_2		1
#define HUD_COLOR_PRESET_3		2
#define NUM_HUD_COLOR_PRESETS		3	// Number of default colors.  Keep this up to date.

// specify the max distance that the radar should detect objects
// Index in Radar_ranges[] array to get values

#define RR_MAX_RANGES		3				// keep up to date
#define RR_SHORT			0
#define RR_LONG				1	
#define RR_INFINITY			2
extern float Radar_ranges[RR_MAX_RANGES];
extern const char *Radar_range_text(int range_num);

class HC_gauge_mappings {
  public:
	static HC_gauge_mappings& get_instance()
	{
		static HC_gauge_mappings instance;
		return instance;
	}

	// Prevent copying
	HC_gauge_mappings(const HC_gauge_mappings&) = delete;
	HC_gauge_mappings& operator=(const HC_gauge_mappings&) = delete;

	// Numeric ID -> New String ID
	SCP_string get_string_id_from_numeric_id(int numeric_id) const
	{
		auto it = gauge_map.find(numeric_id);
		return (it != gauge_map.end()) ? it->second.second : "";
	}

	// HCF String Name -> New String ID
	SCP_string get_string_id_from_hcf_id(const SCP_string& hcf_name) const
	{
		for (const auto& [id, mapping] : gauge_map) {
			if (XSTR(mapping.first.first.c_str(), mapping.first.second) == hcf_name) {
				return mapping.second;
			}
		}
		return "";
	}

	// New String ID -> Numeric ID (for saving to player file)
	int get_numeric_id_from_string_id(const SCP_string& string_id) const
	{
		for (const auto& [id, mapping] : gauge_map) {
			if (mapping.second == string_id) {
				return id;
			}
		}
		return -1;
	}

	// New String ID -> HCF String Name (for saving HCF files)
	SCP_string get_hcf_name_from_string_id(const SCP_string& string_id) const
	{
		for (const auto& [id, mapping] : gauge_map) {
			if (mapping.second == string_id) {
				return XSTR(mapping.first.first.c_str(), mapping.first.second);
			}
		}
		return "";
	}

  private:
	HC_gauge_mappings()
	{
		// Map the old gauge_config numeric IDs, the HCF file IDs (which, yes, were translated...), and the newer string
		// IDs for the retail built-in gauges. Custom gauges will only have a string ID and do not need to be mapped.
		gauge_map = {{0, {{"lead indicator", 249}, "Builtin::LeadIndicator"}},
			{1, {{"target orientation", 250}, "Builtin::TargetOrientation"}},
			{2, {{"closest attacking hostile", 251}, "Builtin::ClosestAttackingHostile"}},
			{3, {{"current target direction", 252}, "Builtin::CurrentTargetDirection"}},
			{4, {{"mission time", 253}, "Builtin::MissionTime"}},
			{5, {{"reticle", 254}, "Builtin::Reticle"}},
			{6, {{"throttle", 255}, "Builtin::Throttle"}},
			{7, {{"radar", 256}, "Builtin::Radar"}},
			{8, {{"target monitor", 257}, "Builtin::TargetMonitor"}},
			{9, {{"center of reticle", 258}, "Builtin::CenterOfReticle"}},
			{10, {{"extra target info", 259}, "Builtin::ExtraTargetInfo"}},
			{11, {{"target shield", 260}, "Builtin::TargetShield"}},
			{12, {{"player shield", 261}, "Builtin::PlayerShield"}},
			{13, {{"power management", 262}, "Builtin::PowerManagement"}},
			{14, {{"auto-target icon", 263}, "Builtin::AutoTargetIcon"}},
			{15, {{"auto-speed-match icon", 264}, "Builtin::AutoSpeedMatchIcon"}},
			{16, {{"weapons display", 265}, "Builtin::WeaponsDisplay"}},
			{17, {{"monitoring view", 266}, "Builtin::MonitoringView"}},
			{18, {{"directives view", 267}, "Builtin::DirectivesView"}},
			{19, {{"threat gauge", 268}, "Builtin::ThreatGauge"}},
			{20, {{"afterburner energy", 269}, "Builtin::AfterburnerEnergy"}},
			{21, {{"weapons energy", 270}, "Builtin::WeaponsEnergy"}},
			{22, {{"weapon linking", 271}, "Builtin::WeaponLinking"}},
			{23, {{"target hull/shield icon", 272}, "Builtin::TargetHullShieldIcon"}},
			{24, {{"offscreen indicator", 273}, "Builtin::OffscreenIndicator"}},
			{25, {{"comm video", 274}, "Builtin::CommVideo"}},
			{26, {{"damage display", 275}, "Builtin::DamageDisplay"}},
			{27, {{"message output", 276}, "Builtin::MessageOutput"}},
			{28, {{"locked missile direction", 277}, "Builtin::LockedMissileDirection"}},
			{29, {{"countermeasures", 278}, "Builtin::Countermeasures"}},
			{30, {{"objective notify", 279}, "Builtin::ObjectiveNotify"}},
			{31, {{"wingmen status", 280}, "Builtin::WingmenStatus"}},
			{32, {{"offscreen range", 281}, "Builtin::OffscreenRange"}},
			{33, {{"kills gauge", 282}, "Builtin::KillsGauge"}},
			{34, {{"attacking target count", 283}, "Builtin::AttackingTargetCount"}},
			{35, {{"warning flash", 1459}, "Builtin::WarningFlash"}},
			{36, {{"comm menu", 1460}, "Builtin::CommMenu"}},
			{37, {{"support gauge", 1461}, "Builtin::SupportGauge"}},
			{38, {{"lag gauge", 1462}, "Builtin::LagGauge"}}};
	}

	SCP_unordered_map<int, std::pair<std::pair<SCP_string, int>, SCP_string>> gauge_map;
};

/*!
 * @brief Vector for storing the filenames of hud preset files
 * @note main definition in hudconfig.cpp
 */
extern SCP_vector<SCP_string> HC_preset_filenames;

extern SCP_vector<SCP_string> observer_visible_gauges;

extern const int HC_gauge_config_coords[GR_NUM_RESOLUTIONS][4];
extern int HC_resize_mode;

/*!
 * @brief get the gauge pointer for the given gauge index
 */
HudGauge* hud_config_get_gauge_pointer(const SCP_string& gauge_id);

/**
 * @brief Contains core HUD configuration data
 * @note Is not default init'd.  Assumes new player, PLR, or CSG reads will correctly set data.
 */
typedef struct HUD_CONFIG_TYPE {
	int rp_dist;				// one of RR_ #defines above; Is the maxium radar view distance setting
	bool is_observer;			// 1 or 0, observer mode or not, respectively
	int main_color;				// the default HUD_COLOR selection for all gauges; each gauge may override this with a custom RGB

	// Maps for HUD gauge configuration
	SCP_unordered_map<SCP_string, color> gauge_colors;     // Gauge-specific colors
	SCP_unordered_map<SCP_string, bool> show_flags_map;   // Show/hide state for gauges
	SCP_unordered_map<SCP_string, bool> popup_flags_map;   // Popup state for gauges
	SCP_unordered_map<SCP_string, int> gauge_types;      // Gauge types used for backup colors for custom gauges to preseve old behavior

	static color default_white_color()
	{
		color white;
		gr_init_alphacolor(&white, 255, 255, 255, 255);
		return white;
	}

	// Methods for setting and getting gauge properties where getting will return a default value

	void set_gauge_color(const SCP_string& gauge_id, const color& col)
	{
		gauge_colors[gauge_id] = col;
	}

	// Get the gauge color, the color based on its type, or white if the gauge is not found
	color get_gauge_color(const SCP_string& gauge_id, bool check_exact_match = true) const
	{
		auto it = gauge_colors.find(gauge_id);

		// Got a match? Return it
		// but only if we are using the exact match
		if (check_exact_match && it != gauge_colors.end()) {
			return it->second;
		}

		// No match yet.. try using the gauge type
		auto type_it = gauge_types.find(gauge_id);
		if (type_it != gauge_types.end()) {
			const HC_gauge_mappings& gauge_map = HC_gauge_mappings::get_instance();

			it = gauge_colors.find(gauge_map.get_string_id_from_numeric_id(type_it->second));
			if (it != gauge_colors.end()) {
				return it->second;
			}
		}

		// Still nothing.. return white
		return default_white_color();
	}

	void set_gauge_type(const SCP_string& gauge_id, int type) {
		gauge_types[gauge_id] = type;
	}

	void set_gauge_visibility(const SCP_string& gauge_id, bool visible)
	{
		show_flags_map[gauge_id] = visible;
	}

	// Get the gauge config visibility setting or false (invisible) if not found
	bool is_gauge_visible(const SCP_string& gauge_id) const
	{
		auto it = show_flags_map.find(gauge_id);
		return (it != show_flags_map.end()) ? it->second : false; // Default to invisible
	}

	void set_gauge_popup(const SCP_string& gauge_id, bool popup)
	{
		popup_flags_map[gauge_id] = popup;
	}

	// Get the gauge config popup setting or false if not found
	bool is_gauge_popup(const SCP_string& gauge_id) const
	{
		auto it = popup_flags_map.find(gauge_id);
		return (it != popup_flags_map.end()) ? it->second : false; // Default to not a popup
	}

	// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
	bool is_gauge_shown_in_config(const SCP_string& gauge_id) const
	{
		HudGauge* gauge = hud_config_get_gauge_pointer(gauge_id);
		return gauge && gauge->getVisibleInConfig();
	}
} HUD_CONFIG_TYPE;

extern HUD_CONFIG_TYPE HUD_config;

// HUD Color presets
typedef struct hc_col {
	int r, g, b;
	SCP_string name;
	int xstr = -1;
} hc_col;

extern hc_col HC_colors[NUM_HUD_COLOR_PRESETS];

extern int HC_default_color;
extern SCP_string HC_default_preset_file;


/**
 * @struct HC_gauge_setting
 * @brief Contains core HUD configuration data
 */
struct HC_gauge_setting
{
	bool use_iff_color;  // if the gauge uses target IFF color for its color
	bool can_popup;      // if the gauge can use the popup method
	bool use_tag_color;  // If the gauge color respects target tagging?

	HC_gauge_setting(bool iff, bool cp, bool cl) : use_iff_color(iff), can_popup(cp), use_tag_color(cl){}
};

class BoundingBox {
  public:
	int x1, x2, y1, y2;

	// Default constructor (initializes to invalid state)
	constexpr BoundingBox() : x1(-1), x2(-1), y1(-1), y2(-1) {}

	// Constructor
	constexpr BoundingBox(int nx1, int nx2, int ny1, int ny2) : x1(nx1), x2(nx2), y1(ny1), y2(ny2) {}

	// Equality operator
	bool operator==(const BoundingBox& other) const
	{
		return x1 == other.x1 && x2 == other.x2 && y1 == other.y1 && y2 == other.y2;
	}

	bool isOverlapping(const BoundingBox& other) const
	{
		return (x2 >= other.x1 && // Not completely to the left
				x1 <= other.x2 && // Not completely to the right
				y2 >= other.y1 && // Not completely above
				y1 <= other.y2);  // Not completely below
	}

	// Check if the bounding box is valid (no negative coordinates)
	bool isValid() const
	{
		return x1 >= 0 && x2 >= 0 && y1 >= 0 && y2 >= 0;
	}

	// Static function to check if any bounding box in an array overlaps with a new one
	static bool isOverlappingAny(const SCP_vector<std::pair<SCP_string, BoundingBox>>& mouse_coords, const BoundingBox& newBox, SCP_string self_id)
	{
		return std::any_of(mouse_coords.begin(), mouse_coords.end(), [&](const std::pair<SCP_string, BoundingBox>& item) {
			const auto& [gauge_id, bbox] = item;
			if (gauge_id == self_id) {
				return false; // Skip checking against itself
			}
			return bbox.isValid() && bbox.isOverlapping(newBox);
		});
	}
};

extern char HC_wingam_gauge_status_names[MAX_SQUADRON_WINGS][32];

extern SCP_vector<std::pair<SCP_string, HudGauge*>> HC_gauge_map;
extern SCP_string HC_gauge_hot;
extern SCP_string HC_gauge_selected;
extern SCP_vector<std::pair<size_t, SCP_string>> HC_available_huds;
extern int HC_chosen_hud;
extern bool HC_select_all;
extern int HC_gauge_coordinates[6]; // x1, x2, y1, y2, w, h for gauge rendering
extern SCP_vector<std::pair<SCP_string, BoundingBox>> HC_gauge_mouse_coords;

extern int HC_talking_head_frame;
extern SCP_string HC_head_anim_filename;
extern bool HC_show_default_hud;
extern std::unordered_set<SCP_string> HC_ignored_huds;
extern SCP_map<SCP_string, std::array<SCP_string, num_shield_gauge_types>> HC_hud_shield_ships;
extern SCP_map<SCP_string, SCP_vector<SCP_string>> HC_hud_primary_weapons;
extern SCP_map<SCP_string, SCP_vector<SCP_string>> HC_hud_secondary_weapons;


/*!
 * @brief init hud config screen, setting up the hud preview display
 * 
 * param[in] API_Access		whether or not this method has been called from the lua api
 * param[in] x				the x coord to render the preview display
 * param[in] y				the y coord to render the preview display
 * param[in] w				the width to render the preview display
 * param[in] h				the height to render the preview display
 */
void hud_config_init(bool API_Access = false, int x = 0, int y = 0, int w = -1, int h = -1);

/*!
 * @brief do a hud config frame, including rendering the preview display and checking for button presses
 * 
 * param[in] frametime		unused
 * param[in] API_Access		whether or not this method has been called from the lua api
 * param[in] mx				mouse x coordinate
 * param[in] my				mouse y coordinate
 */
void hud_config_do_frame(float frametime, bool API_Access = false, int mx = 0, int my = 0);

/*!
 * @brief close hud config and release all the bitmaps
 * 
 * param[in] API_Access		whether or not this method has been called from the lua api
 */
void hud_config_close(bool API_Access = false);

/*!
 * @brief toggles selecting of all gauges for color modification
 * 
 * param[bool]              toggle true to toggle on, false for off
 * param[in] API_Access		whether or not this method has been called from the lua api
 */
void hud_config_select_all_toggle(bool toggle, bool API_Access = false);

/*!
 * @brief sets no gauges as selected
 */
void hud_config_select_none();

/*!
 * @brief sets no gauges as selected
 */
void hud_config_select_hud(bool next);

/*!
 * @brief init the list of preset files found by cfile
 */
void hud_config_preset_init();

/*!
 * @brief set the entire hud to the default setting or the filename provided
 * 
 * param[in] filename	the preset filename to use as the default
 */
void hud_set_default_hud_config(player* p, const SCP_string& filename);

/*!
 * @brief delete a preset file and remove it from the preset vector
 */
void hud_config_delete_preset(SCP_string filename);

/*!
 * @brief set gauge flags
 * 
 * param[in] gauge_index	the gauge to modify
 * param[in] on_flag		if the gauge is on or off, 1 for on, 0 for off
 * param[in] popup_flag		if the gauge is set to popup, 1 for popup, 0 otherwise
 */
void hud_config_set_gauge_flags(const SCP_string& gauge, bool on_flag, bool popup_flag);

void hud_config_restore();
void hud_config_backup();
void hud_config_as_observer(ship *shipp,ai_info *aif);

void hud_config_as_player();

/*!
 * @brief leave hud config without accepting the player's changes
 */
void hud_config_cancel(bool change_state = true);

/*!
 * @brief leave hud config and save the player's changes
 */
void hud_config_commit();

void hud_config_record_color(int color);
void hud_config_set_color(int color);

/*!
 * @brief load a preset file to the hud gauges
 */
void hud_config_color_load(const char *name);

/*!
 * @brief save the current hud gauges settings to a preset file
 */
void hud_config_color_save(const char* name, int version = 2);

/*!
 * @brief same as hud_config_covert_coords but only returns the scale and doesn't convert any coords
 */
float hud_config_get_scale(int baseW, int baseH);

/*!
 * @brief converts a set of coordinates to HUD Config's rendering coordinates
 */
std::pair<int, int> hud_config_convert_coords(int x, int y, float scale);

/*!
 * @brief converts a set of coordinates to HUD Config's rendering coordinates
 */
std::pair<float, float> hud_config_convert_coords(float x, float y, float scale);

/*!
* @brief convert the given HUD gauge position coordinates to a set more appropriate for the HUD Config UI and return the smaller scale value used
* so that other offsets and positions can be multiplied in turn
*/
std::tuple<int, int, float> hud_config_convert_coord_sys(int x, int y, int baseW, int baseH);

/*!
 * @brief convert the given HUD gauge position coordinates to a set more appropriate for the HUD Config UI and return
 * the smaller scale value used so that other offsets and positions can be multiplied in turn
 */
std::tuple<float, float, float> hud_config_convert_coord_sys(float x, float y, int baseW, int baseH);

/*!
 * @brief save gauge coords during rendering time so hud config can check if the mouse is hovering over the gauge
 */
void hud_config_set_mouse_coords(const SCP_string& gauge_id, int x1, int x2, int y1, int y2);

/*!
 * @brief Function to calculate screen coordinates based on angle. Used for radial positioning gauges
 */
std::pair<float, float> hud_config_calc_coords_from_angle(float angle_degrees, int centerX, int centerY, float radius);

/*!
 * @brief try to find an angle with no overlapping mouse coordinates for target-related gauges
 */
float hud_config_find_valid_angle(const SCP_string& gauge, float initial_angle, int centerX, int centerY, float radius);

/*!
 * @brief generage a custom gauge id
 */
SCP_string create_custom_gauge_id(const SCP_string& gauge_name);

#endif

