



#ifndef _HUDPARSE_H
#define _HUDPARSE_H

#include "ai/ai.h"
#include "globalincs/globals.h"
#include "graphics/2d.h"

//Variables
extern int Num_custom_gauges;
extern float Hud_unit_multiplier;
extern float Hud_speed_multiplier;

#define NUM_HUD_RETICLE_STYLES	2

#define HUD_RETICLE_STYLE_FS1	0
#define HUD_RETICLE_STYLE_FS2	1

extern int Hud_reticle_style;
extern bool Scale_retail_gauges;
extern int Force_scaling_above_res_global[2];
extern int Hud_font;
extern bool Chase_view_only_ex;

typedef struct gauge_settings {
	int base_res[2];
	int font_num;
	bool scale_gauge;
	int force_scaling_above_res[2];
	float aspect_quotient;
	SCP_vector<int>* ship_idx;
	color *use_clr;
	float origin[2];
	int offset[2];
	bool use_coords;
	int coords[2];
	bool set_position;
	bool set_colour;
	bool slew;
	bool chase_view_only;
	int cockpit_view_choice;

	gauge_settings()
		: font_num(Hud_font), scale_gauge(Scale_retail_gauges), ship_idx(nullptr), use_clr(nullptr), use_coords(false),
		  set_position(true), set_colour(true), slew(false), chase_view_only(Chase_view_only_ex), cockpit_view_choice(0)
	{
		base_res[0] = -1;
		base_res[1] = -1;
		memcpy(force_scaling_above_res, Force_scaling_above_res_global, sizeof(force_scaling_above_res));
		aspect_quotient = 1.0f;
		origin[0] = 0.0f;
		origin[1] = 0.0f;
		offset[0] = 0;
		offset[1] = 0;
		coords[0] = 0;
		coords[1] = 0;
	}
} gauge_settings;

//Functions
void hud_positions_init();
void set_current_hud();
void init_hud();
void load_missing_retail_gauges();
void check_color(int *colorp);

int parse_gauge_type();
void load_gauge(int gauge, gauge_settings* settings);

#define HUD_OBJECT_CUSTOM				0
void load_gauge_custom(gauge_settings* settings);

#define HUD_OBJECT_MESSAGES				1
void load_gauge_messages(gauge_settings* settings);

#define HUD_OBJECT_TRAINING_MESSAGES		2
void load_gauge_training_messages(gauge_settings* settings);

#define HUD_OBJECT_SUPPORT				3
void load_gauge_support(gauge_settings* settings);

#define HUD_OBJECT_DAMAGE				4
void load_gauge_damage(gauge_settings* settings);

#define HUD_OBJECT_WINGMAN_STATUS		5
void load_gauge_wingman_status(gauge_settings* settings);

#define HUD_OBJECT_AUTO_SPEED			6
void load_gauge_auto_speed(gauge_settings* settings);

#define HUD_OBJECT_AUTO_TARGET			7
void load_gauge_auto_target(gauge_settings* settings);

#define HUD_OBJECT_CMEASURES				8
void load_gauge_countermeasures(gauge_settings* settings);

#define HUD_OBJECT_TALKING_HEAD			9
void load_gauge_talking_head(gauge_settings* settings);

#define HUD_OBJECT_DIRECTIVES			10
void load_gauge_directives(gauge_settings* settings);

#define HUD_OBJECT_WEAPONS				11
void load_gauge_weapons(gauge_settings* settings);

#define HUD_OBJECT_OBJ_NOTIFY			12
void load_gauge_objective_notify(gauge_settings* settings);

#define HUD_OBJECT_SQUAD_MSG				13
void load_gauge_squad_message(gauge_settings* settings);

#define HUD_OBJECT_LAG					14
void load_gauge_lag(gauge_settings* settings);

#define HUD_OBJECT_MINI_SHIELD			15
void load_gauge_mini_shields(gauge_settings* settings);

#define HUD_OBJECT_PLAYER_SHIELD			16
void load_gauge_player_shields(gauge_settings* settings);

#define HUD_OBJECT_TARGET_SHIELD			17
void load_gauge_target_shields(gauge_settings* settings);

#define HUD_OBJECT_ESCORT				18
void load_gauge_escort_view(gauge_settings* settings);

#define HUD_OBJECT_MISSION_TIME			19
void load_gauge_mission_time(gauge_settings* settings);

#define HUD_OBJECT_ETS_WEAPONS			20
void load_gauge_ets_weapons(gauge_settings* settings);

#define HUD_OBJECT_ETS_SHIELDS			21
void load_gauge_ets_shields(gauge_settings* settings);

#define HUD_OBJECT_ETS_ENGINES			22
void load_gauge_ets_engines(gauge_settings* settings);

#define HUD_OBJECT_TARGET_MONITOR		23
void load_gauge_target_monitor(gauge_settings* settings);

#define HUD_OBJECT_EXTRA_TARGET_DATA		24
void load_gauge_extra_target_data(gauge_settings* settings);

#define HUD_OBJECT_RADAR_STD				25
void load_gauge_radar_std(gauge_settings* settings);

#define HUD_OBJECT_RADAR_ORB				26
void load_gauge_radar_orb(gauge_settings* settings);

#define HUD_OBJECT_RADAR_BSG				27
void load_gauge_radar_dradis(gauge_settings* settings);

#define HUD_OBJECT_AFTERBURNER			28
void load_gauge_afterburner(gauge_settings* settings);

#define HUD_OBJECT_WEAPON_ENERGY			29
void load_gauge_weapon_energy(gauge_settings* settings);

#define HUD_OBJECT_TEXT_WARNINGS			30
void load_gauge_text_warnings(gauge_settings* settings);

#define HUD_OBJECT_CENTER_RETICLE		31
void load_gauge_center_reticle(gauge_settings* settings);

#define HUD_OBJECT_THROTTLE				32
void load_gauge_throttle(gauge_settings* settings);

#define HUD_OBJECT_THREAT				33
void load_gauge_threat_indicator(gauge_settings* settings);

#define HUD_OBJECT_LEAD					34
void load_gauge_lead(gauge_settings* settings);

#define HUD_OBJECT_LEAD_SIGHT			35
void load_gauge_lead_sight(gauge_settings* settings);

#define HUD_OBJECT_LOCK					36
void load_gauge_lock(gauge_settings* settings);

#define HUD_OBJECT_WEAPON_LINKING		37
void load_gauge_weapon_linking(gauge_settings* settings);

#define HUD_OBJECT_MULTI_MSG				38
void load_gauge_multi_msg(gauge_settings* settings);

#define HUD_OBJECT_VOICE_STATUS			39
void load_gauge_voice_status(gauge_settings* settings);

#define HUD_OBJECT_PING					40
void load_gauge_ping(gauge_settings* settings);

#define HUD_OBJECT_SUPERNOVA				41
void load_gauge_supernova(gauge_settings* settings);

#define HUD_OBJECT_OFFSCREEN				42
void load_gauge_offscreen(gauge_settings* settings);

#define HUD_OBJECT_BRACKETS				43
void load_gauge_brackets(gauge_settings* settings);

#define HUD_OBJECT_ORIENTATION_TEE		44
void load_gauge_orientation_tee(gauge_settings* settings);

#define HUD_OBJECT_HOSTILE_TRI			45
void load_gauge_hostile_tri(gauge_settings* settings);

#define HUD_OBJECT_TARGET_TRI			46
void load_gauge_target_tri(gauge_settings* settings);

#define HUD_OBJECT_MISSILE_TRI			47
void load_gauge_missile_tri(gauge_settings* settings);

#define HUD_OBJECT_KILLS				48
void load_gauge_kills(gauge_settings* settings);

#define HUD_OBJECT_FIXED_MESSAGES		49
void load_gauge_fixed_messages(gauge_settings* settings);

#define HUD_OBJECT_ETS_RETAIL			50
void load_gauge_ets_retail(gauge_settings* settings);

#define HUD_OBJECT_FLIGHT_PATH			51
void load_gauge_flight_path(gauge_settings* settings);

#define HUD_OBJECT_WARHEAD_COUNT		52
void load_gauge_warhead_count(gauge_settings* settings);

#define HUD_OBJECT_HARDPOINTS			53
void load_gauge_hardpoints(gauge_settings* settings);

#define HUD_OBJECT_PRIMARY_WEAPONS		54
void load_gauge_primary_weapons(gauge_settings* settings);

#define HUD_OBJECT_SECONDARY_WEAPONS	55
void load_gauge_secondary_weapons(gauge_settings* settings);

#define HUD_OBJECT_SCRIPTING 56
void load_gauge_scripting(gauge_settings* settings);

#endif // _HUDPARSE_H
