/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _HUDTARGET_H
#define _HUDTARGET_H

#include "graphics/2d.h"
#include "hud/hud.h"

class ship;
class ship_subsys;
class object;
struct weapon_info;

#define INCREASING	0
#define DECREASING	1
#define NO_CHANGE		2	

#define MATCH_SPEED_THRESHOLD				0.1f		// minimum speed target must be moving for match speed to apply
#define CARGO_RADIUS_DELTA					100		// distance added to radius required for cargo scanning
#define CAPITAL_CARGO_RADIUS_DELTA		250		// distance added to radius required for cargo scanning
#define CARGO_REVEAL_MIN_DIST				150		// minimum distance for reveal cargo (used if radius+CARGO_RADIUS_DELTA < CARGO_REVEAL_MIN_DIST)
#define CAP_CARGO_REVEAL_MIN_DIST		300		// minimum distance for reveal cargo (used if radius+CARGO_RADIUS_DELTA < CARGO_REVEAL_MIN_DIST)
#define CARGO_MIN_DOT_TO_REVEAL			0.95		// min dot to proceed to have cargo scanning take place

// structure and defines used for hotkey targeting
//WMC - bumped from 50 to 150; 10/24/2005
#define MAX_HOTKEY_TARGET_ITEMS		150		// maximum number of ships that can be targeted on *all* keys
#define SELECTION_SET					0x5000	// variable used for drawing brackets.  The bracketing code uses
															// TEAM_* values.  I picked this value to be totally out of that
															// range.  Only used for drawing selection sets
#define MESSAGE_SENDER					0x5001	// variable used for drawing brackets around a message sender.
															// See above comments for SELECTION_SET

// defines used to tell how a particular hotkey was added
#define HOTKEY_USER_ADDED				1
#define HOTKEY_MISSION_FILE_ADDED	2

// regular and ballistic weapon gauges
#define NUM_HUD_SETTINGS	2

typedef struct htarget_list {
	struct htarget_list	*next, *prev;		// for linked lists
	int						how_added;			// determines how this hotkey was added (mission default or player)
	object					*objp;				// the actual object
} htarget_list;

//for nebula toggle SEXP
#define		TOGGLE_TEXT_NEBULA_ALPHA	127
#define		TOGGLE_TEXT_NORMAL_ALPHA	160
extern int Toggle_text_alpha;

extern htarget_list htarget_free_list;
extern int Hud_target_w, Hud_target_h;

extern shader Training_msg_glass;

extern char **Ai_class_names;
extern char *Submode_text[];
extern char *Strafe_submode_text[];

extern void hud_init_targeting_colors();

/// \brief An abbreviation for "Evaluate Ship as Closest Target", defines a 
///        data structure used to hold the required arguments for evaluating 
///        a prospective closest target to an attacked object.
typedef struct esct
{
	int				team_mask;
	int				filter;
	ship*				shipp;
	float				min_distance;
	int				check_nearest_turret;
	int				attacked_objnum;
	int				check_all_turrets;
	int				turret_attacking_target;		// check that turret is actually attacking the attacked_objnum
} esct;

bool evaluate_ship_as_closest_target(esct *esct);
void	hud_init_targeting();
void	hud_target_next(int team_mask = -1);
void	hud_target_prev(int team_mask = -1);
int	hud_target_closest(int team_mask = -1, int attacked_objnum = -1, int play_fail_sound = TRUE, int filter = 0, int turret_attacking_target = 0);
void	hud_target_in_reticle_old();
void	hud_target_in_reticle_new();
void	hud_target_subsystem_in_reticle();
void	hud_show_targeting_gauges(float frametime);
void	hud_target_targets_target();
void	hud_check_reticle_list();
void	hud_target_closest_locked_missile(object *A);
void	hud_target_missile(object *source_obj, int next_flag);
void	hud_target_next_list(int hostile=1, int next_flag=1, int team_mask = -1, int attacked_objnum = -1, int play_fail_sound = TRUE, int filter = 0, int turret_attacking_target = 0);
int	hud_target_closest_repair_ship(int goal_objnum=-1);
void	hud_target_auto_target_next();
void	hud_process_remote_detonate_missile();

void	hud_target_uninspected_object(int next_flag);
void	hud_target_newest_ship();
void	hud_target_live_turret(int next_flag, int auto_advance=0, int turret_attacking_target=0);

void hud_target_last_transmit_level_init();
void hud_target_last_transmit();
void hud_target_last_transmit_add(int ship_num);

void hud_target_random_ship();

void	hud_target_next_subobject();
void	hud_target_prev_subobject();
void	hud_cease_subsystem_targeting(int print_message=1);
void	hud_cease_targeting();
void	hud_restore_subsystem_target(ship* shipp);
int	subsystem_in_sight(object* objp, ship_subsys* subsys, vec3d *eye, vec3d* subsystem);
vec3d* get_subsystem_world_pos(object* parent_obj, ship_subsys* subsys, vec3d* world_pos);
void	hud_target_change_check();

void hud_show_hostile_triangle();
void hud_start_flash_weapon(int index);
void hud_update_weapon_flash();
void hud_process_homing_missiles(void);

int hud_sensors_ok(ship *sp, int show_msg = 1);
int hud_communications_state(ship *sp);

int hud_get_best_primary_bank(float *range);
void hud_target_toggle_hidden_from_sensors();
int hud_target_invalid_awacs(object *objp);

// functions for hotkey selection sets

extern void hud_target_hotkey_select( int k );
extern void hud_target_hotkey_clear( int k );

extern void hud_target_hotkey_add_remove( int k, object *objp, int how_to_add);
extern void hud_show_selection_set();
extern void hud_show_message_sender();
void			hud_prune_hotkeys();
void			hud_keyed_targets_clear();

// Code to draw filled triangles
void hud_tri(float x1,float y1,float x2,float y2,float x3,float y3);
// Code to draw empty triangles.
void hud_tri_empty(float x1,float y1,float x2,float y2,float x3,float y3);

float hud_find_target_distance( object *targetee, object *targeter );

extern void polish_predicted_target_pos(weapon_info *wip, object *targetp, vec3d *enemy_pos, vec3d *predicted_enemy_pos, float dist_to_enemy, vec3d *last_delta_vec, int num_polish_steps);
void hud_calculate_lead_pos(vec3d *lead_target_pos, vec3d *target_pos, object *targetp, weapon_info	*wip, float dist_to_target, vec3d *rel_pos = NULL);

void hud_stuff_ship_name(char *ship_name_text, ship *shipp);
void hud_stuff_ship_callsign(char *ship_callsign_text, ship *shipp);
void hud_stuff_ship_class(char *ship_class_text, ship *shipp);

#define TARGET_DISPLAY_DIST		(1<<0)
#define TARGET_DISPLAY_DOTS		(1<<1)
#define TARGET_DISPLAY_LEAD		(1<<2)
#define TARGET_DISPLAY_SUBSYS	(1<<3)
#define TARGET_DISPLAY_NAME		(1<<4)
#define TARGET_DISPLAY_CLASS	(1<<5)

typedef struct target_display_info {
	object* objp;
	vertex target_point;
	vec3d target_pos;
	color bracket_clr;
	int correction;
	int flags;
	char name[32];
} target_display_info;

extern SCP_vector<target_display_info> target_display_list;

void hud_target_add_display_list(object *objp, vertex *target_point, vec3d *target_pos, int correction, color *bracket_clr, char *name, int flags);
void hud_target_clear_display_list();

class HudGaugeAutoTarget: public HudGauge
{
protected:
	hud_frames Toggle_frame;

	int Auto_text_offsets[2];
	int Target_text_offsets[2];

	color On_color;
	bool Use_on_color;
	color Off_color;
	bool Use_off_color;
public:
	HudGaugeAutoTarget();
	void initAutoTextOffsets(int x, int y);
	void initTargetTextOffsets(int x, int y);
	void initBitmaps(char *fname);
	void initOnColor(int r, int g, int b, int a);
	void initOffColor(int r, int g, int b, int a);
	void render(float frametime);
	void pageIn();
};

class HudGaugeAutoSpeed: public HudGauge
{
protected:
	hud_frames Toggle_frame;

	int Auto_text_offsets[2];
	int Speed_text_offsets[2];

	color On_color;
	bool Use_on_color;
	color Off_color;
	bool Use_off_color;
public:
	HudGaugeAutoSpeed();
	void initAutoTextOffsets(int x, int y);
	void initSpeedTextOffsets(int x, int y);
	void initBitmaps(char *fname);
	void initOnColor(int r, int g, int b, int a);
	void initOffColor(int r, int g, int b, int a);
	void render(float frametime);
	void pageIn();
};

class HudGaugeCmeasures: public HudGauge
{
protected:
	hud_frames Cmeasure_gauge;
	
	int Cm_text_offsets[2];
	int Cm_text_val_offsets[2];
public:
	HudGaugeCmeasures();
	void initBitmaps(char *fname);
	void initCountTextOffsets(int x, int y);
	void initCountValueOffsets(int x, int y);
	void render(float frametime);
	void pageIn();
};

class HudGaugeAfterburner: public HudGauge
{
protected:
	hud_frames Energy_bar;

	int Energy_h;
public:
	HudGaugeAfterburner();
	void initEnergyHeight(int h);
	void initBitmaps(char *fname);
	void render(float frametime);
	void pageIn();
};

class HudGaugeWeaponEnergy: public HudGauge
{
protected:
	hud_frames Energy_bar;

	int Wenergy_text_offsets[2];
	int Wenergy_h;
	int Text_alignment;

	int Armed_name_offsets[2];
	int Armed_alignment;
	bool Show_armed;
	int Armed_name_h;
	
	bool Always_show_text;
	bool Moving_text;
	bool Show_ballistic;
public:
	HudGaugeWeaponEnergy();
	void initBitmaps(char *fname);
	void initTextOffsets(int x, int y);
	void initEnergyHeight(int h);
	void initAlwaysShowText(bool show_text);
	void initMoveText(bool move_text);
	void initShowBallistics(bool show_ballistics);
	void initAlignments(int text_align, int armed_align);
	void initArmedOffsets(int x, int y, int h, bool show);
	void render(float frametime);
	void pageIn();
};

class HudGaugeWeapons: public HudGauge
{
protected:
	hud_frames primary_top[NUM_HUD_SETTINGS]; // Weapon_gauges[ballistic_hud_index][0]
	int top_offset_x[NUM_HUD_SETTINGS]; // Weapon_gauge_primary_coords[ballistic_hud_index][gr_screen.res][0]

	int Weapon_header_offsets[NUM_HUD_SETTINGS][2];

	hud_frames primary_middle[NUM_HUD_SETTINGS]; // Weapon_gauges[ballistic_hud_index][1]
	hud_frames primary_last[NUM_HUD_SETTINGS]; // New_weapon
	
	// for the rest of the gauge
	int frame_offset_x[NUM_HUD_SETTINGS]; // Weapon_gauge_primary_coords[ballistic_hud_index][gr_screen.res][1][0]

	hud_frames secondary_top[NUM_HUD_SETTINGS];
	hud_frames secondary_middle[NUM_HUD_SETTINGS];
	hud_frames secondary_bottom[NUM_HUD_SETTINGS];
	
	int Weapon_plink_offset_x; // Weapon_plink_coords[gr_screen.res][0][0]
	int Weapon_pname_offset_x; // Weapon_pname_coords[gr_screen.res][0][0]
	int Weapon_pammo_offset_x; 

	int Weapon_sammo_offset_x;
	int Weapon_sname_offset_x;
	int Weapon_sreload_offset_x;
	int Weapon_slinked_offset_x;
	int Weapon_sunlinked_offset_x;

	int top_primary_h;
	int pname_start_offset_y;

	int top_secondary_h;
	int sname_start_offset_y;

	int primary_text_h;

	int secondary_text_h;
public:
	HudGaugeWeapons();
	void initBitmapsPrimaryTop(char *fname, char *fname_ballistic);
	void initBitmapsPrimaryMiddle(char *fname, char *fname_ballistic);
	void initBitmapsPrimaryLast(char *fname, char *fname_ballistic);
	void initBitmapsSecondaryTop(char *fname, char *fname_ballistic);
	void initBitmapsSecondaryMiddle(char *fname, char *fname_ballistic);
	void initBitmapsSecondaryBottom(char *fname, char *fname_ballistic);
	void initTopOffsetX(int x, int x_b);
	void initHeaderOffsets(int x, int y, int x_b, int y_b);
	void initFrameOffsetX(int x, int x_b);
	void initPrimaryWeaponOffsets(int link_x, int name_x, int ammo_x);
	void initSecondaryWeaponOffsets(int ammo_x, int name_x, int reload_x, int linked_x, int unlinked_x);
	void initStartNameOffsetsY(int p_y, int s_y);
	void initPrimaryHeights(int top_h, int text_h);
	void initSecondaryHeights(int top_h, int text_h);

	void render(float frametime);
	void pageIn();
	void maybeFlashWeapon(int index);
};

class HudGaugeWeaponList: public HudGauge
{
protected:
	hud_frames _background_first;
	hud_frames _background_entry;
	hud_frames _background_last;

	int _bg_first_offset_x;
	int _bg_entry_offset_x;
	int _bg_last_offset_x;
	int _bg_last_offset_y;

	int _background_first_h;
	int _background_entry_h;

	int _header_offsets[2];
	int _entry_start_y;
	int _entry_h;

	char header_text[NAME_LENGTH];
public:
	HudGaugeWeaponList(int gauge_object);
	void initBitmaps(char *fname_first, char *fname_entry, char *fname_last);
	void initBgFirstOffsetX(int x);
	void initBgEntryOffsetX(int x);
	void initBgLastOffsetX(int x);
	void initBgLastOffsetY(int x);
	void initBgFirstHeight(int h);
	void initBgEntryHeight(int h);
	void initHeaderText(char *text);
	void initHeaderOffsets(int x, int y);
	void initEntryStartY(int y);
	void initEntryHeight(int h);

	virtual void render(float frametime);
	void pageIn();
	void maybeFlashWeapon(int index);
};

class HudGaugePrimaryWeapons: public HudGaugeWeaponList
{
protected:
	int _plink_offset_x; 
	int _pname_offset_x; 
	int _pammo_offset_x; 
public:
	HudGaugePrimaryWeapons();
	void initPrimaryLinkOffsetX(int x);
	void initPrimaryNameOffsetX(int x);
	void initPrimaryAmmoOffsetX(int x);

	void render(float frametime);
};

class HudGaugeSecondaryWeapons: public HudGaugeWeaponList
{
protected:
	int _sammo_offset_x;
	int _sname_offset_x;
	int _sreload_offset_x;
	int _slinked_offset_x;
	int _sunlinked_offset_x;
public:
	HudGaugeSecondaryWeapons();
	void initSecondaryAmmoOffsetX(int x);
	void initSecondaryNameOffsetX(int x);
	void initSecondaryReloadOffsetX(int x);
	void initSecondaryLinkedOffsetX(int x);
	void initSecondaryUnlinkedOffsetX(int x);

	void render(float frametime);
};

class HudGaugeHardpoints: public HudGauge
{
	int _size[2];
	float _line_width;
	int _view_direction;

	bool draw_secondary_models;
	bool draw_primary_models;
public:
	enum {TOP, FRONT};

	void initSizes(int w, int h);
	void initLineWidth(float w);
	void initViewDir(int dir);
	void initDrawOptions(bool primary_models, bool secondary_models);

	HudGaugeHardpoints();
	void render(float frametime);
};

class HudGaugeWarheadCount: public HudGauge
{
	hud_frames Warhead;
	
	int Warhead_name_offsets[2];
	int Warhead_count_offsets[2];
	int Warhead_count_size[2];

	int Max_symbols;
	int Text_align;
	int Max_columns;
public:
	HudGaugeWarheadCount();
	void initBitmap(char *fname);
	void initNameOffsets(int x, int y);
	void initCountOffsets(int x, int y);
	void initCountSizes(int w, int h);
	void initMaxSymbols(int count);
	void initMaxColumns(int count);
	void initTextAlign(int align);
	void render(float frametime);
	void pageIn();
};

class HudGaugeOrientationTee: public HudGauge
{
protected:
	int Radius;
public:
	HudGaugeOrientationTee();
	void initRadius(int length);
	void render(float frametime);
	void renderOrientation(object *from_objp, object *to_objp, matrix *from_orientp);
	void pageIn();
};

class HudGaugeReticleTriangle: public HudGauge
{
protected:
	int Radius; 
	float Target_triangle_base;
	float Target_triangle_height;
public:
	HudGaugeReticleTriangle();
	HudGaugeReticleTriangle(int _gauge_object, int _gauge_config);
	void initRadius(int length);
	void initTriBase(float length);
	void initTriHeight(float h);
	virtual void render(float frametime);
	void renderTriangle(vec3d *hostile_pos, int aspect_flag, int show_interior, int split_tri);
	void renderTriangleMissileTail(float ang, float xpos, float ypos, float cur_dist, int draw_solid, int draw_inside);
};

class HudGaugeHostileTriangle: public HudGaugeReticleTriangle
{
protected:
public:
	HudGaugeHostileTriangle();
	void render(float frametime);
};

class HudGaugeTargetTriangle: public HudGaugeReticleTriangle
{
protected:
public:
	HudGaugeTargetTriangle();
	void render(float frametime);
};

class HudGaugeMissileTriangles: public HudGaugeReticleTriangle
{
protected:
public:
	HudGaugeMissileTriangles();
	void render(float frametime);
};

class HudGaugeOffscreen: public HudGauge
{
protected:
	float Max_offscreen_tri_seperation;
	float Max_front_seperation;
	float Offscreen_tri_base;
	float Offscreen_tri_height;
public:
	HudGaugeOffscreen();
	void initMaxTriSeperation(float length);
	void initMaxFrontSeperation(float length);
	void initTriBase(float length);
	void initTriHeight(float length);
	void render(float frametime);
	void calculatePosition(vertex* target_point, vec3d *tpos, vec2d *outcoords, int *dir, float *half_triangle_sep);
	void renderOffscreenIndicator(vec2d *coords, int dir, float distance, float half_triangle_sep, bool draw_solid = true);
	void pageIn();
};

class HudGaugeLeadIndicator: public HudGauge
{
protected:
	hud_frames Lead_indicator_gauge;
	float Lead_indicator_half[2];
public:
	HudGaugeLeadIndicator();
	void initHalfSize(float w, float h);
	void initBitmaps(char *fname);
	void render(float frametime);
	void renderIndicator(int frame_offset, object *targetp, vec3d *lead_target_pos);
	void renderLeadCurrentTarget();
	void renderLeadQuick(vec3d *target_pos, object *targetp);
	int pickFrame(float prange, float srange, float dist_to_target);
	void pageIn();
};

class HudGaugeLeadSight: public HudGauge
{
protected:
	hud_frames Lead_sight;
	int Lead_sight_half[2];
public:
	HudGaugeLeadSight();
	void initBitmaps(char *fname);
	void render(float frametime);
	void renderSight(int indicator_frame, vec3d *target_pos, vec3d *lead_target_pos);
	void pageIn();
};

#endif
