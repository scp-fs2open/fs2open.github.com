/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "hud/hudreticle.h"
#include "hud/hud.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "io/timer.h"
#include "gamesnd/gamesnd.h"
#include "hud/hudtargetbox.h"
#include "weapon/emp.h"
#include "localization/localize.h"
#include "network/multi.h"

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

// coordinates
static int Max_speed_coords[GR_NUM_RESOLUTIONS][2] = 
{
	{ // GR_640
		236, 254
	}, 
	{ // GR_1024
		377, 406
	}
};
static int Zero_speed_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		252, 303
	},
	{ // GR_1024
		403, 485
	}
};

HudGaugeReticle::HudGaugeReticle():
HudGauge(HUD_OBJECT_CENTER_RETICLE, HUD_CENTER_RETICLE, true, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_TOPDOWN | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeReticle::initBitmaps(char *fname)
{
	crosshair.first_frame = bm_load_animation(fname, &crosshair.num_frames);
	if (crosshair.first_frame < 0) {
		mprintf(("Cannot load hud ani: %s\n", fname));
	}
}

void HudGaugeReticle::initFirepointDisplay(bool firepoint, int scaleX, int scaleY, int size) {
	firepoint_display = firepoint;
	firepoint_scale_x = scaleX;
	firepoint_scale_y = scaleY;
	firepoint_size = size;
}

void HudGaugeReticle::render(float frametime)
{
	setGaugeColor(HUD_C_BRIGHT);

	renderBitmap(crosshair.first_frame, position[0], position[1]);

	if (firepoint_display) {
		fp.clear();
		getFirepointStatus();
		
		if (fp.size() > 0) {
			int ax, ay;
			bm_get_info(crosshair.first_frame, &ax, &ay);
			int centerX = position[0] + (ax / 2);
			int centerY = position[1] + (ay / 2);

			for (SCP_vector<firepoint>::iterator fpi = fp.begin(); fpi != fp.end(); ++fpi) {
				if (fpi->active == 2)
					setGaugeColor(HUD_C_BRIGHT);
				else if (fpi->active == 1)
					setGaugeColor(HUD_C_NORMAL);
				else
					setGaugeColor(HUD_C_DIM);
			
				renderCircle((int) (centerX + (fpi->xy.x * firepoint_scale_x)), (int) (centerY + (fpi->xy.y * firepoint_scale_y)), firepoint_size);
			}
		}
	}
}

void HudGaugeReticle::getFirepointStatus() {
	//First, get the player ship
	ship_info* pship;
	ship* shipp;
	polymodel* pm;

	Assert(Objects[Player->objnum].type == OBJ_SHIP);

	if (Objects[Player->objnum].type == OBJ_SHIP) {
		shipp = &Ships[Objects[Player->objnum].instance];
		pship = &Ship_info[shipp->ship_info_index];
	
		//Get the player eyepoint
		pm = model_get(pship->model_num);

		if (pm->n_guns > 0) { 
			eye eyepoint = pm->view_positions[shipp->current_viewpoint];
			vec2d ep = {eyepoint.pnt.xyz.x, eyepoint.pnt.xyz.y};

			for (int i = 0; i < pm->n_guns; i++) {
				int isactive = 0;

				if ( !timestamp_elapsed(shipp->weapons.next_primary_fire_stamp[i]) )
					isactive = 1;
				else if (timestamp_elapsed(shipp->weapons.primary_animation_done_time[i]))
					isactive = 1;
				else if (i == shipp->weapons.current_primary_bank || shipp->flags & SF_PRIMARY_LINKED)
					isactive = 2;

				for (int j = 0; j < pm->gun_banks[i].num_slots; j++) {
					firepoint tmp = { {ep.x - pm->gun_banks[i].pnt[j].xyz.x, ep.y - pm->gun_banks[i].pnt[j].xyz.y}, isactive};
					fp.push_back(tmp);
				}
			}
		}
	}
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

void HudGaugeThrottle::initMaxSpeedOffsets(int x, int y)
{
	Max_speed_offsets[0] = x;
	Max_speed_offsets[1] = y;
}

void HudGaugeThrottle::initZeroSpeedOffsets(int x, int y)
{
	Zero_speed_offsets[0] = x;
	Zero_speed_offsets[1] = y;
}

void HudGaugeThrottle::initOrbitCenterOffsets(int x, int y)
{
	Orbit_center_offsets[0] = x;
	Orbit_center_offsets[1] = y;
}

void HudGaugeThrottle::initOrbitRadius(int radius)
{
	orbit_radius = radius;
}

void HudGaugeThrottle::showBackground(bool show)
{
	Show_background = show;
}

void HudGaugeThrottle::initBitmaps(char *fname)
{
	throttle_frames.first_frame = bm_load_animation(fname, &throttle_frames.num_frames);

	if (throttle_frames.first_frame < 0) {
		mprintf(("Cannot load hud ani: %s\n", fname));
	}
}

void HudGaugeThrottle::pageIn()
{
	bm_page_in_aabitmap( throttle_frames.first_frame, throttle_frames.num_frames);
}

void HudGaugeThrottle::render(float frametime)
{
	float	desired_speed, max_speed, current_speed, absolute_speed, absolute_displayed_speed, max_displayed_speed, percent_max, percent_aburn_max;
	int	desired_y_pos, y_end;

	ship_info	*sip;
	sip = &Ship_info[Player_ship->ship_info_index];

	current_speed = Player_obj->phys_info.fspeed;
	if ( current_speed < 0.0f){
		current_speed = 0.0f;
	}

	max_speed = Ships[Player_obj->instance].current_max_speed;
	if ( max_speed <= 0 ) {
		max_speed = sip->max_vel.xyz.z;
	}

	absolute_speed = Player_obj->phys_info.speed;

	// scale by distance modifier from hud_guages.tbl for display purposes
	absolute_displayed_speed = absolute_speed * Hud_speed_multiplier;
	max_displayed_speed = max_speed * Hud_speed_multiplier;

	desired_speed = Player->ci.forward * max_speed;
	if ( desired_speed < 0.0f ){		// so ships that go backwards don't force the indicators below where they can go
		desired_speed = 0.0f;
	}

	desired_y_pos = position[1] + Bottom_offset_y - fl2i(throttle_h*desired_speed/max_speed+0.5f) - 1;

	if (max_speed <= 0) {
		percent_max = 0.0f;
	} else {
		percent_max = current_speed / max_speed;
	}

	percent_aburn_max = 0.0f;
	if ( percent_max > 1 ) {
		percent_max = 1.0f;
		percent_aburn_max = (current_speed - max_speed) / (sip->afterburner_max_vel.xyz.z - max_speed);
		if ( percent_aburn_max > 1.0f ) {
			percent_aburn_max = 1.0f;
		}
		if ( percent_aburn_max < 0 ) {
			percent_aburn_max = 0.0f;
		}
	}

	y_end = position[1] + Bottom_offset_y - fl2i(throttle_h*percent_max+0.5f);
	if ( percent_aburn_max > 0 ) {
		y_end -= fl2i(percent_aburn_max * throttle_aburn_h + 0.5f);
	}

	if ( Player_obj->phys_info.flags & PF_AFTERBURNER_ON ) {
		// default value is 240 when afterburner is on. 
		//I'm assuming that this value is basically Bottom_offset_y - throttle_aburn_h - throttle_h
		desired_y_pos = position[1] + Bottom_offset_y - throttle_aburn_h - throttle_h; 
	}

	setGaugeColor();
	
	if(Show_background) {
		renderThrottleBackground(y_end);
	} else {
		renderBitmap(throttle_frames.first_frame, position[0], position[1]);			
	}

	// draw throttle speed number
	//hud_render_throttle_speed(current_speed, y_end);
	// Absolute speed, not forward speed, for hud speed reticle - fixes the guage for sliding -- kazan
	renderThrottleSpeed(absolute_displayed_speed, y_end);

	// draw the "desired speed" bar on the throttle
	renderThrottleLine(desired_y_pos);

	// draw left arc (the bright portion of the throttle gauge)
	renderThrottleForeground(y_end);

	renderPrintf(position[0] + Max_speed_offsets[0], position[1] + Max_speed_offsets[1], "%d",fl2i(max_displayed_speed+0.5f));
	renderPrintf(position[0] + Zero_speed_offsets[0], position[1] + Zero_speed_offsets[1], XSTR( "0", 292));
}

void HudGaugeThrottle::renderThrottleSpeed(float current_speed, int y_end)
{
	char buf[32];
	int sx, sy, x_pos, y_pos, w, h;

	//setGaugeColor();

	// y_end is the y-coordinate of the current throttle setting, calc x-coordinate for edge of 
	// circle (x^2 + y^2 = r^2)
	y_pos = position[1] + Orbit_center_offsets[1] - y_end;
	x_pos = (int)sqrt(double(orbit_radius * orbit_radius - y_pos * y_pos) );
	x_pos = position[0] + Orbit_center_offsets[0] - x_pos;

	// draw current speed at (x_pos, y_end);
	sprintf(buf, "%d", fl2i(current_speed+0.5f));
	hud_num_make_mono(buf);
	gr_get_string_size(&w, &h, buf);
	sx = x_pos - w - 2;
	sy = fl2i(y_end - h/2.0f + 1.5);
	renderPrintf(sx, sy, buf);

	if ( object_get_gliding(Player_obj) ) { 
		int offset;
		if ( current_speed <= 9.5 ) {
			offset = -31;
		} else if ( current_speed <= 99.5 ) {
			offset = -22;
		} else {
			offset = -13;
		}
		renderString(sx+offset, sy + h, "GLIDE");
	} else if ( Players[Player_num].flags & PLAYER_FLAGS_MATCH_TARGET ) {
		int offset;
		if ( current_speed <= 9.5 ) {
			offset = 0;
		} else {
			offset = 3;
		}

		if (Lcl_gr) {
			// print an m, cuz the voice says its an m.  
			// its a normal m cuz the german font has no special m (its an a)
			renderString(sx+offset, sy + h, "m");
		} else {
			renderPrintf(sx+offset, sy + h, "%c", Lcl_special_chars + 3);
		}
	}
}

void HudGaugeThrottle::renderThrottleLine(int y_line)
{
	// hud_set_bright_color();
	//setGaugeColor(HUD_C_BRIGHT);

	renderBitmapEx(throttle_frames.first_frame+3, 
		position[0], y_line, 
		throttle_w, 1, 
		0, 
		y_line-position[1]);
}

void HudGaugeThrottle::renderThrottleForeground(int y_end)
{
	int w,h;

	//setGaugeColor();

	bm_get_info(throttle_frames.first_frame+1,&w,&h);

	if ( y_end < (position[1] + h - 1) ) {		
		renderBitmapEx(throttle_frames.first_frame + 2, position[0], y_end, w, h - (y_end - position[1]), 0, y_end - position[1]);
	}
}

void HudGaugeThrottle::renderThrottleBackground(int y_end)
{
	int w,h;

	//setGaugeColor();

	bm_get_info( throttle_frames.first_frame+1,&w,&h);

	if ( y_end > position[1] ) {
		renderBitmapEx(throttle_frames.first_frame+1, position[0], position[1], w, y_end-position[1]+1, 0, 0);		
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
		mprintf(("Cannot load hud ani: %s\n", fname_arc));
	}
	
	laser_warn.first_frame = bm_load_animation(fname_laser, &laser_warn.num_frames);
	if (laser_warn.first_frame < 0) {
		mprintf(("Cannot load hud ani: %s\n", fname_laser));
	}

	lock_warn.first_frame = bm_load_animation(fname_lock, &lock_warn.num_frames);
	if (lock_warn.first_frame < 0) {
		mprintf(("Cannot load hud ani: %s\n", fname_lock));
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

void HudGaugeThreatIndicator::render(float frametime)
{
	setGaugeColor();
	renderBitmap(threat_arc.first_frame+1, position[0], position[1]);

	renderLaserThreat();
	renderLockThreat();
}

void HudGaugeThreatIndicator::renderLaserThreat()
{
	int frame_offset, num_frames;

	//Check how many frames the ani actually has
	num_frames = laser_warn.num_frames;
	//We need at least two frames here
	Assert( num_frames >= 2 );

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

	renderBitmap(laser_warn.first_frame + frame_offset, position[0] + Laser_warn_offsets[0], position[1] + Laser_warn_offsets[1]);	
}

void HudGaugeThreatIndicator::renderLockThreat()
{
	int frame_offset, num_frames;

	//Let's find out how many frames our ani has, and adjust accordingly
	num_frames = lock_warn.num_frames;
	//We need at least two frames here
	Assert( num_frames >= 2 );

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

	renderBitmap(lock_warn.first_frame+frame_offset, position[0] + Lock_warn_offsets[0], position[1] + Lock_warn_offsets[1]);
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
		mprintf(("Cannot load hud ani: %s\n", fname_arc));
	}
	
	weapon_linking_modes[LINK_ONE_PRIMARY].first_frame = bm_load_animation(fname_primary_link_1, &weapon_linking_modes[LINK_ONE_PRIMARY].num_frames);
	if (weapon_linking_modes[LINK_ONE_PRIMARY].first_frame < 0) {
		mprintf(("Cannot load hud ani: %s\n", fname_primary_link_1));
	}

	weapon_linking_modes[LINK_TWO_PRIMARY].first_frame = bm_load_animation(fname_primary_link_2, &weapon_linking_modes[LINK_TWO_PRIMARY].num_frames);
	if (weapon_linking_modes[LINK_TWO_PRIMARY].first_frame < 0) {
		mprintf(("Cannot load hud ani: %s\n", fname_primary_link_2));
	}

	weapon_linking_modes[LINK_ONE_SECONDARY].first_frame = bm_load_animation(fname_secondary_link_1, &weapon_linking_modes[LINK_ONE_SECONDARY].num_frames);
	if (weapon_linking_modes[LINK_ONE_SECONDARY].first_frame < 0) {
		mprintf(("Cannot load hud ani: %s\n", fname_secondary_link_1));
	}

	weapon_linking_modes[LINK_TWO_SECONDARY].first_frame = bm_load_animation(fname_secondary_link_2, &weapon_linking_modes[LINK_TWO_SECONDARY].num_frames);
	if (weapon_linking_modes[LINK_TWO_SECONDARY].first_frame < 0) {
		mprintf(("Cannot load hud ani: %s\n", fname_secondary_link_2));
	}

	weapon_linking_modes[LINK_THREE_SECONDARY].first_frame = bm_load_animation(fname_secondary_link_3, &weapon_linking_modes[LINK_THREE_SECONDARY].num_frames);
	if (weapon_linking_modes[LINK_THREE_SECONDARY].first_frame < 0) {
		mprintf(("Cannot load hud ani: %s\n", fname_secondary_link_3));
	}
}


void HudGaugeWeaponLinking::pageIn()
{
	bm_page_in_aabitmap(arc.first_frame, arc.num_frames);

	for(int i = 0; i < NUM_WEAPON_LINK_MODES; i++) {
		bm_page_in_aabitmap(weapon_linking_modes[i].first_frame, weapon_linking_modes[i].num_frames);
	}
}

void HudGaugeWeaponLinking::render(float frametime)
{
	int			gauge_index=0, frame_offset=0;
	ship_weapon	*swp;

	renderBitmap(arc.first_frame+1, position[0], position[1]);

	swp = &Player_ship->weapons;

	switch( swp->num_primary_banks ) {
		case 0:
			gauge_index = -1;
			break;

		case 1:
			gauge_index = LINK_ONE_PRIMARY;
			if ( Player_ship->weapons.current_primary_bank == -1 ) {
				frame_offset = 0;	
			} else {
				frame_offset = 1;	
			}
			break;

		case 2:
			gauge_index = LINK_TWO_PRIMARY;
			if ( swp->current_primary_bank == -1 ) {
				frame_offset = 0;	
			} else {
				if ( Player_ship->flags & SF_PRIMARY_LINKED ) {
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
		renderBitmap(weapon_linking_modes[gauge_index].first_frame+frame_offset, position[0] + Weapon_link_offsets[gauge_index][0], position[1] + Weapon_link_offsets[gauge_index][1]);
	}

	int num_banks = swp->num_secondary_banks;
	if ( num_banks <= 0 ) {
		num_banks = Ship_info[Player_ship->ship_info_index].num_secondary_banks;
	}

	switch( num_banks ) {
		case 0:
			Int3();
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
		if ( swp->num_secondary_banks <= 0 ) {
			frame_offset = 0;
		} else {
			frame_offset = swp->current_secondary_bank+1;
		}

		renderBitmap(weapon_linking_modes[gauge_index].first_frame+frame_offset, position[0] + Weapon_link_offsets[gauge_index][0],  position[1] + Weapon_link_offsets[gauge_index][1]);
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
				snd_play( &Snds[ship_get_sound(Player_obj, SND_THREAT_FLASH)]);
			}
		}
	} 
}
