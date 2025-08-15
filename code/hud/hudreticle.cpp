/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "gamesnd/gamesnd.h"
#include "hud/hudconfig.h"
#include "hud/hudreticle.h"
#include "hud/hudtargetbox.h"
#include "io/timer.h"
#include "localization/localize.h"
#include "network/multi.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "weapon/emp.h"
#include "weapon/weapon.h"
#include "globalincs/alphacolors.h"

#define NUM_RETICLE_ANIS			11		// keep up to date when modifying the number of reticle ani files

#define RETICLE_TOP_ARC				0
#define RETICLE_LASER_WARN			1
#define RETICLE_LOCK_WARN			2
#define RETICLE_LEFT_ARC			3
#define RETICLE_RIGHT_ARC			4
#define RETICLE_ONE_PRIMARY			5
#define RETICLE_TWO_PRIMARY			6
#define RETICLE_ONE_SECONDARY		7
#define RETICLE_TWO_SECONDARY		8
#define RETICLE_THREE_SECONDARY		9
//#define RETICLE_LAUNCH_LABEL		5
#define RETICLE_CENTER				10

int Hud_throttle_frame_w[GR_NUM_RESOLUTIONS] = {
	49, 
	78
};
int Hud_throttle_h[GR_NUM_RESOLUTIONS] = {
	50, 80
};
int Hud_throttle_bottom_y[NUM_HUD_RETICLE_STYLES][GR_NUM_RESOLUTIONS] = {
	{ 309, 494 },
	{ 307, 491 }
};
int Hud_throttle_aburn_h[GR_NUM_RESOLUTIONS] = {
	17,
	27
};
int Hud_throttle_aburn_button[GR_NUM_RESOLUTIONS] = {
	259,
	414
};

int Outer_circle_radius[GR_NUM_RESOLUTIONS] = {
	104,
	166
};

int Hud_reticle_center[GR_NUM_RESOLUTIONS][2] =
{
	{ // GR_640
		320, 242
	},
	{ // GR_1024
		512, 387
	}
};

char Reticle_frame_names[NUM_HUD_RETICLE_STYLES][GR_NUM_RESOLUTIONS][NUM_RETICLE_ANIS][MAX_FILENAME_LEN] = 
{
//XSTR:OFF
	{
		{ // GR_640
			"toparc1_fs1",
			"toparc2_fs1",
			"toparc3_fs1",
			"leftarc_fs1",
			"rightarc1_fs1",
			"rightarc2_fs1",
			"rightarc3_fs1",
			"rightarc4_fs1",
			"rightarc5_fs1",
			"rightarc6_fs1",
//			"toparc4_fs1",
			"reticle1_fs1",	
		}, 
		{ // GR_1024
			"2_toparc1_fs1",
			"2_toparc2_fs1",
			"2_toparc3_fs1",
			"2_leftarc_fs1",
			"2_rightarc1_fs1",
			"2_rightarc2_fs1",
			"2_rightarc3_fs1",
			"2_rightarc4_fs1",
			"2_rightarc5_fs1",
			"2_rightarc6_fs1",
//			"2_toparc4_fs1",
			"2_reticle1_fs1",	
		}
	},
	{
		{ // GR_640
			"toparc1",
			"toparc2",
			"toparc3",
			"leftarc",
			"rightarc1",
			"<none>",
			"<none>",
			"<none>",
			"<none>",
			"<none>",
//			"<none>",
			"reticle1",	
		}, 
		{ // GR_1024
			"2_toparc1",
			"2_toparc2",
			"2_toparc3",
			"2_leftarc",
			"2_rightarc1",
			"<none>",
			"<none>",
			"<none>",
			"<none>",
			"<none>",
//			"<none>",
			"2_reticle1",	
		}
	}
//XSTR:ON
};

// reticle frame coords
int Reticle_frame_coords[NUM_HUD_RETICLE_STYLES][GR_NUM_RESOLUTIONS][NUM_RETICLE_ANIS][2] =
{
	{
		{ // GR_640
			{241, 137},
			{300, 137},
			{320, 137},
			{217, 244},
			{374, 242},
			{406, 253},
			{406, 253},
			{391, 276},
			{391, 276},
			{391, 276},
//			{297, 162},
			{308, 235}
		}, 
		{ // GR_1024
			{386, 219},
			{480, 219},
			{512, 219},
			{347, 390},
			{598, 387},
			{650, 405},
			{650, 405},
			{626, 442},
			{626, 442},
			{626, 442},
//			{475, 259},
			{493, 376}
		},
	},
	{
		{ // GR_640
			{241, 137},
			{400, 245},
			{394, 261},
			{216, 168},
			{359, 168},
			{406, 253},
			{406, 253},
			{391, 276},
			{391, 276},
			{391, 276},
//			{297, 161},
			{308, 235}
		}, 
		{ // GR_1024
			{386, 219},
			{640, 393},
			{631, 419},
			{346, 269},
			{574, 269},
			{649, 401},
			{649, 401},
			{625, 438},
			{625, 438},
			{625, 438},
//			{475, 258},
			{493, 370}
		}
	}
};

// "launch" gauge coords
int Reticle_launch_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		297,	161
	},
	{ // GR_1024
		475,	258
	}
};

#define THREAT_DUMBFIRE				(1<<0)
#define THREAT_ATTEMPT_LOCK			(1<<1)
#define THREAT_LOCK					(1<<2)

#define THREAT_UPDATE_DUMBFIRE_TIME		1000		// time between checking for dumbfire threats
#define THREAT_UPDATE_LOCK_TIME			500		// time between checking for lock threats

#define THREAT_DUMBFIRE_FLASH				180
#define THREAT_LOCK_FLASH					180
static int Threat_lock_timer;				// timestamp for when to show next flashing frame for lock threat
static int Threat_lock_frame;				// frame offset of current lock flashing warning

static vertex Player_flight_cursor_offset;


HudGaugeReticle::HudGaugeReticle():
HudGauge(HUD_OBJECT_CENTER_RETICLE, HUD_CENTER_RETICLE, true, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_TOPDOWN | VM_OTHER_SHIP), 255, 255, 255)
{
	has_autoaim_lock = false;
}

void HudGaugeReticle::initBitmaps(char *fname)
{
	crosshair.first_frame = bm_load_animation(fname, &crosshair.num_frames);
	if (crosshair.first_frame < 0) {
		Warning(LOCATION, "Cannot load hud ani: %s\n", fname);
	}
}

void HudGaugeReticle::initFirepointDisplay(bool firepoint, int scaleX, int scaleY, int size) {
	firepoint_display = firepoint;
	firepoint_scale_x = scaleX;
	firepoint_scale_y = scaleY;
	firepoint_size = size;
}

void HudGaugeReticle::render(float  /*frametime*/, bool config)
{
	if (crosshair.first_frame < 0) {
		return;
	}

	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
		int bmw, bmh;
		bm_get_info(crosshair.first_frame, &bmw, &bmh);
		hud_config_set_mouse_coords(gauge_config_id, x, x + fl2i(bmw * scale), y, y + fl2i(bmh * scale));
	}

	ship_info* sip = nullptr;
	if (!config) {
		sip = &Ship_info[Player_ship->ship_info_index];
	}

	int bitmap_size_x, bitmap_size_y;
	bm_get_info(crosshair.first_frame, &bitmap_size_x, &bitmap_size_y);

	if (!config && (autoaim_frame_offset > 0 || sip->autoaim_lock_snd.isValid() || sip->autoaim_lost_snd.isValid())) {
		ship *shipp = &Ships[Objects[Player->objnum].instance];
		ship_weapon *swp = &shipp->weapons;
		ai_info *aip = &Ai_info[shipp->ai_index];

		if (aip->target_objnum != -1) {
			bool autoaiming = false;

			autoaiming = in_autoaim_fov(shipp, swp->current_primary_bank, &Objects[aip->target_objnum]);

			if (autoaiming) {
				if (!has_autoaim_lock && sip->autoaim_lock_snd.isValid()) {
					snd_play(gamesnd_get_game_sound(sip->autoaim_lock_snd));
					//snd_play( gamesnd_get_game_sound(ship_get_sound(Player_obj, GameSounds::THREAT_FLASH)));
				}
				has_autoaim_lock = true;
			}
			else {
				if (has_autoaim_lock && sip->autoaim_lost_snd.isValid()) {
					snd_play(gamesnd_get_game_sound(sip->autoaim_lost_snd));
				}
				has_autoaim_lock = false;
			}
		} else {
			has_autoaim_lock = false;
		}
	}

	setGaugeColor(HUD_C_BRIGHT, config);

	// the typical reticle indicating the direction of shooting
	int shoot_reticle = 0;
	if (has_autoaim_lock)
		shoot_reticle = crosshair.first_frame + autoaim_frame_offset;
	else
		shoot_reticle = crosshair.first_frame;

	// a secondary 'reticle' for distinguishing the flight cursor from the above
	int flight_reticle = fl2i(flight_cursor_frame_offset * scale) >= 0 ? crosshair.first_frame + fl2i(flight_cursor_frame_offset * scale) : -1;

	int mobile_reticle = flight_reticle;
	int fixed_reticle = shoot_reticle;
	// depending on the parameters of the ship the 'mobile' reticle may be the one indicating the shoot direction
	if (!config && sip->aims_at_flight_cursor) {
		mobile_reticle = shoot_reticle;
		fixed_reticle = flight_reticle;
	}


	if (fixed_reticle == shoot_reticle)
		setGaugeColor(HUD_C_BRIGHT, config);
	else
		setGaugeColor(HUD_C_NORMAL, config);

	if (fixed_reticle >= 0) {
		if (HUD_shadows) {
			color cur = gr_screen.current_color;
			gr_set_color_fast(&Color_black);

			// Render the shadow twice to increase visibility
			renderBitmap(fixed_reticle, x + 1, y + 1, scale, config);
			renderBitmap(fixed_reticle, x + 1, y + 1, scale, config);
			gr_set_color_fast(&cur);
		}

		renderBitmap(fixed_reticle, x, y, scale, config);
	} else {
		renderCircle(fl2i(base_w * 0.5f), fl2i(base_h * 0.5f), fl2i(base_h * 0.03f), false, config);
	}

	if (!config && (Player_flight_mode == FlightMode::FlightCursor || sip->aims_at_flight_cursor)) {
		if (mobile_reticle == shoot_reticle)
			setGaugeColor(HUD_C_BRIGHT, config);
		else
			setGaugeColor(HUD_C_NORMAL, config);

		int cx = fl2i(Player_flight_cursor_offset.screen.xyw.x + 0.5f);
		int cy = fl2i(Player_flight_cursor_offset.screen.xyw.y + 0.5f);
		unsize(&cx, &cy);
		if (mobile_reticle >= 0)
			renderBitmap(mobile_reticle, fl2i(cx - base_w * 0.5f) + x, fl2i(cy - base_h * 0.5f) + y, scale, config);
		else {
			renderCircle(cx, cy, fl2i(base_h * 0.03f), false, config);
		}
	}

	if (!config && firepoint_display) {
		fp.clear();
		getFirepointStatus();
		
		if (!fp.empty()) {

			for (SCP_vector<firepoint>::iterator fpi = fp.begin(); fpi != fp.end(); ++fpi) {
				if (fpi->active == 2)
					setGaugeColor(HUD_C_BRIGHT, config);
				else if (fpi->active == 1)
					setGaugeColor(HUD_C_NORMAL, config);
				else
					setGaugeColor(HUD_C_DIM, config);

				int centerX = x + (bitmap_size_x / 2);
				int centerY = y + (bitmap_size_y / 2);
				renderCircle(fl2i(centerX + (fpi->xy.x * firepoint_scale_x)), fl2i(centerY + (fpi->xy.y * firepoint_scale_y)), firepoint_size, false, config);
			}
		}
	}
}

void HudGaugeReticle::getFirepointStatus() {

	// allow the firepoint status to be empty when a multiplayer observer
	// this is not a bug, the observer will simply *not* have any firepoints.
	if (Objects[Player->objnum].type == OBJ_OBSERVER) {
		// only multiplayer instances should be getting here!
		Assertion((Game_mode & GM_MULTIPLAYER), "Somehow FSO thinks its player object is an observer even though it's not in Multiplayer. Please report!");
		return; 
	}

	//First, get the player ship
	ship_info* sip;
	ship* shipp;
	polymodel* pm;

	Assertion(Objects[Player->objnum].type == OBJ_SHIP, "HudGaugeReticle::getFirepointStatus was passed an invalid object type of %d. Please report!", Objects[Player->objnum].type);

	if (Objects[Player->objnum].type == OBJ_SHIP) {
		shipp = &Ships[Objects[Player->objnum].instance];
		sip = &Ship_info[shipp->ship_info_index];
	
		//Get the player eyepoint
		pm = model_get(sip->model_num);

		if (pm->n_view_positions == 0) {
			mprintf(("Model %s does not have a defined eyepoint. Firepoint display could not be generated\n", pm->filename));
		} else  {
			if (pm->n_guns > 0) {
				eye eyepoint = pm->view_positions[shipp->current_viewpoint];
				vec2d ep = { eyepoint.pnt.xyz.x, eyepoint.pnt.xyz.y };

				for (int i = 0; i < pm->n_guns; i++) {
					int bankactive = 0;
					ship_weapon *swp = &shipp->weapons;

					// If this firepoint doesn't actually have a weapon mounted, skip all of this
					if (swp->primary_bank_weapons[i] < 0) {
						continue;
					}

					// bank is firing
					// (make sure we haven't cycled banks recently, since that will also reset the timestamp)
					if (   (!Control_config[CYCLE_NEXT_PRIMARY].digital_used.isValid() || timestamp_since(Control_config[CYCLE_NEXT_PRIMARY].digital_used) > BANK_SWITCH_DELAY)
						&& (!Control_config[CYCLE_PREV_PRIMARY].digital_used.isValid() || timestamp_since(Control_config[CYCLE_PREV_PRIMARY].digital_used) > BANK_SWITCH_DELAY)
						&& (!timestamp_elapsed(shipp->weapons.next_primary_fire_stamp[i]) || !timestamp_elapsed(shipp->weapons.primary_animation_done_time[i]))) {
						bankactive = 2;
					}
					// bank is selected
					else if (i == shipp->weapons.current_primary_bank || shipp->flags[Ship::Ship_Flags::Primary_linked]) {
						bankactive = 1;
					}

					int num_slots = pm->gun_banks[i].num_slots;
					int point_count = 0;
					FiringPattern firing_pattern;
					if (sip->flags[Ship::Info_Flags::Dyn_primary_linking]) {
						firing_pattern = sip->dyn_firing_patterns_allowed[shipp->weapons.current_primary_bank][swp->dynamic_firing_pattern[shipp->weapons.current_primary_bank]];
					} else {
						firing_pattern = Weapon_info[swp->primary_bank_weapons[i]].firing_pattern;
					}

					if (sip->flags[Ship::Info_Flags::Dyn_primary_linking]) {
						point_count = MIN(num_slots, swp->primary_bank_slot_count[shipp->weapons.current_primary_bank] );
					} else if (firing_pattern != FiringPattern::STANDARD) {
						point_count = MIN(num_slots, Weapon_info[swp->primary_bank_weapons[i]].shots);
					} else {
						point_count = num_slots;
					}
					
					for (int j = 0; j < num_slots; j++) {
						int fpactive = bankactive;

						fpactive--;

						for (int q = 0; q < point_count; q++) {
							// If this firepoint is not among the next shot(s) to be fired, dim it one step
							switch (firing_pattern) {
								case FiringPattern::CYCLE_FORWARD: {
								if (j == (swp->primary_firepoint_next_to_fire_index[i] + q) % num_slots) {
										fpactive++;
									}
									break;
								}
								case FiringPattern::CYCLE_REVERSE: {
									if (j == ((swp->primary_firepoint_next_to_fire_index[i] - (q + 1)) % num_slots + num_slots) % num_slots) {
										fpactive++;
									}
									break;
								}
								case FiringPattern::RANDOM_EXHAUSTIVE: {
									if (j == swp->primary_firepoint_indices[i][(swp->primary_firepoint_next_to_fire_index[i] + q) % num_slots]) {
										fpactive++;
									}
									break;
								}
								case FiringPattern::RANDOM_NONREPEATING:
								case FiringPattern::RANDOM_REPEATING: {
									if (j == swp->primary_firepoint_indices[i][q]) {
										fpactive++;
									}
									break;
								}
								default:
								case FiringPattern::STANDARD: {
									fpactive++;
									break;
								}
							}
							if (fpactive == bankactive) {
								break;
							}
						}

						vec3d fpfromeye;

						matrix eye_orient, player_transpose;

						vm_copy_transpose(&player_transpose, &Objects[Player->objnum].orient);
						vm_matrix_x_matrix(&eye_orient, &player_transpose, &Eye_matrix);
						vm_vec_rotate(&fpfromeye, &pm->gun_banks[i].pnt[j], &eye_orient);

						firepoint tmp = { { fpfromeye.xyz.x - ep.x, ep.y - fpfromeye.xyz.y }, fpactive };
						fp.push_back(tmp);
					}
				}
			}
		}
	}
}

void HudGaugeReticle::setFlightCursorFrame(int framenum) {
	if (framenum < 0 || framenum > crosshair.num_frames - 1)
		flight_cursor_frame_offset = -1;
	else
		flight_cursor_frame_offset = framenum;
}

void HudGaugeReticle::setAutoaimFrame(int framenum) {
	if (framenum < 0 || framenum > crosshair.num_frames - 1)
		autoaim_frame_offset = 0;
	else
		autoaim_frame_offset = framenum;
}

void HudGaugeReticle::pageIn()
{
	bm_page_in_aabitmap( crosshair.first_frame, crosshair.num_frames);
}

HudGaugeThrottle::HudGaugeThrottle():
HudGauge(HUD_OBJECT_THROTTLE, HUD_THROTTLE_GAUGE, true, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{

}
void HudGaugeThrottle::initThrottleStartY(int y)
{
	Bottom_offset_y = y;
}

void HudGaugeThrottle::initThrottleSizes(int w, int h)
{
	throttle_w = w;
	throttle_h = h;
}

void HudGaugeThrottle::initAburnHeight(int h)
{
	throttle_aburn_h = h;
}

void HudGaugeThrottle::initMaxSpeedOffsets(int x, int y, bool show)
{
	Max_speed_offsets[0] = x;
	Max_speed_offsets[1] = y;
	Show_max_speed = show;
}

void HudGaugeThrottle::initZeroSpeedOffsets(int x, int y, bool show)
{
	Zero_speed_offsets[0] = x;
	Zero_speed_offsets[1] = y;
	Show_min_speed = show;
}

void HudGaugeThrottle::initOrbitCenterOffsets(int x, int y, bool orbiting)
{
	Orbit_center_offsets[0] = x;
	Orbit_center_offsets[1] = y;
	orbit = orbiting;
}

void HudGaugeThrottle::initOrbitRadius(int radius)
{
	orbit_radius = radius;
}

void HudGaugeThrottle::initTargetSpeedOffsets(int x, int y, bool show, bool percent)
{
	Target_speed_offsets[0] = x;
	Target_speed_offsets[1] = y;
	Show_target_speed = show;
	Show_percent = percent;
}

void HudGaugeThrottle::showBackground(bool show)
{
	Show_background = show;
}

void HudGaugeThrottle::initGlideOffsets(int x, int y, bool custom)
{
	Glide_offsets[0] = x;
	Glide_offsets[1] = y;
	Use_custom_glide = custom;
}

void HudGaugeThrottle::initMatchSpeedOffsets(int x, int y, bool custom)
{
	Match_speed_offsets[0] = x;
	Match_speed_offsets[1] = y;
	Use_custom_match_speed = custom;

	auto match_speed_str = XSTR("m", 1667);

	font::FSFont* fsFont = font::get_font(font_num);
	if (fsFont->getType() == font::VFNT_FONT)
	{
		ubyte sc = lcl_get_font_index(font_num);
		// NOTE: default to normal m because either
		// a) the german font has no special m (its an a)
		// b) the font has no special characters
		if (sc == 0 || Lcl_gr) {
			Match_speed_icon = match_speed_str[0];
		}
		else {
			Match_speed_icon = sc + 3;
		}
		Match_speed_draw_background = false;
	}
	else
	{
		// Default version for other fonts, draw a black character on a rectangle
		Match_speed_icon = match_speed_str[0];
		Match_speed_draw_background = true;
	}

	SCP_stringstream stream;
	stream << static_cast<char>(Match_speed_icon);
	const SCP_string& iconStr = stream.str();

	gr_get_string_size(&Match_speed_icon_width, nullptr, iconStr.c_str());
}

void HudGaugeThrottle::initBitmaps(char *fname)
{
	throttle_frames.first_frame = bm_load_animation(fname, &throttle_frames.num_frames);

	if (throttle_frames.first_frame < 0) {
		Warning(LOCATION, "Cannot load hud ani: %s\n", fname);
	}
}

void HudGaugeThrottle::pageIn()
{
	bm_page_in_aabitmap( throttle_frames.first_frame, throttle_frames.num_frames);
}

void HudGaugeThrottle::render(float  /*frametime*/, bool config)
{
	if (throttle_frames.first_frame < 0) {
		return;
	}

	ship_info	*sip = nullptr;
	if (!config) {
		sip = &Ship_info[Player_ship->ship_info_index];
	}

	float current_speed = config ? 50 : Player_obj->phys_info.fspeed;
	if ( current_speed < 0.0f){
		current_speed = 0.0f;
	}

	float max_speed = config ? 100 : Player_obj->phys_info.max_vel.xyz.z;
	if ( max_speed <= 0 ) {
		max_speed = sip->max_vel.xyz.z;
	}

	float absolute_speed = config ? 50 : Player_obj->phys_info.speed;

	// scale by distance modifier from hud_guages.tbl for display purposes
	float absolute_displayed_speed = absolute_speed * Hud_speed_multiplier;
	float max_displayed_speed = max_speed * Hud_speed_multiplier;

	float desired_speed = config ? 100 : Player->ci.forward * max_speed;
	if ( desired_speed < 0.0f ){		// so ships that go backwards don't force the indicators below where they can go
		desired_speed = 0.0f;
	}

	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
		int bmw, bmh;
		bm_get_info(throttle_frames.first_frame + 1, &bmw, &bmh);
		hud_config_set_mouse_coords(gauge_config_id, x, x + fl2i(bmw * scale), y, y + fl2i(bmh * scale));
	}

	int desired_y_pos = y + fl2i(Bottom_offset_y * scale) - fl2i(std::lround(fl2i(throttle_h * scale) * desired_speed / max_speed)) - 1;
	int desired_y_pos_unscaled = position[1] + Bottom_offset_y - fl2i(std::lround(throttle_h * desired_speed / max_speed)) - 1;

	float percent_max = 0.0f;
	if (max_speed > 0) {
		percent_max = current_speed / max_speed;
	}

	float percent_aburn_max = 0.0f;
	if ( percent_max > 1 ) {
		percent_max = 1.0f;
		percent_aburn_max = (current_speed - max_speed) / (Player_obj->phys_info.afterburner_max_vel.xyz.z - max_speed);
		if ( percent_aburn_max > 1.0f ) {
			percent_aburn_max = 1.0f;
		}
		if ( percent_aburn_max < 0 ) {
			percent_aburn_max = 0.0f;
		}
	}

	int y_end = y + fl2i(Bottom_offset_y * scale) - fl2i(std::lround(fl2i(throttle_h * scale) * percent_max));
	int y_end_unscaled = position[1] + Bottom_offset_y - fl2i(std::lround(throttle_h * percent_max));
	if ( percent_aburn_max > 0 ) {
		y_end -= fl2i(std::lround(percent_aburn_max * fl2i(throttle_aburn_h * scale)));
		y_end_unscaled -= fl2i(std::lround(percent_aburn_max * throttle_aburn_h));
	}

	if ( !config && Player_obj->phys_info.flags & PF_AFTERBURNER_ON ) {
		// default value is 240 when afterburner is on. 
		//I'm assuming that this value is basically Bottom_offset_y - throttle_aburn_h - throttle_h
		desired_y_pos = y + fl2i(Bottom_offset_y * scale) - fl2i(throttle_aburn_h * scale) - fl2i(throttle_h * scale);
		desired_y_pos_unscaled = position[1] + Bottom_offset_y - throttle_aburn_h - throttle_h;
	}

	setGaugeColor(HUD_C_NONE, config);
	
	if(Show_background) {
		renderThrottleBackground(y_end_unscaled, config);
	} else {
		renderBitmap(throttle_frames.first_frame, x, y, scale, config);			
	}

	// Absolute speed, not forward speed, for hud speed reticle - fixes the guage for sliding -- kazan
	renderThrottleSpeed(absolute_displayed_speed, y_end, config);

	// draw target speed if necessary
	if (!config && Show_target_speed ) {
		char buf[32];
		int w, h;

		if ( Show_percent ) {
			if ( !config && Player_obj->phys_info.flags & PF_AFTERBURNER_ON ) {
				strcpy_s(buf, XSTR( "A/B", 1669 ));
			} else {
				sprintf(buf, XSTR( "%d%%", 326), (int)std::lround( (desired_speed/max_speed)*100 ));
			}
		} else {
			sprintf(buf, "%d", (int)std::lround(desired_speed * Hud_speed_multiplier));
		}

		hud_num_make_mono(buf, font_num);
		gr_get_string_size(&w, &h, buf, scale);

		renderString(x + fl2i(Target_speed_offsets[0] * scale) - w, y + fl2i(Target_speed_offsets[1] * scale), buf, scale, config);
	}

	// draw the "desired speed" bar on the throttle
	renderThrottleLine(desired_y_pos_unscaled, desired_y_pos, config);

	// draw left arc (the bright portion of the throttle gauge)
	renderThrottleForeground(y_end_unscaled, y_end, config);

	if (Show_max_speed ) {
		renderPrintf(x + fl2i(Max_speed_offsets[0] * scale), y + fl2i(Max_speed_offsets[1] * scale), scale, config, "%d", (int)std::lround(max_displayed_speed));
	}
	
	if (Show_min_speed ) {
		renderPrintf(x + fl2i(Zero_speed_offsets[0] * scale), y + fl2i(Zero_speed_offsets[1] * scale), scale, config, "%s", XSTR("0", 292));
	}
}

void HudGaugeThrottle::renderThrottleSpeed(float current_speed, int y_scaled, bool config)
{
	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}

	char buf[32];
	sprintf(buf, "%d", fl2i(std::lround(current_speed)));
	hud_num_make_mono(buf, font_num);

	int w, h;
	gr_get_string_size(&w, &h, buf, scale);

	int sx, sy, x_pos, y_pos;
	if (orbit ) {
		y_pos = y + fl2i(Orbit_center_offsets[1] * scale) - y_scaled;
		x_pos = static_cast<int>(sqrt(double(fl2i(orbit_radius * scale) * fl2i(orbit_radius * scale) - y_pos * y_pos)));
		x_pos = x + fl2i(Orbit_center_offsets[0] * scale) - x_pos;

		sx = x_pos - w - 2;
		sy = fl2i(y_scaled - h / 2.0f + 1.5);
	} else {
		sx = x + fl2i(Orbit_center_offsets[0] * scale) - w;
		sy = y + fl2i(Orbit_center_offsets[1] * scale);
	}
	
	renderPrintf(sx, sy, scale, config, "%s", buf);

	if (!config &&  object_get_gliding(Player_obj) ) { 
		auto glide_str = XSTR("GLIDE", 1668);

		if ( Use_custom_glide ) {
			renderString(x + fl2i(Glide_offsets[0] * scale), y + fl2i(Glide_offsets[1] * scale), glide_str, scale, config);
		} else {
			int offset;
			if ( current_speed <= 9.5 ) {
				offset = -31;
			} else if ( current_speed <= 99.5 ) {
				offset = -22;
			} else {
				offset = -13;
			}

			renderString(sx+offset, sy + h, glide_str, scale, config);
		}
	} else if (config || Players[Player_num].flags & PLAYER_FLAGS_MATCH_TARGET ) {
		if ( Use_custom_match_speed ) {
			renderMatchSpeedIcon(x + fl2i(Match_speed_offsets[0] * scale), y + fl2i(Match_speed_offsets[1] * scale), scale, config);
		} else {
			int offset;
			if ( current_speed <= 9.5 ) {
				offset = 0;
			} else {
				offset = 3;
			}

			renderMatchSpeedIcon(sx+offset, sy + h, scale, config);
		}
	}
}

void HudGaugeThrottle::renderThrottleLine(int y_unscaled, int y_scaled, bool config)
{
	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}

	int height = 1;

	// If we scale down then the line can become invisible
	// so let's proportionally scale up the line height
	if (scale < 1.0f) {
		float min_height = 1.0f / scale;
		height = fl2i(min_height / scale);
	}

	renderBitmapEx(throttle_frames.first_frame+3, 
		x, y_scaled, 
		throttle_w,
		height, 
		0, 
		y_unscaled - position[1], //Explicitly unscaled
		scale,
		config);
}

void HudGaugeThrottle::renderThrottleForeground(int y_unscaled, int y_scaled, bool config)
{
	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}

	int w, h;
	bm_get_info(throttle_frames.first_frame+1,&w,&h);

	if (y_unscaled < (x + static_cast<int>(h * scale) - 1)) {		
		renderBitmapEx(throttle_frames.first_frame + 2,
			x,
			y_scaled,
			w,
			h - (y_unscaled - position[1]), // Explicitly unscaled
			0,
			y_unscaled - position[1], // Explicitly unscaled
			scale,
			config);
	}
}

void HudGaugeThrottle::renderThrottleBackground(int y_unscaled, bool config)
{
	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}

	int w, h;
	bm_get_info( throttle_frames.first_frame+1,&w,&h);

	if (y_unscaled > position[1]) {
		renderBitmapEx(throttle_frames.first_frame + 1,
			x,
			y,
			w,
			y_unscaled - position[1] + 1, // Explicitly unscaled
			0,
			0,
			scale,
			config);	
	}
}

void HudGaugeThrottle::renderMatchSpeedIcon(int x, int y, float scale, bool config)
{
	if (Match_speed_draw_background)
	{
		// One pixel boundary
		renderRect(x, y, static_cast<int>(Match_speed_icon_width * scale) + 2, static_cast<int>(gr_get_font_height() * scale) + 2, config);
		
		gr_set_color_fast(&Color_black);
		renderPrintf(x + 1, y + 1, scale, config, "%c", Match_speed_icon);

		setGaugeColor(HUD_C_NONE, config);
	}
	else
	{
		renderPrintf(x, y, scale, config, "%c", Match_speed_icon);
	}
}

HudGaugeThreatIndicator::HudGaugeThreatIndicator():
HudGauge(HUD_OBJECT_THREAT, HUD_THREAT_GAUGE, true, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeThreatIndicator::initLaserWarnOffsets(int x, int y)
{
	Laser_warn_offsets[0] = x;
	Laser_warn_offsets[1] = y;
}

void HudGaugeThreatIndicator::initLockWarnOffsets(int x, int y)
{
	Lock_warn_offsets[0] = x;
	Lock_warn_offsets[1] = y;
}

void HudGaugeThreatIndicator::initBitmaps(char *fname_arc, char *fname_laser, char *fname_lock)
{
	threat_arc.first_frame = bm_load_animation(fname_arc, &threat_arc.num_frames);
	if (threat_arc.first_frame < 0) {
		Warning(LOCATION, "Cannot load hud ani: %s\n", fname_arc);
	}
	
	laser_warn.first_frame = bm_load_animation(fname_laser, &laser_warn.num_frames);
	if (laser_warn.first_frame < 0) {
		Warning(LOCATION, "Cannot load hud ani: %s\n", fname_laser);
	}

	lock_warn.first_frame = bm_load_animation(fname_lock, &lock_warn.num_frames);
	if (lock_warn.first_frame < 0) {
		Warning(LOCATION, "Cannot load hud ani: %s\n", fname_lock);
	}
}

void HudGaugeThreatIndicator::initialize()
{
	laser_warn_timer = timestamp(1);
	laser_warn_frame = 0;

	lock_warn_timer = timestamp(1);
	lock_warn_frame = 0;

	HudGauge::initialize();
}

void HudGaugeThreatIndicator::pageIn()
{
	bm_page_in_aabitmap(threat_arc.first_frame, threat_arc.num_frames);
	bm_page_in_aabitmap(laser_warn.first_frame, laser_warn.num_frames);
	bm_page_in_aabitmap(lock_warn.first_frame, lock_warn.num_frames);
}

void HudGaugeThreatIndicator::render(float  /*frametime*/, bool config)
{

	int x = position[0];
	int y = position[1];
	float scale = 1.0f;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
        int bmw, bmh;
		bm_get_info(threat_arc.first_frame + 1, &bmw, &bmh);
		hud_config_set_mouse_coords(gauge_config_id,
			x,
			x + static_cast<int>(bmw * scale),
			y,
			y + static_cast<int>(bmh * scale));
	}

	setGaugeColor(HUD_C_NONE, config);

	if (threat_arc.first_frame >= 0)
		renderBitmap(threat_arc.first_frame+1, x, y, scale, config);

	renderLaserThreat(config);
	renderLockThreat(config);
}

void HudGaugeThreatIndicator::renderLaserThreat(bool config)
{
	if (laser_warn.first_frame < 0)
		return;

	//Check how many frames the ani actually has
	int num_frames = laser_warn.num_frames;
	//We need at least two frames here
	Assert( num_frames >= 2 );

	int x = position[0];
	int y = position[1];
	float scale = 1.0f;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}

	int frame_offset;
	if ( Player->threat_flags & THREAT_DUMBFIRE ) {
		if ( timestamp_elapsed(laser_warn_timer) ) {
			laser_warn_timer = timestamp(THREAT_DUMBFIRE_FLASH);
			laser_warn_frame++;
			if ( laser_warn_frame > (num_frames - 1) ) { //The first frame being the default "off" setting, we need to cycle through all the other frames
				laser_warn_frame = 1;
			}
		}
		frame_offset = laser_warn_frame;
	} else {
		frame_offset = 0;
	}

	renderBitmap(laser_warn.first_frame + frame_offset, x + static_cast<int>(Laser_warn_offsets[0] * scale), y + static_cast<int>(Laser_warn_offsets[1] * scale), scale, config);
}

void HudGaugeThreatIndicator::renderLockThreat(bool config)
{
	if (lock_warn.first_frame < 0)
		return;

	//Let's find out how many frames our ani has, and adjust accordingly
	int num_frames = lock_warn.num_frames;
	//We need at least two frames here
	Assert( num_frames >= 2 );

	int x = position[0];
	int y = position[1];
	float scale = 1.0f;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
	}

	int frame_offset;
	if ( Player->threat_flags & (THREAT_LOCK | THREAT_ATTEMPT_LOCK) ) {
		if ( timestamp_elapsed(lock_warn_timer) ) {
			if ( Player->threat_flags & THREAT_LOCK )  {
				lock_warn_timer = timestamp(fl2i(THREAT_LOCK_FLASH/2.0f));
			} else {
				lock_warn_timer = timestamp(THREAT_LOCK_FLASH);
			}
			lock_warn_frame++;
			if ( lock_warn_frame > (num_frames - 1) ) { //The first frame being the default "off" setting, we need to cycle through all the other frames
				lock_warn_frame = 1;
			}
		}
		frame_offset = lock_warn_frame;
	} else {
		frame_offset = 0;
	}

	renderBitmap(lock_warn.first_frame+frame_offset, x + static_cast<int>(Lock_warn_offsets[0] * scale), y + static_cast<int>(Lock_warn_offsets[1] * scale), scale, config);
}

HudGaugeWeaponLinking::HudGaugeWeaponLinking():
HudGauge(HUD_OBJECT_WEAPON_LINKING, HUD_THREAT_GAUGE, true, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeWeaponLinking::init1PrimaryOffsets(int x, int y)
{
	Weapon_link_offsets[LINK_ONE_PRIMARY][0] = x;
	Weapon_link_offsets[LINK_ONE_PRIMARY][1] = y;
}

void HudGaugeWeaponLinking::init2PrimaryOffsets(int x, int y)
{
	Weapon_link_offsets[LINK_TWO_PRIMARY][0] = x;
	Weapon_link_offsets[LINK_TWO_PRIMARY][1] = y;
}

void HudGaugeWeaponLinking::init1SecondaryOffsets(int x, int y)
{
	Weapon_link_offsets[LINK_ONE_SECONDARY][0] = x;
	Weapon_link_offsets[LINK_ONE_SECONDARY][1] = y;
}

void HudGaugeWeaponLinking::init2SecondaryOffsets(int x, int y)
{
	Weapon_link_offsets[LINK_TWO_SECONDARY][0] = x;
	Weapon_link_offsets[LINK_TWO_SECONDARY][1] = y;
}

void HudGaugeWeaponLinking::init3SecondaryOffsets(int x, int y)
{
	Weapon_link_offsets[LINK_THREE_SECONDARY][0] = x;
	Weapon_link_offsets[LINK_THREE_SECONDARY][1] = y;
}

void HudGaugeWeaponLinking::initBitmaps(char *fname_arc, 
										char *fname_primary_link_1, 
										char *fname_primary_link_2, 
										char *fname_secondary_link_1, 
										char *fname_secondary_link_2, 
										char *fname_secondary_link_3)
{
	arc.first_frame = bm_load_animation(fname_arc, &arc.num_frames);
	if (arc.first_frame < 0) {
		Warning(LOCATION, "Cannot load hud ani: %s\n", fname_arc);
	}
	
	weapon_linking_modes[LINK_ONE_PRIMARY].first_frame = bm_load_animation(fname_primary_link_1, &weapon_linking_modes[LINK_ONE_PRIMARY].num_frames);
	if (weapon_linking_modes[LINK_ONE_PRIMARY].first_frame < 0) {
		Warning(LOCATION, "Cannot load hud ani: %s\n", fname_primary_link_1);
	}

	weapon_linking_modes[LINK_TWO_PRIMARY].first_frame = bm_load_animation(fname_primary_link_2, &weapon_linking_modes[LINK_TWO_PRIMARY].num_frames);
	if (weapon_linking_modes[LINK_TWO_PRIMARY].first_frame < 0) {
		Warning(LOCATION, "Cannot load hud ani: %s\n", fname_primary_link_2);
	}

	weapon_linking_modes[LINK_ONE_SECONDARY].first_frame = bm_load_animation(fname_secondary_link_1, &weapon_linking_modes[LINK_ONE_SECONDARY].num_frames);
	if (weapon_linking_modes[LINK_ONE_SECONDARY].first_frame < 0) {
		Warning(LOCATION, "Cannot load hud ani: %s\n", fname_secondary_link_1);
	}

	weapon_linking_modes[LINK_TWO_SECONDARY].first_frame = bm_load_animation(fname_secondary_link_2, &weapon_linking_modes[LINK_TWO_SECONDARY].num_frames);
	if (weapon_linking_modes[LINK_TWO_SECONDARY].first_frame < 0) {
		Warning(LOCATION, "Cannot load hud ani: %s\n", fname_secondary_link_2);
	}

	weapon_linking_modes[LINK_THREE_SECONDARY].first_frame = bm_load_animation(fname_secondary_link_3, &weapon_linking_modes[LINK_THREE_SECONDARY].num_frames);
	if (weapon_linking_modes[LINK_THREE_SECONDARY].first_frame < 0) {
		Warning(LOCATION, "Cannot load hud ani: %s\n", fname_secondary_link_3);
	}
}


void HudGaugeWeaponLinking::pageIn()
{
	bm_page_in_aabitmap(arc.first_frame, arc.num_frames);

	for(int i = 0; i < NUM_WEAPON_LINK_MODES; i++) {
		bm_page_in_aabitmap(weapon_linking_modes[i].first_frame, weapon_linking_modes[i].num_frames);
	}
}

void HudGaugeWeaponLinking::render(float  /*frametime*/, bool config)
{
	int x = position[0];
	int y = position[1];
	float scale = 1.0;

	if (config) {
		std::tie(x, y, scale) = hud_config_convert_coord_sys(position[0], position[1], base_w, base_h);
		int bmw, bmh;
		bm_get_info(arc.first_frame, &bmw, &bmh);
		hud_config_set_mouse_coords(gauge_config_id, x, x + fl2i(bmw * scale), y, y + fl2i(bmh * scale));
	}

	setGaugeColor(HUD_C_NONE, config);

	if (arc.first_frame >= 0) {
		renderBitmap(arc.first_frame + 1, x, y, scale, config);
	}

	ship_weapon* swp = &Player_ship->weapons;

	int num_primary_banks = config ? 2 : swp->num_primary_banks;
	
	int gauge_index = 0, frame_offset = 0;
	switch (num_primary_banks) {
		case 0:
			gauge_index = -1;
			break;

		case 1:
			gauge_index = LINK_ONE_PRIMARY;
			if (!config && Player_ship->weapons.current_primary_bank == -1 ) {
				frame_offset = 0;	
			} else {
				frame_offset = 1;	
			}
			break;

		case 2:
			gauge_index = LINK_TWO_PRIMARY;
			if (!config && swp->current_primary_bank == -1 ) {
				frame_offset = 0;	
			} else {
				if (config || Player_ship->flags[Ship::Ship_Flags::Primary_linked] ) {
					frame_offset = 3;
				} else {
					if ( swp->current_primary_bank == 0 ) {
						frame_offset = 1;
					} else {
						frame_offset = 2;
					}
				}
			}
			break;

		default:
			Int3();	// shouldn't happen (get Alan if it does)
			return;
			break;
	}
	
	if ( gauge_index != -1 ) {
		if (weapon_linking_modes[gauge_index].first_frame >= 0) {
			renderBitmap(weapon_linking_modes[gauge_index].first_frame+frame_offset, x + fl2i(Weapon_link_offsets[gauge_index][0] * scale), y + fl2i(Weapon_link_offsets[gauge_index][1] * scale), scale, config);
		}
	}

	int num_banks = config ? 3 : swp->num_secondary_banks;
	if ( num_banks <= 0 ) {
		num_banks = Ship_info[Player_ship->ship_info_index].num_secondary_banks;
	}

	switch( num_banks ) {
		case 0:
			gauge_index = -1;
			break;

		case 1:
			gauge_index = LINK_ONE_SECONDARY;
			break;

		case 2:
			gauge_index = LINK_TWO_SECONDARY;
			break;

		case 3:
			gauge_index = LINK_THREE_SECONDARY;
			break;

		default:
			Int3();	// shouldn't happen (get Alan if it does)
			return;
			break;
	}
	
	if ( gauge_index != -1 ) {
		if (config || swp->num_secondary_banks <= 0 ) {
			frame_offset = 0;
		} else {
			frame_offset = swp->current_secondary_bank+1;
		}
		if (weapon_linking_modes[gauge_index].first_frame >= 0) {
			renderBitmap(weapon_linking_modes[gauge_index].first_frame+frame_offset, x + fl2i(Weapon_link_offsets[gauge_index][0] * scale),  y + fl2i(Weapon_link_offsets[gauge_index][1] * scale), scale, config);
		}
	}
}

// Called at the start of each level.. use Reticle_inited so we only load frames once
void hud_init_reticle()
{
	Threat_lock_timer = timestamp(0);
	Threat_lock_frame = 1;
	Player->threat_flags = 0;
	Player->update_dumbfire_time = timestamp(0);		
	Player->update_lock_time = timestamp(0);
}

// called once per frame to update the reticle gauges.  Makes calls to
// ship_dumbfire_threat() and ship_lock_threat() and updates Threat_flags.
void hud_update_reticle( player *pp )
{
	int rval;
	ship *shipp;

	// multiplayer clients won't call this routine
	if ( MULTIPLAYER_CLIENT || MULTI_OBSERVER(Net_players[MY_NET_PLAYER_NUM]))
		return;

	shipp = &Ships[Objects[pp->objnum].instance];

	if ( ship_dumbfire_threat(shipp) ) {
		pp->threat_flags |= THREAT_DUMBFIRE;
		pp->update_dumbfire_time = timestamp(THREAT_UPDATE_DUMBFIRE_TIME);
	}

	if ( timestamp_elapsed(pp->update_dumbfire_time) ) {
		pp->update_dumbfire_time = timestamp(THREAT_UPDATE_DUMBFIRE_TIME);
		pp->threat_flags &= ~THREAT_DUMBFIRE;
	}

	if ( timestamp_elapsed(pp->update_lock_time) ) {
		pp->threat_flags &= ~(THREAT_LOCK | THREAT_ATTEMPT_LOCK);
		pp->update_lock_time = timestamp(THREAT_UPDATE_LOCK_TIME);
		rval = ship_lock_threat(shipp);
		if ( rval == 1 ) {
			pp->threat_flags |= THREAT_ATTEMPT_LOCK;
		} else if ( rval == 2 ) {
			pp->threat_flags |= THREAT_LOCK;
		}
	}

	if(Player->threat_flags & THREAT_LOCK ) {
		// a less hacked up version of the missile launch warning
		hud_start_text_flash(XSTR("Launch", 1507), THREAT_LOCK_FLASH, fl2i(THREAT_LOCK_FLASH/2.0f));
	}

	if ( Player->threat_flags & (THREAT_ATTEMPT_LOCK) ) {
		if ( timestamp_elapsed(Threat_lock_timer) ) {
			Threat_lock_timer = timestamp(THREAT_LOCK_FLASH);
			
			Threat_lock_frame++;
			if ( Threat_lock_frame > 2 ) {
				Threat_lock_frame = 1;
			}
			if ( (Threat_lock_frame == 2) && (Player->threat_flags & THREAT_ATTEMPT_LOCK ) ) {
				snd_play( gamesnd_get_game_sound(ship_get_sound(Player_obj, GameSounds::THREAT_FLASH)));
			}
		}
	} 
}

// calculates what the screen position of the flight cursor should be
void hud_reticle_set_flight_cursor_offset() {
	matrix view_mat;
	vm_angles_2_matrix(&view_mat, &Player_flight_cursor);
	view_mat = view_mat * Eye_matrix;

	vec3d view_pos = Eye_position + view_mat.vec.fvec * 10000.0f;
	g3_rotate_vertex(&Player_flight_cursor_offset, &view_pos);
	g3_project_vertex(&Player_flight_cursor_offset);
}
