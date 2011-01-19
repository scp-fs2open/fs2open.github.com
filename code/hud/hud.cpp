/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "hud/hud.h"
#include "asteroid/asteroid.h"
#include "cmdline/cmdline.h"
#include "freespace2/freespace.h"
#include "gamesnd/eventmusic.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "globalincs/linklist.h"
#include "graphics/font.h"
#include "hud/hudconfig.h"
#include "hud/hudescort.h"
#include "hud/hudets.h"
#include "hud/hudlock.h"
#include "hud/hudmessage.h"
#include "hud/hudobserver.h"
#include "hud/hudreticle.h"
#include "hud/hudshield.h"
#include "hud/hudsquadmsg.h"
#include "hud/hudtarget.h"
#include "hud/hudtargetbox.h"
#include "hud/hudwingmanstatus.h"
#include "hud/hudparse.h"
#include "object/objectdock.h"
#include "hud/hudnavigation.h"	//kazan
#include "io/timer.h"
#include "localization/localize.h"
#include "mission/missiongoals.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "mission/missiontraining.h"
#include "missionui/redalert.h"
#include "model/model.h"
#include "object/object.h"
#include "playerman/player.h"
#include "radar/radar.h"
#include "render/3d.h"
#include "ai/aigoals.h"
#include "ship/ship.h"
#include "starfield/supernova.h"
#include "weapon/emp.h"
#include "weapon/weapon.h"
#include "radar/radarsetup.h"
#include "iff_defs/iff_defs.h"
#include "network/multiutil.h"
#include "network/multi_voice.h"
#include "network/multi_pmsg.h"
#include "bmpman/bmpman.h"

SCP_vector<HudGauge*> default_hud_gauges;

// new values for HUD alpha
#define HUD_NEW_ALPHA_DIM				80	
#define HUD_NEW_ALPHA_NORMAL			120
#define HUD_NEW_ALPHA_BRIGHT			220

// high contrast
#define HUD_NEW_ALPHA_DIM_HI			130
#define HUD_NEW_ALPHA_NORMAL_HI		190
#define HUD_NEW_ALPHA_BRIGHT_HI		255

// Externals not related to the HUD code itself
extern float View_zoom;

// globals that will control the color of the HUD gauges
int HUD_color_red = 0;
int HUD_color_green = 255;
int HUD_color_blue = 0;
int HUD_color_alpha = HUD_COLOR_ALPHA_DEFAULT;		// 1 -> HUD_COLOR_ALPHA_USER_MAX

int HUD_draw     = 1;
int HUD_contrast = 0;										// high or lo contrast (for nebula, etc)

// Goober5000
int HUD_disable_except_messages = 0;

color HUD_color_defaults[HUD_NUM_COLOR_LEVELS];		// array of colors with different alpha blending
color HUD_color_debug;										// grey debug text shown on HUD

static int Player_engine_snd_loop = -1;

// HUD render frame offsets
float HUD_offset_x = 0.0f;
float HUD_offset_y = 0.0f;

// the offset of the player's view vector and the ship forward vector in pixels (Swifty)
int HUD_nose_x;
int HUD_nose_y;
// Global: integrity of player's target
float Pl_target_integrity;

int Hud_max_targeting_range;

static int Hud_last_can_target;	// whether Player is able to target in the last frame
static int Hud_can_target_timer;	// timestamp to allow target gauge to draw static once targeting functions are not allowed

// centered text message gauges (collision, emp, etc)
char Hud_text_flash[512] = "";
int Hud_text_flash_timer = 0;
int Hud_text_flash_interval = 0;

void hud_init_text_flash_gauge();
void hud_start_text_flash(char *txt, int t, int interval);

// multiplayer messaging text
int Multi_msg_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		5, 150
	},
	{ // GR_1024
		8, 240
	}
};

// multiplayer voice stuff
int Voice_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		5, 165
	},
	{ // GR_1024
		8, 255
	}
};

// ping text coords
int Ping_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		560, 3
	},
	{ // GR_1024
		896, 5
	}
};

// supernova coords
int Supernova_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		100, 100
	},
	{ // GR_1024
		170, 170
	}
};

// used to draw the kills gauge
hud_frames Kills_gauge;
int Kills_gauge_loaded = 0;
int Kills_gauge_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		497, 361
	},
	{ // GR_1024
		880, 624
	}
};
int Kills_text_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		503, 365
	},
	{ // GR_1024
		886, 628
	}
};

int Kills_text_val_coords_gr[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		615, 365
	},
	{ // GR_1024
		984, 628
	}
};

int Kills_text_val_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		571, 365
	},
	{ // GR_1024
		954, 628
	}
};

char Kills_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"kills1",
	"kills1"
};

// used to draw the hud support view
static int Hud_support_view_active;
static int Hud_support_view_abort;		// active when we need to display abort message
static int Hud_support_view_fade;		// timer
static int Hud_support_obj_sig, Hud_support_objnum, Hud_support_target_sig;

// flashing gauges
#define HUD_GAUGE_FLASH_DURATION		5000
#define HUD_GAUGE_FLASH_INTERVAL		200
int HUD_gauge_flash_duration[NUM_HUD_GAUGES];
int HUD_gauge_flash_next[NUM_HUD_GAUGES];
int HUD_gauge_bright;

// Objective display
typedef struct objective_display_info
{
	int display_timer;
	int goal_type;
	int goal_status;
	int goal_ntotal;
	int goal_nresolved;

} objective_display_info;

static objective_display_info Objective_display;

// Subspace notify display
static int Subspace_notify_active;
static int Objective_notify_active;
static int HUD_abort_subspace_timer = 1;

// used to track how player subsystems are getting damaged
typedef struct hud_subsys_info
{
	float	last_str;
	int	flash_duration_timestamp;
} hud_subsys_info;

static hud_subsys_info	Pl_hud_subsys_info[SUBSYSTEM_MAX];
static int					Pl_hud_next_flash_timestamp;
static int					Pl_hud_is_bright;

#define SUBSYS_DAMAGE_FLASH_DURATION	1800
#define SUBSYS_DAMAGE_FLASH_INTERVAL	100

float Player_rearm_eta = 0;

// forward declarations
void update_throttle_sound();
void hud_damage_popup_init();
void hud_support_view_init();
void hud_gauge_flash_init();
void hud_objective_message_init();
void hud_stop_subspace_notify();
void hud_start_subspace_notify();
void hud_stop_objective_notify();
void hud_start_objective_notify();
int hud_subspace_notify_active();
int hud_objective_notify_active();
void hud_subspace_notify_abort();
void hud_maybe_display_subspace_notify();
void hud_init_kills_gauge();
int hud_maybe_render_emp_icon();
void hud_init_emp_icon();

//	Saturate a value in minv..maxv.
void saturate(int *i, int minv, int maxv)
{
	if (*i < minv)
		*i = minv;
	else if (*i > maxv)
		*i = maxv;
}

// init the colors used for the different shades of the HUD
void HUD_init_hud_color_array()
{
	int i;

	for ( i = 0; i < HUD_NUM_COLOR_LEVELS; i++ ) {
		gr_init_alphacolor( &HUD_color_defaults[i], HUD_color_red, HUD_color_green, HUD_color_blue, (i+1)*16 );
	}
}

// HUD_init will call all the various HUD gauge init functions.  This function is called at the
// start of each mission (level)
void HUD_init_colors()
{
	saturate(&HUD_color_red, 0, 255);
	saturate(&HUD_color_green, 0, 255);
	saturate(&HUD_color_blue, 0, 255);
	saturate(&HUD_color_alpha, 0, HUD_COLOR_ALPHA_USER_MAX);

	gr_init_alphacolor( &HUD_color_debug, 128, 255, 128, HUD_color_alpha*16 );
	HUD_init_hud_color_array();

	hud_init_targeting_colors();
	hud_gauge_flash_init();
}

// The following global data is used to determine if we should change the engine sound.
// We only check if the throttle has changed every THROTTLE_SOUND_CHECK_INTERVAL ms, and
// then we make sure that the throttle has actually changed.  If it has changed, we start
// a new sound and/or adjust the volume.  This occurs in update_throttle_sound()
//
static float last_percent_throttle;
#define THROTTLE_SOUND_CHECK_INTERVAL	50	// in ms
static int throttle_sound_check_id;

// used for the display of damaged subsystems
typedef struct hud_subsys_damage
{
	int	str;
	int	type;
	char	*name;
} hud_subsys_damage;

#define DAMAGE_FLASH_TIME 150
static int Damage_flash_bright;
static int Damage_flash_timer;

HudGauge::HudGauge():
base_w(0), base_h(0), gauge_config(-1), config_override(true), reticle_follow(false), active(false), pop_up(false), disabled_views(0), texture_target(-1), 
texture_cache(-1), target_x(-1), target_y(-1), target_w(-1), target_h(-1), cache_w(-1), cache_h(-1), custom_gauge(false), font_num(FONT1), off_by_default(false), sexp_override(false)
{
	position[0] = 0;
	position[1] = 0;

	gr_init_alphacolor(&gauge_color, 255, 255, 255, (HUD_color_alpha+1)*16);
	flash_duration = timestamp(1);
	flash_next = timestamp(1);
	flash_status = false;

	popup_timer = timestamp(1);

	texture_target_fname[0] = '\0';

	custom_name[0] = '\0';
	custom_text[0] = '\0';
	custom_frame.first_frame = -1;
	custom_frame.num_frames = 0;
	custom_frame_offset = 0;
}

HudGauge::HudGauge(int _gauge_object, int _gauge_config, bool _allow_override, bool _slew, bool _message, int _disabled_views, 
				   int r, int g, int b):
base_w(0), base_h(0), gauge_object(_gauge_object), gauge_config(_gauge_config), config_override(_allow_override), reticle_follow(_slew), 
message_gauge(_message), active(false), pop_up(false), disabled_views(_disabled_views), texture_target(-1), texture_cache(-1), target_x(-1), target_y(-1), 
target_w(-1), target_h(-1), textoffset_x(0), textoffset_y(0), cache_w(-1), cache_h(-1), custom_gauge(false), font_num(FONT1), off_by_default(false), sexp_override(false)
{
	Assert(gauge_config <= NUM_HUD_GAUGES && gauge_config >= 0);

	position[0] = 0;
	position[1] = 0;

	if(r >= 0 && r <= 255 && 
		g >= 0 && g <= 255 && 
		b >= 0 && b <= 255) {
		gr_init_alphacolor(&gauge_color, r, g, b, (HUD_color_alpha+1)*16);
	} else {
		gr_init_alphacolor(&gauge_color, 255, 255, 255, (HUD_color_alpha+1)*16);
	}

	flash_duration = timestamp(1);
	flash_next = timestamp(1);
	flash_status = false;

	popup_timer = timestamp(1);

	texture_target_fname[0] = '\0';

	custom_name[0] = '\0';
	custom_text[0] = '\0';
	custom_frame.first_frame = -1;
	custom_frame.num_frames = 0;
	custom_frame_offset = 0;
}

// constructor for custom gauges
HudGauge::HudGauge(int _gauge_config, bool _slew, int r, int g, int b, char* _custom_name, char* _custom_text, char* frame_fname, int txtoffset_x, int txtoffset_y):
gauge_object(HUD_OBJECT_CUSTOM), base_w(0), base_h(0), gauge_config(_gauge_config), config_override(true), reticle_follow(_slew), message_gauge(false), 
active(false), pop_up(false), disabled_views(VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY), texture_target(-1), texture_cache(-1), 
target_x(-1), target_y(-1), target_w(-1), target_h(-1), textoffset_x(txtoffset_x), textoffset_y(txtoffset_y), cache_w(-1), cache_h(-1), custom_gauge(true), font_num(FONT1), off_by_default(false), sexp_override(false)
{
	position[0] = 0;
	position[1] = 0;

	if(r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
		gr_init_alphacolor(&gauge_color, r, g, b, (HUD_color_alpha+1)*16);
	} else {
		gr_init_alphacolor(&gauge_color, 255, 255, 255, (HUD_color_alpha+1)*16);
	}

	flash_duration = timestamp(1);
	flash_next = timestamp(1);
	flash_status = false;

	popup_timer = timestamp(1);

	texture_target_fname[0] = '\0';

	if(_custom_name) {
		strcpy_s(custom_name, _custom_name);
	} else {
		custom_name[0] = '\0';
	}

	if(_custom_text) {
		strcpy_s(custom_text, _custom_text);
	} else {
		custom_text[0] = '\0';
	}

	custom_frame.first_frame = -1;
	custom_frame.num_frames = 0;
	custom_frame_offset = 0;

	if(frame_fname) {
		custom_frame.first_frame = bm_load_animation(frame_fname, &custom_frame.num_frames);
		if (custom_frame.first_frame < 0 ) {
			Warning(LOCATION,"Cannot load hud ani: %s\n", frame_fname);
		}
	}
}

void HudGauge::initPosition(int x, int y)
{
	Assert(x >= 0 && y >= 0);

	position[0] = x;
	position[1] = y;
}

void HudGauge::initBaseResolution(int w, int h)
{
	Assert(w >= 640 && h >= 480);

	base_w = w;
	base_h = h;
}

void HudGauge::initSlew(bool slew)
{
	reticle_follow = slew;
}

void HudGauge::initFont(int font)
{
	if ( font >= 0 && font < Num_fonts) {
		font_num = font;
	}
}

char* HudGauge::getCustomGaugeName()
{
	return custom_name;
}

char* HudGauge::getCustomGaugeText()
{
	return custom_text;
}

void HudGauge::updateCustomGaugeCoords(int _x, int _y)
{
	if(!custom_gauge) {
		return;
	}

	position[0] = _x;
	position[1] = _y;
}

void HudGauge::updateCustomGaugeFrame(int frame_offset)
{
	if(!custom_gauge) {
		return;
	}

	custom_frame_offset = frame_offset;
}

void HudGauge::updateCustomGaugeText(char* txt)
{
	if(!custom_gauge) {
		return;
	}

	strcpy_s(custom_text, txt);
}

bool HudGauge::configOverride()
{
	return config_override;
}

void HudGauge::setFont()
{
	gr_set_font(font_num);
}

void HudGauge::setGaugeColor(int bright_index)
{
	int alpha;

	// if we're drawing it as bright
	if(bright_index != HUD_C_NONE){
		switch(bright_index){
		case HUD_C_DIM:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_DIM_HI : HUD_NEW_ALPHA_DIM;
			gr_init_alphacolor(&gauge_color, gauge_color.red, gauge_color.green, gauge_color.blue, alpha);
			break;

		case HUD_C_NORMAL:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_NORMAL_HI : HUD_NEW_ALPHA_NORMAL;
			gr_init_alphacolor(&gauge_color, gauge_color.red, gauge_color.green, gauge_color.blue, alpha);
			break;

		case HUD_C_BRIGHT:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_BRIGHT_HI : HUD_NEW_ALPHA_BRIGHT;
			gr_init_alphacolor(&gauge_color, gauge_color.red, gauge_color.green, gauge_color.blue, alpha);
			break;

		// intensity
		default: 
			Assert((bright_index >= 0) && (bright_index < HUD_NUM_COLOR_LEVELS));
			if(bright_index < 0){
				bright_index = 0;
			}
			if(bright_index >= HUD_NUM_COLOR_LEVELS){
				bright_index = HUD_NUM_COLOR_LEVELS - 1;
			}

			// alpha = 255 - (255 / (bright_index + 1));
			// alpha = (int)((float)alpha * 1.5f);
			int level = 255 / (HUD_NUM_COLOR_LEVELS);
			alpha = level * bright_index;
			if(alpha > 255){
				alpha = 255;
			}
			if(alpha < 0){
				alpha = 0;
			}
			gr_init_alphacolor(&gauge_color, gauge_color.red, gauge_color.green, gauge_color.blue, alpha);
			break;
		}
	} else {
		switch(maybeFlashSexp()) {
		case 0:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_DIM_HI : HUD_NEW_ALPHA_DIM;
			gr_init_alphacolor(&gauge_color, gauge_color.red, gauge_color.green, gauge_color.blue, alpha);
			break;
		case 1:			
			alpha = HUD_contrast ? HUD_NEW_ALPHA_BRIGHT_HI : HUD_NEW_ALPHA_BRIGHT;
			gr_init_alphacolor(&gauge_color, gauge_color.red, gauge_color.green, gauge_color.blue, alpha);
			break;
		default:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_NORMAL_HI : HUD_NEW_ALPHA_NORMAL;
			gr_init_alphacolor(&gauge_color, gauge_color.red, gauge_color.green, gauge_color.blue, alpha);
		}
	}

	gr_set_color_fast(&gauge_color);	
}

int HudGauge::getConfigType()
{
	//return gauge_type;
	return gauge_config;
}

int HudGauge::getObjectType()
{
	return gauge_object;
}

void HudGauge::updateColor(int r, int g, int b, int a)
{
	gr_init_alphacolor(&gauge_color, r, g, b, a);
}

void HudGauge::updateActive(bool show)
{
	active = show;
}

void HudGauge::initRenderStatus(bool render)
{
	off_by_default = !render;
}

bool HudGauge::isOffbyDefault()
{
	return off_by_default;
}

bool HudGauge::isActive()
{
	return active && !sexp_override;
}

void HudGauge::updateSexpOverride(bool sexp)
{
	sexp_override = sexp;
}

void HudGauge::updatePopUp(bool pop_up_flag)
{
	pop_up = pop_up_flag;
}

void HudGauge::startPopUp(int time) 
{
	//Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);
	if ( !pop_up ) {
		return;
	}

	popup_timer = timestamp(time);
}

int HudGauge::popUpActive()
{
	//Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);
	if ( !pop_up ) {
		return 0;
	}

	if ( !timestamp_elapsed(popup_timer) ) {
		return 1;
	} else {
		return 0;
	}
}

void HudGauge::resetTimers()
{
	flash_duration = timestamp(1);
	flash_next = timestamp(1);
	flash_status = false;

	popup_timer = timestamp(1);
}

void HudGauge::startFlashSexp()
{
	flash_duration = timestamp(HUD_GAUGE_FLASH_DURATION);
	flash_next = timestamp(1);
	flash_status = false;
}

bool HudGauge::flashExpiredSexp()
{
	if(timestamp_elapsed(flash_duration)) {
		return true;
	}
	
	return false;
}

int HudGauge::maybeFlashSexp()
{
	if ( !timestamp_elapsed(flash_duration) ) {
		if ( timestamp_elapsed(flash_next) ) {
			flash_next = timestamp(HUD_GAUGE_FLASH_INTERVAL);

			// toggle between default and bright frames
			flash_status = !flash_status;
		}
		return (int)flash_status;
	}

	return -1;
}

void HudGauge::render(float frametime)
{
	if(!custom_gauge) {
		return;
	}

	setGaugeColor();

	if(custom_text) {
		if(strlen(custom_text) > 0) {
			hud_num_make_mono(custom_text);
			renderString(position[0] + textoffset_x, position[1] + textoffset_y, custom_text);
		}
	}

	if(custom_frame.first_frame > -1) {
		renderBitmap(custom_frame.first_frame + custom_frame_offset, position[0], position[1]);
	}
}

void HudGauge::renderString(int x, int y, char *str)
{
	int nx = 0, ny = 0;

	if(reticle_follow) {
		nx = HUD_nose_x;
		ny = HUD_nose_y;
		gr_resize_screen_pos(&nx, &ny);
	}

	bool resize = true;
	if(gr_screen.rendering_to_texture != -1) {
		resize = false;
	}

	gr_set_screen_scale(base_w, base_h);
	gr_unsize_screen_pos(&nx, &ny);
	gr_string(x + nx, y + ny, str, resize);
	gr_reset_screen_scale();
}

void HudGauge::renderString(int x, int y, int gauge_id, char *str)
{
	int  nx = 0, ny = 0;

	if(reticle_follow) {
		nx = HUD_nose_x;
		ny = HUD_nose_y;
		gr_resize_screen_pos(&nx, &ny);
	} 

	gr_set_screen_scale(base_w, base_h);

	gr_unsize_screen_pos(&nx, &ny);

	bool resize = true;
	if(gr_screen.rendering_to_texture != -1) {
		resize = false;
	}

	if(gauge_id > -2) {
		emp_hud_string(x + nx, y + ny, gauge_id, str, resize);
	} else {
		gr_string(x + nx, y + ny, str, resize);
	}

	gr_reset_screen_scale();
}

void HudGauge::renderStringAlignCenter(int x, int y, int area_width, char *s)
{
	int w, h;

	gr_get_string_size(&w, &h, s);
	renderString(x + ((area_width - w) / 2), y, s);
}

void HudGauge::renderPrintf(int x, int y, char* format, ...)
{
	char tmp[256] = "";
	va_list args;	
	
	// format the text
	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);

	renderString(x, y, tmp);
}

void HudGauge::renderPrintf(int x, int y, int gauge_id, char* format, ...)
{
	char tmp[256] = "";
	va_list args;	
	
	// format the text
	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);

	renderString(x, y, gauge_id, tmp);
}

void HudGauge::renderBitmap(int x, int y)
{
	int jx, jy, nx = 0, ny = 0;

	if(!emp_should_blit_gauge()) {
		return;
	}

	jx = x;
	jy = y;
	emp_hud_jitter(&jx, &jy);

	if(reticle_follow) {
		nx = HUD_nose_x;
		ny = HUD_nose_y;
		gr_resize_screen_pos(&nx, &ny);
	}

	gr_set_screen_scale(base_w, base_h);

	gr_unsize_screen_pos(&nx, &ny);
	jx += nx;
	jy += ny;

	bool resize = true;
	if(gr_screen.rendering_to_texture != -1) {
		resize = false;
	}
	
	gr_aabitmap(jx, jy, resize); 

	gr_reset_screen_scale();
}

void HudGauge::renderBitmap(int frame, int x, int y)
{
	gr_set_bitmap(frame);
	renderBitmap(x, y);
}

void HudGauge::renderBitmapUv(int frame, int x, int y, int w, int h, float u0, float v0, float u1, float v1)
{
	int jx, jy, nx = 0, ny = 0; 
	
	if(!emp_should_blit_gauge()) { 
		return;
	}

	jx = x; 
	jy = y; 
	emp_hud_jitter(&jx, &jy); 

	if(reticle_follow) {
		nx = HUD_nose_x;
		ny = HUD_nose_y;
		gr_resize_screen_pos(&nx, &ny);
	}

	gr_set_screen_scale(base_w, base_h);

	gr_unsize_screen_pos(&nx, &ny);
	jx += nx;
	jy += ny;

	gr_set_bitmap(frame);

	bool resize = true;
	if(gr_screen.rendering_to_texture != -1) {
		resize = false;
	}

	gr_bitmap_uv(jx, jy, w, h, u0, v0, u1, v1, resize);

	gr_reset_screen_scale();
}

void HudGauge::renderBitmapEx(int frame, int x, int y, int w, int h, int sx, int sy)
{
	int jx, jy, nx = 0, ny = 0; 
	
	if(!emp_should_blit_gauge()) { 
		return;
	}

	jx = x; 
	jy = y; 
	emp_hud_jitter(&jx, &jy); 

	if(reticle_follow) {
		nx = HUD_nose_x;
		ny = HUD_nose_y;
		gr_resize_screen_pos(&nx, &ny);
	}

	gr_set_screen_scale(base_w, base_h);

	gr_unsize_screen_pos(&nx, &ny);
	jx += nx;
	jy += ny;

	gr_set_bitmap(frame);

	bool resize = true;
	if(gr_screen.rendering_to_texture != -1) {
		resize = false;
	}

	gr_aabitmap_ex(jx, jy, w, h, sx, sy, resize);

	gr_reset_screen_scale();
}

void HudGauge::renderLine(int x1, int y1, int x2, int y2)
{
	int nx = 0, ny = 0;

	if(reticle_follow) {
		nx = HUD_nose_x;
		ny = HUD_nose_y;
		gr_resize_screen_pos(&nx, &ny);
	}

	gr_set_screen_scale(base_w, base_h);
	gr_unsize_screen_pos(&nx, &ny);
	gr_line(x1+nx, y1+ny, x2+nx, y2+ny);
	gr_reset_screen_scale();
}

void HudGauge::renderGradientLine(int x1, int y1, int x2, int y2, bool resize)
{
	int nx = 0, ny = 0;

	if(reticle_follow) {
		nx = HUD_nose_x;
		ny = HUD_nose_y;
		gr_resize_screen_pos(&nx, &ny);
	} 

	gr_set_screen_scale(base_w, base_h);
	gr_unsize_screen_pos(&nx, &ny);
	gr_gradient(x1+nx, y1+ny, x2+nx, y2+ny, resize);
	gr_reset_screen_scale();
}

void HudGauge::renderRect(int x, int y, int w, int h)
{
	int nx = 0, ny = 0;

	if(reticle_follow) {
		nx = HUD_nose_x;
		ny = HUD_nose_y;
		gr_resize_screen_pos(&nx, &ny);
	} 

	gr_set_screen_scale(base_w, base_h);
	gr_unsize_screen_pos(&nx, &ny);
	gr_rect(x+nx, y+ny, w, h);
	gr_reset_screen_scale();
}

void HudGauge::renderCircle(int x, int y, int diameter) {
	int nx = 0, ny = 0;

	if(reticle_follow) {
		nx = HUD_nose_x;
		ny = HUD_nose_y;
		gr_resize_screen_pos(&nx, &ny);
	} 

	gr_set_screen_scale(base_w, base_h);
	gr_unsize_screen_pos(&nx, &ny);
	gr_circle(x+nx, y+ny, diameter);
	gr_reset_screen_scale();
}

void HudGauge::setClip(int x, int y, int w, int h)
{
	int nx = 0, ny = 0;

	if(reticle_follow) {
		nx = HUD_nose_x;
		ny = HUD_nose_y;
	}

	int hx = fl2i(HUD_offset_x) + nx;
	int hy = fl2i(HUD_offset_y) + ny;

	// bring the HUD shake and nose offsets to the actual resolution scale
	gr_resize_screen_pos(&hx, &hy);

	gr_set_screen_scale(base_w, base_h);

	if(gr_screen.rendering_to_texture == -1) {
		// bring the clip coords from base resolution scale to actual resolution scale.
		gr_resize_screen_pos(&x, &y);
		gr_resize_screen_pos(&w, &h);
	}

	// clip the screen based on the actual resolution
	gr_set_clip(hx+x, hy+y, w, h, false);

	gr_reset_screen_scale();
}

void HudGauge::resetClip()
{
	int hx = fl2i(HUD_offset_x);
	int hy = fl2i(HUD_offset_y);

	// bring the HUD shake and nose offsets to the actual resolution scale
	gr_resize_screen_pos(&hx, &hy);

	gr_set_screen_scale(base_w, base_h);

	// clip the screen based on the actual resolution
	gr_set_clip(hx, hy, gr_screen.max_w, gr_screen.max_h, false);

	gr_reset_screen_scale();
}

void HudGauge::unsize(int *x, int *y)
{
	gr_set_screen_scale(base_w, base_h);
	gr_unsize_screen_pos(x, y);
	gr_reset_screen_scale();
}

void HudGauge::unsize(float *x, float *y)
{
	gr_set_screen_scale(base_w, base_h);
	gr_unsize_screen_posf(x, y);
	gr_reset_screen_scale();
}

void HudGauge::resize(int *x, int *y)
{
	gr_set_screen_scale(base_w, base_h);
	gr_resize_screen_pos(x, y);
	gr_reset_screen_scale();
}

void HudGauge::resize(float *x, float *y)
{
	gr_set_screen_scale(base_w, base_h);
	gr_resize_screen_posf(x, y);
	gr_reset_screen_scale();
}

void HudGauge::pageIn()
{
	if(custom_gauge) {
		if(custom_frame.first_frame > -1 && custom_frame.num_frames > 0) {
			bm_page_in_aabitmap( custom_frame.first_frame, custom_frame.num_frames );
		}
	}
}

void HudGauge::initialize()
{
}

bool HudGauge::canRender()
{
	if (sexp_override) {
		return false;
	}

	if(hud_disabled_except_messages() && !message_gauge) {
		return false;
	}

	if (hud_disabled() && !hud_disabled_except_messages()) {
		return false;
	}

	if(!active)
		return false;
	
	if ( !(Game_detail_flags & DETAIL_FLAG_HUD) ) {
		return false;
	}

	if ((Viewer_mode & disabled_views)) {
		return false;
	}

	if(pop_up) {
		if(!popUpActive()) {
			return false;
		}
	}

	return true;
}

void HudGauge::initCockpitTarget(char* display_name, int _target_x, int _target_y, int _target_w, int _target_h, int _canvas_w, int _canvas_h)
{
	if ( strlen(display_name) <= 0 ) {
		return;
	}

	strcpy_s(texture_target_fname, display_name);
	target_x = _target_x;
	target_y = _target_y;
	target_w = _target_w;
	target_h = _target_h;

	if ( _canvas_w > 0 || _canvas_h > 0 ) {
		cache_w = _canvas_w;
		cache_h = _canvas_h;
	} else {
		cache_w = _target_w;
		cache_h = _target_h;
	}
}

void HudGauge::createRenderCanvas()
{
	if ( strlen(texture_target_fname) <= 0 ) {
		return;
	}

	int texture_size;

	// get the bigger of the two
	if(target_w > target_h) {
		texture_size = target_w;
	} else {
		texture_size = target_h;
	}

	// create a texture that will fit our gauge
	// try to find the smallest power of two texture that can accomodate
	int i = 4; // start at 2^4 (16)
	while(texture_size > (int)pow(2.0, i)) {
		i++;
	}

	texture_size = (int)pow(2.0, i);

	texture_cache = bm_make_render_target(texture_size, texture_size, BMP_FLAG_RENDER_TARGET_DYNAMIC);
}

void HudGauge::clearRenderCanvas()
{
	if ( texture_cache >= 0 ) {
		bm_release(texture_cache);
	}

	texture_cache = -1;
}

bool HudGauge::setupRenderCanvas()
{
	// check if we're rendering to a canvas 

	if( texture_cache >= 0) {
		// have a render canvas so, prep this hud gauge to render to it.

		bm_set_render_target(texture_cache);
		//gr_set_cull(0);
		gr_clear();

		return true;
	} else if ( strlen(texture_target_fname) > 0 ) {
		// we don't have a render canvas but this gauge was intended to be rendered to one.
		// return false to tell the caller to skip rendering this gauge

		return false;
	}

	// don't need to do anything special for this gauge, so tell the caller that everything is fine
	return true;
}

void HudGauge::doneRenderCanvas()
{
	if( texture_cache >= 0) {
		//gr_set_cull(1);
		bm_set_render_target(-1);
	} 
}

void HudGauge::setCockpitTarget(cockpit_display *display)
{
	if ( !display ) {
		return;
	}

	if ( strcmp(texture_target_fname, display->name) ) {
		return;
	}

	if ( display->target < 0 ) {
		texture_cache = -1;
		texture_target = -1;
		return;
	}

	texture_target = display->target;
	display_offset_x = display->offset[0];
	display_offset_y = display->offset[1];
}

void HudGauge::resetCockpitTarget()
{
	texture_target = -1;
}

void HudGauge::renderToCockpit()
{
	if ( strlen(texture_target_fname) <= 0 ) {
		return;
	}

	if ( texture_cache < 0 ) {
		return;
	}

	if ( texture_target < 0 ) {
		return;
	}

	if( Ship_info[Player_ship->ship_info_index].cockpit_model_num < 0 ) { 
		return;
	}

	HUD_reset_clip();

	bm_set_render_target(texture_target);

	int w, h;

	// get texture canvas dimensions so we can generate UVs
	bm_get_info(texture_cache, &w, &h);

	int cull = gr_set_cull(0);

	// draw it to the cockpit!
	gr_set_bitmap( texture_cache, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f );
	gr_bitmap_uv(target_x+display_offset_x, target_y+display_offset_y, target_w, target_h, 0.0f, 0.0f, i2fl(cache_w)/i2fl(w), i2fl(cache_h)/i2fl(h), false);

	gr_set_cull(cull);

	bm_set_render_target(-1);
}

// ----------------------------------------------------------------------
// HUD_init()
//
// Called each level to initialize HUD systems
//
void HUD_init()
{
	HUD_init_colors();
	hud_init_msg_window();
	hud_init_targeting();
	hud_init_reticle();
	hud_shield_level_init();
	hud_targetbox_init_flash();
	hud_escort_init();
	hud_damage_popup_init();
	hud_support_view_init();
	hud_init_squadmsg();		// initialize the vars needed for squadmate messaging
	hud_objective_message_init();
	hud_init_wingman_status_gauge();
	hud_init_target_static();
	hud_init_text_flash_gauge();
	hud_init_kills_gauge();
	hud_stop_subspace_notify();
	hud_stop_objective_notify();
	hud_target_last_transmit_level_init();

	throttle_sound_check_id = timestamp(THROTTLE_SOUND_CHECK_INTERVAL);
	HUD_abort_subspace_timer = 1;
	Hud_last_can_target = 1;
	Hud_can_target_timer = 1;
	last_percent_throttle = 0.0f;

	// default to high contrast in the nebula
	HUD_contrast = 0;
	HUD_draw     = 1;
	HUD_disable_except_messages = 0;

	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		HUD_contrast = 1;
	}

	// reset to infinite
	Hud_max_targeting_range = 0;

	int i, num_gauges;

	// go through all HUD gauges and call their initialization functions
	for (i = 0; i < Num_ship_classes; i++) {
		if(Ship_info[i].hud_enabled) {
			num_gauges = Ship_info[i].hud_gauges.size();

			for(int j = 0; j < num_gauges; j++) {
				Ship_info[i].hud_gauges[j]->createRenderCanvas();
				Ship_info[i].hud_gauges[j]->initialize();
				Ship_info[i].hud_gauges[j]->resetTimers();
				Ship_info[i].hud_gauges[j]->updateSexpOverride(false);
			}
		}
	}

	num_gauges = default_hud_gauges.size();

	for(i = 0; i < num_gauges; i++) {
		default_hud_gauges[i]->initialize();
		default_hud_gauges[i]->resetTimers();
		default_hud_gauges[i]->updateSexpOverride(false);
	}
}

void hud_level_close()
{
	int num_gauges;

	// do post mission cleanup for HUD
	for (int i = 0; i < Num_ship_classes; i++) {
		if(Ship_info[i].hud_enabled) {
			num_gauges = Ship_info[i].hud_gauges.size();

			for(int j = 0; j < num_gauges; j++) {
				Ship_info[i].hud_gauges[j]->clearRenderCanvas();
			}
		}
	}
}

void hud_close()
{
	int i, num_gauges = 0;

	// for all ships, delete all hud gauge objects
	for (i = 0; i < Num_ship_classes; i++) {
		num_gauges = (int)Ship_info[i].hud_gauges.size();

		for(int j = 0; j < num_gauges; j++) {
			delete Ship_info[i].hud_gauges[j];
			Ship_info[i].hud_gauges[j] = NULL;
		}
		Ship_info[i].hud_gauges.clear();
	}

	num_gauges = (int)default_hud_gauges.size();

	for(i = 0; i < num_gauges; i++) {
		delete default_hud_gauges[i];
		default_hud_gauges[i] = NULL;
	}
	default_hud_gauges.clear();
}

void hud_toggle_draw()
{
	HUD_draw = !HUD_draw;
}

// Goober5000
void hud_set_draw(int draw)
{
	HUD_draw = draw;
}

//WMC
int hud_get_draw()
{
	return HUD_draw;
}

// Goober5000
void hud_disable_except_messages(int disable)
{
	HUD_disable_except_messages = disable;
}

// Goober5000
// like hud_disabled, except messages are still drawn
int hud_disabled_except_messages()
{
	return HUD_disable_except_messages;
}

// return !0 if HUD is disabled (ie no gauges are shown/usable), otherwise return 0
int hud_disabled()
{
	return !HUD_draw;
}

// Determine if we should popup the weapons gauge on the HUD.
void hud_maybe_popup_weapons_gauge()
{
	if ( hud_gauge_is_popup(HUD_WEAPONS_GAUGE) ) {
		ship_weapon *swp = &Player_ship->weapons;
		int			i;

		for ( i = 0; i < swp->num_secondary_banks; i++ ) {
			if ( swp->secondary_bank_ammo[i] > 0 ) {
				int ms_till_fire = timestamp_until(swp->next_secondary_fire_stamp[i]);
				if ( ms_till_fire >= 1000 ) {
					hud_gauge_popup_start(HUD_WEAPONS_GAUGE, 2500);
				}
			}
		}
	}
}

// hud_update_frame() will update hud systems
//
// This function updates those parts of the hud that are not dependant on the
// rendering of the hud.
void hud_update_frame(float frametime)
{
	object	*targetp;
	vec3d target_pos;
	int		can_target;

	update_throttle_sound();
	hud_check_reticle_list();
	hud_wingman_status_update();

	// Check hotkey selections to see if any ships need to be removed
	hud_prune_hotkeys();

	// Remove dead/departed ships from the escort list
	hud_escort_cull_list();
	hud_escort_update_list();

	hud_update_reticle( Player );

	hud_shield_hit_update();
	hud_update_weapon_flash();
	hud_maybe_popup_weapons_gauge();
	hud_update_objective_message();
	hud_support_view_update();
	message_training_update_frame();

	// if emp is active we have to allow targeting by the "random emp" system
	// we will intercept player targeting requests in hud_sensors_ok() when checking key commands
	// DB - 8/24/98
	can_target = hud_sensors_ok(Player_ship, 0);
	if(emp_active_local()){
		can_target = 1;
	}
	if ( !can_target && Hud_last_can_target ) {
		Hud_can_target_timer = timestamp(1200);		
	}
	Hud_last_can_target = can_target;

	if ( timestamp_elapsed(Hud_can_target_timer) ) {
		if ( (Player_ai->target_objnum != -1) && !can_target ){
			Player_ai->target_objnum = -1;
		}
	}

	// if there is no target, check if auto-targeting is enabled, and select new target
	int retarget = 0;
	int retarget_turret = 0;

	if (Player_ai->target_objnum == -1){
		retarget = 1;
	} else if (Objects[Player_ai->target_objnum].type == OBJ_SHIP) {
		if (Ships[Objects[Player_ai->target_objnum].instance].flags & SF_DYING){
			if (timestamp_elapsed(Ships[Objects[Player_ai->target_objnum].instance].final_death_time)) {
				retarget = 1;
			}
		}
	}

	// check if big ship and currently selected subsys is turret and turret is dead
	// only do this is not retargeting
	if ((!retarget) && (Player_ai->target_objnum != -1)) {
		if (Objects[Player_ai->target_objnum].type == OBJ_SHIP) {
			if ( !(Ships[Objects[Player_ai->target_objnum].instance].flags & SF_DYING) ) {
				if ( Ship_info[Ships[Objects[Player_ai->target_objnum].instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP) ) {
					ship_subsys *ss = Player_ai->targeted_subsys;
					if (ss != NULL) {
						if ((ss->system_info->type == SUBSYSTEM_TURRET) && (ss->current_hits == 0)) {
							retarget_turret = 1;
						}
					}
				}
			}
		}
	}

	if ( retarget && can_target ) {
		Player_ai->current_target_is_locked = 0;
		if ( Players[Player_num].flags & PLAYER_FLAGS_AUTO_TARGETING ) {
			Player_ai->target_objnum = -1;
			hud_target_auto_target_next();
		}
	}

	if (retarget_turret && can_target) {
		Assert(!retarget);
		// get closest weighted live turret
		// hud_target_closest(OBJ_INDEX(Player_obj), FALSE, FALSE);
		void hud_update_closest_turret();
		hud_update_closest_turret();
	}

	hud_target_change_check();

	// check to see if we are in messaging mode.  If so, send the key to the code
	// to deal with the message.  hud_sqaudmsg_do_frame will return 0 if the key
	// wasn't used in messaging mode, otherwise 1.  In the event the key was used,
	// return immediately out of this function.
	if ( Players->flags & PLAYER_FLAGS_MSG_MODE ) {
		hud_squadmsg_do_frame();
	}

	if (Player_ai->target_objnum == -1) {
		if ( Target_static_looping != -1 ) {
			snd_stop(Target_static_looping);
		}
		return;
	}

	targetp = &Objects[Player_ai->target_objnum];

	if ( Player_ai->targeted_subsys != NULL ) {
		get_subsystem_world_pos(targetp, Player_ai->targeted_subsys, &target_pos);

		// get new distance of current target
		Player_ai->current_target_distance = vm_vec_dist_quick(&target_pos,&Player_obj->pos);

		float shield_strength = Player_ai->targeted_subsys->current_hits/Player_ai->targeted_subsys->max_hits * 100.0f;
		int screen_integrity = fl2i(shield_strength+0.5f);

		if ( screen_integrity < 0 ) {
			screen_integrity = 0;
		}

		if ( screen_integrity == 0 ) {
			if ( shield_strength > 0 ) {
				screen_integrity = 1;
			}
		}

		// Goober5000 - don't flash if this subsystem can't be destroyed
		if ( ship_subsys_takes_damage(Player_ai->targeted_subsys) )
		{
			if ( screen_integrity <= 0 ){
				hud_targetbox_start_flash(TBOX_FLASH_SUBSYS);	// need to flash 0% continuously
			}
		}
	} else {
		target_pos = targetp->pos;

		Player_ai->current_target_distance = hud_find_target_distance(targetp,Player_obj);
	}

	int stop_targetting_this_thing = 0;

	// check to see if the target is still alive
	if ( targetp->flags&OF_SHOULD_BE_DEAD ) {
		stop_targetting_this_thing = 1;
	}

	Player->target_is_dying = FALSE;
	ship	*target_shipp = NULL;
	
	if ( targetp->type == OBJ_SHIP ) {
		Assert ( targetp->instance >=0 && targetp->instance < MAX_SHIPS );
		target_shipp = &Ships[targetp->instance];
		Player->target_is_dying = target_shipp->flags & SF_DYING;

		// If it is warping out (or exploded), turn off targeting
		if ( target_shipp->flags & (SF_DEPART_WARP|SF_EXPLODED) ) {
			stop_targetting_this_thing = 1;
		}
	}

	// Check if can still be seen in Nebula
	if ( hud_target_invalid_awacs(targetp) ) {
		stop_targetting_this_thing = 1;
	}

	// If this was found to be something we shouldn't
	// target anymore, just remove it
	if ( stop_targetting_this_thing )	{
		Player_ai->target_objnum = -1;
		Player_ai->targeted_subsys = NULL;
		hud_stop_looped_locking_sounds();
	}
	
	if (Player->target_is_dying) {
		hud_stop_looped_locking_sounds();
		if ( Players[Player_num].flags & PLAYER_FLAGS_AUTO_TARGETING ) {
			hud_target_auto_target_next();
		}
	}

	// Switch to battle track when a targeted ship is hostile (it attacks you) and within BATTLE_START_MIN_TARGET_DIST
	if (targetp->type == OBJ_SHIP && Event_Music_battle_started == 0 ) {
		Assert( target_shipp != NULL );

		// Goober5000
		if (iff_x_attacks_y(target_shipp->team, Player_ship->team))
		{
			float	dist_to_target;

			dist_to_target = vm_vec_dist_quick(&targetp->pos, &Player_obj->pos);
			if (dist_to_target < BATTLE_START_MIN_TARGET_DIST) {

				// If the target has an AI class of none, it is a Cargo, NavBuoy or other non-aggressive
				// ship, so don't start the battle music	
				if (stricmp(Ai_class_names[Ai_info[target_shipp->ai_index].ai_class], NOX("none")))
					event_music_battle_start();
			}
		}
	}

	//make sure that the player isn't targeting a 3rd stage local ssm
	if (Objects[Player_ai->target_objnum].type == OBJ_WEAPON)
	{
		if (Weapons[Objects[Player_ai->target_objnum].instance].lssm_stage==3)
		{
			set_target_objnum( Player_ai, -1 );
			hud_lock_reset();
		}
	}

	// Since we need to reference the player's target integrity in several places this upcoming 
	// frame, only calculate once here
	if ( target_shipp ) {
		Pl_target_integrity = get_hull_pct(targetp);
	}

	// update cargo scanning
	hud_update_cargo_scan_sound();

	if ( Viewer_mode & ( VM_EXTERNAL | VM_WARP_CHASE ) ) {
		Player->cargo_inspect_time = 0;
		player_stop_cargo_scan_sound();
	}

	hud_update_target_static();
	hud_update_ship_status(targetp);
}

// Draw white brackets around asteroids which has the AF_DRAW_BRACKETS flag set
void hud_show_asteroid_brackets()
{
	if ( hud_sensors_ok(Player_ship, 0) ) {
		asteroid_show_brackets();
	}
}

// Render gauges that need to be between a g3_start_frame() and a g3_end_frame()
void hud_render_preprocess(float frametime)
{
	Player->subsys_in_view = -1;
	hud_target_clear_display_list();

	if ( (Game_detail_flags & DETAIL_FLAG_HUD) && (supernova_active() >= 3) ) {
		return;
	}

	if ( hud_disabled() ) {
		return;
	}

	if ( Viewer_mode & ( VM_DEAD_VIEW ) ) {
		return;
	}

	if ( Viewer_mode & (VM_EXTERNAL | VM_WARP_CHASE | VM_PADLOCK_ANY ) ) {
		// If the player is warping out, don't draw the targeting gauges
		Assert(Player != NULL);
		if ( Player->control_mode != PCM_NORMAL ) {
			return;
		}
	}
	
	// process targeting data around any message sender
	hud_show_message_sender();

	// if messages are disabled then skip everything else
	if ( hud_disabled_except_messages() ) {
		return;
	}

	// process navigation stuff
	hud_draw_navigation();

	// process current selection set, if any
	hud_show_selection_set();

	// process asteroid brackets if necessary
	hud_show_asteroid_brackets();

	// process targetting data around the current target
	hud_show_targeting_gauges(frametime);

	// process brackets and distance to remote detonate missile
	hud_process_remote_detonate_missile();

	// process any incoming missiles
	hud_process_homing_missiles();
}

HudGaugeMissionTime::HudGaugeMissionTime():
HudGauge(HUD_OBJECT_MISSION_TIME, HUD_MISSION_TIME, true, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeMissionTime::initTextOffsets(int x, int y)
{
	time_text_offsets[0] = x;
	time_text_offsets[1] = y;
}

void HudGaugeMissionTime::initValueOffsets(int x, int y)
{
	time_val_offsets[0] = x;
	time_val_offsets[1] = y;
}

void HudGaugeMissionTime::initBitmaps(char *fname)
{
	time_gauge.first_frame = bm_load_animation(fname, &time_gauge.num_frames);

	if ( time_gauge.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in ani: %s\n", fname);
	}
}

void HudGaugeMissionTime::pageIn()
{
	bm_page_in_aabitmap(time_gauge.first_frame, time_gauge.num_frames );
}

void HudGaugeMissionTime::render(float frametime)
{
	float mission_time, time_comp;
	int minutes=0;
	int seconds=0;
	
	mission_time = f2fl(Missiontime);  // convert to seconds

	minutes=(int)(mission_time/60);
	seconds=(int)mission_time%60;

	setGaugeColor();

	// blit background frame
	if ( time_gauge.first_frame >= 0 ) {
		renderBitmap(time_gauge.first_frame, position[0], position[1]);				
	}

	// print out mission time in MM:SS format
	renderPrintf(position[0] + time_text_offsets[0], position[1] + time_text_offsets[1], NOX("%02d:%02d"), minutes, seconds);

	// display time compression as xN
	time_comp = f2fl(Game_time_compression);
	if ( time_comp < 1 ) {
		renderPrintf(position[0] + time_val_offsets[0], position[1] + time_val_offsets[1], /*XSTR( "x%.1f", 215), time_comp)*/ NOX("%.2f"), time_comp);
	} else {
		renderPrintf(position[0] + time_val_offsets[0], position[1] + time_val_offsets[1], XSTR( "x%.2f", 216), time_comp);
	}
}

void hud_maybe_display_supernova()
{
	float time_left;

	// if there's a supernova coming
	time_left = supernova_time_left();
	if(time_left < 0.0f){
		return;
	}

	gr_set_color_fast(&Color_bright_red);
	gr_printf(Supernova_coords[gr_screen.res][0], Supernova_coords[gr_screen.res][1], "Supernova Warning: %.2f s", time_left);
}

void hud_render_all()
{
	if(supernova_active() >= 3) {
		return;
	}

	int num_gauges;

	ship_info* sip = &Ship_info[Player_ship->ship_info_index];

	ship_render_backgrounds_cockpit_display(Player_ship);

	// check if this ship has its own hud gauges. 
	if(sip->hud_enabled) {
		num_gauges = (int)sip->hud_gauges.size();
		for(int i = 0; i < num_gauges; i++) {
			if(sip->hud_gauges[i]->canRender()) {
				sip->hud_gauges[i]->resetClip();
				sip->hud_gauges[i]->setFont();
				if ( sip->hud_gauges[i]->setupRenderCanvas() ) {
					sip->hud_gauges[i]->render(flFrametime);
					sip->hud_gauges[i]->doneRenderCanvas();
					sip->hud_gauges[i]->renderToCockpit();
				}
			}
		}
	} else {
		num_gauges = (int)default_hud_gauges.size();
		for(int i = 0; i < num_gauges; i++) {
			if(default_hud_gauges[i]->canRender()) {
				default_hud_gauges[i]->resetClip();
				default_hud_gauges[i]->setFont();
				default_hud_gauges[i]->render(flFrametime);
			}
		}
	}

	hud_clear_msg_buffer();
	ship_render_foregrounds_cockpit_display(Player_ship);

	// set font back the way it was
	gr_set_font(FONT1);
}

// hud_stop_looped_engine_sounds()
//
// This function will set the loop id's for the engine noises to -1, this will force any
// looping engine sounds to stop.  This should only be called when the game decides to
// stop all looping sounds
//

void hud_stop_looped_engine_sounds()
{
	if ( Player_engine_snd_loop > -1 )	{
		snd_stop(Player_engine_snd_loop);
		//snd_chg_loop_status(Player_engine_snd_loop, 0);
		Player_engine_snd_loop = -1;
	}
}

#define ZERO_PERCENT			0.01f
#define ENGINE_MAX_VOL		1.0f
#define ENGINE_MAX_PITCH	44100

void update_throttle_sound()
{
	// determine what engine sound to play
	float percent_throttle;
//	int	throttle_pitch;

	// if we're a multiplayer observer, stop any engine sounds from playing and return
	if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER)){
		// stop engine sound if it is playing
		if(Player_engine_snd_loop != -1){
			snd_stop(Player_engine_snd_loop);
			Player_engine_snd_loop = -1;
		}

		// return
		return;
	}

	if ( timestamp_elapsed(throttle_sound_check_id) ) {

		throttle_sound_check_id = timestamp(THROTTLE_SOUND_CHECK_INTERVAL);
	
		if ( object_get_gliding(Player_obj) ) {	// Backslash
			percent_throttle = Player_obj->phys_info.forward_thrust;
		} else if ( Ships[Player_obj->instance].current_max_speed == 0 ) {
			percent_throttle = Player_obj->phys_info.fspeed / Ship_info[Ships[Player_obj->instance].ship_info_index].max_speed;
		} else {
			percent_throttle = Player_obj->phys_info.fspeed / Ships[Player_obj->instance].current_max_speed;
		}

		// If the throttle has changed, modify the sound
		if ( percent_throttle != last_percent_throttle || Player_engine_snd_loop == -1 ) {

			if ( percent_throttle < ZERO_PERCENT ) {
				if ( Player_engine_snd_loop > -1 )	{
					snd_chg_loop_status(Player_engine_snd_loop, 0);
					snd_stop(Player_engine_snd_loop); // Backslash - otherwise, long engine loops keep playing
					Player_engine_snd_loop = -1;
				}
			}
			else {
				if ( Player_engine_snd_loop == -1 ){
					Player_engine_snd_loop = snd_play_looping( &Snds[SND_ENGINE], 0.0f , -1, -1, percent_throttle * ENGINE_MAX_VOL );
				} else {
					// The sound may have been trashed at the low-level if sound channel overflow.
					// TODO: implement system where certain sounds cannot be interrupted (priority?)
					if ( snd_is_playing(Player_engine_snd_loop) ) {
						snd_set_volume(Player_engine_snd_loop, percent_throttle * ENGINE_MAX_VOL);
					}
					else {
						Player_engine_snd_loop = -1;
					}
				}
			}

//			throttle_pitch = snd_get_pitch(Player_engine_snd_loop);
//			if ( percent_throttle > 0.5 ) {
//				snd_set_pitch(Player_engine_snd_loop, fl2i(22050 + (percent_throttle-0.5f)*1000));
//			}

		}	// end if (percent_throttle != last_percent_throttle)

		last_percent_throttle = percent_throttle;

	}	// end if ( timestamp_elapsed(throttle_sound_check_id) )
}

// called at the beginning of each level.  Loads frame data in once, and initializes any damage
// gauge specific data
void hud_damage_popup_init()
{
	int i;

	Damage_flash_bright = 0;
	Damage_flash_timer =	1;

	for ( i = 0; i < SUBSYSTEM_MAX; i++ ) {
		Pl_hud_subsys_info[i].last_str = 1000.0f;
		Pl_hud_subsys_info[i].flash_duration_timestamp = 1;
		Pl_hud_next_flash_timestamp = 1;
		Pl_hud_is_bright = 0;
	}
}

HudGaugeDamage::HudGaugeDamage():
HudGauge(HUD_OBJECT_DAMAGE, HUD_DAMAGE_GAUGE, true, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeDamage::initHeaderOffsets(int x, int y)
{
	header_offsets[0] = x;
	header_offsets[1] = y;
}

void HudGaugeDamage::initHullIntegOffsets(int x, int y)
{
	hull_integ_offsets[0] = x;
	hull_integ_offsets[1] = y;
}

void HudGaugeDamage::initHullIntegValueOffsetX(int x)
{
	hull_integ_val_offset_x = x;
}

void HudGaugeDamage::initMiddleFrameStartOffsetY(int y)
{
	middle_frame_start_offset_y = y;
}

void HudGaugeDamage::initSubsysIntegStartOffsets(int x, int y)
{
	subsys_integ_start_offsets[0] = x;
	subsys_integ_start_offsets[1] = y;
}

void HudGaugeDamage::initSubsysIntegValueOffsetX(int x)
{
	subsys_integ_val_offset_x = x;
}

void HudGaugeDamage::initLineHeight(int h)
{
	line_h = h;
}

void HudGaugeDamage::initBitmaps(char *fname_top, char *fname_middle, char *fname_bottom)
{
	damage_top.first_frame = bm_load_animation(fname_top, &damage_top.num_frames);
	if ( damage_top.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in the ani: %s\n", fname_top);
	}

	damage_middle.first_frame = bm_load_animation(fname_middle, &damage_middle.num_frames);
	if ( damage_middle.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in the ani: %s\n", fname_middle);
	}

	damage_bottom.first_frame = bm_load_animation(fname_bottom, &damage_bottom.num_frames);
	if ( damage_bottom.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in the ani: %s\n", fname_bottom);
	}
}

void HudGaugeDamage::initialize()
{
	Damage_flash_bright = false;
	Damage_flash_timer = timestamp(1);
	next_flash = timestamp(1);
	flash_status = false;
}

void HudGaugeDamage::pageIn()
{
	bm_page_in_aabitmap(damage_top.first_frame, damage_top.num_frames);
	bm_page_in_aabitmap(damage_middle.first_frame, damage_middle.num_frames);
	bm_page_in_aabitmap(damage_bottom.first_frame, damage_bottom.num_frames);
}

void HudGaugeDamage::render(float frametime)
{
	model_subsystem	*psub;
	ship_subsys			*pss;
	ship_info			*sip;
	int					sx, sy, bx, by, w, h, screen_integrity, num, best_str, best_index;
	float					strength, shield, integrity;
	char					buf[128];
	hud_subsys_damage	hud_subsys_list[SUBSYSTEM_MAX];	

	if ( (Player_ship->ship_max_hull_strength - Player_obj->hull_strength) <= 1.0f ) {
		return;
	}

	if ( damage_top.first_frame == -1 ) {
		return;
	}

	if ( (The_mission.game_type & MISSION_TYPE_TRAINING) && Training_message_visible ){
		return;
	}
		
	sip = &Ship_info[Player_ship->ship_info_index];
	hud_get_target_strength(Player_obj, &shield, &integrity);
	screen_integrity = fl2i(integrity*100);

	if ( hud_gauge_is_popup(gauge_config) ) {
		if ( screen_integrity >= 100 ) {
			return;
		}
	}

	if ( timestamp_elapsed(Damage_flash_timer) ) {
		Damage_flash_timer = timestamp(DAMAGE_FLASH_TIME);
		Damage_flash_bright = !Damage_flash_bright;
	}

	setGaugeColor();

	// draw the top of the damage pop-up
	renderBitmap(damage_top.first_frame, position[0], position[1]);	
	renderString(position[0] + header_offsets[0], position[1] + header_offsets[1], XSTR( "damage", 218));

	// show hull integrity
	if ( screen_integrity < 100 ) {		
		if ( screen_integrity == 0 ) {
			screen_integrity = 1;
		}
		sprintf(buf, XSTR( "%d%%", 219), screen_integrity);
		hud_num_make_mono(buf);
		gr_get_string_size(&w, &h, buf);
		if ( screen_integrity < 30 ) {
			gr_set_color_fast(&Color_red);
		}
		renderString(position[0] + hull_integ_offsets[0], position[1] + hull_integ_offsets[1], XSTR( "Hull Integrity", 220));
		renderString(position[0] + hull_integ_val_offset_x - w, position[1] + hull_integ_offsets[1], buf);
	} 

	// show damaged subsystems
	sx = position[0] + subsys_integ_start_offsets[0];
	sy = position[1] + subsys_integ_start_offsets[1];
	bx = position[0];
	by = position[1] + middle_frame_start_offset_y;

	num = 0;
	for ( pss = GET_FIRST(&Player_ship->subsys_list); pss !=END_OF_LIST(&Player_ship->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;
		strength = ship_get_subsystem_strength(Player_ship, psub->type);
		if ( strength < 1 ) {
			screen_integrity = fl2i(strength*100);
			if ( screen_integrity == 0 ) {
				if ( strength > 0 ) {
					screen_integrity = 1;
				}
			}

			if (strlen(psub->alt_dmg_sub_name))
				hud_subsys_list[num].name = psub->alt_dmg_sub_name;
			else {
				hud_subsys_list[num].name = ship_subsys_get_name(pss);
			}

			hud_subsys_list[num].str  = screen_integrity;
			hud_subsys_list[num].type = psub->type;
			num++;

			if ( strength < Pl_hud_subsys_info[psub->type].last_str ) {
				Pl_hud_subsys_info[psub->type].flash_duration_timestamp = timestamp(SUBSYS_DAMAGE_FLASH_DURATION);
			}
			Pl_hud_subsys_info[psub->type].last_str = strength;

			//Don't display more than the max number of damaged subsystems.
			if (num >= SUBSYSTEM_MAX)
			{
				break;
			}
		}
	}

	int type;
	for ( int i = 0; i < num; i++ ) {
		best_str = 1000;
		best_index = -1;
		for ( int j = 0; j < num-i; j++ ) {
			if ( hud_subsys_list[j].str < best_str ) {
				best_str = hud_subsys_list[j].str;
				best_index = j;
			}
		}

		Assert(best_index >= 0);
		Assert(best_str >= 0);

		// display strongest subsystem left in list
		// draw the bitmap
		// hud_set_default_color();
		setGaugeColor();

		renderBitmap(damage_middle.first_frame, bx, by);
		by += line_h;

		type = hud_subsys_list[best_index].type;
		if ( !timestamp_elapsed( Pl_hud_subsys_info[type].flash_duration_timestamp ) ) {
			if ( timestamp_elapsed(next_flash) ) {
				flash_status = !flash_status;
				next_flash = timestamp(SUBSYS_DAMAGE_FLASH_INTERVAL);
			}
			
			if ( flash_status ) {
				int alpha_color;
				alpha_color = MIN(HUD_COLOR_ALPHA_MAX,HUD_color_alpha+HUD_BRIGHT_DELTA);
				// gr_set_color_fast(&HUD_color_defaults[alpha_color]);

				setGaugeColor(alpha_color);
			} else {				
				setGaugeColor();
			}
		}

		// draw the text
		if ( best_str < 30 ) {
			if ( best_str <= 0 ) {
				if ( Damage_flash_bright ) {
					gr_set_color_fast(&Color_bright_red);
				} else {
					gr_set_color_fast(&Color_red);
				}

			} else {
				gr_set_color_fast(&Color_red);
			}
		} else {
			setGaugeColor();
		}		

		char *n_firstline;
		n_firstline = strrchr(hud_subsys_list[best_index].name, '|');
		if (n_firstline) {
			// print only the last line
			n_firstline++;
			renderString(sx, sy, n_firstline);
		} else {
			char temp_name[NAME_LENGTH];
			strcpy_s(temp_name, hud_subsys_list[best_index].name);
			hud_targetbox_truncate_subsys_name(temp_name);
			renderString(sx, sy, temp_name);
		}

		sprintf(buf, XSTR( "%d%%", 219), best_str);
		hud_num_make_mono(buf);
		gr_get_string_size(&w, &h, buf);
		renderString(position[0] + subsys_integ_val_offset_x - w, sy, buf);
		sy += line_h;

		// remove it from hud_subsys_list
		if ( best_index < (num-i-1) ) {
			hud_subsys_list[best_index] = hud_subsys_list[num-i-1];
		}
	}

	// draw the bottom of the gauge
	// hud_set_default_color();
	setGaugeColor();

	renderBitmap(damage_bottom.first_frame, bx, by);		
}

// init the members of the hud_anim struct to default values
void hud_anim_init(hud_anim *ha, int sx, int sy, char *filename)
{
	ha->first_frame		= -1;
	ha->num_frames		= 0;
	ha->total_time		= 0.0f;
	ha->time_elapsed	= 0.0f;
	ha->sx				= sx;
	ha->sy				= sy;
	strcpy_s(ha->filename, filename);
}

// init the members of the hud_frames struct to default values
void hud_frames_init(hud_frames *hf)
{
	hf->first_frame		= -1;
	hf->num_frames		= 0;
}

// load a hud_anim
// return 0 is successful, otherwise return -1
int hud_anim_load(hud_anim *ha)
{
	int		fps;

	ha->first_frame = bm_load_animation(ha->filename, &ha->num_frames, &fps);

	// Goober5000 - try to bypass the Volition bug
	if ( (ha->first_frame == -1) && !stricmp(ha->filename, "FadeIconS-FreighterCW") )
	{
		ha->first_frame = bm_load_animation("FadeIconS-FreighterWC", &ha->num_frames, &fps);
	}

	if ( ha->first_frame == -1 )
	{
		Warning(LOCATION, "Couldn't load hud animation for file '%s'", ha->filename);
		return -1;
	}

	Assert(fps != 0);
	ha->total_time = i2fl(ha->num_frames)/fps;
	return 0;
}

// render out a frame of the targetbox static animation, based on how much time has
// elapsed
// input:	ha				=>	pointer to hud anim info
//				frametime	=>	seconds elapsed since last frame
//				draw_alpha	=>	draw bitmap as alpha-bitmap (default 0)
//				loop			=>	anim should loop (default 1)
//				hold_last	=>	should last frame be held (default 0)
//				reverse		=>	play animation in reverse (default 0)
//				resize		=>  resize for non-standard resolutions
//				mirror		=>	mirror along y-axis so icon points left instead of right
int hud_anim_render(hud_anim *ha, float frametime, int draw_alpha, int loop, int hold_last, int reverse,bool resize,bool mirror)
{
	int framenum;

	if ( ha->num_frames <= 0 ) {
		if ( hud_anim_load(ha) == -1 )
			return 0;
	}

	ha->time_elapsed += frametime;
	if ( ha->time_elapsed > ha->total_time ) {
		if ( loop ) {
			ha->time_elapsed = 0.0f;
		} else {
			if ( !hold_last ) {
				return 0;
			}
		}
	}

	// draw the correct frame of animation
	framenum = fl2i( (ha->time_elapsed * ha->num_frames) / ha->total_time );
	if (reverse) {
		framenum = (ha->num_frames-1) - framenum;
	}

	if ( framenum < 0 )
		framenum = 0;
	if ( framenum >= ha->num_frames )
		framenum = ha->num_frames-1;

	// Blit the bitmap for this frame
	if(emp_should_blit_gauge()){
		gr_set_bitmap(ha->first_frame + framenum);
		if ( draw_alpha ){
			gr_aabitmap(ha->sx, ha->sy,resize,mirror);
		} else {
			gr_bitmap(ha->sx, ha->sy,resize);
		}
	}

	return 1;
}

// convert a number string to use mono-spaced 1 character
void hud_num_make_mono(char *num_str)
{
	int len, i, sc;
	len = strlen(num_str);

	sc = Lcl_special_chars;
	for ( i = 0; i < len; i++ ) {
		if ( num_str[i] == '1' ) {
			num_str[i] = (char)(sc + 1);
		}
	}
}

// flashing text gauge
void hud_init_text_flash_gauge()
{
	strcpy_s(Hud_text_flash, "");
	Hud_text_flash_timer = timestamp(0);
	Hud_text_flash_interval = 0;
}

void hud_start_text_flash(char *txt, int t, int interval)
{
	// bogus
	if(txt == NULL){
		strcpy_s(Hud_text_flash, "");
		return;
	}

	// HACK. don't override EMP if its still going    :)
	// An additional hack: don't interrupt other warnings if this is a missile launch alert (Swifty)
	if(!timestamp_elapsed(Hud_text_flash_timer))
		if( !stricmp(Hud_text_flash, NOX("Emp")) || !stricmp(txt, NOX("Launch")) )
			return;

	strncpy(Hud_text_flash, txt, 500);
	Hud_text_flash_timer = timestamp(t);
	Hud_text_flash_interval = interval;
}

HudGaugeTextWarnings::HudGaugeTextWarnings():
HudGauge(HUD_OBJECT_TEXT_WARNINGS, HUD_TEXT_FLASH, true, true, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
	
}

void HudGaugeTextWarnings::initialize()
{
	next_flash = timestamp(0);
	flash_flags = false;
}

int HudGaugeTextWarnings::maybeTextFlash()
{
	if ( !timestamp_elapsed(Hud_text_flash_timer) ) {
		if ( timestamp_elapsed(next_flash) ) {
			next_flash = timestamp(Hud_text_flash_interval);

			// toggle between default and bright frames
			flash_flags = !flash_flags;
		}
	}

	return flash_flags;
}

void HudGaugeTextWarnings::render(float frametime)
{
	if ( timestamp_elapsed(Hud_text_flash_timer) || !Hud_text_flash) {
		return;
	}

	if(strlen(Hud_text_flash) <= 0) {
		return;
	}

	int w, h;

	// string size
	gr_get_string_size(&w, &h, Hud_text_flash);

	// set color
	if(maybeTextFlash()){
		setGaugeColor(HUD_C_DIM);
		
		// draw the box	
		renderRect( (int)( (float)position[0] - (float)w / 2.0f - 1.0f), (int)((float)position[1] - 1.0f), w + 2, h + 1);
	}

	// string
	setGaugeColor(HUD_C_BRIGHT);
	renderString(fl2i((float)position[0] - ((float)w / 2.0f)), position[1], Hud_text_flash);
}

HudGaugeKills::HudGaugeKills():
HudGauge(HUD_OBJECT_KILLS, HUD_KILLS_GAUGE, true, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeKills::initBitmaps(char *fname)
{
	Kills_gauge.first_frame = bm_load_animation(fname, &Kills_gauge.num_frames);
	if ( Kills_gauge.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in the ani: %s\n", fname);
	}
}

void HudGaugeKills::initTextOffsets(int x, int y)
{
	text_offsets[0] = x;
	text_offsets[1] = y;
}

void HudGaugeKills::initTextValueOffsets(int x, int y)
{
	text_value_offsets[0] = x;
	text_value_offsets[1] = y;
}

void HudGaugeKills::pageIn()
{
	bm_page_in_aabitmap(Kills_gauge.first_frame, Kills_gauge.num_frames);
}

// maybe display the kills gauge on the HUD
void HudGaugeKills::render(float frametime)
{
	if ( Kills_gauge.first_frame < 0 ) {
		return;
	}

	// hud_set_default_color();
	setGaugeColor();

	// draw background
	renderBitmap(Kills_gauge.first_frame, position[0], position[1]);	

	renderString(position[0] + text_offsets[0], position[1] + text_offsets[1], XSTR( "kills:", 223));

	// display how many kills the player has so far
	char	num_kills_string[32];
	int	w,h;

	if ( !Player ) {
		Int3();
		return;
	}

	sprintf(num_kills_string, "%d", Player->stats.m_kill_count_ok);

	gr_get_string_size(&w, &h, num_kills_string);
	renderString(position[0]+text_value_offsets[0]-w, position[1]+text_value_offsets[1], num_kills_string);
}

HudGaugeLag::HudGaugeLag():
HudGauge(HUD_OBJECT_LAG, HUD_LAG_GAUGE, true, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{

}

void HudGaugeLag::initBitmaps(char *fname)
{
	Netlag_icon.first_frame = bm_load_animation(fname, &Netlag_icon.num_frames);

	if ( Netlag_icon.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in the netlag ani: %s\n", fname);
	}
}

void HudGaugeLag::pageIn()
{
	bm_page_in_aabitmap(Netlag_icon.first_frame, Netlag_icon.num_frames);
}

void HudGaugeLag::startFlashLag(int duration)
{
	flash_timer[0] = timestamp(duration);
}

bool HudGaugeLag::maybeFlashLag(bool flash_fast)
{
	bool draw_bright = false;

	if(!timestamp_elapsed(flash_timer[0])) {
		if(timestamp_elapsed(flash_timer[1])) {
			if(flash_fast) {
				flash_timer[1] = timestamp(fl2i(TBOX_FLASH_INTERVAL/2.0f));
			} else {
				flash_timer[1] = timestamp(TBOX_FLASH_INTERVAL);
			}
			flash_flag = !flash_flag;
		}

		if(flash_flag) {
			draw_bright = true;
		} 
	}

	return draw_bright;
}

void HudGaugeLag::render(float frametime)
{
	int lag_status;

	if( !(Game_mode & GM_MULTIPLAYER) ){
		return;
	}

	if ( Netlag_icon.first_frame == -1 ) {
		Int3();
		return;
	}

	lag_status = multi_query_lag_status();

	switch(lag_status) {
	case 0:
		// draw the net lag icon flashing
		startFlashLag();
		if(maybeFlashLag()){
			setGaugeColor(HUD_C_BRIGHT);
		} else {
			setGaugeColor();
		}
		renderBitmap(Netlag_icon.first_frame, position[0], position[1]);
		break;
	case 1:
		// draw the disconnected icon flashing fast
		if(maybeFlashLag(true)){
			setGaugeColor(HUD_C_BRIGHT);
		} else {
			setGaugeColor();
		}
		renderBitmap(Netlag_icon.first_frame+1, position[0], position[1]);
		break;
	default:
		// nothing to draw
		return;
	}
}

// load in kills gauge if required
void hud_init_kills_gauge()
{
	if ( !Kills_gauge_loaded ) {
		Kills_gauge.first_frame = bm_load_animation(Kills_fname[gr_screen.res], &Kills_gauge.num_frames);
		if ( Kills_gauge.first_frame == -1 ) {
			Warning(LOCATION, "Could not load in the kills ani: Kills_fname[gr_screen.res]\n");
			return;
		}
		Kills_gauge_loaded = 1;
	}
}

// called at mission start to init data, and load support view bitmap if required
void hud_support_view_init()
{
	Hud_support_view_fade = 1;
	Hud_support_obj_sig = -1;
	Hud_support_target_sig = -1;
	Hud_support_objnum = -1;
	Hud_support_view_active = 0;
	Hud_support_view_abort = 0;
}

// start displaying the support view pop-up.  This will remain up until hud_support_view_stop is called.
// input:	objnum	=>		object number for the support ship
void hud_support_view_start()
{
	Hud_support_view_active = 1;
	Hud_support_view_fade = 1;
}

// stop displaying the support view pop-up
void hud_support_view_stop(int stop_now)
{
	if ( stop_now ) {
		Hud_support_view_active = 0;
		Hud_support_view_fade = 1;
		Hud_support_view_abort = 0;
	} else {
		Hud_support_view_fade = timestamp(2000);
	}

	Hud_support_obj_sig = -1;
	Hud_support_target_sig = -1;
	Hud_support_objnum = -1;
}

void hud_support_view_abort()
{
	hud_support_view_stop(0);
	Hud_support_view_abort = 1;
}

// return the number of seconds until repair ship will dock with player, return -1 if error
// 
// mwa made this function more general purpose
// Goober5000 made clearer
//
// NOTE: This function is pretty stupid now.  It just assumes the dockee is sitting still, and
//		   the docker is moving directly to the dockee.
int hud_get_dock_time( object *docker_objp )
{
	ai_info	*aip;
	object	*dockee_objp;
	float		dist, rel_speed, docker_speed;
	vec3d	rel_vel;

	aip = &Ai_info[Ships[docker_objp->instance].ai_index];

	// get the dockee object pointer
	if (aip->goal_objnum == -1) {
		// this can happen when you target a support ship as it warps in
		// just give a debug warning instead of a fault - taylor
	//	Int3();	//	Shouldn't happen, but let's recover gracefully.
		mprintf(("'aip->goal_objnum == -1' in hud_get_dock_time(), line %i\n", __LINE__));
		return 0;
	}

	dockee_objp = &Objects[aip->goal_objnum];

	// if the ship is docked, return 0
	if ( dock_check_find_direct_docked_object(docker_objp, dockee_objp) )
		return 0;

	vm_vec_sub(&rel_vel, &docker_objp->phys_info.vel, &dockee_objp->phys_info.vel);
	rel_speed = vm_vec_mag_quick(&rel_vel);

	dist = vm_vec_dist_quick(&dockee_objp->pos, &docker_objp->pos);

	docker_speed = docker_objp->phys_info.speed;

	if ( rel_speed <= docker_speed/2.0f) {	//	This means the player is moving away fast from the support ship.
		return (int) (dist/docker_speed);
	} else {
		float	d1;
		float	d = dist;
		float	time = 0.0f;
		
		if (rel_speed < 20.0f)
			rel_speed = 20.0f;

		//	When faraway, use max speed, not current speed.  Might not have sped up yet.
		if (d > 100.0f) {
			time += (d - 100.0f)/docker_objp->phys_info.max_vel.xyz.z;
		}

		//	For mid-range, use current speed.
		if (d > 60.0f) {
			d1 = MIN(d, 100.0f);

			time += (d1 - 60.0f)/rel_speed;
		}

		//	For nearby, ship will have to slow down a bit for docking maneuver.
		if (d > 30.0f) {
			d1 = MIN(d, 60.0f);

			time += (d1 - 30.0f)/5.0f;
		}

		//	For very nearby, ship moves quite slowly.
		d1 = MIN(d, 30.0f);
		time += d1/7.5f;

		return fl2i(time);
	}
}

// Locate the closest support ship which is trying to dock with player, return -1 if there is no support
// ship currently trying to dock with the player
// MA:  4/22/98 -- pass in objp to find support ship trying to dock with objp
int hud_support_find_closest( int objnum )
{
	ship_obj		*sop;
	ai_info		*aip;
	object		*objp;
	int i;

	objp = &Objects[objnum];

	sop = GET_FIRST(&Ship_obj_list);
	while(sop != END_OF_LIST(&Ship_obj_list)){
		if ( Ship_info[Ships[Objects[sop->objnum].instance].ship_info_index].flags & SIF_SUPPORT ) {
			int pship_index, sindex;

			// make sure support ship is not dying
			if ( !(Ships[Objects[sop->objnum].instance].flags & (SF_DYING|SF_EXPLODED)) ) {

				Assert( objp->type == OBJ_SHIP );
				aip = &Ai_info[Ships[Objects[sop->objnum].instance].ai_index];
				pship_index = objp->instance;

				// we must check all goals for this support ship -- not just the first one
				for ( i = 0; i < MAX_AI_GOALS; i++ ) {

					// we can use == in the next statement (and should) since a ship will only ever be
					// following one order at a time.
					if ( aip->goals[i].ai_mode == AI_GOAL_REARM_REPAIR ) {
						Assert( aip->goals[i].ship_name );
						sindex = ship_name_lookup( aip->goals[i].ship_name );
						if ( sindex == pship_index )
							return sop->objnum;
					}
				}
			}
		}
		sop = GET_NEXT(sop);
	}

	return -1;
}

void hud_support_view_update()
{
	if ( !Hud_support_view_active ) {
		return;
	}

	if((Game_mode & GM_MULTIPLAYER) && ((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type == OBJ_OBSERVER))){
		return;
	}

	// If we haven't determined yet who the rearm ship is, try to!
	if (Hud_support_objnum == -1) {
		Hud_support_objnum = hud_support_find_closest( OBJ_INDEX(Player_obj) );
		if ( Hud_support_objnum >= 0 ) {
			Hud_support_obj_sig = Objects[Hud_support_objnum].signature;
			Hud_support_target_sig = Player_obj->signature;
		}
	} else {
		// check to see if support ship is still alive
		if ( (Objects[Hud_support_objnum].signature != Hud_support_obj_sig) || (Hud_support_target_sig != Player_obj->signature) ) {
			hud_support_view_stop(1);
			return;
		}
	}

	if ( Hud_support_view_fade > 1 ) {
		if ( timestamp_elapsed(Hud_support_view_fade) ) {

			Hud_support_view_abort = 0;
			Hud_support_view_active = 0;
			Hud_support_view_fade = 1;
			Hud_support_objnum = -1;
		}
	}
}

HudGaugeSupport::HudGaugeSupport():
HudGauge(HUD_OBJECT_SUPPORT, HUD_SUPPORT_GAUGE, true, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeSupport::initHeaderOffsets(int x, int y)
{
	Header_offsets[0] = x;
	Header_offsets[1] = y;
}

void HudGaugeSupport::initTextValueOffsetY(int y)
{
	text_val_offset_y = y;
}

void HudGaugeSupport::initTextDockOffsetX(int x)
{
	text_dock_offset_x = x;
}

void HudGaugeSupport::initTextDockValueOffsetX(int x)
{
	text_dock_val_offset_x = x;
}

void HudGaugeSupport::initBitmaps(char *fname)
{
	background.first_frame = bm_load_animation(fname, &background.num_frames);
	if ( background.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in ani: %s\n", fname);
	}
}

void HudGaugeSupport::pageIn()
{
	bm_page_in_aabitmap( background.first_frame, background.num_frames);
}

void HudGaugeSupport::render(float frametime)
{
	int	show_time, w, h;
	char	outstr[64];

	if ( !Hud_support_view_active ) {
		return;
	}

	// don't render this gauge for multiplayer observers
	if((Game_mode & GM_MULTIPLAYER) && ((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type == OBJ_OBSERVER))){
		return;
	}

	if ( Hud_support_objnum >= 0 ) {
		// check to see if support ship is still alive
		if ( (Objects[Hud_support_objnum].signature != Hud_support_obj_sig) || (Hud_support_target_sig != Player_obj->signature) ) {
			return;
		}
	}

	bm_get_info(background.first_frame, &w, &h);

	// set hud color
	setGaugeColor();

	renderBitmap(background.first_frame, position[0], position[1]);	

	renderString(position[0] + Header_offsets[0], position[1] + Header_offsets[1], XSTR( "support", 224));

	if ( Hud_support_view_fade > 1 ) {
		if ( !timestamp_elapsed(Hud_support_view_fade) ) {
			if ( Hud_support_view_abort){
				renderStringAlignCenter(position[0], position[1] + text_val_offset_y, w, XSTR( "aborted", 225));
			} else {
				renderStringAlignCenter(position[0], position[1] + text_val_offset_y, w, XSTR( "complete", 1407));
			}
		}
		return;
	}

	show_time = 0;
	if (Player_ai->ai_flags & AIF_BEING_REPAIRED) {
		Assert(Player_ship->ship_max_hull_strength > 0);
		
		if (!Cmdline_rearm_timer)
		{
			int i;
			bool repairing = false;
			for (i = 0; i < SUBSYSTEM_MAX; i++)
			{
				if (Player_ship->subsys_info[i].num > 0) 
				{
					if (ship_get_subsystem_strength(Player_ship, i) < 1.0f)
					{
						repairing = true;
						break;
					}
				}
			}

			if (repairing)
				sprintf(outstr, XSTR("repairing", 227));
			else
				sprintf(outstr, XSTR("rearming", 228));
		}
		else
		{
			if (Player_rearm_eta > 0)
			{
				int min, sec, hund;
		
				min = (int)Player_rearm_eta / 60;
				sec = (int)Player_rearm_eta % 60;
				hund = (int)(Player_rearm_eta * 100) % 100;

				sprintf(outstr, "%02d:%02d.%02d", min, sec, hund);
			}
			else
			{
				sprintf(outstr, "Waiting...");
			}	
		}
		renderStringAlignCenter(position[0], position[1] + text_val_offset_y, w, outstr);
	}
	else if (Player_ai->ai_flags & AIF_REPAIR_OBSTRUCTED) {
		sprintf(outstr, XSTR( "obstructed", 229));
		renderStringAlignCenter(position[0], position[1] + text_val_offset_y, w, outstr);
	} else {
		if ( Hud_support_objnum == -1 ) {
			if (The_mission.support_ships.arrival_location == ARRIVE_FROM_DOCK_BAY)
			{
				sprintf(outstr, XSTR( "exiting hangar", -1));
			}
			else
			{
				sprintf(outstr, XSTR( "warping in", 230));
			}
			renderStringAlignCenter(position[0], position[1] + text_val_offset_y, w, outstr);
		} else {
			ai_info *aip;

			// display "busy" when support ship isn't actually enroute to me
			aip = &Ai_info[Ships[Objects[Hud_support_objnum].instance].ai_index];
			if ( aip->goal_objnum != OBJ_INDEX(Player_obj) ) {
				sprintf(outstr, XSTR( "busy", 231));
				show_time = 0;

			} else {
				sprintf(outstr, XSTR( "dock in:", 232));
				show_time = 1;
			}		

			if (!show_time) {
				renderString(position[0] + text_dock_offset_x, position[1] + text_val_offset_y, outstr);
			} else {			
				renderString(position[0] + text_dock_offset_x, position[1] + text_val_offset_y, outstr);
			}
		}
	}

	if ( show_time ) {
		int seconds, minutes;

		Assert( Hud_support_objnum != -1 );

		// ensure support ship is still alive
		if ( (Objects[Hud_support_objnum].signature != Hud_support_obj_sig) || (Hud_support_target_sig != Player_obj->signature) ) {
			seconds = 0;
		} else {
			seconds = hud_get_dock_time( &Objects[Hud_support_objnum] );
		}

		if ( seconds >= 0 ) {
			minutes = seconds/60;
			seconds = seconds%60;
			if ( minutes > 99 ) {
				minutes = 99;
				seconds = 99;
			}
		} else {
			minutes = 99;
			seconds = 99;
		}
		renderPrintf(position[0] + text_dock_val_offset_x, position[1] + text_val_offset_y, NOX("%02d:%02d"), minutes, seconds);
	}
}

// Set the current color to the default HUD color (with default alpha)
void hud_set_default_color()
{
	Assert(HUD_color_alpha >= 0 && HUD_color_alpha < HUD_NUM_COLOR_LEVELS);
	gr_set_color_fast(&HUD_color_defaults[HUD_color_alpha]);
}

// Set the current color to a bright HUD color (ie high alpha)
void hud_set_bright_color()
{
	int alpha_color;
	alpha_color = MIN(HUD_COLOR_ALPHA_MAX,HUD_color_alpha+HUD_BRIGHT_DELTA);
	gr_set_color_fast(&HUD_color_defaults[alpha_color]);
}

// Set the current color to a dim HUD color (ie low alpha)
void hud_set_dim_color()
{
	if ( HUD_color_alpha > 2 ) {
		gr_set_color_fast(&HUD_color_defaults[2]);
	}
}

// hud_set_iff_color() will set the color to the IFF color based on the team
//
// input:		team		=>		team to base color on
//				is_bright	=>		default parameter (value 0) which uses bright version of IFF color
void hud_set_iff_color(object *objp, int is_bright)
{
	color *use_color;

	if (ship_is_tagged(objp))
	{
		use_color = iff_get_color(IFF_COLOR_TAGGED, is_bright);
	}
	else
	{
		// figure out how we get the team
		if (objp->type == OBJ_ASTEROID)
		{
			if (OBJ_INDEX(objp) == Player_ai->target_objnum)
			{
				use_color = iff_get_color_by_team(Iff_traitor, Player_ship->team, is_bright);
			}
			else
			{
				use_color = iff_get_color(IFF_COLOR_SELECTION, is_bright);
			}
		}
		else
		{
			use_color = iff_get_color_by_team_and_object(obj_team(objp), Player_ship->team, is_bright, objp);
		}
	}

	gr_set_color_fast(use_color);
}

// reset gauge flashing data
void hud_gauge_flash_init()
{
	int i;
	for ( i=0; i<NUM_HUD_GAUGES; i++ ) {
		HUD_gauge_flash_duration[i]=timestamp(0);
		HUD_gauge_flash_next[i]=timestamp(0);
	}
	HUD_gauge_bright=0;
}

#define NUM_VM_OTHER_SHIP_GAUGES 5
static int Vm_other_ship_gauges[NUM_VM_OTHER_SHIP_GAUGES] = 
{
	HUD_CENTER_RETICLE,
	HUD_TARGET_MONITOR,
	HUD_TARGET_MONITOR_EXTRA_DATA,
	HUD_MESSAGE_LINES,
	HUD_TALKING_HEAD
};

// determine if the specified HUD gauge should be displayed
int hud_gauge_active(int gauge_index)
{
	Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);

	// AL: Special code: Only show two gauges when not viewing from own ship
	if ( Viewer_mode & VM_OTHER_SHIP ) {
		for ( int i = 0; i < NUM_VM_OTHER_SHIP_GAUGES; i++ ) {
			if ( gauge_index == Vm_other_ship_gauges[i] ) {
				return 1;
			}
		}
		return 0;
	}

	return hud_config_show_flag_is_set(gauge_index);
}

// determine if gauge is in pop-up mode or not
int hud_gauge_is_popup(int gauge_index)
{
	Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);
	return hud_config_popup_flag_is_set(gauge_index);
}

// start a gauge to popup
void hud_gauge_popup_start(int gauge_index, int time) 
{
	Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);
	if ( !hud_gauge_is_popup(gauge_index) ) {
		return;
	}

	int num_gauges, i, gauge_type;

	// go through all HUD gauges. Load gauge properties defined in the HUD config if gauge is not customized.
	if(Ship_info[Player_ship->ship_info_index].hud_gauges.size() > 0) {
		num_gauges = Ship_info[Player_ship->ship_info_index].hud_gauges.size();

		for(i = 0; i < num_gauges; i++) {
			gauge_type = Ship_info[Player_ship->ship_info_index].hud_gauges[i]->getConfigType();

			if(gauge_type == gauge_index) {
				Ship_info[Player_ship->ship_info_index].hud_gauges[i]->startPopUp(time);
			}
		}
	} else {
		num_gauges = default_hud_gauges.size();

		for(i = 0; i < num_gauges; i++) {
			gauge_type = default_hud_gauges[i]->getConfigType();

			if(gauge_type == gauge_index)
				default_hud_gauges[i]->startPopUp(time);
		}
	}
}

// call HUD function to flash gauge
void hud_gauge_start_flash(int gauge_index)
{
	Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);

	int i, num_gauges, gauge_type;

	HUD_gauge_flash_duration[gauge_index] = timestamp(HUD_GAUGE_FLASH_DURATION);
	HUD_gauge_flash_next[gauge_index] = 1;

	// go through all HUD gauges. Load gauge properties defined in the HUD config if gauge is not customized.
	if(Ship_info[Player_ship->ship_info_index].hud_gauges.size() > 0) {
		num_gauges = Ship_info[Player_ship->ship_info_index].hud_gauges.size();

		for(i = 0; i < num_gauges; i++) {
			gauge_type = Ship_info[Player_ship->ship_info_index].hud_gauges[i]->getConfigType();

			if(gauge_type == gauge_index) {
				Ship_info[Player_ship->ship_info_index].hud_gauges[i]->startFlashSexp();
			}
		}
	} else {
		num_gauges = default_hud_gauges.size();

		for(i = 0; i < num_gauges; i++) {
			gauge_type = default_hud_gauges[i]->getConfigType();

			if(gauge_type == gauge_index)
				default_hud_gauges[i]->startFlashSexp();
		}
	}
}

// Set the HUD color for the gauge, based on whether it is flashing or not
void hud_set_gauge_color(int gauge_index, int bright_index)
{
//	color use_color;
	int flash_status = hud_gauge_maybe_flash(gauge_index);
	color *use_color = &HUD_config.clr[gauge_index];
	int alpha;

	// if we're drawing it as bright
	if(bright_index != HUD_C_NONE){
		switch(bright_index){
		case HUD_C_DIM:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_DIM_HI : HUD_NEW_ALPHA_DIM;
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;

		case HUD_C_NORMAL:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_NORMAL_HI : HUD_NEW_ALPHA_NORMAL;
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;

		case HUD_C_BRIGHT:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_BRIGHT_HI : HUD_NEW_ALPHA_BRIGHT;
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;

		// intensity
		default: 
			Assert((bright_index >= 0) && (bright_index < HUD_NUM_COLOR_LEVELS));
			if(bright_index < 0){
				bright_index = 0;
			}
			if(bright_index >= HUD_NUM_COLOR_LEVELS){
				bright_index = HUD_NUM_COLOR_LEVELS - 1;
			}

			// alpha = 255 - (255 / (bright_index + 1));
			// alpha = (int)((float)alpha * 1.5f);
			int level = 255 / (HUD_NUM_COLOR_LEVELS);
			alpha = level * bright_index;
			if(alpha > 255){
				alpha = 255;
			}
			if(alpha < 0){
				alpha = 0;
			}
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;
		}
	} else {
		switch(flash_status) {
		case 0:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_DIM_HI : HUD_NEW_ALPHA_DIM;
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;
		case 1:			
			alpha = HUD_contrast ? HUD_NEW_ALPHA_BRIGHT_HI : HUD_NEW_ALPHA_BRIGHT;
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;
		default:			
			alpha = HUD_contrast ? HUD_NEW_ALPHA_NORMAL_HI : HUD_NEW_ALPHA_NORMAL;	
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;
		}
	}

	gr_set_color_fast(use_color);	
}

// set the color for a gauge that may be flashing
// exit:	-1	=>	gauge is not flashing
//			0	=>	gauge is flashing, draw dim
//			1	=>	gauge is flashing, draw bright
int hud_gauge_maybe_flash(int gauge_index)
{
	Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);
	int flash_status=-1;
	if ( !timestamp_elapsed(HUD_gauge_flash_duration[gauge_index]) ) {
		if ( timestamp_elapsed(HUD_gauge_flash_next[gauge_index]) ) {
			HUD_gauge_flash_next[gauge_index] = timestamp(HUD_GAUGE_FLASH_INTERVAL);
			HUD_gauge_bright ^= (1<<gauge_index);	// toggle between default and bright frames
		}

		if ( HUD_gauge_bright & (1<<gauge_index) ) {
			flash_status=1;
		} else {
			flash_status=0;
		}
	}
	return flash_status;
}

// Init the objective message display data
void hud_objective_message_init()
{
	Objective_display.display_timer=timestamp(0);
}

void hud_update_objective_message()
{
	// find out if we should display the subspace status notification
	if ( (Player->control_mode == PCM_WARPOUT_STAGE1) || (Player->control_mode == PCM_WARPOUT_STAGE2) || (Player->control_mode == PCM_WARPOUT_STAGE3) 
		|| (Sexp_hud_display_warpout > 0) ) {
		if (!hud_subspace_notify_active()) {
			// keep sound from being played 1e06 times
			hud_start_subspace_notify();
		}
	} else {
		if ( timestamp_elapsed(HUD_abort_subspace_timer) ) {
			hud_stop_subspace_notify();
		}
	}

	if ( (Sexp_hud_display_warpout > 1) && hud_subspace_notify_active() ) {
		if ( Sexp_hud_display_warpout < timestamp()) {
			Sexp_hud_display_warpout = 0;
		}
	}
	
	// find out if we should display the objective status notification
	if ( timestamp_elapsed(Objective_display.display_timer) ) {
		hud_stop_objective_notify();
	} else if (!hud_objective_notify_active() && !hud_subspace_notify_active()) {
		hud_start_objective_notify();
	}
}

// Display objective status on the HUD
// input:	type			=>	type of goal, one of:	PRIMARY_GOAL
//																	SECONDARY_GOAL
//																	BONUS_GOAL
//
//				status		=> status of goal, one of:	GOAL_FAILED
//																	GOAL_COMPLETE
//																	GOAL_INCOMPLETE
//
void hud_add_objective_messsage(int type, int status)
{
	Objective_display.display_timer=timestamp(7000);
	Objective_display.goal_type=type;
	Objective_display.goal_status=status;

	// if this is a multiplayer tvt game
	if(MULTI_TEAM && (Net_player != NULL)) {
		mission_goal_fetch_num_resolved(type, &Objective_display.goal_nresolved, &Objective_display.goal_ntotal, Net_player->p_info.team);
	} else {
		mission_goal_fetch_num_resolved(type, &Objective_display.goal_nresolved, &Objective_display.goal_ntotal);
	}

	// TODO: play a sound?
}

HudGaugeObjectiveNotify::HudGaugeObjectiveNotify():
HudGauge(HUD_OBJECT_OBJ_NOTIFY, HUD_OBJECTIVES_NOTIFY_GAUGE, true, false, false, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP, 255, 255, 255)
{

}

void HudGaugeObjectiveNotify::initObjTextOffsetY(int y)
{
	Objective_text_offset_y = y;
}

void HudGaugeObjectiveNotify::initObjValueOffsetY(int y)
{
	Objective_text_val_offset_y = y;
}

void HudGaugeObjectiveNotify::initSubspaceTextOffsetY(int y)
{
	Subspace_text_offset_y = y;
}

void HudGaugeObjectiveNotify::initSubspaceValueOffsetY(int y)
{
	Subspace_text_val_offset_y = y;
}

void HudGaugeObjectiveNotify::initRedAlertTextOffsetY(int y)
{
	Red_text_offset_y = y;
}

void HudGaugeObjectiveNotify::initRedAlertValueOffsetY(int y)
{
	Red_text_val_offset_y = y;
}

void HudGaugeObjectiveNotify::initBitmaps(char *fname)
{
	Objective_display_gauge.first_frame = bm_load_animation(fname, &Objective_display_gauge.num_frames);
	if ( Objective_display_gauge.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in ani: %s\n", fname);
	}
}

void HudGaugeObjectiveNotify::initialize()
{
	flash_timer[0] = timestamp(1);
	flash_timer[1] = timestamp(1);
	flash_flag = false;
}

void HudGaugeObjectiveNotify::pageIn()
{
	bm_page_in_aabitmap( Objective_display_gauge.first_frame, Objective_display_gauge.num_frames);
}

void HudGaugeObjectiveNotify::startFlashNotify(int duration)
{
	flash_timer[0] = timestamp(duration);
}

bool HudGaugeObjectiveNotify::maybeFlashNotify(bool flash_fast)
{
	bool draw_bright = false;

	if(!timestamp_elapsed(flash_timer[0])) {
		if(timestamp_elapsed(flash_timer[1])) {
			if(flash_fast) {
				flash_timer[1] = timestamp(fl2i(TBOX_FLASH_INTERVAL/2.0f));
			} else {
				flash_timer[1] = timestamp(TBOX_FLASH_INTERVAL);
			}
			flash_flag = !flash_flag;
		}

		if(flash_flag) {
			draw_bright = true;
		} 
	}

	return draw_bright;
}

void HudGaugeObjectiveNotify::render(float frametime)
{
	renderSubspace();
	renderRedAlert();
	renderObjective();
}

void HudGaugeObjectiveNotify::renderSubspace()
{
	int w, h;
	int warp_aborted = 0;

	if ( (Player->control_mode != PCM_WARPOUT_STAGE1) && (Player->control_mode != PCM_WARPOUT_STAGE2) && (Player->control_mode != PCM_WARPOUT_STAGE3) 
		&& (Sexp_hud_display_warpout <= 0) ) {
		if ( !timestamp_elapsed(HUD_abort_subspace_timer) ) {
			warp_aborted = 1;
		}
	}

	if ( !hud_subspace_notify_active() ) {
		return;
	}

	if ( Objective_display_gauge.first_frame < 0 ) {
		return;
	}

	// blit the background	
	setGaugeColor();
	renderBitmap(Objective_display_gauge.first_frame, position[0], position[1]);

	startFlashNotify();
	if(maybeFlashNotify()){
		setGaugeColor(HUD_C_BRIGHT);
	} else {
		setGaugeColor();
	}

	bm_get_info(Objective_display_gauge.first_frame, &w, &h);

	renderStringAlignCenter(position[0], position[1] + Subspace_text_offset_y, w, XSTR( "subspace drive", 233));
	if ( warp_aborted ) {
		renderStringAlignCenter(position[0], position[1] + Subspace_text_val_offset_y, w, XSTR( "aborted", 225));
	} else {
		renderStringAlignCenter(position[0], position[1] + Subspace_text_val_offset_y, w, XSTR( "engaged", 234));
	}
}

void HudGaugeObjectiveNotify::renderRedAlert()
{
	int w, h;

	if ( !red_alert_check_status() ) {
		return;
	}

	if ( Objective_display_gauge.first_frame < 0 ) {
		return;
	}

	if ( hud_subspace_notify_active() ) {
		return;
	}

	if ( hud_objective_notify_active() ) {
		return;
	}

	// blit the background
	gr_set_color_fast(&Color_red);		// color box red, cuz its an emergency for cryin out loud

	GR_AABITMAP(Objective_display_gauge.first_frame, position[0], position[1]);	

	startFlashNotify();
	if(maybeFlashNotify()) {
		gr_set_color_fast(&Color_red);
	} else {
		gr_set_color_fast(&Color_bright_red);
	}

	bm_get_info(Objective_display_gauge.first_frame, &w, &h);

	renderStringAlignCenter(position[0], position[1] + Red_text_offset_y, w, XSTR( "downloading new", 235));
	renderStringAlignCenter(position[0], position[1] + Red_text_val_offset_y, w, XSTR( "orders...", 236));

	// TODO: play a sound?
}

void HudGaugeObjectiveNotify::renderObjective()
{
	int w, h;
	char buf[128];

	if ( timestamp_elapsed(Objective_display.display_timer) ) {
		return;
	}

	if ( Objective_display_gauge.first_frame < 0 ) {
		return;
	}

	if ( hud_subspace_notify_active() ) {
		return;
	}
	
	// blit the background
	setGaugeColor();
	renderBitmap(Objective_display_gauge.first_frame, position[0], position[1]);	

	startFlashNotify();
	if(maybeFlashNotify()){
		setGaugeColor(HUD_C_BRIGHT);
	} else {
		setGaugeColor();
	}

	bm_get_info(Objective_display_gauge.first_frame, &w, &h);

	// draw the correct goal type
	switch(Objective_display.goal_type) {
	case PRIMARY_GOAL:
		renderStringAlignCenter(position[0], position[1] + Objective_text_offset_y, w, XSTR( "primary objective", 237));
		break;
	case SECONDARY_GOAL:
		renderStringAlignCenter(position[0], position[1] + Objective_text_offset_y, w, XSTR( "secondary objective", 238));
		break;
	case BONUS_GOAL:
		renderStringAlignCenter(position[0], position[1] + Objective_text_offset_y, w, XSTR( "bonus objective", 239));
		break;
	}

	// show the status
	switch(Objective_display.goal_type) {
	case PRIMARY_GOAL:
	case SECONDARY_GOAL:
		switch(Objective_display.goal_status) {
		case GOAL_FAILED:
			sprintf(buf, XSTR( "failed (%d/%d)", 240), Objective_display.goal_nresolved, Objective_display.goal_ntotal);
			renderStringAlignCenter(position[0], position[1] + Objective_text_val_offset_y, w, buf);
			break;
		default:
			sprintf(buf, XSTR( "complete (%d/%d)", 241), Objective_display.goal_nresolved, Objective_display.goal_ntotal);
			renderStringAlignCenter(position[0], position[1] + Objective_text_val_offset_y, w, buf);
			break;
		}		
		break;
	case BONUS_GOAL:
		switch(Objective_display.goal_status) {
		case GOAL_FAILED:
			renderStringAlignCenter(position[0], position[1] + Objective_text_val_offset_y, w, XSTR( "failed", 242));
			break;
		default:
			renderStringAlignCenter(position[0], position[1] + Objective_text_val_offset_y, w, XSTR( "complete", 226));
			break;
		}		
		break;
	}
}

void hud_subspace_notify_abort()
{
	HUD_abort_subspace_timer = timestamp(1500);
}

void hud_stop_subspace_notify()
{
	Subspace_notify_active=0;
}

void hud_start_subspace_notify()
{

	Subspace_notify_active=1;
}

int hud_subspace_notify_active()
{
	return Subspace_notify_active;
}

void hud_stop_objective_notify()
{
	Objective_notify_active = 0;
}

void hud_start_objective_notify()
{
	snd_play(&(Snds[SND_DIRECTIVE_COMPLETE]));
	Objective_notify_active = 1;
}

int hud_objective_notify_active()
{
	return Objective_notify_active;
}

// set the offset values for this render frame
void HUD_set_offsets(object *viewer_obj, int wiggedy_wack, matrix *eye_orient)
{
	if ( (viewer_obj == Player_obj) && wiggedy_wack ){		
		vec3d tmp;
		vertex pt;
		ubyte flags;		

		HUD_offset_x = 0.0f;
		HUD_offset_y = 0.0f;

		vm_vec_scale_add( &tmp, &Eye_position, &eye_orient->vec.fvec, 100.0f );
		
		flags = g3_rotate_vertex(&pt,&tmp);

		g3_project_vertex(&pt);

		gr_unsize_screen_posf( &pt.sx, &pt.sy );
		HUD_offset_x -= 0.45f * (i2fl(gr_screen.clip_width_unscaled)*0.5f - pt.sx);
		HUD_offset_y -= 0.45f * (i2fl(gr_screen.clip_height_unscaled)*0.5f - pt.sy);

		if ( HUD_offset_x > 100.0f )	{
			HUD_offset_x = 100.0f;
		} else if ( HUD_offset_x < -100.0f )	{
			HUD_offset_x += 100.0f;
		}

		if ( HUD_offset_y > 100.0f )	{
			HUD_offset_y = 100.0f;
		} else if ( HUD_offset_y < -100.0f )	{
			HUD_offset_y += 100.0f;
		}

	} else {
		HUD_offset_x = 0.0f;
		HUD_offset_y = 0.0f;
	}

	// Since the player's view vector may be different from the ship's forward vector, 
	// we calculate the offset of those two in pixels and store the x and y offsets in 
	// variables HUD_nose_x and HUD_nose_y (Swifty)
	if ( Viewer_mode & VM_TOPDOWN ) {
		HUD_nose_x = 0;
		HUD_nose_y = 0;
	} else {
		HUD_get_nose_coordinates(&HUD_nose_x, &HUD_nose_y);
	}
}
// Function returns the offset between the player's view vector and the forward vector of the ship in pixels
// (Swifty)
void HUD_get_nose_coordinates(int *x, int *y)
{
	vertex	v0;
	vec3d	p0;

	float x_nose;
	float y_nose;
	float x_center = gr_screen.clip_center_x;
	float y_center = gr_screen.clip_center_y;

	*x = 0;
	*y = 0;
	
	vm_vec_scale_add(&p0, &Player_obj->pos, &Player_obj->orient.vec.fvec, 10000.0f);
	g3_rotate_vertex(&v0, &p0);

	if (v0.codes == 0) {
		g3_project_vertex(&v0);

		if ( !(v0.codes & PF_OVERFLOW) ) {
			x_nose = v0.sx;
			y_nose = v0.sy;
		} else {
			// Means that the ship forward vector is not going through the frame buffer.
			// We're assigning a high negative value so that the the bitmaps will be drawn offscreen so that
			// we can give the illusion that the player is looking away from the slewable HUD reticle.
			*x = -100000;
			*y = -100000;
			return;
		}
	} else {
		// Means that the ship forward vector is not going through the frame buffer.
		// We're assigning a high negative value so that the the bitmaps will be drawn offscreen so that
		// we can give the illusion that the player is looking away from the slewable HUD reticle.
		*x = -100000;
		*y = -100000;
		return;
	}

	gr_unsize_screen_posf(&x_nose, &y_nose);
	gr_unsize_screen_posf(&x_center, &y_center);

	*x = fl2i(x_nose - x_center);
	*y = fl2i(y_nose - y_center);
	return;
}

// Basically like gr_reset_clip only it accounts for hud jittering
void HUD_reset_clip()
{
	int hx = fl2i(HUD_offset_x);
	int hy = fl2i(HUD_offset_y);

	gr_set_clip(hx, hy, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled);
}

// Basically like gr_set_clip only it accounts for hud jittering
void HUD_set_clip(int x, int y, int w, int h)
{
	int hx = fl2i(HUD_offset_x);
	int hy = fl2i(HUD_offset_y);

	gr_set_clip(hx+x, hy+y, w, h);
}

// -------------------------------------------------------------------------------------
// hud_save_restore_camera_data()
//
//	Called to save and restore the 3D camera settings.
//
void hud_save_restore_camera_data(int save)
{
	static vec3d	save_view_position;
	static float	save_view_zoom;
	static matrix	save_view_matrix;
	static matrix	save_eye_matrix;
	static vec3d	save_eye_position;

	// save global view variables, so we can restore them
	if ( save ) {
		save_view_position	= View_position;
		save_view_zoom			= View_zoom;
		save_view_matrix		= View_matrix;
		save_eye_matrix		= Eye_matrix;
		save_eye_position		= Eye_position;
	}
	else {
		// restore global view variables
		View_position	= save_view_position;
		View_zoom		= save_view_zoom;
		View_matrix		= save_view_matrix;
		Eye_matrix		= save_eye_matrix;
		Eye_position	= save_eye_position;
	}
}


void hud_toggle_contrast()
{
	HUD_contrast = !HUD_contrast;
}

void hud_set_contrast(int high)
{
	HUD_contrast = high;
}

// Paging functions for the rest of the hud code
extern void hudtarget_page_in();

// Page in all hud bitmaps
void hud_page_in()
{
	bm_page_in_aabitmap( Kills_gauge.first_frame, Kills_gauge.num_frames );

	// Paging functions for the rest of the hud code
	hudtarget_page_in();

	// go through all hud gauges to page them in 
	int i, num_gauges = 0;
	for (i = 0; i < Num_ship_classes; i++) {
		if(Ship_info[i].hud_enabled) {
			if(Ship_info[i].hud_gauges.size() > 0) {
				num_gauges = Ship_info[i].hud_gauges.size();

				for(int j = 0; j < num_gauges; j++) {
					Ship_info[i].hud_gauges[j]->pageIn();
				}
			}
		}
	}

	num_gauges = default_hud_gauges.size();

	for(i = 0; i < num_gauges; i++) {
		default_hud_gauges[i]->pageIn();
	}
}

HudGauge* hud_get_gauge(char* name)
{
	char* gauge_name;

	// go through all gauges and return the gauge that matches
	if(Ship_info[Player_ship->ship_info_index].hud_gauges.size() > 0) {
		for(int i = 0; i < (int)Ship_info[Player_ship->ship_info_index].hud_gauges.size(); i++) {

			gauge_name = Ship_info[Player_ship->ship_info_index].hud_gauges[i]->getCustomGaugeName();
			if(!strcmp(name, gauge_name)) {
				return Ship_info[Player_ship->ship_info_index].hud_gauges[i];
			}
		}
	} else {
		for(int i = 0; i < (int)default_hud_gauges.size(); i++) {

			gauge_name = default_hud_gauges[i]->getCustomGaugeName();
			if(!strcmp(name, gauge_name)) {
				return default_hud_gauges[i];
			}
		}
	}

	return NULL;
}

HudGaugeMultiMsg::HudGaugeMultiMsg():
HudGauge(HUD_OBJECT_MULTI_MSG, HUD_MESSAGE_LINES, true, false, true, 0, 255, 255, 255)
{
}

// render multiplayer text message currently being entered if any
void HudGaugeMultiMsg::render(float frametime)
{
	char txt[MULTI_MSG_MAX_TEXT_LEN+20];

	// clear the text
	memset(txt,0,MULTI_MSG_MAX_TEXT_LEN+1);

	// if there is valid multiplayer message text to be displayed
	if(multi_msg_message_text(txt)){
		gr_set_color_fast(&Color_normal);
		renderString(position[0], position[1], txt);
	}
}

HudGaugeVoiceStatus::HudGaugeVoiceStatus():
HudGauge(HUD_OBJECT_VOICE_STATUS, HUD_MESSAGE_LINES, true, false, true, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY, 255, 255, 255) 
{
}

void HudGaugeVoiceStatus::render(float frametime)
{
	if(!(Game_mode & GM_MULTIPLAYER)){
		return;
	}

	char play_callsign[CALLSIGN_LEN+5];
	
	// if we are currently playing a rtvoice sound stream from another player back
	memset(play_callsign,0,CALLSIGN_LEN+5);
	switch(multi_voice_status()){
	// the player has been denied the voice token
	case MULTI_VOICE_STATUS_DENIED:
		// show a red indicator or something
		renderString(position[0], position[1], XSTR( "[voice denied]", 243));
		break;

	// the player is currently recording
	case MULTI_VOICE_STATUS_RECORDING:
		renderString(position[0], position[1], XSTR( "[recording voice]", 244));
		break;
		
	// the player is current playing back voice from someone
	case MULTI_VOICE_STATUS_PLAYING:
		renderString(position[0], position[1], XSTR( "[playing voice]", 245));
		break;

	// nothing voice related is happening on my machine
	case MULTI_VOICE_STATUS_IDLE:
		// probably shouldn't be displaying anything
		break;
	}	
}

HudGaugePing::HudGaugePing():
HudGauge(HUD_OBJECT_PING, HUD_LAG_GAUGE, true, false, false, 0, 255, 255, 255)
{

}

// render multiplayer ping time to the server if appropriate
void HudGaugePing::render(float frametime)
{
	// if we shouldn't be displaying a ping time, return here
	if(!multi_show_ingame_ping()){
		return;
	}
	
	// if we're in multiplayer mode, display our ping time to the server
	if(MULTIPLAYER_CLIENT && (Net_player != NULL)){
		char ping_str[50];
		memset(ping_str,0,50);

		// if our ping is positive, display it
		if((Netgame.server != NULL) && (Netgame.server->s_info.ping.ping_avg > 0)){
			// get the string
			if(Netgame.server->s_info.ping.ping_avg >= 1000){
				sprintf(ping_str,XSTR("> 1 sec",628));
			} else {
				sprintf(ping_str,XSTR("%d ms",629),Netgame.server->s_info.ping.ping_avg);
			}

			// blit the string out
			hud_set_default_color();
			renderString(position[0], position[1], ping_str);
		}
	}
}

HudGaugeSupernova::HudGaugeSupernova():
HudGauge(HUD_OBJECT_SUPERNOVA, HUD_DIRECTIVES_VIEW, true, false, false, 0, 255, 255, 255)
{
}

void HudGaugeSupernova::render(float frametime)
{
	float time_left;

	// if there's a supernova coming
	time_left = supernova_time_left();
	if(time_left < 0.0f){
		return;
	}

	gr_set_color_fast(&Color_bright_red);
	renderPrintf(position[0], position[1], "Supernova Warning: %.2f s", time_left);
}
