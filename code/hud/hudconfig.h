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
#include "ui/ui.h"

class player;
class ship;
struct ai_info;

#define HUD_COLOR_GREEN		0
#define HUD_COLOR_BLUE		1
#define HUD_COLOR_AMBER		2
#define HUD_COLOR_SIZE		3	// Number of default colors.  Keep this up to date.

// specify the max distance that the radar should detect objects
// Index in Radar_ranges[] array to get values

#define RR_MAX_RANGES		3				// keep up to date
#define RR_SHORT			0
#define RR_LONG				1	
#define RR_INFINITY			2
extern float Radar_ranges[RR_MAX_RANGES];
extern const char *Radar_range_text(int range_num);

#define RP_SHOW_DEBRIS						(1<<0)
#define RP_SHOW_FRIENDLY_MISSILES		(1<<1)
#define RP_SHOW_HOSTILE_MISSILES			(1<<2)

#define RP_DEFAULT ( RP_SHOW_DEBRIS | RP_SHOW_FRIENDLY_MISSILES | RP_SHOW_HOSTILE_MISSILES )

/*!
 * @brief Vector for storing the filenames of hud preset files
 * @note main definition in hudconfig.cpp
 */
extern SCP_vector<SCP_string> HC_preset_filenames;

extern int HUD_observer_default_flags;
extern int HUD_observer_default_flags2;
extern int HUD_default_popup_mask;
extern int HUD_default_popup_mask2;
extern int HUD_config_default_flags;
extern int HUD_config_default_flags2;

extern const int HC_gauge_config_coords[GR_NUM_RESOLUTIONS][4];
extern int HC_resize_mode;

/**
 * @brief Contains core HUD configuration data
 * @note Is not default init'd.  Assumes new player, PLR, or CSG reads will correctly set data.
 */
typedef struct HUD_CONFIG_TYPE {		
	int show_flags;				//!< bitfield, whether to show gauge (0 ~ 31)
	int show_flags2;			//!< bitfield, whether to show gauge (32 ~ 63)
	int popup_flags;			//!< bitfield, whether gauge is popup (0 ~ 31)
	int popup_flags2;			//!< bitfield, whether gauge is popup (32 ~ 63)
	int rp_flags;				//!< one of RP_ #defines in hudconfig.h;  Chiefly shows/hides non-ship objects
	int rp_dist;				//!< one of RR_ #defines above; Is the maxium radar view distance setting
	int is_observer;			//!< 1 or 0, observer mode or not, respectively
	int main_color;				//!< the default HUD_COLOR selection for all gauges; each gauge may override this with a custom RGB
	ubyte num_msg_window_lines;	//!< Number of message lines. (Deprecated by HudGaugeMessages::Max_lines)

	color clr[NUM_HUD_GAUGES];	//!< colors for all the gauges
} HUD_CONFIG_TYPE;

extern HUD_CONFIG_TYPE HUD_config;

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
	static bool isOverlappingAny(const SCP_vector<std::pair<int, BoundingBox>>& mouse_coords, const BoundingBox& newBox, int self_index)
	{
		return std::any_of(mouse_coords.begin(), mouse_coords.end(), [&](const std::pair<int, BoundingBox>& item) {
			const auto& [gauge_id, bbox] = item;
			if (gauge_id == self_index) {
				return false; // Skip checking against itself
			}
			return bbox.isValid() && bbox.isOverlapping(newBox);
		});
	}
};

extern char HC_wingam_gauge_status_names[MAX_SQUADRON_WINGS][32];

extern int HC_gauge_hot;
extern int HC_gauge_selected;
extern SCP_vector<std::pair<size_t, SCP_string>> HC_available_huds;
extern int HC_chosen_hud;
extern bool HC_select_all;
extern int HC_gauge_coordinates[6]; // x1, x2, y1, y2, w, h for gauge rendering
extern SCP_vector<std::pair<int, BoundingBox>> HC_gauge_mouse_coords;

const char* HC_gauge_descriptions(int n);

extern int HC_talking_head_frame;
extern SCP_string HC_head_anim_filename;
extern SCP_string HC_shield_gauge_ship;
extern bool HC_show_default_hud;
extern std::unordered_set<SCP_string> HC_ignored_huds;

/*!
 * @brief get the gauge pointer for the given gauge index
 */
HudGauge* hud_config_get_gauge_pointer(int gauge_index);

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
void hud_set_default_hud_config(player* p, const char* filename = "hud_3.hcf");

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
void hud_config_set_gauge_flags(int gauge_index, int on_flag, int popup_flag);

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

// flag access/manipulation routines
int	hud_config_show_flag_is_set(int i);
void	hud_config_show_flag_set(int i);
void	hud_config_show_flag_clear(int i);
int	hud_config_popup_flag_is_set(int i);
void	hud_config_popup_flag_set(int i);
void	hud_config_popup_flag_clear(int i);

void hud_config_record_color(int color);
void hud_config_set_color(int color);

/*!
 * @brief load a preset file to the hud gauges
 */
void hud_config_color_load(const char *name);

/*!
 * @brief save the current hud gauges settings to a preset file
 */
void hud_config_color_save(const char* name);

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
void hud_config_set_mouse_coords(int gauge_config, int x1, int x2, int y1, int y2);

/*!
 * @brief Function to calculate screen coordinates based on angle. Used for radial positioning gauges
 */
std::pair<float, float> hud_config_calc_coords_from_angle(float angle_degrees, int centerX, int centerY, float radius);

/*!
 * @brief try to find an angle with no overlapping mouse coordinates for target-related gauges
 */
float hud_config_find_valid_angle(int gauge_index, float initial_angle, int centerX, int centerY, float radius);

#endif

