/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifndef __HUD_H__
#define __HUD_H__

#include "globalincs/vmallocator.h"
#include "graphics/2d.h"
#include "graphics/font.h"
#include "hud/hudgauges.h"
#include "hud/hudparse.h"

class object;
struct cockpit_display;

typedef struct hud_anim {
	char filename[MAX_FILENAME_LEN];
	int first_frame;	// the bitmap id for the first frame in the animation... note that
						// all bitmap id's following this frame are numbered sequentially
	int num_frames;		// number of frames in the animation
	int sx, sy;			// screen (x,y) of top-left corner of animation
	float total_time;	// total time in seconds for the animation (depends on animation fps)
	float time_elapsed;	// time that has elapsed (in seconds) since animation started playing
} hud_anim;

typedef struct hud_frames {
	int	first_frame;
	int	num_frames;
} hud_frames;

// Objective display
typedef struct objective_display_info
{
	int display_timer;
	int goal_type;
	int goal_status;
	int goal_ntotal;
	int goal_nresolved;
	
} objective_display_info;

// used to track how player subsystems are getting damaged
typedef struct hud_subsys_info
{
	float	last_str;
	int	flash_duration_timestamp;
} hud_subsys_info;

// used for the display of damaged subsystems
typedef struct hud_subsys_damage
{
	int	str;
	int	type;
	const char* name;
} hud_subsys_damage;

// used for hudtarget
enum class HudAlignment
{
	NONE,
	LEFT,
	RIGHT
};

extern int HUD_draw;
extern bool HUD_high_contrast;
extern bool HUD_shadows;

#define HUD_NUM_COLOR_LEVELS	16
extern color HUD_color_defaults[HUD_NUM_COLOR_LEVELS];

// extern globals that will control the color of the HUD gauges
#define HUD_COLOR_ALPHA_USER_MAX		13		// max user-settable alpha, absolute max is 15
#define HUD_COLOR_ALPHA_USER_MIN		3		// min user-settable alpha, absolute min is 0

#define HUD_COLOR_ALPHA_MAX			15
#define HUD_COLOR_ALPHA_DEFAULT		8

#define HUD_BRIGHT_DELTA				7		// Level added to HUD_color_alpha to make brightness used for flashing

// hud macro for maybe flickering all gauges
#define GR_AABITMAP(a, b, c)						{ int jx, jy; if(emp_should_blit_gauge()) { gr_set_bitmap(a); jx = b; jy = c; emp_hud_jitter(&jx, &jy); gr_aabitmap(jx, jy); } }
#define GR_AABITMAP_EX(a, b, c, d, e, f, g)	{ int jx, jy; if(emp_should_blit_gauge()) { gr_set_bitmap(a); jx = b; jy = c; emp_hud_jitter(&jx, &jy); gr_aabitmap_ex(jx, jy, d, e, f, g); } }

// radar target identification flags
#define RTIF_CROSSHAIRS	(1<<0)		// draw crosshairs
#define RTIF_BLINK		(1<<1)		// make targeted blip blink
#define RTIF_PULSATE	(1<<2)		// make targeted blips size pulsate
#define RTIF_ENLARGE	(1<<3)		// make targeted blip appear larger than the other blips

extern int radar_target_id_flags;

extern int HUD_color_red;
extern int HUD_color_green;
extern int HUD_color_blue;
extern int HUD_color_alpha;

extern color HUD_color_debug;

// the offset of the player's view vector and the ship forward vector in pixels (Swifty)
extern int HUD_nose_x;
extern int HUD_nose_y;
// Global: integrity of player's target
extern float Pl_target_integrity;
extern float Player_rearm_eta;

extern int Hud_max_targeting_range;

// maps of hud gauges configs to apply to ships as parsed in ships.tbl
// this is cleared once parsing is completed
// First string in the pair is the gauge name, second is the ship name
extern SCP_vector<std::pair<SCP_string, SCP_string>> Hud_parsed_ships;

void HUD_init_colors();
void HUD_init();
void hud_scripting_close(lua_State*);
void hud_close();
void hud_level_close();
void hud_update_frame(float frametime);		// updates hud systems not dependant on rendering
void hud_render_preprocess(float frametime);			// renders 3d dependant gauges
void hud_render_all(float frametime);
void hud_render_gauges(int cockpit_display_num, float frametime);
void hud_stop_looped_engine_sounds();

// set the offset values for this render frame
void HUD_set_offsets();
// returns the offset between the player's view vector and the forward vector of the ship in pixels (Swifty)
void HUD_get_nose_coordinates(int *x, int *y);

// Basically like gr_reset_clip only it accounts for hud jittering
void HUD_reset_clip();
void hud_save_restore_camera_data(int save);

// Basically like gr_set_clip only it accounts for hud jittering
void HUD_set_clip(int x, int y, int w, int h);

// do flashing text gauge
void hud_start_text_flash(const char *txt, int t, int interval = 200);

// convert a string to use mono spaced numbers
void hud_num_make_mono(char *num_str, int font_num = font::FONT1);

// functions for handling hud animations
void hud_anim_init(hud_anim *ha, int sx, int sy, const char *filename);
void hud_frames_init(hud_frames *hf);
int	hud_anim_render(hud_anim *ha, float frametime, int draw_alpha=0, int loop=1, int hold_last=0, int reverse=0,int resize_mode=GR_RESIZE_FULL, bool mirror = false, float scale_factor = 1.0f);
int	hud_anim_load(hud_anim *ha);

// functions for displaying the support view popup
void hud_support_view_start();
void hud_support_view_stop(int stop_now=0);
void hud_support_view_abort();
void hud_support_view_update();

void HUD_init_hud_color_array();

// setting HUD colors
void hud_set_default_color();
void hud_set_iff_color(object *objp, int is_bright=0);
color* hud_get_iff_color(object *objp, int is_bright=0);
void hud_set_bright_color();
void hud_set_dim_color();

// HUD gauge functions
#define HUD_C_NONE				-4
#define HUD_C_BRIGHT			-3
#define HUD_C_DIM				-2
#define HUD_C_NORMAL			-1
void	hud_set_gauge_color(int gauge_index, int bright_index = HUD_C_NONE);
int	hud_gauge_active(int gauge_index);
void	hud_gauge_start_flash(int gauge_index);
int	hud_gauge_maybe_flash(int gauge_index);

HudAlignment hud_alignment_lookup(const char *name);

// popup gauges
void	hud_gauge_popup_start(int gauge_index, int time=4000);
int	hud_gauge_is_popup(int gauge_index);

// objective status gauge
void hud_update_objective_message();
void hud_add_objective_messsage(int type, int status);

int	hud_get_dock_time( object *docker_objp );
void	hud_show_target_model();
void	hud_show_voice_status();

void	hud_subspace_notify_abort();

// render multiplayer text message currently being entered if any
void hud_maybe_render_multi_text();

int hud_get_draw();
void hud_toggle_draw();
int	hud_disabled();
int hud_support_find_closest( object *objp );

// Goober5000
void hud_set_draw(int draw);
void hud_disable_except_messages(int disable);
int hud_disabled_except_messages();

// contrast stuff
void hud_toggle_contrast();
void hud_set_contrast(bool high);
void hud_toggle_shadows();

class HudGauge 
{
protected:
	int position[2];
	int base_w, base_h;
	color gauge_color;
	int gauge_type; // Used to be the gauge_config numeric ID but now more accurately is used as the type. Will be one of the HUD_ defines from hudgauges.h
	int gauge_object;
	SCP_string gauge_config_id;

	int font_num;

	// useful information to keep around for scripts
	float tabled_origin[2];
	int tabled_offset[2];
	bool tabled_use_coords;
	int tabled_coords[2];
	float aspect_quotient;
	bool hi_res;

	bool lock_color;
	bool sexp_lock_color;
	bool reticle_follow;
	bool active;
	bool off_by_default;
	bool sexp_override;
	bool pop_up;
	int popup_timer;
	bool message_gauge;
	int disabled_views;
	bool scripting_render_override;

	// Config stuff
	SCP_string config_name;
	bool can_popup;
	bool use_iff_color;
	bool use_tag_color;
	bool visible_in_config;

	int flash_duration;
	int flash_next;
	bool flash_status;
	bool only_render_in_chase_view;
	int render_for_cockpit_toggle;

	// custom gauge specific stuff
	bool custom_gauge;
	hud_frames custom_frame;
	int custom_frame_offset;
	int textoffset_x, textoffset_y;
	char custom_name[NAME_LENGTH];
	SCP_string custom_text;
	
	SCP_string default_text;

	// Render to texture stuff
	char texture_target_fname[MAX_FILENAME_LEN];
	int texture_target;
	int canvas_w, canvas_h;
	int target_w, target_h;
	int target_x, target_y;
	int display_offset_x, display_offset_y;
public:
	// constructors
	HudGauge();
	HudGauge(int _gauge_object, int _gauge_config, bool _slew, bool _message, int _disabled_views, int r, int g, int b);
	// constructor for custom gauges
	HudGauge(int _gauge_config, bool _slew, int r, int g, int b, char* _custom_name, char* _custom_text, char* frame_fname, int txtoffset_x, int txtoffset_y);
	virtual ~HudGauge();

	void initPosition(int x, int y);
	void initBaseResolution(int w, int h, float aq);
	void initFont(int input_font_num);
	void initOriginAndOffset(float originX, float originY, int offsetX, int offsetY);
	void initCoords(bool use_coords, int coordsX, int coordsY);
	void initHiRes(const char* fname);

	virtual void initSlew(bool slew);
	void initCockpitTarget(const char* display_name, int _target_x, int _target_y, int _target_w, int _target_h, int _canvas_w, int _canvas_h);
	void initRenderStatus(bool render);

	bool isCustom() const;
	bool isHiRes() const;
	int getBaseWidth() const;
	int getBaseHeight() const;
	float getAspectQuotient() const;

	int getConfigType() const;
	int getObjectType() const;
	void getPosition(int *x, int *y) const;
	bool isOffbyDefault() const;
	bool isActive() const;

	// Config getters
	SCP_string getConfigName() const;
	SCP_string getConfigId() const;
	bool getConfigUseIffColor() const;
	bool getConfigCanPopup() const;
	bool getConfigUseTagColor() const;
	bool getVisibleInConfig() const;

	int getFont() const;
	void getOriginAndOffset(float *originX, float *originY, int *offsetX, int *offsetY) const;
	void getCoords(bool* use_coords, int* coordsX, int* coordsY) const;
	
	void updateColor(int r, int g, int b, int a = 255);
	const color& getColor() const;
	void lockConfigColor(bool lock);
	void sexpLockConfigColor(bool lock);
	void updateActive(bool show);
	void updatePopUp(bool pop_up_flag);
	void updateSexpOverride(bool sexp);
	bool getScriptingOverride() const;
	void updateScriptingOverride(bool toggle);
	void initChase_view_only(bool chase_view_only);
	void initCockpit_view_choice(int cockpit_view_choice);
	void initVisible_in_config(bool visible);

	// SEXP interfacing functions
	// For flashing gauges in training missions
	void startFlashSexp();
	int maybeFlashSexp();
	bool flashExpiredSexp() const;
	void resetTimers();

	// For updating custom gauges
	const char* getCustomGaugeName() const;
	void updateCustomGaugeText(const char* txt);
	void updateCustomGaugeText(const SCP_string& txt);
	const char* getCustomGaugeText() const;

	void startPopUp(int time=4000);
	int popUpActive() const;

	virtual void preprocess();
	virtual void render(float frametime, bool config = false);
	virtual bool canRender() const;
	virtual void pageIn();
	virtual void initialize();
	virtual void onFrame(float frametime);

	bool setupRenderCanvas(int render_target = -1);
	void setCockpitTarget(const cockpit_display *display);
	void resetCockpitTarget();
	
	void setFont();
	void setGaugeColor(int bright_index = HUD_C_NONE, bool config = false);
	void setGaugeCoords(int _x, int _y);
	void setGaugeFrame(int frame_offset);

	// rendering functions
	void renderBitmap(int x, int y, float scale = 1.0f, bool config = false) const;
	void renderBitmap(int frame, int x, int y, float scale = 1.0f, bool config = false) const;
	void renderBitmapColor(int frame, int x, int y, float scale = 1.0f, bool config = false) const;
	void renderBitmapEx(int frame, int x, int y, int w, int h, int sx, int sy, float scale = 1.0f, bool config = false) const;
	void renderString(int x, int y, const char *str, float scale = 1.0f, bool config = false);
	void renderString(int x, int y, int gauge_id, const char *str, float scale = 1.0f, bool config = false);
	void renderStringAlignCenter(int x, int y, int area_width, const char *s, float scale = 1.0f, bool config = false);
	void renderPrintf(int x, int y, float scale, bool config, SCP_FORMAT_STRING const char* format, ...) SCP_FORMAT_STRING_ARGS(6, 7);
	void renderPrintfWithGauge(int x, int y, int gauge_id, float scale, bool config, SCP_FORMAT_STRING const char* format, ...)  SCP_FORMAT_STRING_ARGS(7, 8);
	void renderLine(int x1, int y1, int x2, int y2, bool config = false) const;
	void renderGradientLine(int x1, int y1, int x2, int y2, bool config = false) const;
	void renderRect(int x, int y, int w, int h, bool config = false) const;
	void renderCircle(int x, int y, int diameter, bool filled = true, bool config = false) const;

	void unsize(int *x, int *y);
	void unsize(float *x, float *y);
	void resize(int *x, int *y);
	void resize(float *x, float *y);
	void setClip(int x, int y, int w, int h);
	void resetClip();
};

//Use this instead of HudGauge whenever you have a HudGauge that is anchored in 3D-space (i.e. takes its rendering coordinates from g3_rotate/project_vertex, as these MUST NEVER slew.
class HudGauge3DAnchor : public HudGauge {

public:
	HudGauge3DAnchor() 
		: HudGauge() { }
	HudGauge3DAnchor(int _gauge_object, int _gauge_config, bool /*_slew*/, bool _message, int _disabled_views, int r, int g, int b)
		: HudGauge(_gauge_object, _gauge_config, false, _message, _disabled_views, r, g, b) { }
	// constructor for custom gauges
	HudGauge3DAnchor(int _gauge_config, bool /*_slew*/, int r, int g, int b, char* _custom_name, char* _custom_text, char* frame_fname, int txtoffset_x, int txtoffset_y)
		: HudGauge(_gauge_config, false, r, g, b, _custom_name, _custom_text, frame_fname, txtoffset_x, txtoffset_y) { }

	void initSlew(bool /*slew*/) override {};
};

class HudGaugeMissionTime: public HudGauge // HUD_MISSION_TIME
{
	hud_frames time_gauge;

	int time_text_offsets[2]; // Mission_time_text_coords[gr_screen.res]
	int time_val_offsets[2]; // Mission_time_text_val_coords[gr_screen.res]
public:
	HudGaugeMissionTime();
	void initBitmaps(const char *fname);
	void initTextOffsets(int x, int y);
	void initValueOffsets(int x, int y);
	void render(float frametime, bool config = false) override;
	void pageIn() override;
};

class HudGaugeTextWarnings: public HudGauge // HUD_TEXT_FLASH
{
	int next_flash;
	bool flash_flags;
public:
	HudGaugeTextWarnings();
  void render(float frametime, bool config = false) override;
	void initialize() override;
	int maybeTextFlash();
};

class HudGaugeKills: public HudGauge
{
	hud_frames Kills_gauge;

	int text_offsets[2];
	int text_value_offsets[2];
public:
	HudGaugeKills();
	void initBitmaps(const char *fname);
	void initTextOffsets(int x, int y);
	void initTextValueOffsets(int x, int y);
	void render(float frametime, bool config = false) override;
	void pageIn() override;
};

class HudGaugeLag: public HudGauge
{
	hud_frames Netlag_icon;

	int flash_timer[2];
	bool flash_flag;
public:
	HudGaugeLag();
	void initBitmaps(const char *fname);
	void render(float frametime, bool config = false) override;
	void pageIn() override;

	void startFlashLag(int duration = 1400);
	bool maybeFlashLag(bool flash_fast = false);
};

class HudGaugeObjectiveNotify: public HudGauge
{
protected:
	hud_frames	Objective_display_gauge;

	int Objective_text_offset_y;
	int Objective_text_val_offset_y;
	int Subspace_text_offset_y;
	int Subspace_text_val_offset_y;
	int Red_text_offset_y;
	int Red_text_val_offset_y;

	int flash_timer[2];
	bool flash_flag;
public:
	HudGaugeObjectiveNotify();
	void initBitmaps(const char *fname);
	void initObjTextOffsetY(int y);
	void initObjValueOffsetY(int y);
	void initSubspaceTextOffsetY(int y);
	void initSubspaceValueOffsetY(int y);
	void initRedAlertTextOffsetY(int y);
	void initRedAlertValueOffsetY(int y);

	void render(float frametime, bool config = false) override;
	void startFlashNotify(int duration = 1400);
	bool maybeFlashNotify(bool flash_fast = false);
	void renderObjective(bool config);
	void renderRedAlert(bool config);
	void renderSubspace(bool config);
	void pageIn() override;
	void initialize() override;
};

class HudGaugeDamage: public HudGauge
{
	struct DamageInfo {
		SCP_string name;
		int strength = -1;

		int bright_index = HUD_C_NONE;
		color* color_override = nullptr;

		int name_x = -1;
		int name_y = -1;

		int value_x = -1;
		int value_y = -1;

		bool draw_background = false;
		int background_x = -1;
		int background_y = -1;
	};

protected:
	hud_frames damage_top;
	hud_frames damage_middle;
	hud_frames damage_bottom;

	int header_offsets[2];
	int hull_integ_offsets[2];
	int hull_integ_val_offset_x;
	int middle_frame_start_offset_y;
	int subsys_integ_start_offsets[2];
	int subsys_integ_val_offset_x;
	int bottom_bg_offset;
	int line_h;

	int Damage_flash_timer;
	bool Damage_flash_bright;

	int next_flash;
	bool flash_status;

	bool always_display;

  public:
	HudGaugeDamage();
	void initBitmaps(const char *fname_top, const char *fname_middle, const char *fname_bottom);
	void initHeaderOffsets(int x, int y);
	void initHullIntegOffsets(int x, int y);
	void initHullIntegValueOffsetX(int x);
	void initMiddleFrameStartOffsetY(int y);
	void initSubsysIntegStartOffsets(int x, int y);
	void initSubsysIntegValueOffsetX(int x);
	void initBottomBgOffset(int offset);
	void initLineHeight(int h);
	void initDisplayValue(bool value);
	void render(float frametime, bool config = false) override;
	void pageIn() override;
	void initialize() override;
};

class HudGaugeSupport: public HudGauge
{
protected:
	hud_frames background;

	int Header_offsets[2];
	int text_val_offset_y;
	int text_dock_offset_x;
	int text_dock_val_offset_x;
	bool enable_rearm_timer;
public:
	HudGaugeSupport();
	void initBitmaps(const char *fname);
	void initHeaderOffsets(int x, int y);
	void initTextValueOffsetY(int y);
	void initTextDockOffsetX(int x);
	void initTextDockValueOffsetX(int x);
	void initRearmTimer(bool choice);
	void render(float frametime, bool config = false) override;
	void pageIn() override;
};

class HudGaugeMultiMsg: public HudGauge
{
protected:
public:
	HudGaugeMultiMsg();
	bool canRender() const override;
	void render(float frametime, bool config = false) override;
};

class HudGaugeVoiceStatus: public HudGauge
{
protected:
public:
	HudGaugeVoiceStatus();
  void render(float frametime, bool config = false) override;
};

class HudGaugePing: public HudGauge
{
protected:
public:
	HudGaugePing();
  void render(float frametime, bool config = false) override;
};

class HudGaugeSupernova: public HudGauge
{
public:
	HudGaugeSupernova();
  void render(float frametime, bool config = false) override;
};

class HudGaugeFlightPath: public HudGauge3DAnchor
{
	hud_frames Marker;

	int Marker_half[2];
public:
	HudGaugeFlightPath();
	void initBitmap(const char *fname);
	void initHalfSize(int w, int h);
	void render(float frametime, bool config = false) override;
};

HudGauge *hud_get_custom_gauge(const char *name, bool check_all_gauges = false);
int hud_get_default_gauge_index(const char *name);
HudGauge *hud_get_gauge(const char *name, bool check_all_custom_gauges = false);

extern SCP_vector<std::unique_ptr<HudGauge>> default_hud_gauges;

extern flag_def_list Hud_gauge_types[];
extern int Num_hud_gauge_types;

extern bool Extra_target_info;

#endif	/* __HUD_H__ */

