



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

//Functions
int hud_get_gauge_index(char* name);
void hud_positions_init();
void set_current_hud();
void init_hud();
void load_missing_retail_gauges();
void check_color(int *colorp);

#define NUM_HUD_OBJECT_ENTRIES			56		// not used anywhere?
int parse_gauge_type();
void load_gauge(int gauge, int base_w = -1, int base_h = -1, int font = -1, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_CUSTOM				0
void load_gauge_custom(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_MESSAGES				1
void load_gauge_messages(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_TRAINING_MESSAGES		2
void load_gauge_training_messages(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_SUPPORT				3
void load_gauge_support(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_DAMAGE				4
void load_gauge_damage(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_WINGMAN_STATUS		5
void load_gauge_wingman_status(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_AUTO_SPEED			6
void load_gauge_auto_speed(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_AUTO_TARGET			7
void load_gauge_auto_target(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_CMEASURES				8
void load_gauge_countermeasures(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_TALKING_HEAD			9
void load_gauge_talking_head(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_DIRECTIVES			10
void load_gauge_directives(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_WEAPONS				11
void load_gauge_weapons(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_OBJ_NOTIFY			12
void load_gauge_objective_notify(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_SQUAD_MSG				13
void load_gauge_squad_message(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_LAG					14
void load_gauge_lag(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_MINI_SHIELD			15
void load_gauge_mini_shields(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_PLAYER_SHIELD			16
void load_gauge_player_shields(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_TARGET_SHIELD			17
void load_gauge_target_shields(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_ESCORT				18
void load_gauge_escort_view(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_MISSION_TIME			19
void load_gauge_mission_time(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_ETS_WEAPONS			20
void load_gauge_ets_weapons(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_ETS_SHIELDS			21
void load_gauge_ets_shields(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_ETS_ENGINES			22
void load_gauge_ets_engines(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_TARGET_MONITOR		23
void load_gauge_target_monitor(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_EXTRA_TARGET_DATA		24
void load_gauge_extra_target_data(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_RADAR_STD				25
void load_gauge_radar_std(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_RADAR_ORB				26
void load_gauge_radar_orb(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_RADAR_BSG				27
void load_gauge_radar_dradis(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_AFTERBURNER			28
void load_gauge_afterburner(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_WEAPON_ENERGY			29
void load_gauge_weapon_energy(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_TEXT_WARNINGS			30
void load_gauge_text_warnings(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_CENTER_RETICLE		31
void load_gauge_center_reticle(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_THROTTLE				32
void load_gauge_throttle(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_THREAT				33
void load_gauge_threat_indicator(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_LEAD					34
void load_gauge_lead(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_LEAD_SIGHT			35
void load_gauge_lead_sight(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_LOCK					36
void load_gauge_lock(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_WEAPON_LINKING		37
void load_gauge_weapon_linking(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_MULTI_MSG				38
void load_gauge_multi_msg(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_VOICE_STATUS			39
void load_gauge_voice_status(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_PING					40
void load_gauge_ping(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_SUPERNOVA				41
void load_gauge_supernova(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_OFFSCREEN				42
void load_gauge_offscreen(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_BRACKETS				43
void load_gauge_brackets(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_ORIENTATION_TEE		44
void load_gauge_orientation_tee(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_HOSTILE_TRI			45
void load_gauge_hostile_tri(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_TARGET_TRI			46
void load_gauge_target_tri(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_MISSILE_TRI			47
void load_gauge_missile_tri(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_KILLS				48
void load_gauge_kills(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_FIXED_MESSAGES		49
void load_gauge_fixed_messages(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_ETS_RETAIL			50
void load_gauge_ets_retail(int base_w, int base_h, int hud_font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_FLIGHT_PATH			51
void load_gauge_flight_path(int base_w, int base_h, int font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_WARHEAD_COUNT		52
void load_gauge_warhead_count(int base_w, int base_h, int font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_HARDPOINTS			53
void load_gauge_hardpoints(int base_w, int base_h, int font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_PRIMARY_WEAPONS		54
void load_gauge_primary_weapons(int base_w, int base_h, int font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#define HUD_OBJECT_SECONDARY_WEAPONS	55
void load_gauge_secondary_weapons(int base_w, int base_h, int font, bool scale_gauge = true, SCP_vector<int>* ship_idx = NULL, color *use_clr = NULL);

#endif // _HUDPARSE_H
