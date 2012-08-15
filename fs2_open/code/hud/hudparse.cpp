/*
 * Created by WMCoolmon for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

 

#include <cstddef>

#include "parse/parselo.h"
#include "graphics/2d.h"
#include "localization/localize.h"
#include "hud/hud.h"
#include "hud/hudescort.h"
#include "hud/hudets.h"
#include "hud/hudmessage.h"
#include "hud/hudreticle.h"
#include "hud/hudshield.h"
#include "hud/hudsquadmsg.h"
#include "hud/hudtarget.h"
#include "hud/hudwingmanstatus.h"
#include "hud/hudbrackets.h"
#include "hud/hudlock.h"
#include "mission/missiontraining.h"
#include "mission/missionmessage.h"
#include "hud/hudparse.h" //Duh.
#include "radar/radarsetup.h"
#include "radar/radar.h"
#include "radar/radarorb.h"
#include "radar/radardradis.h"
#include "ship/ship.h" //for ship struct
#include "graphics/font.h" //for gr_force_fit_string
#include "hud/hudtargetbox.h" 
#include "cmdline/cmdline.h"
#include "hud/hudconfig.h" // for retrieving user's hud config

//Global stuffs
extern int ships_inited; //Need this

float Hud_unit_multiplier = 1.0f;	//Backslash
float Hud_speed_multiplier = 1.0f;	//The E

// Goober5000
int Hud_reticle_style = HUD_RETICLE_STYLE_FS2;

bool Hud_retail = true;

int Hud_font = -1;

//WARNING: If you add gauges to this array, make sure to bump num_default_gauges!
int num_default_gauges = 42;
static int retail_gauges[] = {
	HUD_OBJECT_MESSAGES,
	HUD_OBJECT_TRAINING_MESSAGES,
	HUD_OBJECT_SUPPORT,
	HUD_OBJECT_DAMAGE,
	HUD_OBJECT_WINGMAN_STATUS,
	HUD_OBJECT_AUTO_SPEED,
	HUD_OBJECT_AUTO_TARGET,
	HUD_OBJECT_CMEASURES,
	HUD_OBJECT_TALKING_HEAD,
	HUD_OBJECT_DIRECTIVES,
	HUD_OBJECT_WEAPONS,
	HUD_OBJECT_OBJ_NOTIFY,
	HUD_OBJECT_SQUAD_MSG,
	HUD_OBJECT_LAG,
	HUD_OBJECT_MINI_SHIELD,
	HUD_OBJECT_PLAYER_SHIELD,
	HUD_OBJECT_TARGET_SHIELD,
	HUD_OBJECT_ESCORT,
	HUD_OBJECT_MISSION_TIME,
	HUD_OBJECT_TARGET_MONITOR,
	HUD_OBJECT_EXTRA_TARGET_DATA,
	HUD_OBJECT_AFTERBURNER,
	HUD_OBJECT_WEAPON_ENERGY,
	HUD_OBJECT_TEXT_WARNINGS,
	HUD_OBJECT_CENTER_RETICLE,
	HUD_OBJECT_THROTTLE,
	HUD_OBJECT_THREAT,
	HUD_OBJECT_LEAD,
	HUD_OBJECT_LOCK,
	HUD_OBJECT_MULTI_MSG,
	HUD_OBJECT_VOICE_STATUS,
	HUD_OBJECT_PING,
	HUD_OBJECT_SUPERNOVA,
	HUD_OBJECT_OFFSCREEN,
	HUD_OBJECT_BRACKETS,
	HUD_OBJECT_ORIENTATION_TEE,
	HUD_OBJECT_HOSTILE_TRI,
	HUD_OBJECT_TARGET_TRI,
	HUD_OBJECT_MISSILE_TRI,
	HUD_OBJECT_KILLS,
	HUD_OBJECT_FIXED_MESSAGES,
	HUD_OBJECT_ETS_RETAIL
};

flag_def_list Hud_gauge_types[] = {
	{ "Messages",			HUD_OBJECT_MESSAGES,			0},
	{ "Training messages",	HUD_OBJECT_TRAINING_MESSAGES,	0},
	{ "Support",			HUD_OBJECT_SUPPORT,				0},
	{ "Damage",				HUD_OBJECT_DAMAGE,				0},
	{ "Wingman status",		HUD_OBJECT_WINGMAN_STATUS,		0},
	{ "Auto speed",			HUD_OBJECT_AUTO_SPEED,			0},
	{ "Auto target",		HUD_OBJECT_AUTO_TARGET,			0},
	{ "Countermeasures",	HUD_OBJECT_CMEASURES,			0},
	{ "Talking head",		HUD_OBJECT_TALKING_HEAD,		0},
	{ "Directives",			HUD_OBJECT_DIRECTIVES,			0},
	{ "Weapons",			HUD_OBJECT_WEAPONS,				0},
	{ "Objective notifier",	HUD_OBJECT_OBJ_NOTIFY,			0},
	{ "Comm menu",			HUD_OBJECT_SQUAD_MSG,			0},
	{ "Lag indicator",		HUD_OBJECT_LAG,					0},
	{ "Mini shield",		HUD_OBJECT_MINI_SHIELD,			0},
	{ "Player shield",		HUD_OBJECT_PLAYER_SHIELD,		0},
	{ "Target shield",		HUD_OBJECT_TARGET_SHIELD,		0},
	{ "Escort list",		HUD_OBJECT_ESCORT,				0},
	{ "Mission time",		HUD_OBJECT_MISSION_TIME,		0},
	{ "Ets weapons",		HUD_OBJECT_ETS_WEAPONS,			0},
	{ "Ets shields",		HUD_OBJECT_ETS_SHIELDS,			0},
	{ "Ets engines",		HUD_OBJECT_ETS_ENGINES,			0},
	{ "Target monitor",		HUD_OBJECT_TARGET_MONITOR,		0},
	{ "Extra target data",	HUD_OBJECT_EXTRA_TARGET_DATA,	0},
	{ "Radar",				HUD_OBJECT_RADAR_STD,			0},
	{ "Radar orb",			HUD_OBJECT_RADAR_ORB,			0},
	{ "Radar BSG",			HUD_OBJECT_RADAR_BSG,			0},
	{ "Afterburner energy",	HUD_OBJECT_AFTERBURNER,			0},
	{ "Weapon energy",		HUD_OBJECT_WEAPON_ENERGY,		0},
	{ "Text warnings",		HUD_OBJECT_TEXT_WARNINGS,		0},
	{ "Center reticle",		HUD_OBJECT_CENTER_RETICLE,		0},
	{ "Throttle",			HUD_OBJECT_THROTTLE,			0},
	{ "Threat indicator",	HUD_OBJECT_THREAT,				0},
	{ "Lead indicator",		HUD_OBJECT_LEAD,				0},
	{ "Lead sight",			HUD_OBJECT_LEAD_SIGHT,			0},
	{ "Lock indicator",		HUD_OBJECT_LOCK,				0},
	{ "Weapon linking",		HUD_OBJECT_WEAPON_LINKING,		0},
	{ "Multiplayer messages",	HUD_OBJECT_MULTI_MSG,			0},
	{ "Voice status",		HUD_OBJECT_VOICE_STATUS,		0},
	{ "Ping",				HUD_OBJECT_PING,				0},
	{ "Supernova",			HUD_OBJECT_SUPERNOVA,			0},
	{ "Offscreen indicator",	HUD_OBJECT_OFFSCREEN,			0},
	{ "Targeting brackets",	HUD_OBJECT_BRACKETS,			0},
	{ "Orientation",		HUD_OBJECT_ORIENTATION_TEE,		0},
	{ "Hostile direction",	HUD_OBJECT_HOSTILE_TRI,			0},
	{ "Target direction",	HUD_OBJECT_TARGET_TRI,			0},
	{ "Missile indicator",	HUD_OBJECT_MISSILE_TRI,			0},
	{ "Kills",				HUD_OBJECT_KILLS,				0},
	{ "Fixed messages",		HUD_OBJECT_FIXED_MESSAGES,		0},
	{ "Ets retail",			HUD_OBJECT_ETS_RETAIL,			0}
};

int Num_hud_gauge_types = sizeof(Hud_gauge_types)/sizeof(flag_def_list);

int parse_ship_start()
{
	char shipname[NAME_LENGTH];
	int ship_index;
	
	stuff_string(shipname, F_NAME, NAME_LENGTH);
	ship_index = ship_info_lookup(shipname);

	return ship_index;
}

void parse_hud_gauges_tbl(char *filename)
{
	int rval;
	char *saved_Mp = NULL;

	int colors[3] = {255, 255, 255};
	color hud_color;
	color ship_color;
	color *hud_clr_p = NULL;
	color *ship_clr_p = NULL;

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", filename, rval));
		lcl_ext_close();
		return;
	}

	read_file_text(filename, CF_TYPE_TABLES);
	reset_parse();

	if(optional_string("$Load Retail Configuration:")) {
		stuff_boolean(&Hud_retail);
	}

	if ( optional_string("$Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);
		gr_init_alphacolor(&hud_color, colors[0], colors[1], colors[2], 255);
		hud_clr_p = &hud_color;
	}

	if(optional_string("$Font:")) {
		stuff_int(&Hud_font);
	}
	
	if(optional_string("$Max Directives:")) {
		stuff_int(&Max_directives);
	}

	if(optional_string("$Max Escort Ships:")) {
		stuff_int(&Max_escort_ships);
	}

	if(optional_string("$Length Unit Multiplier:"))	{
		stuff_float(&Hud_unit_multiplier);

		if (Hud_unit_multiplier <= 0.0f) {
			Warning(LOCATION, "\"$Length Unit Multiplier:\" value of \"%f\" is invalid!  Resetting to default.", Hud_unit_multiplier);
			Hud_unit_multiplier = 1.0f;
		}
	}

	if(optional_string("$Speed Unit Multiplier:")) {
		stuff_float(&Hud_speed_multiplier);

		if (Hud_speed_multiplier <= 0.0f) {
			Warning(LOCATION, "\"$Speed Unit Multiplier:\" value of \"%f\" is invalid!  Resetting to default.", Hud_speed_multiplier);
			Hud_speed_multiplier = 1.0f;
		}
	} else {
		Hud_speed_multiplier = Hud_unit_multiplier;
	} 

	if (optional_string("$Wireframe Targetbox:")) {
		stuff_int(&Targetbox_wire);
		if ((Targetbox_wire < 0) || (Targetbox_wire > 3)) {
			Targetbox_wire = 0;
		}
	}

	if (optional_string("$Targetbox Shader Effect:")) {
		stuff_int(&Targetbox_shader_effect);
		if (Targetbox_shader_effect < 0) {
			Targetbox_shader_effect = 0;
		}
	}

	if (optional_string("$Lock Wireframe Mode:")) {
		stuff_boolean(&Lock_targetbox_mode);
	}

	if(optional_string("$Reticle Style:")) {
		int temp = required_string_either("FS1", "FS2"); 

		// using require_string_either won't advance the Mp pointer to the next token so force it instead
		skip_to_start_of_string("#Gauge Config");

		if (temp < 0)
			Warning(LOCATION, "Undefined reticle style in hud_gauges.tbl!");
		else
			Hud_reticle_style = temp;
	}

	int base_res[2];
	int ship_idx = -1;
	int ship_font = -1;
	int gauge_type = -1;
	int use_font = -1;
	color *use_clr_p = NULL;

	while(optional_string("#Gauge Config")) {
		if(optional_string("$Ship:")) {

			if(!ships_inited) {
				// just in case ship info has not been initialized.
				skip_to_start_of_string("#Gauge Config");
				continue;
			}

			// get the ship number for this HUD configuration.	
			ship_idx = parse_ship_start();

			if(ship_idx >= 0) {
				Ship_info[ship_idx].hud_enabled = true;

				// see if we need to load defaults for this configuration
				if(optional_string("$Load Retail Configuration:")) {
					stuff_boolean(&Ship_info[ship_idx].hud_retail);
				}

				if ( optional_string("$Color:") ) {
					stuff_int_list(colors, 3);

					check_color(colors);
					gr_init_alphacolor(&ship_color, colors[0], colors[1], colors[2], 255);
					ship_clr_p = &ship_color;
				}

				if(optional_string("$Font:")) {
					stuff_int(&ship_font);
				}
			} else {
				// can't find ship class. move on.
				skip_to_start_of_string("#Gauge Config");
				//skip_to_start_of_string_either("#Gauge Config", "#End");
				continue;
			}
		} else {
			// No particular ship. -1 for default HUD configuration.
			ship_idx = -1;
			ship_font = -1;
			ship_clr_p = NULL;
		}

		if ( ship_clr_p != NULL ) {
			use_clr_p = ship_clr_p;
		} else {
			use_clr_p = hud_clr_p;
		}

		if(ship_font >= 0) {
			use_font = ship_font;
		} else {
			use_font = Hud_font;
		}

		// Now start going through resolution info for this HUD layout
		required_string("$Base:");

		// get the base width and height describing this HUD
		stuff_int_list(base_res, 2, RAW_INTEGER_TYPE);

		// Pruning time. Let's see if the current resolution defined by the user matches the conditions set by this entry
		if(optional_string("$Required Aspect:")) {
			// filter aspect ratio.
			if(optional_string("Full Screen")) {
				if( (float)gr_screen.max_w / (float)gr_screen.max_h > 1.5) {
					skip_to_start_of_string("#Gauge Config");
					//skip_to_start_of_string_either("#Gauge Config", "#End");
					continue;
				}
			} else if(optional_string("Wide Screen")) {
				if( (float)gr_screen.max_w / (float)gr_screen.max_h <= 1.5) {
					skip_to_start_of_string("#Gauge Config");
					//skip_to_start_of_string_either("#Gauge Config", "#End");
					continue;
				}
			} 
		}
		
		// check minimum resolution
		if(optional_string("$Min:")) {
			int min_res[2];
			stuff_int_list(min_res, 2, RAW_INTEGER_TYPE);
			
			if(min_res[0] > gr_screen.max_w) {
				skip_to_start_of_string("#Gauge Config");
				continue;
			} else if (min_res[0] == gr_screen.max_w) {
				if(min_res[1] > gr_screen.max_h) {
					skip_to_start_of_string("#Gauge Config");
					continue;
				}
			}
		}

		// check maximum resolution
		if(optional_string("$Max:")) {
			int max_res[2];
			stuff_int_list(max_res, 2, RAW_INTEGER_TYPE);

			if(max_res[0] < gr_screen.max_w) {
				skip_to_start_of_string("#Gauge Config");
				continue;
			} else if (max_res[0] == gr_screen.max_w) {
				if(max_res[1] < gr_screen.max_h) {
					skip_to_start_of_string("#Gauge Config");
					continue;
				}
			}
		}

		// let's start parsing for gauges.
		required_string("$Gauges:");

		while (!check_for_string("$Gauges:") && !check_for_string("$End Gauges")) {
			// find out what type of gauge we're parsing.
			gauge_type = parse_gauge_type();

			// then call the specific gauge load handler function for this gauge type.
			if(optional_string("default")) {
				// sending -1 base width and height will indicate GR_640 or GR_1024 to the handlers
				load_gauge(gauge_type, -1, -1, use_font, ship_idx, use_clr_p);
			} else {
				load_gauge(gauge_type, base_res[0], base_res[1], use_font, ship_idx, use_clr_p);
			}

			if ( saved_Mp && (saved_Mp == Mp) ) {
				Mp++;
			}

			skip_to_start_of_string_either("$", "+");
			// stolened from AI_profiles
			// if we've been through once already and are at the same place, force a move

			saved_Mp = Mp;
		}

		required_string("$End Gauges");
		required_string("#End");
	}

	// close localization
	lcl_ext_close();
}

void hud_positions_init()
{
	if(!ships_inited)
	{
		Error(LOCATION, "Could not initialize hudparse.cpp as ships were not inited first.");
		return;
	}

	default_hud_gauges.clear();

	if (cf_exists_full("hud_gauges.tbl", CF_TYPE_TABLES))
		parse_hud_gauges_tbl("hud_gauges.tbl");

	parse_modular_table(NOX("*-hdg.tbm"), parse_hud_gauges_tbl);

	// load missing retail gauges for the default and ship-specific HUDs
	load_missing_retail_gauges();
	
}

void load_missing_retail_gauges()
{
	bool retail_gauge_loaded = false;

	// load missing retail gauges for the retail HUD if needed
	if(Hud_retail) {
		int num_loaded_gauges = (int)default_hud_gauges.size();

		for(int i = 0; i < num_default_gauges; i++) {
			retail_gauge_loaded = false;

			for(int j = 0; j < num_loaded_gauges; j++) {
				if(retail_gauges[i] == default_hud_gauges[j]->getObjectType()) {
					retail_gauge_loaded = true;
					break;
				}
			}

			if(!retail_gauge_loaded) {
				load_gauge(retail_gauges[i], -1, -1, Hud_font);
			}
		}

		// if we're missing a radar gauge, load either orb or standard
		retail_gauge_loaded = false;
		for(int j = 0; j < num_loaded_gauges; j++) {
			if(HUD_OBJECT_RADAR_STD == default_hud_gauges[j]->getObjectType() ||
				HUD_OBJECT_RADAR_ORB == default_hud_gauges[j]->getObjectType() ||
				HUD_OBJECT_RADAR_BSG == default_hud_gauges[j]->getObjectType()) {
				retail_gauge_loaded = true;
			}
		}

		// load radar gauge if not loaded.
		if(!retail_gauge_loaded) {
			if(Cmdline_orb_radar) {
				load_gauge(HUD_OBJECT_RADAR_ORB, -1, -1, Hud_font);
			} else {
				load_gauge(HUD_OBJECT_RADAR_STD, -1, -1, Hud_font);
			}
		}

		// Throw in the weapon linking reticle gauge if using FS1 defaults
		retail_gauge_loaded = false;
		if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
			for(int j = 0; j < num_loaded_gauges; j++) {
				if(HUD_OBJECT_WEAPON_LINKING == default_hud_gauges[j]->getObjectType()) {
					retail_gauge_loaded = true;
				}
			}

			if(!retail_gauge_loaded) {
				load_gauge(HUD_OBJECT_WEAPON_LINKING, -1, -1, Hud_font);
			}
		}
	}

	// for each ship class, check if their specific HUD config is enabled
	for (int k = 0; k < Num_ship_classes; k++) {
		if(Ship_info[k].hud_enabled && Ship_info[k].hud_retail) {
			int num_loaded_gauges = (int)Ship_info[k].hud_gauges.size();

			for(int i = 0; i < num_default_gauges; i++) {
				for(int j = 0; j < num_loaded_gauges; j++) {
					if(retail_gauges[i] == Ship_info[k].hud_gauges[j]->getObjectType()) {
						retail_gauge_loaded = true;
					}
				}

				if(!retail_gauge_loaded) {
					load_gauge(retail_gauges[i], -1, -1, Hud_font, k);
				}
			}

			// if we're missing a radar gauge, load either orb or standard
			retail_gauge_loaded = false;
			for(int j = 0; j < num_loaded_gauges; j++) {
				if(HUD_OBJECT_RADAR_ORB == Ship_info[k].hud_gauges[j]->getObjectType() || 
					HUD_OBJECT_RADAR_STD == Ship_info[k].hud_gauges[j]->getObjectType() ||
					HUD_OBJECT_RADAR_BSG == Ship_info[k].hud_gauges[j]->getObjectType()) {
					retail_gauge_loaded = true;
				}
			}

			// load radar gauge if not loaded.
			if(!retail_gauge_loaded) {
				if(Cmdline_orb_radar) {
					load_gauge(HUD_OBJECT_RADAR_ORB, -1, -1, Hud_font, k);
				} else {
					load_gauge(HUD_OBJECT_RADAR_STD, -1, -1, Hud_font, k);
				}
			}

			// Throw in the weapon linking reticle gauge if using FS1 defaults
			retail_gauge_loaded = false;
			if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
				for(int j = 0; j < num_loaded_gauges; j++) {
					if(HUD_OBJECT_WEAPON_LINKING == Ship_info[k].hud_gauges[j]->getObjectType()) {
						retail_gauge_loaded = true;
					}
				}

				if(!retail_gauge_loaded) {
					load_gauge(HUD_OBJECT_WEAPON_LINKING, -1, -1, Hud_font, k);
				}
			}
		}
	}
}

// Called once after mission load is complete. Sets initial gauge activity states.
void init_hud() {
	int i, num_gauges, config_type;

	if(Ship_info[Player_ship->ship_info_index].hud_gauges.size() > 0) {
		num_gauges = Ship_info[Player_ship->ship_info_index].hud_gauges.size();

		for(i = 0; i < num_gauges; i++) {
			config_type = Ship_info[Player_ship->ship_info_index].hud_gauges[i]->getConfigType();

			if ( !Ship_info[Player_ship->ship_info_index].hud_gauges[i]->isOffbyDefault() && hud_config_show_flag_is_set(config_type) )
				Ship_info[Player_ship->ship_info_index].hud_gauges[i]->updateActive(true);
			else
				Ship_info[Player_ship->ship_info_index].hud_gauges[i]->updateActive(false);

			Ship_info[Player_ship->ship_info_index].hud_gauges[i]->updatePopUp(hud_config_popup_flag_is_set(config_type) ? true : false);
			Ship_info[Player_ship->ship_info_index].hud_gauges[i]->updateColor(
				HUD_config.clr[config_type].red, 
				HUD_config.clr[config_type].green, 
				HUD_config.clr[config_type].blue, 
				HUD_config.clr[config_type].alpha
				);
		}
	} else {
		num_gauges = default_hud_gauges.size();

		for(i = 0; i < num_gauges; i++) {
			config_type = default_hud_gauges[i]->getConfigType();

			if ( !default_hud_gauges[i]->isOffbyDefault() && hud_config_show_flag_is_set(config_type) )
				default_hud_gauges[i]->updateActive(true);
			else
				default_hud_gauges[i]->updateActive(false);

			default_hud_gauges[i]->updatePopUp(hud_config_popup_flag_is_set(config_type) ? true : false);
			default_hud_gauges[i]->updateColor(
				HUD_config.clr[config_type].red, 
				HUD_config.clr[config_type].green, 
				HUD_config.clr[config_type].blue, 
				HUD_config.clr[config_type].alpha
				);
		}
	}
}

void set_current_hud()
{
	int i, num_gauges, config_type;

	// go through all HUD gauges. Load gauge properties defined in the HUD config if gauge is not customized.
	if(Ship_info[Player_ship->ship_info_index].hud_gauges.size() > 0) {
		num_gauges = Ship_info[Player_ship->ship_info_index].hud_gauges.size();

		for(i = 0; i < num_gauges; i++) {
			config_type = Ship_info[Player_ship->ship_info_index].hud_gauges[i]->getConfigType();

			if ( ( (!Ship_info[Player_ship->ship_info_index].hud_gauges[i]->isOffbyDefault() || Ship_info[Player_ship->ship_info_index].hud_gauges[i]->isActive()) && hud_config_show_flag_is_set(config_type)) )
				Ship_info[Player_ship->ship_info_index].hud_gauges[i]->updateActive(true);
			else
				Ship_info[Player_ship->ship_info_index].hud_gauges[i]->updateActive(false);

			//Ship_info[Player_ship->ship_info_index].hud_gauges[i]->updateActive(hud_config_show_flag_is_set(config_type) ? true : false);
			Ship_info[Player_ship->ship_info_index].hud_gauges[i]->updatePopUp(hud_config_popup_flag_is_set(config_type) ? true : false);
			Ship_info[Player_ship->ship_info_index].hud_gauges[i]->updateColor(
				HUD_config.clr[config_type].red, 
				HUD_config.clr[config_type].green, 
				HUD_config.clr[config_type].blue, 
				HUD_config.clr[config_type].alpha
				);
		}
	} else {
		num_gauges = default_hud_gauges.size();

		for(i = 0; i < num_gauges; i++) {
			config_type = default_hud_gauges[i]->getConfigType();

			if ( ( (!default_hud_gauges[i]->isOffbyDefault() || default_hud_gauges[i]->isActive()) && hud_config_show_flag_is_set(config_type)) )
				default_hud_gauges[i]->updateActive(true);
			else
				default_hud_gauges[i]->updateActive(false);

			//default_hud_gauges[i]->updateActive(hud_config_show_flag_is_set(config_type) ? true : false);
			default_hud_gauges[i]->updatePopUp(hud_config_popup_flag_is_set(config_type) ? true : false);
			default_hud_gauges[i]->updateColor(
				HUD_config.clr[config_type].red, 
				HUD_config.clr[config_type].green, 
				HUD_config.clr[config_type].blue, 
				HUD_config.clr[config_type].alpha
				);
		}
	}
}

int parse_gauge_type()
{
	// probably a more elegant way to do this. 
	// Likely involving a for loop, an array of strings and only one if statement with a strcmp.
	if(optional_string("+Custom:")) 
		return HUD_OBJECT_CUSTOM;

	if(optional_string("+Messages:")) 
		return HUD_OBJECT_MESSAGES;

	if(optional_string("+Training Messages:")) 
		return HUD_OBJECT_TRAINING_MESSAGES;

	if(optional_string("+Support:")) 
		return HUD_OBJECT_SUPPORT;

	if(optional_string("+Damage:")) 
		return HUD_OBJECT_DAMAGE;

	if(optional_string("+Wingman Status:")) 
		return HUD_OBJECT_WINGMAN_STATUS;

	if(optional_string("+Auto Speed:")) 
		return HUD_OBJECT_AUTO_SPEED;

	if(optional_string("+Auto Target:")) 
		return HUD_OBJECT_AUTO_TARGET;

	if(optional_string("+Countermeasures:")) 
		return HUD_OBJECT_CMEASURES;

	if(optional_string("+Talking Head:")) 
		return HUD_OBJECT_TALKING_HEAD;

	if(optional_string("+Directives:")) 
		return HUD_OBJECT_DIRECTIVES;

	if(optional_string("+Weapons:")) 
		return HUD_OBJECT_WEAPONS;

	if(optional_string("+Objective Notify:")) 
		return HUD_OBJECT_OBJ_NOTIFY;

	if(optional_string("+Squad Message:")) 
		return HUD_OBJECT_SQUAD_MSG;

	if(optional_string("+Lag:")) 
		return HUD_OBJECT_LAG;

	if(optional_string("+Mini Target Shields:")) 
		return HUD_OBJECT_MINI_SHIELD;

	if(optional_string("+Player Shields:")) 
		return HUD_OBJECT_PLAYER_SHIELD;

	if(optional_string("+Target Shields:")) 
		return HUD_OBJECT_TARGET_SHIELD;

	if(optional_string("+Escort View:")) 
		return HUD_OBJECT_ESCORT;

	if(optional_string("+Mission Time:")) 
		return HUD_OBJECT_MISSION_TIME;

	if(optional_string("+ETS Weapons:")) 
		return HUD_OBJECT_ETS_WEAPONS;

	if(optional_string("+ETS Shields:")) 
		return HUD_OBJECT_ETS_SHIELDS;

	if(optional_string("+ETS Engines:")) 
		return HUD_OBJECT_ETS_ENGINES;

	if(optional_string("+ETS Retail:"))
		return HUD_OBJECT_ETS_RETAIL;

	if(optional_string("+Target Monitor:")) 
		return HUD_OBJECT_TARGET_MONITOR;

	if(optional_string("+Extra Target Data:")) 
		return HUD_OBJECT_EXTRA_TARGET_DATA;

	if(optional_string("+Radar:")) {
		if(Cmdline_orb_radar) {
			return HUD_OBJECT_RADAR_ORB;
		} else {
			return HUD_OBJECT_RADAR_STD;
		}
	}

	if(optional_string("+Radar Orb:")) 
		return HUD_OBJECT_RADAR_ORB;

	if(optional_string("+Radar BSG:")) 
		return HUD_OBJECT_RADAR_BSG;

	if(optional_string("+Afterburner Energy:")) 
		return HUD_OBJECT_AFTERBURNER;

	if(optional_string("+Weapon Energy:")) 
		return HUD_OBJECT_WEAPON_ENERGY;

	if(optional_string("+Text Warnings:")) 
		return HUD_OBJECT_TEXT_WARNINGS;

	if(optional_string("+Center Reticle:")) 
		return HUD_OBJECT_CENTER_RETICLE;

	if(optional_string("+Throttle:")) 
		return HUD_OBJECT_THROTTLE;

	if(optional_string("+Threat Indicator:")) 
		return HUD_OBJECT_THREAT;
	
	if(optional_string("+Lead Indicator:"))
		return HUD_OBJECT_LEAD;

	if(optional_string("+Lead Sight:"))
		return HUD_OBJECT_LEAD_SIGHT;

	if(optional_string("+Lock Indicator:"))
		return HUD_OBJECT_LOCK;

	if(optional_string("+Weapon Linking:"))
		return HUD_OBJECT_WEAPON_LINKING;

	if(optional_string("+Multiplayer Messages:"))
		return HUD_OBJECT_MULTI_MSG;

	if(optional_string("+Voice Status:"))
		return HUD_OBJECT_VOICE_STATUS;

	if(optional_string("+Ping:"))
		return HUD_OBJECT_PING;
	
	if(optional_string("+Supernova:"))
		return HUD_OBJECT_SUPERNOVA;

	if(optional_string("+Orientation Tee:"))
		return HUD_OBJECT_ORIENTATION_TEE;

	if(optional_string("+Offscreen Indicator:"))
		return HUD_OBJECT_OFFSCREEN;

	if(optional_string("+Target Brackets:"))
		return HUD_OBJECT_BRACKETS;

	if(optional_string("+Hostile Triangle:"))
		return HUD_OBJECT_HOSTILE_TRI;

	if(optional_string("+Target Triangle:"))
		return HUD_OBJECT_TARGET_TRI;

	if(optional_string("+Missile Triangles:"))
		return HUD_OBJECT_MISSILE_TRI;

	if(optional_string("+Kills:"))
		return HUD_OBJECT_KILLS;

	if(optional_string("+Fixed Messages:"))
		return HUD_OBJECT_FIXED_MESSAGES;

	return -1;
}

void load_gauge(int gauge, int base_w, int base_h, int hud_font, int ship_idx, color *use_clr)
{
	switch(gauge) {
	case HUD_OBJECT_CUSTOM:
		load_gauge_custom(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_MESSAGES:
		load_gauge_messages(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_TRAINING_MESSAGES:
		load_gauge_training_messages(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_SUPPORT:
		load_gauge_support(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_DAMAGE:
		load_gauge_damage(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_WINGMAN_STATUS:
		load_gauge_wingman_status(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_AUTO_SPEED:
		load_gauge_auto_speed(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_AUTO_TARGET:
		load_gauge_auto_target(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_CMEASURES:
		load_gauge_countermeasures(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_TALKING_HEAD:
		load_gauge_talking_head(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_DIRECTIVES:
		load_gauge_directives(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_WEAPONS:
		load_gauge_weapons(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_OBJ_NOTIFY:
		load_gauge_objective_notify(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_SQUAD_MSG:
		load_gauge_squad_message(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_LAG:
		load_gauge_lag(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_MINI_SHIELD:
		load_gauge_mini_shields(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_PLAYER_SHIELD:
		load_gauge_player_shields(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_TARGET_SHIELD:
		load_gauge_target_shields(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_ESCORT:
		load_gauge_escort_view(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_MISSION_TIME:
		load_gauge_mission_time(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_ETS_WEAPONS:
		load_gauge_ets_weapons(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_ETS_SHIELDS:
		load_gauge_ets_shields(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_ETS_ENGINES:
		load_gauge_ets_engines(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_ETS_RETAIL:
		load_gauge_ets_retail(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_TARGET_MONITOR:
		load_gauge_target_monitor(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_EXTRA_TARGET_DATA:
		load_gauge_extra_target_data(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_RADAR_STD:
		load_gauge_radar_std(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_RADAR_ORB:
		load_gauge_radar_orb(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_RADAR_BSG:
		load_gauge_radar_dradis(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_AFTERBURNER:
		load_gauge_afterburner(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_WEAPON_ENERGY:
		load_gauge_weapon_energy(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_TEXT_WARNINGS:
		load_gauge_text_warnings(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_CENTER_RETICLE:
		load_gauge_center_reticle(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_THROTTLE:
		load_gauge_throttle(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_THREAT:
		load_gauge_threat_indicator(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_LEAD:
		load_gauge_lead(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_LEAD_SIGHT:
		load_gauge_lead_sight(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_LOCK:
		load_gauge_lock(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_WEAPON_LINKING:
		load_gauge_weapon_linking(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_MULTI_MSG:
		load_gauge_multi_msg(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_VOICE_STATUS:
		load_gauge_voice_status(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_PING:
		load_gauge_ping(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_SUPERNOVA:
		load_gauge_supernova(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_OFFSCREEN:
		load_gauge_offscreen(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_BRACKETS:
		load_gauge_brackets(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_ORIENTATION_TEE:
		load_gauge_orientation_tee(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_HOSTILE_TRI:
		load_gauge_hostile_tri(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_TARGET_TRI:
		load_gauge_target_tri(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_MISSILE_TRI:
		load_gauge_missile_tri(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_KILLS:
		load_gauge_kills(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	case HUD_OBJECT_FIXED_MESSAGES:
		load_gauge_fixed_messages(base_w, base_h, hud_font, ship_idx, use_clr);
		break;
	default:
		Warning(LOCATION, "Invalid gauge found in hud_gauges.tbl");
		break;
	}
}

inline bool check_base_res(int w, int h)
{
	return (w >= 640) && (h >= 480);
}

void check_color(int *colorp)
{
	int i;

	for ( i = 0; i < 3; ++i ) {
		if ( colorp[i] > 255 ) {
			colorp[i] = 255;
		} else if ( colorp[i] < 0 ) {
			colorp[i] = 0;
		}
	}
}

void load_gauge_custom(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int i;
	int coords[2];
	int base_res[2] = {640, 480};
	char gauge_string[MAX_FILENAME_LEN];
	char name[MAX_FILENAME_LEN];
	char text[MAX_FILENAME_LEN];
	char filename[MAX_FILENAME_LEN];
	int gauge_type = HUD_CENTER_RETICLE;
	bool slew = false;
	bool active_by_default = true;
	int font_num = FONT1;
	int txtoffset_x = 0, txtoffset_y = 0;
	ubyte r = 255, g = 255, b = 255;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
		
		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}

		if ( use_clr != NULL ) {
			colors[0] = use_clr->red;
			colors[1] = use_clr->green;
			colors[2] = use_clr->blue;

			lock_color = true;
		} else if ( optional_string("Color:") ) {
			stuff_int_list(colors, 3);

			check_color(colors);

			lock_color = true;
		}

		if ( optional_string("Font:") ) {
			stuff_int(&font_num);
		} else {
			if ( hud_font >=0 ) {
				font_num = hud_font;
			}
		}

		required_string("Name:");
		stuff_string(name, F_NAME, MAX_FILENAME_LEN);

		required_string("Text:");
		stuff_string(text, F_NAME, MAX_FILENAME_LEN);

		if (optional_string("X Offset:")) {
			stuff_int(&txtoffset_x);
		}

		if (optional_string("Y Offset:")) {
			stuff_int(&txtoffset_y);
		}

		required_string("Gauge Type:");
		stuff_string(gauge_string, F_NAME, MAX_FILENAME_LEN);
		
		for(i = 0; i < NUM_HUD_GAUGES; i++) {
			if(!strcmp(HUD_gauge_text[i], gauge_string)) {
				gauge_type = i;
				break;
			}
		}

		if(optional_string("Slew:")) {
			stuff_boolean(&slew);
		}

		if(optional_string("Active by default:")) {
			stuff_boolean(&active_by_default);
		}

		required_string("Filename:");
		stuff_string(filename, F_NAME, MAX_FILENAME_LEN);
	}

	HudGauge* hud_gauge = new HudGauge(gauge_type, slew, r, g, b, name, text, filename, txtoffset_x, txtoffset_y);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initFont(font_num);
	hud_gauge->initRenderStatus(active_by_default);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_lag(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	char fname[MAX_FILENAME_LEN] = "netlag1";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 386;
		coords[1] = 331;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 627;
		coords[1] = 529;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
		
		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}

	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}

	HudGaugeLag* hud_gauge = new HudGaugeLag();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_mini_shields(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Mini_3digit_offsets[2];
	int Mini_1digit_offsets[2];
	int Mini_2digit_offsets[2];
	char fname[MAX_FILENAME_LEN] = "targhit1";
	bool slew = true;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 305;
		coords[1] = 291;

		base_res[0] = 640;
		base_res[1] = 480;

		Mini_3digit_offsets[0] = 5;
		Mini_3digit_offsets[1] = 7;
		Mini_1digit_offsets[0] = 11;
		Mini_1digit_offsets[1] = 7;
		Mini_2digit_offsets[0] = 8;
		Mini_2digit_offsets[1] = 7;
	} else {
		coords[0] = 497;
		coords[1] = 470;

		base_res[0] = 1024;
		base_res[1] = 768;

		Mini_3digit_offsets[0] = 5;
		Mini_3digit_offsets[1] = 7;
		Mini_1digit_offsets[0] = 14;
		Mini_1digit_offsets[1] = 7;
		Mini_2digit_offsets[0] = 9;
		Mini_2digit_offsets[1] = 7;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("3 Digit Hull Offsets:")) {
		stuff_int_list(Mini_3digit_offsets, 2);
	}
	if(optional_string("2 Digit Hull Offsets:")) {
		stuff_int_list(Mini_2digit_offsets, 2);
	}
	if(optional_string("1 Digit Hull Offsets:")) {
		stuff_int_list(Mini_1digit_offsets, 2);
	}

	HudGaugeShieldMini* hud_gauge = new HudGaugeShieldMini();
	hud_gauge->init1DigitOffsets(Mini_1digit_offsets[0], Mini_1digit_offsets[1]);
	hud_gauge->init2DigitOffsets(Mini_2digit_offsets[0], Mini_2digit_offsets[1]);
	hud_gauge->init3DigitOffsets(Mini_3digit_offsets[0], Mini_3digit_offsets[1]);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_weapon_energy(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Wenergy_text_offsets[2];
	int Wenergy_h;
	char fname[MAX_FILENAME_LEN];
	bool slew = true;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
		if(gr_screen.res == GR_640) {
			strcpy_s(fname, "energy2_fs1");
		} else {
			strcpy_s(fname, "2_energy2_fs1");
		}
	} else {
		if(gr_screen.res == GR_640) {
			strcpy_s(fname, "energy2");
		} else {
			strcpy_s(fname, "2_energy2");
		}
	}

	if(gr_screen.res == GR_640) {
		coords[0] = 416;
		coords[1] = 265;

		base_res[0] = 640;
		base_res[1] = 480;

		Wenergy_text_offsets[0] = 23;
		Wenergy_text_offsets[1] = 53;

		Wenergy_h = 60;
	} else {
		coords[0] = 666;
		coords[1] = 424;

		base_res[0] = 1024;
		base_res[1] = 768;

		Wenergy_text_offsets[0] = 43;
		Wenergy_text_offsets[1] = 85;

		Wenergy_h = 96;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}	
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Foreground Clip Height:")) {
		stuff_int(&Wenergy_h);
	}
	if(optional_string("Text Offsets:")) {
		stuff_int_list(Wenergy_text_offsets, 2);
	}

	HudGaugeWeaponEnergy* hud_gauge = new HudGaugeWeaponEnergy();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initEnergyHeight(Wenergy_h);
	hud_gauge->initTextOffsets(Wenergy_text_offsets[0], Wenergy_text_offsets[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_target_shields(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 142;
		coords[1] = 379;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 292;
		coords[1] = 670;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}

	HudGaugeShieldTarget* hud_gauge = new HudGaugeShieldTarget();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_player_shields(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 396;
		coords[1] = 379;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 634;
		coords[1] = 670;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}

	HudGaugeShieldPlayer* hud_gauge = new HudGaugeShieldPlayer();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_escort_view(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int header_text_offsets[2];
	int list_start_offsets[2];
	int entry_h;
	int entry_stagger_w;
	int ship_name_offsets[2];
	int ship_integrity_offsets[2];
	int ship_status_offsets[2];
	char header_text[MAX_FILENAME_LEN] = "monitoring";
	char fname_top[MAX_FILENAME_LEN] = "escort1";
	char fname_middle[MAX_FILENAME_LEN] = "escort2";
	char fname_bottom[MAX_FILENAME_LEN] = "escort3";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 486;
		coords[1] = 206;

		base_res[0] = 640;
		base_res[1] = 480;

		header_text_offsets[0] = 3;
		header_text_offsets[1] = 2;
		list_start_offsets[0] = 0;
		list_start_offsets[1] = 12;
		entry_h = 11;
		entry_stagger_w = 0;
		ship_name_offsets[0] = 3;
		ship_name_offsets[1] = 0;
		ship_integrity_offsets[0] = 128;
		ship_integrity_offsets[1] = 0;
		ship_status_offsets[0] = -12;
		ship_status_offsets[1] = 0;
	} else {
		coords[0] = 865;
		coords[1] = 330;

		base_res[0] = 1024;
		base_res[1] = 768;

		header_text_offsets[0] = 3;
		header_text_offsets[1] = 2;
		list_start_offsets[0] = 0;
		list_start_offsets[1] = 13;
		entry_h = 11;
		entry_stagger_w = 0;
		ship_name_offsets[0] = 4;
		ship_name_offsets[1] = 0;
		ship_integrity_offsets[0] = 116;
		ship_integrity_offsets[1] = 0;
		ship_status_offsets[0] = -11;
		ship_status_offsets[1] = 0;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
		
		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Top Background Filename:")) {
		stuff_string(fname_top, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Entry Background Filename:")) {
		stuff_string(fname_middle, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Bottom Background Filename:")) {
		stuff_string(fname_bottom, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Entry Height:")) {
		stuff_int(&entry_h);
	}
	if(optional_string("Entry Stagger Width:")) {
		stuff_int(&entry_stagger_w);
	}
	if(optional_string("Header Text:")) {
		stuff_string(header_text, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Header Offsets:")) {
		stuff_int_list(header_text_offsets, 2);
	}
	if(optional_string("List Start Offsets:")) {
		stuff_int_list(list_start_offsets, 2);
	}
	if(optional_string("Hull X-offset:")) {
		stuff_int(&ship_integrity_offsets[0]);
	}
	if(optional_string("Name X-offset:")) {
		stuff_int(&ship_name_offsets[0]);
	}
	if(optional_string("Status X-offset:")) {
		stuff_int(&ship_status_offsets[0]);
	}

	HudGaugeEscort* hud_gauge = new HudGaugeEscort();
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname_top, fname_middle, fname_bottom);
	hud_gauge->initEntryHeight(entry_h);
	hud_gauge->initEntryStaggerWidth(entry_stagger_w);
	hud_gauge->initHeaderText(header_text);
	hud_gauge->initHeaderTextOffsets(header_text_offsets[0], header_text_offsets[1]);
	hud_gauge->initListStartOffsets(list_start_offsets[0], list_start_offsets[1]);
	hud_gauge->initShipIntegrityOffsets(ship_integrity_offsets[0], ship_integrity_offsets[1]);
	hud_gauge->initShipNameOffsets(ship_name_offsets[0], ship_name_offsets[1]);
	hud_gauge->initShipStatusOffsets(ship_status_offsets[0], ship_status_offsets[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_afterburner(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int energy_h;
	char fname[MAX_FILENAME_LEN];
	bool slew = true;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
		if(gr_screen.res == GR_640) {
			strcpy_s(fname, "energy2_fs1");
		} else {
			strcpy_s(fname, "2_energy2_fs1");
		}
	} else {
		if(gr_screen.res == GR_640) {
			strcpy_s(fname, "energy2");
		} else {
			strcpy_s(fname, "2_energy2");
		}
	}

	if(gr_screen.res == GR_640) {
		coords[0] = 171;
		coords[1] = 265;

		base_res[0] = 640;
		base_res[1] = 480;

		energy_h = 60;
	} else {
		coords[0] = 274;
		coords[1] = 424;

		base_res[0] = 1024;
		base_res[1] = 768;

		energy_h = 96;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}	
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Foreground Clip Height:")) {
		stuff_int(&energy_h);
	}

	HudGaugeAfterburner *hud_gauge = new HudGaugeAfterburner();

	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initEnergyHeight(energy_h);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}


void load_gauge_mission_time(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int time_text_offsets[2];
	int time_val_offsets[2];
	char fname[MAX_FILENAME_LEN] = "time1";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 587;
		coords[1] = 448;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 969;
		coords[1] = 716;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	time_text_offsets[0] = 4;
	time_text_offsets[1] = 4;

	time_val_offsets[0] = 26;
	time_val_offsets[1] = 12;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
		
		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Text Offsets:")) {
		stuff_int_list(time_text_offsets, 2);
	}
	if(optional_string("Value Offsets:")) {
		stuff_int_list(time_val_offsets, 2);
	}

	HudGaugeMissionTime* hud_gauge = new HudGaugeMissionTime();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initTextOffsets(time_text_offsets[0], time_text_offsets[1]);
	hud_gauge->initValueOffsets(time_val_offsets[0], time_val_offsets[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_threat_indicator(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Laser_warn_offsets[2];
	int Lock_warn_offsets[2];
	char fname_arc[MAX_FILENAME_LEN];
	char fname_laser[MAX_FILENAME_LEN];
	char fname_lock[MAX_FILENAME_LEN];
	bool slew = true;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
		if(gr_screen.res == GR_640) {
			coords[0] = 241;
			coords[1] = 137;

			base_res[0] = 640;
			base_res[1] = 480;

			Laser_warn_offsets[0] = 59;
			Laser_warn_offsets[1] = 0;

			Lock_warn_offsets[0] = 79;
			Lock_warn_offsets[1] = 0;

			strcpy_s(fname_arc, "toparc1_fs1");
			strcpy_s(fname_laser, "toparc2_fs1");
			strcpy_s(fname_lock, "toparc3_fs1");
		} else {
			coords[0] = 386;
			coords[1] = 219;

			base_res[0] = 1024;
			base_res[1] = 768;

			Laser_warn_offsets[0] = 94;
			Laser_warn_offsets[1] = 0;

			Lock_warn_offsets[0] = 126;
			Lock_warn_offsets[1] = 0;

			strcpy_s(fname_arc, "2_toparc1_fs1");
			strcpy_s(fname_laser, "2_toparc2_fs1");
			strcpy_s(fname_lock, "2_toparc3_fs1");
		}
	} else {
		if(gr_screen.res == GR_640) {
			coords[0] = 359;
			coords[1] = 168;

			base_res[0] = 640;
			base_res[1] = 480;

			Laser_warn_offsets[0] = 41;
			Laser_warn_offsets[1] = 77;

			Lock_warn_offsets[0] = 35;
			Lock_warn_offsets[1] = 93;

			strcpy_s(fname_arc, "rightarc1");
			strcpy_s(fname_laser, "toparc2");
			strcpy_s(fname_lock, "toparc3");
		} else {
			coords[0] = 574;
			coords[1] = 269;

			base_res[0] = 1024;
			base_res[1] = 768;

			Laser_warn_offsets[0] = 66;
			Laser_warn_offsets[1] = 124;

			Lock_warn_offsets[0] = 57;
			Lock_warn_offsets[1] = 150;

			strcpy_s(fname_arc, "2_rightarc1");
			strcpy_s(fname_laser, "2_toparc2");
			strcpy_s(fname_lock, "2_toparc3");
		}
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Arc Filename:")) {
		stuff_string(fname_arc, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Dumbfire Filename:")) {
		stuff_string(fname_laser, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Lock Filename:")) {
		stuff_string(fname_lock, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Dumbfire Offsets:")) {
		stuff_int_list(Laser_warn_offsets, 2);
	}
	if(optional_string("Lock Offsets:")) {
		stuff_int_list(Lock_warn_offsets, 2);
	}

	HudGaugeThreatIndicator* hud_gauge = new HudGaugeThreatIndicator();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initBitmaps(fname_arc, fname_laser, fname_lock);
	hud_gauge->initLaserWarnOffsets(Laser_warn_offsets[0], Laser_warn_offsets[1]);
	hud_gauge->initLockWarnOffsets(Lock_warn_offsets[0], Lock_warn_offsets[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_center_reticle(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	char fname[MAX_FILENAME_LEN];
	int font_num = FONT1;
	bool firepoints = false;
	int scaleX = 15;
	int scaleY = 10;
	int size = 5;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
		if(gr_screen.res == GR_640) {
			coords[0] = 308;
			coords[1] = 235;

			base_res[0] = 640;
			base_res[1] = 480;

			strcpy_s(fname, "reticle1_fs1");
		} else {
			coords[0] = 493;
			coords[1] = 376;

			base_res[0] = 1024;
			base_res[1] = 768;

			strcpy_s(fname, "2_reticle1_fs1");
		}
	} else {
		if(gr_screen.res == GR_640) {
			coords[0] = 308;
			coords[1] = 235;

			base_res[0] = 640;
			base_res[1] = 480;

			strcpy_s(fname, "reticle1");
		} else {
			coords[0] = 493;
			coords[1] = 370;

			base_res[0] = 1024;
			base_res[1] = 768;

			strcpy_s(fname, "2_reticle1");
		}
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}

	if(optional_string("Firepoint display:")) 
		stuff_boolean(&firepoints);

	if (optional_string("Firepoint size:"))
		stuff_int(&size);

	if (optional_string("Firepoint X coordinate multiplier:"))
		stuff_int(&scaleX);

	if (optional_string("Firepoint Y coordinate multiplier:"))
		stuff_int(&scaleY);

	HudGaugeReticle* hud_gauge = new HudGaugeReticle();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initFont(font_num);
	hud_gauge->initFirepointDisplay(firepoints, scaleX, scaleY, size);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_throttle(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int bottom_offset_y;
	int throttle_h, throttle_w;
	int throttle_aburn_h;
	int max_speed_offset[2];
	int zero_speed_offset[2];
	int orbit_center_offset[2];
	int orbit_radius;
	char fname[MAX_FILENAME_LEN];
	bool show_background = false;
	bool slew = true;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	// default values for the throttle
	if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
		if(gr_screen.res == GR_640) {
			coords[0] = 217;
			coords[1] = 244;

			base_res[0] = 640;
			base_res[1] = 480;

			bottom_offset_y = 65;
			throttle_h = 50;
			throttle_w = 49;
			throttle_aburn_h = 17;
			max_speed_offset[0] = 19;
			max_speed_offset[1] = 10;
			zero_speed_offset[0] = 35;
			zero_speed_offset[1] = 59;
			orbit_center_offset[0] = 103;
			orbit_center_offset[1] = -1;
			orbit_radius = 104;
			strcpy_s(fname, "leftarc_fs1");
		} else {
			coords[0] = 347;
			coords[1] = 390;

			base_res[0] = 1024;
			base_res[1] = 768;

			bottom_offset_y = 104;
			throttle_h = 80;
			throttle_w = 78;
			throttle_aburn_h = 27;
			max_speed_offset[0] = 30;
			max_speed_offset[1] = 16;
			zero_speed_offset[0] = 56;
			zero_speed_offset[1] = 95;
			orbit_center_offset[0] = 165;
			orbit_center_offset[1] = -1;
			orbit_radius = 166;
			strcpy_s(fname, "2_leftarc_fs1");
		}
		show_background = true;
	} else {
		if(gr_screen.res == GR_640) {
			coords[0] = 216;
			coords[1] = 168;

			base_res[0] = 640;
			base_res[1] = 480;

			bottom_offset_y = 139;
			throttle_h = 50;
			throttle_w = 49;
			throttle_aburn_h = 17;
			max_speed_offset[0] = 20;
			max_speed_offset[1] = 86;
			zero_speed_offset[0] = 36;
			zero_speed_offset[1] = 135;
			orbit_center_offset[0] = 104;
			orbit_center_offset[1] = 75;
			orbit_radius = 104;
			strcpy_s(fname, "leftarc");
		} else {
			coords[0] = 346;
			coords[1] = 269;

			base_res[0] = 1024;
			base_res[1] = 768;

			bottom_offset_y = 222;
			throttle_h = 80;
			throttle_w = 78;
			throttle_aburn_h = 27;
			max_speed_offset[0] = 31;
			max_speed_offset[1] = 137;
			zero_speed_offset[0] = 57;
			zero_speed_offset[1] = 216;
			orbit_center_offset[0] = 166;
			orbit_center_offset[1] = 118;
			orbit_radius = 166;
			strcpy_s(fname, "2_leftarc");
		}
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Foreground Clip Bottom Y-offset:")) {
		stuff_int(&bottom_offset_y);
	}
	if(optional_string("Foreground Clip Width:")) {
		stuff_int(&throttle_w);
	}
	if(optional_string("Foreground Clip Height:")) {
		stuff_int(&throttle_h);
	}
	if(optional_string("Afterburner Clip Height:")) {
		stuff_int(&throttle_aburn_h);
	}
	if(optional_string("Show Background:")) {
		stuff_boolean(&show_background);
	}
	if(optional_string("Max Speed Label Offsets:")) {
		stuff_int_list(max_speed_offset, 2);
	}
	if(optional_string("Min Speed Label Offsets:")) {
		stuff_int_list(zero_speed_offset, 2);
	}
	if(optional_string("Orbit Center Offsets:")) {
		stuff_int_list(orbit_center_offset, 2);
	}
	if(optional_string("Orbit Radius:")) {
		stuff_int(&orbit_radius);
	}

	HudGaugeThrottle* hud_gauge = new HudGaugeThrottle();

	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initThrottleStartY(bottom_offset_y);
	hud_gauge->initThrottleSizes(throttle_w, throttle_h);
	hud_gauge->initAburnHeight(throttle_aburn_h);
	hud_gauge->initMaxSpeedOffsets(max_speed_offset[0], max_speed_offset[1]);
	hud_gauge->initZeroSpeedOffsets(zero_speed_offset[0], zero_speed_offset[1]);
	hud_gauge->initOrbitCenterOffsets(orbit_center_offset[0], orbit_center_offset[1]);
	hud_gauge->initOrbitRadius(orbit_radius);
	hud_gauge->initBitmaps(fname);
	hud_gauge->showBackground(show_background);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

/**
 * Load retail style ETS gauge
 * i.e. treats weapons, shields & engines gauges as a single gauge
 */
void load_gauge_ets_retail(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int bar_h;
	int letter_offsets[2];
	int top_offsets[2];
	int bottom_offsets[2];
	char ets_letters[num_retail_ets_gauges];
	char fname[MAX_FILENAME_LEN] = "energy1";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;
	int gauge_offset; // distance between micro gauges
	int i;
	int gauge_positions[num_retail_ets_gauges];

	if (Lcl_gr) {
		ets_letters[0] = 'G'; ets_letters[1] = 'S'; ets_letters[2] = 'A'; // German
	} else if (Lcl_fr) {
		ets_letters[0] = 'C'; ets_letters[1] = 'B'; ets_letters[2] = 'M'; // French
	} else {
		ets_letters[0] = 'G'; ets_letters[1] = 'S'; ets_letters[2] = 'E'; // English
	}

	// default values which may be overwritten by .tbl
	if(gr_screen.res == GR_640) {
		coords[0] = 523;
		coords[1] = 380;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 880;
		coords[1] = 648;

		base_res[0] = 1024;
		base_res[1] = 768;
	}
	bar_h = 41;
	letter_offsets[0] = 2;
	letter_offsets[1] = 42;
	top_offsets[0] = 0;
	top_offsets[1] = 0;
	bottom_offsets[0] = 0;
	bottom_offsets[1] = 50;
	gauge_offset = 20;

	// parse data from .tbl
	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Foreground Clip Height:")) {
		stuff_int(&bar_h);
	}
	if(optional_string("Letter Offsets:")) {
		stuff_int_list(letter_offsets, 2);
	}
	if(optional_string("Top Offsets:")) {
		stuff_int_list(top_offsets, 2);
	}
	if(optional_string("Bottom Offsets:")) {
		stuff_int_list(bottom_offsets, 2);
	}
	if(optional_string("Gauge Offset:")) {
		stuff_int(&gauge_offset);
	}

	// calculate offsets for the three gauges
	for (i = 0; i < num_retail_ets_gauges; ++i)
		gauge_positions[i] = coords[0] + gauge_offset * i;

	HudGaugeEtsRetail* hud_gauge = new HudGaugeEtsRetail();

	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initLetters(ets_letters);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initLetterOffsets(letter_offsets[0], letter_offsets[1]);
	hud_gauge->initTopOffsets(top_offsets[0], top_offsets[1]);
	hud_gauge->initBottomOffsets(bottom_offsets[0], bottom_offsets[1]);
	hud_gauge->initBarHeight(bar_h);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);
	hud_gauge->initGaugePositions(gauge_positions);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_ets_weapons(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int bar_h;
	int letter_offsets[2];
	int top_offsets[2];
	int bottom_offsets[2];
	char letter;
	char fname[MAX_FILENAME_LEN] = "energy1";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(Lcl_gr) {
		// German
		letter = 'G';
	} else if(Lcl_fr) {
		// French
		letter = 'C';
	} else {
		// English
		letter = 'G';
	}

	if(gr_screen.res == GR_640) {
		coords[0] = 523;
		coords[1] = 380;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 880;
		coords[1] = 648;

		base_res[0] = 1024;
		base_res[1] = 768;
	}
	bar_h = 41;
	letter_offsets[0] = 2;
	letter_offsets[1] = 42;
	top_offsets[0] = 0;
	top_offsets[1] = 0;
	bottom_offsets[0] = 0;
	bottom_offsets[1] = 50;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
	
		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Foreground Clip Height:")) {
		stuff_int(&bar_h);
	}
	if(optional_string("Letter Offsets:")) {
		stuff_int_list(letter_offsets, 2);
	}
	if(optional_string("Top Offsets:")) {
		stuff_int_list(top_offsets, 2);
	}
	if(optional_string("Bottom Offsets:")) {
		stuff_int_list(bottom_offsets, 2);
	}

	HudGaugeEtsWeapons* hud_gauge = new HudGaugeEtsWeapons();
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initLetter(letter);
	hud_gauge->initLetterOffsets(letter_offsets[0], letter_offsets[1]);
	hud_gauge->initTopOffsets(top_offsets[0], top_offsets[1]);
	hud_gauge->initBottomOffsets(bottom_offsets[0], bottom_offsets[1]);
	hud_gauge->initBarHeight(bar_h);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_ets_shields(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int bar_h;
	int letter_offsets[2];
	int top_offsets[2];
	int bottom_offsets[2];
	char letter;
	char fname[MAX_FILENAME_LEN] = "energy1";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(Lcl_gr){
		// German
		letter = 'S';
	} else if(Lcl_fr){
		// French
		letter = 'B';
	} else {
		// English
		letter = 'S';
	}

	if(gr_screen.res == GR_640) {
		coords[0] = 540;
		coords[1] = 380;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 898;
		coords[1] = 648;

		base_res[0] = 1024;
		base_res[1] = 768;
	}
	bar_h = 41;

	letter_offsets[0] = 2;
	letter_offsets[1] = 42;
	top_offsets[0] = 0;
	top_offsets[1] = 0;
	bottom_offsets[0] = 0;
	bottom_offsets[1] = 50;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Foreground Clip Height:")) {
		stuff_int(&bar_h);
	}
	if(optional_string("Letter Offsets:")) {
		stuff_int_list(letter_offsets, 2);
	}
	if(optional_string("Top Offsets:")) {
		stuff_int_list(top_offsets, 2);
	}
	if(optional_string("Bottom Offsets:")) {
		stuff_int_list(bottom_offsets, 2);
	}

	HudGaugeEtsShields* hud_gauge = new HudGaugeEtsShields();

	hud_gauge->initBarHeight(bar_h);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initBottomOffsets(bottom_offsets[0], bottom_offsets[1]);
	hud_gauge->initLetter(letter);
	hud_gauge->initLetterOffsets(letter_offsets[0], letter_offsets[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initTopOffsets(top_offsets[0], top_offsets[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_ets_engines(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int bar_h;
	int letter_offsets[2];
	int top_offsets[2];
	int bottom_offsets[2];
	char letter;
	char fname[MAX_FILENAME_LEN] = "energy1";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(Lcl_gr){
		// German
		letter = 'A';
	} else if(Lcl_fr){
		// French
		letter = 'M';
	} else {
		// English
		letter = 'E';
	}

	if(gr_screen.res == GR_640) {
		coords[0] = 557;
		coords[1] = 380;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 916;
		coords[1] = 648;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	bar_h = 41;

	letter_offsets[0] = 2;
	letter_offsets[1] = 42;
	top_offsets[0] = 0;
	top_offsets[1] = 0;
	bottom_offsets[0] = 0;
	bottom_offsets[1] = 50;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Foreground Clip Height:")) {
		stuff_int(&bar_h);
	}
	if(optional_string("Letter Offsets:")) {
		stuff_int_list(letter_offsets, 2);
	}
	if(optional_string("Top Offsets:")) {
		stuff_int_list(top_offsets, 2);
	}
	if(optional_string("Bottom Offsets:")) {
		stuff_int_list(bottom_offsets, 2);
	}

	HudGaugeEtsEngines* hud_gauge = new HudGaugeEtsEngines();

	hud_gauge->initBarHeight(bar_h);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initBottomOffsets(bottom_offsets[0], bottom_offsets[1]);
	hud_gauge->initLetter(letter);
	hud_gauge->initLetterOffsets(letter_offsets[0], letter_offsets[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initTopOffsets(top_offsets[0], top_offsets[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_extra_target_data(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int dock_offsets[2];
	int time_offsets[2];
	int bracket_offsets[2];
	int order_offsets[2];
	char fname[MAX_FILENAME_LEN] = "targetview3";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 5;
		coords[1] = 280;

		base_res[0] = 640;
		base_res[1] = 480;

		dock_offsets[0] = 7;
		dock_offsets[1] = 19;

		time_offsets[0] = 8;
		time_offsets[1] = 10;
	} else {
		coords[0] = 5;
		coords[1] = 552;

		base_res[0] = 1024;
		base_res[1] = 768;

		dock_offsets[0] = 7;
		dock_offsets[1] = 18;

		time_offsets[0] = 8;
		time_offsets[1] = 9; 
	}

	bracket_offsets[0] = 0;
	bracket_offsets[1] = 3;

	order_offsets[0] = 7;
	order_offsets[1] = 0; 

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Bracket Offsets:")) {
		stuff_int_list(bracket_offsets, 2);
	}
	if(optional_string("Dock Offsets:")) {
		stuff_int_list(dock_offsets, 2);
	}
	if(optional_string("Order Offsets:")) {
		stuff_int_list(order_offsets, 2);
	}
	if(optional_string("Time Offsets:")) {
		stuff_int_list(time_offsets, 2);
	}

	HudGaugeExtraTargetData* hud_gauge = new HudGaugeExtraTargetData();

	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initBracketOffsets(bracket_offsets[0], bracket_offsets[1]);
	hud_gauge->initDockOffsets(dock_offsets[0], dock_offsets[1]);
	hud_gauge->initOrderOffsets(order_offsets[0], order_offsets[1]);
	hud_gauge->initTimeOffsets(time_offsets[0], time_offsets[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_radar_std(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Radar_blip_radius_normal;
	int Radar_blip_radius_target;
	int Radar_radius[2];
	int Radar_dist_offsets[RR_MAX_RANGES][2];
	float Radar_center_offsets[2];
	char fname[MAX_FILENAME_LEN];
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 257;
		coords[1] = 369;

		base_res[0] = 640;
		base_res[1] = 480;

		Radar_blip_radius_normal = 2;
		Radar_blip_radius_target = 5;

		Radar_center_offsets[0] = 64.0f;
		Radar_center_offsets[1] = 53.0f;

		Radar_radius[0] = 120;
		Radar_radius[1] = 100;

		Radar_dist_offsets[0][0] = 110;
		Radar_dist_offsets[0][1] = 92;

		Radar_dist_offsets[1][0] = 107;
		Radar_dist_offsets[1][1] = 92;

		Radar_dist_offsets[2][0] = 112;
		Radar_dist_offsets[2][1] = 92;

		strcpy_s(fname, "radar1");
	} else {
		coords[0] = 411;
		coords[1] = 590;

		base_res[0] = 1024;
		base_res[1] = 768;

		Radar_blip_radius_normal = 4;
		Radar_blip_radius_target = 8;

		Radar_center_offsets[0] = 104.0f;
		Radar_center_offsets[1] = 85.0f;

		Radar_radius[0] = 192;
		Radar_radius[1] = 160;

		Radar_dist_offsets[0][0] = 184;
		Radar_dist_offsets[0][1] = 150;

		Radar_dist_offsets[1][0] = 181;
		Radar_dist_offsets[1][1] = 150;

		Radar_dist_offsets[2][0] = 185;
		Radar_dist_offsets[2][1] = 150;

		strcpy_s(fname, "2_radar1");
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Infinity Distance Offsets:")) {
		stuff_int_list(Radar_dist_offsets[2], 2);
	}
	if(optional_string("Long Distance Offsets:")) {
		stuff_int_list(Radar_dist_offsets[1], 2);
	}
	if(optional_string("Short Distance Offsets:")) {
		stuff_int_list(Radar_dist_offsets[0], 2);
	}

	// Only load this if the user hasn't specified a preference
	if (Cmdline_orb_radar == 0) {
		HudGaugeRadarStd* hud_gauge = new HudGaugeRadarStd();
		hud_gauge->initBaseResolution(base_res[0], base_res[1]);
		hud_gauge->initPosition(coords[0], coords[1]);
		hud_gauge->initBitmaps(fname);
		hud_gauge->initBlipRadius(Radar_blip_radius_normal, Radar_blip_radius_target);
		hud_gauge->initCenterOffsets(Radar_center_offsets[0], Radar_center_offsets[1]);
		hud_gauge->initDistanceInfinityOffsets(Radar_dist_offsets[2][0], Radar_dist_offsets[2][1]);
		hud_gauge->initDistanceLongOffsets(Radar_dist_offsets[1][0], Radar_dist_offsets[1][1]);
		hud_gauge->initDistanceShortOffsets(Radar_dist_offsets[0][0], Radar_dist_offsets[0][1]);
		hud_gauge->initRadius(Radar_radius[0], Radar_radius[1]);
		hud_gauge->initFont(font_num);
		hud_gauge->lockConfigColor(lock_color);
		hud_gauge->updateColor(colors[0], colors[1], colors[2]);

		if(ship_index >= 0) {
			Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
		} else {
			default_hud_gauges.push_back(hud_gauge);
		}
	}
}

void load_gauge_radar_orb(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Radar_blip_radius_normal;
	int Radar_blip_radius_target;
	int Radar_radius[2];
	int Radar_dist_offsets[RR_MAX_RANGES][2];
	float Radar_center_offsets[2];
	char fname[MAX_FILENAME_LEN];
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 257;
		coords[1] = 369;

		base_res[0] = 640;
		base_res[1] = 480;

		Radar_blip_radius_normal = 2;
		Radar_blip_radius_target = 5;

		Radar_center_offsets[0] = 65.0f;
		Radar_center_offsets[1] = 53.0f;

		Radar_radius[0] = 120;
		Radar_radius[1] = 100;

		Radar_dist_offsets[0][0]=110;
		Radar_dist_offsets[0][1]=92;

		Radar_dist_offsets[1][0]=107;
		Radar_dist_offsets[1][1]=92;

		Radar_dist_offsets[2][0]=111;
		Radar_dist_offsets[2][1]=92;

		strcpy_s(fname, "radar1");
	} else {
		coords[0] = 411;
		coords[1] = 590;

		base_res[0] = 1024;
		base_res[1] = 768;

		Radar_blip_radius_normal = 4;
		Radar_blip_radius_target = 8;

		Radar_center_offsets[0] = 104.0f;
		Radar_center_offsets[1] = 85.0f;

		Radar_radius[0] = 192;
		Radar_radius[1] = 160;

		Radar_dist_offsets[0][0]=184;
		Radar_dist_offsets[0][1]=150;

		Radar_dist_offsets[1][0]=181;
		Radar_dist_offsets[1][1]=150;

		Radar_dist_offsets[2][0]=185;
		Radar_dist_offsets[2][1]=150;

		strcpy_s(fname, "2_radar1");
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Infinity Distance Offsets:")) {
		stuff_int_list(Radar_dist_offsets[2], 2);
	}
	if(optional_string("Long Distance Offsets:")) {
		stuff_int_list(Radar_dist_offsets[1], 2);
	}
	if(optional_string("Short Distance Offsets:")) {
		stuff_int_list(Radar_dist_offsets[0], 2);
	}

	//only load this if the user actually wants to use the orb radar.
	if (Cmdline_orb_radar == 1) {
		HudGaugeRadarOrb* hud_gauge = new HudGaugeRadarOrb();
		hud_gauge->initBaseResolution(base_res[0], base_res[1]);
		hud_gauge->initPosition(coords[0], coords[1]);
		hud_gauge->initBitmaps(fname);
		hud_gauge->initBlipRadius(Radar_blip_radius_normal, Radar_blip_radius_target);
		hud_gauge->initCenterOffsets(Radar_center_offsets[0], Radar_center_offsets[1]);
		hud_gauge->initDistanceInfinityOffsets(Radar_dist_offsets[2][0], Radar_dist_offsets[2][1]);
		hud_gauge->initDistanceLongOffsets(Radar_dist_offsets[1][0], Radar_dist_offsets[1][1]);
		hud_gauge->initDistanceShortOffsets(Radar_dist_offsets[0][0], Radar_dist_offsets[0][1]);
		hud_gauge->initRadius(Radar_radius[0], Radar_radius[1]);
		hud_gauge->initFont(font_num);
		hud_gauge->updateColor(colors[0], colors[1], colors[2]);
		hud_gauge->lockConfigColor(lock_color);

		if(ship_index >= 0) {
			Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
		} else {
			default_hud_gauges.push_back(hud_gauge);
		}
	}
}

void load_gauge_radar_dradis(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	// basic radar gauge info
	int coords[2];
	int base_res[2];
	int Radar_radius[2];

	// bitmap filenames for the effect
	char xy_fname[MAX_FILENAME_LEN] = "dradis_xy";
	char xz_yz_fname[MAX_FILENAME_LEN] = "dradis_xz_yz"; 
	char sweep_fname[MAX_FILENAME_LEN] = "dradis_sweep";
	char target_fname[MAX_FILENAME_LEN] = "dradis_target";
	char unknown_fname[MAX_FILENAME_LEN] = "dradis_unknown";

	// render to texture parameters
	char display_name[MAX_FILENAME_LEN] = "";
	int display_offset[2] = {0, 0};
	int display_size[2] = {0, 0};
	int canvas_size[2] = {0, 0};

	int font_num = FONT1;

	int loop_snd = -1;
	float loop_snd_volume = 1.0f;

	int arrival_beep_snd = -1;
	int departure_beep_snd = -1;

	int stealth_arrival_snd = -1;
	int stealth_departure_snd = -1;

	float arrival_beep_delay = 0.0f;
	float departure_beep_delay = 0.0f;

	if(gr_screen.res == GR_640) {
		coords[0] = 231;
		coords[1] = 332;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 369;
		coords[1] = 531;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	Radar_radius[0] = 281;
	Radar_radius[1] = 233;
	
	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Size:")) {
		stuff_int_list(Radar_radius, 2);
	}
	if(optional_string("XY Disc Filename:")) {
		stuff_string(xy_fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("XZ YZ Disc Filename:")) {
		stuff_string(xz_yz_fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Sweep Disc Filename:")) {
		stuff_string(sweep_fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Default Contact Filename:")) {
		stuff_string(target_fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Unknown Contact Filename:")) {
		stuff_string(unknown_fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Cockpit Target:") && ship_index >= 0) {
		stuff_string(display_name, F_NAME, MAX_FILENAME_LEN);

		if(optional_string("Canvas Size:")) {
			stuff_int_list(canvas_size, 2);
		}

		required_string("Display Offset:");
		stuff_int_list(display_offset, 2);

		required_string("Display Size:");
		stuff_int_list(display_size, 2);
	}

	parse_sound("Loop Sound:", &loop_snd, "DRADIS HudGauge");

	if (optional_string("Loop Volume:"))
	{
		stuff_float(&loop_snd_volume);

		if (loop_snd_volume <= 0.0f)
		{
			Warning(LOCATION, "\"Loop Volume:\" value of \"%f\" is invalid! Must be more than zero! Resetting to default.", arrival_beep_delay);
			loop_snd_volume = 1.0f;
		}
	}

	parse_sound("Arrival Beep Sound:", &arrival_beep_snd, "DRADIS HudGauge");
	parse_sound("Stealth arrival Beep Sound:", &stealth_arrival_snd, "DRADIS HudGauge");

	if (optional_string("Minimum Beep Delay:"))
	{
		stuff_float(&arrival_beep_delay);

		if (arrival_beep_delay < 0.0f)
		{
			Warning(LOCATION, "\"Minimum Beep Delay:\" value of \"%f\" is invalid! Must be more than or equal to zero! Resetting to default.", arrival_beep_delay);
			arrival_beep_delay = 0.0f;
		}
	}

	parse_sound("Departure Beep Sound:", &departure_beep_snd, "DRADIS HudGauge");
	parse_sound("Stealth departure Beep Sound:", &stealth_departure_snd, "DRADIS HudGauge");

	if (optional_string("Minimum Beep Delay:"))
	{
		stuff_float(&departure_beep_delay);

		if (departure_beep_delay < 0.0f)
		{
			Warning(LOCATION, "\"Minimum Beep Delay:\" value of \"%f\" is invalid! Must be more than or equal to zero! Resetting to default.", departure_beep_delay);
			departure_beep_delay = 0.0f;
		}
	}

	HudGaugeRadarDradis* hud_gauge = new HudGaugeRadarDradis();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initRadius(Radar_radius[0], Radar_radius[1]);
	hud_gauge->initBitmaps(xy_fname, xz_yz_fname, sweep_fname, target_fname, unknown_fname);
	hud_gauge->initCockpitTarget(display_name, display_offset[0], display_offset[1], display_size[0], display_size[1], canvas_size[0], canvas_size[1]);
	hud_gauge->initFont(font_num);
	hud_gauge->initSound(loop_snd, loop_snd_volume, arrival_beep_snd, departure_beep_snd, stealth_arrival_snd, stealth_departure_snd, arrival_beep_delay, departure_beep_delay);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_text_warnings(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	bool slew = true;
	int font_num = FONT3;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 320;
		coords[1] = 172;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 512;
		coords[1] = 275;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}

	HudGaugeTextWarnings* hud_gauge = new HudGaugeTextWarnings();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_target_monitor(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Viewport_size[2];
	int Viewport_offsets[2];
	int Integrity_bar_offsets[2];
	int Integrity_bar_h;
	int Status_offsets[2];
	int Name_offsets[2];
	int Class_offsets[2];
	int Dist_offsets[2];
	int Speed_offsets[2];
	int Cargo_string_offsets[2];
	int Hull_offsets[2];
	int Cargo_scan_start_offsets[2];
	int Cargo_scan_size[2];
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	char fname_monitor[MAX_FILENAME_LEN] = "targetview1";
	char fname_integrity[MAX_FILENAME_LEN] = "targetview2";
	char fname_static[MAX_FILENAME_LEN] = "TargetStatic";

	int font_num = FONT1;

	if(gr_screen.res == GR_640) {
		coords[0] = 5;
		coords[1] = 319;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 5;
		coords[1] = 590;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	Viewport_size[0] = 131;
	Viewport_size[1] = 112;
	Viewport_offsets[0] = 3;
	Viewport_offsets[1] = 39;

	Integrity_bar_offsets[0] = 133;
	Integrity_bar_offsets[1] = 52;
	Integrity_bar_h = 88;
	Status_offsets[0] = 107;
	Status_offsets[1] = 53;
	
	Name_offsets[0] = 8;
	Name_offsets[1] = -3;
	Class_offsets[0] = 8;
	Class_offsets[1] = 7;
	Dist_offsets[0] = 8;
	Dist_offsets[1] = 18;
	Speed_offsets[0] = 85;
	Speed_offsets[1] = 18;
	Cargo_string_offsets[0] = 8;
	Cargo_string_offsets[1] = 30;

	// remember, below coords describe the rightmost position of their respective sub-element, not leftmost like it usually does.
	Hull_offsets[0] = 134;
	Hull_offsets[1] = 42;

	Cargo_scan_start_offsets[0] = 2;
	Cargo_scan_start_offsets[1] = 45;
	Cargo_scan_size[0] = 130;
	Cargo_scan_size[1] = 109;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Monitor Filename:")) {
		stuff_string(fname_monitor, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Integrity Bar Filename:")) {
		stuff_string(fname_integrity, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Viewport Offsets:")) {
		stuff_int_list(Viewport_offsets, 2);
	}
	if(optional_string("Viewport Size:")) {
		stuff_int_list(Viewport_size, 2);
	}
	if(optional_string("Integrity Bar Offsets:")) {
		stuff_int_list(Integrity_bar_offsets, 2);
	}
	if(optional_string("Integrity Bar Foreground Clip Height:")) {
		stuff_int(&Integrity_bar_h);
	}
	if(optional_string("Status Offsets:")) {
		stuff_int_list(Status_offsets, 2);
	}
	if(optional_string("Name Offsets:")) {
		stuff_int_list(Name_offsets, 2);
	}
	if(optional_string("Class Offsets:")) {
		stuff_int_list(Class_offsets, 2);
	}
	if(optional_string("Distance Offsets:")) {
		stuff_int_list(Dist_offsets, 2);
	}
	if(optional_string("Speed Offsets:")) {
		stuff_int_list(Speed_offsets, 2);
	}
	if(optional_string("Hull Offsets:")) {
		stuff_int_list(Hull_offsets, 2);
	}
	if(optional_string("Cargo Contents Offsets:")) {
		stuff_int_list(Cargo_string_offsets, 2);
	}
	if(optional_string("Cargo Scan Start Offsets:")) {
		stuff_int_list(Cargo_scan_start_offsets, 2);
	}
	if(optional_string("Cargo Scan Size:")) {
		stuff_int_list(Cargo_scan_size, 2);
	}

	HudGaugeTargetBox* hud_gauge = new HudGaugeTargetBox();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initViewportOffsets(Viewport_offsets[0], Viewport_offsets[1]);
	hud_gauge->initViewportSize(Viewport_size[0], Viewport_size[1]);
	hud_gauge->initIntegrityOffsets(Integrity_bar_offsets[0], Integrity_bar_offsets[1]);
	hud_gauge->initIntegrityHeight(Integrity_bar_h);
	hud_gauge->initStatusOffsets(Status_offsets[0], Status_offsets[1]);
	hud_gauge->initNameOffsets(Name_offsets[0], Name_offsets[1]);
	hud_gauge->initClassOffsets(Class_offsets[0], Class_offsets[1]);
	hud_gauge->initDistOffsets(Dist_offsets[0], Dist_offsets[1]);
	hud_gauge->initSpeedOffsets(Speed_offsets[0], Speed_offsets[1]);
	hud_gauge->initCargoStringOffsets(Cargo_string_offsets[0], Cargo_string_offsets[1]);
	hud_gauge->initHullOffsets(Hull_offsets[0], Hull_offsets[1]);
	hud_gauge->initCargoScanStartOffsets(Cargo_scan_start_offsets[0], Cargo_scan_start_offsets[1]);
	hud_gauge->initCargoScanSize(Cargo_scan_size[0], Cargo_scan_size[1]);
	hud_gauge->initBitmaps(fname_monitor, fname_integrity, fname_static);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_squad_message(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Pgup_offsets[2];
	int Pgdn_offsets[2];
	int Header_offsets[2];
	int Item_start_offsets[2];
	int Middle_frame_start_offset_y;
	int Item_h;
	int Item_offset_x;
	char fname_top[MAX_FILENAME_LEN] = "message1";
	char fname_middle[MAX_FILENAME_LEN] = "message2";
	char fname_bottom[MAX_FILENAME_LEN] = "message3";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 445;
		coords[1] = 5;

		base_res[0] = 640;
		base_res[1] = 480;

		Pgup_offsets[0] = 145;
		Pgup_offsets[1] = 4;
		Pgdn_offsets[0] = 145;
		Pgdn_offsets[1] = 115;
	} else {
		coords[0] = 827;
		coords[1] = 5;

		base_res[0] = 1024;
		base_res[1] = 768;

		Pgup_offsets[0] = 110;
		Pgup_offsets[1] = 5;
		Pgdn_offsets[0] = 110;
		Pgdn_offsets[1] = 115;
	}

	Header_offsets[0] = 2;
	Header_offsets[1] = 1;
	Item_start_offsets[0] = 4;
	Item_start_offsets[1] = 13;
	Middle_frame_start_offset_y = 12;
	Item_h = 10;
	Item_offset_x = 17;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Top Background Filename:")) {
		stuff_string(fname_top, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Entry Background Filename:")) {
		stuff_string(fname_middle, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Bottom Background Filename:")) {
		stuff_string(fname_bottom, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Header Offsets:")) {
		stuff_int_list(Header_offsets, 2);
	}
	if(optional_string("List Start Offsets:")) {
		stuff_int_list(Item_start_offsets, 2);
	}
	if(optional_string("Top Background Height:")) {
		stuff_int(&Middle_frame_start_offset_y);
	}
	if(optional_string("Entry Height:")) {
		stuff_int(&Item_h);
	}
	if(optional_string("Command X-offset:")) {
		stuff_int(&Item_offset_x);
	}
	if(optional_string("Page Up Offsets:")) {
		stuff_int_list(Pgup_offsets, 2);
	}
	if(optional_string("Page Down Offsets:")) {
		stuff_int_list(Pgdn_offsets, 2);
	}

	HudGaugeSquadMessage* hud_gauge = new HudGaugeSquadMessage();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname_top, fname_middle, fname_bottom);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initHeaderOffsets(Header_offsets[0], Header_offsets[1]);
	hud_gauge->initItemStartOffsets(Item_start_offsets[0], Item_start_offsets[1]);
	hud_gauge->initMiddleFrameStartOffsetY(Middle_frame_start_offset_y);
	hud_gauge->initItemHeight(Item_h);
	hud_gauge->initItemOffsetX(Item_offset_x);
	hud_gauge->initPgUpOffsets(Pgup_offsets[0], Pgup_offsets[1]);
	hud_gauge->initPgDnOffsets(Pgdn_offsets[0], Pgdn_offsets[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_objective_notify(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Objective_text_offset_y;
	int Objective_text_val_offset_y;
	int Subspace_text_offset_y;
	int Subspace_text_val_offset_y;
	int Red_text_offset_y;
	int Red_text_val_offset_y;
	char fname[MAX_FILENAME_LEN] = "objective1";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 245;
		coords[1] = 114;

		base_res[0] = 640;
		base_res[1] = 480;

		Objective_text_offset_y = 2;
		Objective_text_val_offset_y = 10;
		Subspace_text_offset_y = 2;
		Subspace_text_val_offset_y = 10;
		Red_text_offset_y = 2;
		Red_text_val_offset_y = 10;
	} else {
		coords[0] = 436;
		coords[1] = 184;

		base_res[0] = 1024;
		base_res[1] = 768;

		Objective_text_offset_y = 2;
		Objective_text_val_offset_y = 11;
		Subspace_text_offset_y = 2;
		Subspace_text_val_offset_y = 10;
		Red_text_offset_y = 2;
		Red_text_val_offset_y = 10;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Objective Text Y-offset:")) {
		stuff_int(&Objective_text_offset_y);
	}
	if(optional_string("Objective Value Y-offset:")) {
		stuff_int(&Objective_text_val_offset_y);
	}
	if(optional_string("Subspace Text Y-offset:")) {
		stuff_int(&Subspace_text_offset_y);
	}
	if(optional_string("Subspace Value Y-offset:")) {
		stuff_int(&Subspace_text_val_offset_y);
	}
	if(optional_string("Red Alert Text Y-offset:")) {
		stuff_int(&Red_text_offset_y);
	}
	if(optional_string("Red Alert Value Y-offset:")) {
		stuff_int(&Red_text_val_offset_y);
	}

	HudGaugeObjectiveNotify* hud_gauge = new HudGaugeObjectiveNotify();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initObjTextOffsetY(Objective_text_offset_y);
	hud_gauge->initObjValueOffsetY(Objective_text_val_offset_y);
	hud_gauge->initSubspaceTextOffsetY(Subspace_text_offset_y);
	hud_gauge->initSubspaceValueOffsetY(Subspace_text_val_offset_y);
	hud_gauge->initRedAlertTextOffsetY(Red_text_offset_y);
	hud_gauge->initRedAlertValueOffsetY(Red_text_val_offset_y);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_weapons(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int top_offset_x[NUM_HUD_SETTINGS];
	int Weapon_header_offsets[NUM_HUD_SETTINGS][2];
	int frame_offset_x[NUM_HUD_SETTINGS];
	int Weapon_plink_offset_x;
	int Weapon_pname_offset_x;
	int Weapon_pammo_offset_x; 
	int Weapon_sammo_offset_x;
	int Weapon_sname_offset_x;
	int Weapon_sreload_offset_x;
	int Weapon_slinked_offset_x;
	int Weapon_sunlinked_offset_x;
	int top_primary_h;
	int top_secondary_h;
	int pname_start_offset_y;
	int sname_start_offset_y;
	int primary_text_h;
	int secondary_text_h;
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	// thank god both GR640 and GR1024 use the same weapons gauge bitmaps
	char fname_p_top[MAX_FILENAME_LEN] = "weapons1";
	char fname_p_top_b[MAX_FILENAME_LEN] = "weapons1_b";
	char fname_p_middle[MAX_FILENAME_LEN] = "weapons2";
	char fname_p_middle_b[MAX_FILENAME_LEN] = "weapons2_b";
	char fname_p_last[MAX_FILENAME_LEN] = "weapons6";
	// The bottom portion of the primary weapons listing for the ballistic ammo version of this gauge can simply use the middle portion.
	char fname_p_last_b[MAX_FILENAME_LEN] = "weapons2_b"; 
	char fname_s_top[MAX_FILENAME_LEN] = "weapons3";
	char fname_s_top_b[MAX_FILENAME_LEN] = "weapons3_b";
	char fname_s_middle[MAX_FILENAME_LEN] = "weapons4";
	char fname_s_middle_b[MAX_FILENAME_LEN] = "weapons4_b";
	char fname_s_bottom[MAX_FILENAME_LEN] = "weapons5";
	char fname_s_bottom_b[MAX_FILENAME_LEN] = "weapons5_b";

	if(gr_screen.res == GR_640) {
		coords[0] = 497;
		coords[1] = 273;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 880;
		coords[1] = 525;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	top_offset_x[0] = 12;
	top_offset_x[1] = -12;

	Weapon_header_offsets[0][0] = 21;
	Weapon_header_offsets[0][1] = 2;
	Weapon_header_offsets[1][0] = -10;
	Weapon_header_offsets[1][1] = 2;

	frame_offset_x[0] = 0;
	frame_offset_x[1] = -12;
	
	Weapon_plink_offset_x = 33;
	Weapon_pname_offset_x = 39;
	Weapon_pammo_offset_x = 28; 
	Weapon_sammo_offset_x = 28;
	Weapon_sname_offset_x = 39;
	Weapon_sreload_offset_x = 118;
	Weapon_slinked_offset_x = 28;
	Weapon_sunlinked_offset_x = 33;	

	top_primary_h = 20;
	top_secondary_h = 12;
	pname_start_offset_y = 12;
	sname_start_offset_y = 4;

	primary_text_h = 12;
	secondary_text_h = 9;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Primary List Top Background Filename:")) {
		stuff_string(fname_p_top, F_NAME, MAX_FILENAME_LEN);
		if(optional_string("Alt Ballistic Filename:")) {
			stuff_string(fname_p_top_b, F_NAME, MAX_FILENAME_LEN);
		}
	}
	if(optional_string("Primary List Middle Background Filename:")) {
		stuff_string(fname_p_middle, F_NAME, MAX_FILENAME_LEN);
		if(optional_string("Alt Ballistic Filename:")) {
			stuff_string(fname_p_middle_b, F_NAME, MAX_FILENAME_LEN);
		}
	}
	if(optional_string("Primary List Bottom Background Filename:")) {
		stuff_string(fname_p_last, F_NAME, MAX_FILENAME_LEN);
		if(optional_string("Alt Ballistic Filename:")) {
			stuff_string(fname_p_last_b, F_NAME, MAX_FILENAME_LEN);
		}
	}
	if(optional_string("Secondary List Top Background Filename:")) {
		stuff_string(fname_s_top, F_NAME, MAX_FILENAME_LEN);
		if(optional_string("Alt Ballistic Filename:")) {
			stuff_string(fname_s_top_b, F_NAME, MAX_FILENAME_LEN);
		}
	}
	if(optional_string("Secondary List Entry Background Filename:")) {
		stuff_string(fname_s_middle, F_NAME, MAX_FILENAME_LEN);
		if(optional_string("Alt Ballistic Filename:")) {
			stuff_string(fname_s_middle_b, F_NAME, MAX_FILENAME_LEN);
		}
	}
	if(optional_string("Secondary List Bottom Background Filename:")) {
		stuff_string(fname_s_bottom, F_NAME, MAX_FILENAME_LEN);
		if(optional_string("Alt Ballistic Filename:")) {
			stuff_string(fname_s_bottom_b, F_NAME, MAX_FILENAME_LEN);
		}
	}
	if(optional_string("Header Offsets:")) {
		stuff_int_list(Weapon_header_offsets[0], 2);
		if(optional_string("Alt Ballistic Offsets:")) {
			stuff_int_list(Weapon_header_offsets[1], 2);
		}
	}
	if(optional_string("Top Primary Background X-offset:")) {
		stuff_int(&top_offset_x[0]);
		if(optional_string("Alt Ballistic X-offset:")) {
			stuff_int(&top_offset_x[1]);
		}
	}
	if(optional_string("Text X-offset:")) {
		stuff_int(&frame_offset_x[0]);
		if(optional_string("Alt Ballistic X-offset:")) {
			stuff_int(&frame_offset_x[1]);
		}
	}
	if(optional_string("Top Primary Frame Height:")) {
		stuff_int(&top_primary_h);
	}
	if(optional_string("Top Secondary Frame Height:")) {
		stuff_int(&top_secondary_h);
	}
	if(optional_string("Primary List Start Y-offset:")) {
		stuff_int(&pname_start_offset_y);
	}
	if(optional_string("Secondary List Start Y-offset:")) {
		stuff_int(&sname_start_offset_y);
	}
	if(optional_string("Primary Weapon Ammo X-offset:")) {
		stuff_int(&Weapon_pammo_offset_x);
	}
	if(optional_string("Primary Weapon Link X-offset:")) {
		stuff_int(&Weapon_plink_offset_x);
	}
	if(optional_string("Primary Weapon Name X-offset:")) {
		stuff_int(&Weapon_pname_offset_x);
	}
	if(optional_string("Secondary Weapon Ammo X-offset:")) {
		stuff_int(&Weapon_sammo_offset_x);
	}
	if(optional_string("Secondary Weapon Unlinked X-offset:")) {
		stuff_int(&Weapon_sunlinked_offset_x);
	}
	if(optional_string("Secondary Weapon Linked X-offset:")) {
		stuff_int(&Weapon_slinked_offset_x);
	}
	if(optional_string("Secondary Weapon Name X-offset:")) {
		stuff_int(&Weapon_sname_offset_x);
	}
	if(optional_string("Secondary Weapon Reload X-offset:")) {
		stuff_int(&Weapon_sreload_offset_x);
	}
	if(optional_string("Primary Weapon Entry Height:")) {
		stuff_int(&primary_text_h);
	}
	if(optional_string("Secondary Weapon Entry Height:")) {
		stuff_int(&secondary_text_h);
	}

	HudGaugeWeapons* hud_gauge = new HudGaugeWeapons();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initBitmapsPrimaryTop(fname_p_top, fname_p_top_b);
	hud_gauge->initBitmapsPrimaryMiddle(fname_p_middle, fname_p_middle_b);
	hud_gauge->initBitmapsPrimaryLast(fname_p_last, fname_p_last_b);
	hud_gauge->initBitmapsSecondaryTop(fname_s_top, fname_s_top_b);
	hud_gauge->initBitmapsSecondaryMiddle(fname_s_middle, fname_s_middle_b);
	hud_gauge->initBitmapsSecondaryBottom(fname_s_bottom, fname_s_bottom_b);
	hud_gauge->initTopOffsetX(top_offset_x[0], top_offset_x[1]);
	hud_gauge->initHeaderOffsets(Weapon_header_offsets[0][0], Weapon_header_offsets[0][1], 
		Weapon_header_offsets[1][0], Weapon_header_offsets[1][1]);
	hud_gauge->initFrameOffsetX(frame_offset_x[0], frame_offset_x[1]);
	hud_gauge->initStartNameOffsetsY(pname_start_offset_y, sname_start_offset_y);
	hud_gauge->initPrimaryWeaponOffsets(Weapon_plink_offset_x, Weapon_pname_offset_x, Weapon_pammo_offset_x);
	hud_gauge->initSecondaryWeaponOffsets(Weapon_sammo_offset_x, Weapon_sname_offset_x, Weapon_sreload_offset_x, Weapon_slinked_offset_x, Weapon_sunlinked_offset_x);
	hud_gauge->initPrimaryHeights(top_primary_h, primary_text_h);
	hud_gauge->initSecondaryHeights(top_secondary_h, secondary_text_h);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_directives(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int header_offsets[2];
	int middle_frame_offset_y;
	int text_start_offsets[2];
	int text_h;
	char fname_top[MAX_FILENAME_LEN] = "directives1";
	char fname_middle[MAX_FILENAME_LEN] = "directives2";
	char fname_bottom[MAX_FILENAME_LEN] = "directives3";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 5;
		coords[1] = 178;
		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 5;
		coords[1] = 278;
		base_res[0] = 1024;
		base_res[1] = 768;
	}

	header_offsets[0] = 2;
	header_offsets[1] = 2;
	middle_frame_offset_y = 12;
	text_start_offsets[0] = 3;
	text_start_offsets[1] = 13;
	text_h = 9;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
		
		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Top Background Filename:")) {
		stuff_string(fname_top, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Entry Background Filename:")) {
		stuff_string(fname_middle, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Bottom Background Filename:")) {
		stuff_string(fname_bottom, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Header Offsets:")) {
		stuff_int_list(header_offsets, 2);
	}
	if(optional_string("Top Background Height:")) {
		stuff_int(&middle_frame_offset_y);
	}
	if(optional_string("List Start Offsets:")) {
		stuff_int_list(text_start_offsets, 2);
	}
	if(optional_string("Entry Height:")) {
		stuff_int(&text_h);
	}

	HudGaugeDirectives* hud_gauge = new HudGaugeDirectives();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname_top, fname_middle, fname_bottom);
	hud_gauge->initMiddleFrameOffsetY(middle_frame_offset_y);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initTextHeight(text_h);
	hud_gauge->initTextStartOffsets(text_start_offsets[0], text_start_offsets[1]);
	hud_gauge->initHeaderOffsets(header_offsets[0], header_offsets[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_talking_head(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Header_offsets[2];
	int Anim_offsets[2];
	int Anim_size[2];
	char fname[MAX_FILENAME_LEN] = "head1";
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 5;
		coords[1] = 35;
		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 5;
		coords[1] = 56;
		base_res[0] = 1024;
		base_res[1] = 768;
	}

	Header_offsets[0] = 2;
	Header_offsets[1] = 2;	
	Anim_offsets[0] = 2;
	Anim_offsets[1] = 10;
	Anim_size[0] = 160;
	Anim_size[1] = 120;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
		
		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Header Offsets:")) {
		stuff_int_list(Header_offsets, 2);
	}
	if(optional_string("Animation Offsets:")) {
		stuff_int_list(Anim_offsets, 2);
	}
	if(optional_string("Animation Background Size:")) {
		stuff_int_list(Anim_size, 2);
	}

	HudGaugeTalkingHead* hud_gauge = new HudGaugeTalkingHead();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initAnimOffsets(Anim_offsets[0], Anim_offsets[1]);
	hud_gauge->initAnimSizes(Anim_size[0], Anim_size[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initHeaderOffsets(Header_offsets[0], Header_offsets[1]);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_countermeasures(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int cm_text_offset[2];
	int cm_text_val_offset[2];
	char fname[MAX_FILENAME_LEN] = "countermeasure1";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 497;
		coords[1] = 343;
		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 880;
		coords[1] = 602;
		base_res[0] = 1024;
		base_res[1] = 768;
	}
	cm_text_offset[0] = 36;
	cm_text_offset[1] = 4;
	cm_text_val_offset[0] = 9;
	cm_text_val_offset[1] = 4;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}	
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Text Offsets:")) {
		stuff_int_list(cm_text_offset, 2);
	}
	if(optional_string("Value Offsets:")) {
		stuff_int_list(cm_text_val_offset, 2);
	}

	HudGaugeCmeasures* hud_gauge = new HudGaugeCmeasures();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initCountTextOffsets(cm_text_offset[0], cm_text_offset[1]);
	hud_gauge->initCountValueOffsets(cm_text_val_offset[0], cm_text_val_offset[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_auto_target(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int auto_text_offset[2];
	int target_text_offset[2];
	char fname[MAX_FILENAME_LEN] = "toggle1";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		base_res[0] = 640;
		base_res[1] = 480;
		coords[0] = 577;
		coords[1] = 380;
	} else {
		base_res[0] = 1024;
		base_res[1] = 768;
		coords[0] = 960;
		coords[1] = 648;
	}

	auto_text_offset[0] = 13;
	auto_text_offset[1] = 2;
	target_text_offset[0] = 7;
	target_text_offset[1] = 10;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}
	
	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Auto Offsets:")) {
		stuff_int_list(auto_text_offset, 2);
	}
	if(optional_string("Target Offsets:")) {
		stuff_int_list(target_text_offset, 2);
	}

	HudGaugeAutoTarget* hud_gauge = new HudGaugeAutoTarget();
	hud_gauge->initAutoTextOffsets(auto_text_offset[0], auto_text_offset[1]);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initTargetTextOffsets(target_text_offset[0], target_text_offset[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_auto_speed(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int auto_text_offset[2];
	int speed_text_offset[2];
	char fname[MAX_FILENAME_LEN] = "toggle1";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		base_res[0] = 640;
		base_res[1] = 480;
		coords[0] = 577;
		coords[1] = 404;
	} else {
		base_res[0] = 1024;
		base_res[1] = 768;
		coords[0] = 960;
		coords[1] = 672;
	}

	auto_text_offset[0] = 13;
	auto_text_offset[1] = 2;
	speed_text_offset[0] = 10;
	speed_text_offset[1] = 10;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}
	
	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Auto Offsets:")) {
		stuff_int_list(auto_text_offset, 2);
	}
	if(optional_string("Speed Offsets:")) {
		stuff_int_list(speed_text_offset, 2);
	}

	HudGaugeAutoSpeed* hud_gauge = new HudGaugeAutoSpeed();
	hud_gauge->initAutoTextOffsets(auto_text_offset[0], auto_text_offset[1]);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initSpeedTextOffsets(speed_text_offset[0], speed_text_offset[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_wingman_status(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int header_offsets[2];
	int left_frame_end_x;
	
	int single_wing_offsets[2];
	int multiple_wing_offsets[2];
	int wing_width;
	int wing_name_offsets[2];
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	int wingmate_offsets[MAX_SHIPS_PER_WING][2];
	char fname_left[MAX_FILENAME_LEN] = "wingman1";
	char fname_middle[MAX_FILENAME_LEN] = "wingman2";
	char fname_right[MAX_FILENAME_LEN] = "wingman3";
	char fname_dots[MAX_FILENAME_LEN] = "wingman4";
	// "wingman5" isn't used anymore since Goober implemented string based wing names

	if(gr_screen.res == GR_640) {
		coords[0] = 550;
		coords[1] = 114;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 932;
		coords[1] = 144;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	header_offsets[0] = 2;
	header_offsets[1] = 2;
	left_frame_end_x = 71;
	
	single_wing_offsets[0] = 28;
	single_wing_offsets[1] = 15;
	multiple_wing_offsets[0] = 46;
	multiple_wing_offsets[1] = 15;
	wing_width = 35;
	wing_name_offsets[0] = 15;
	wing_name_offsets[1] = 26;

	wingmate_offsets[0][0] = 11;
	wingmate_offsets[0][1] = 0;
	wingmate_offsets[1][0] = 4;
	wingmate_offsets[1][1] = 8;
	wingmate_offsets[2][0] = 18;
	wingmate_offsets[2][1] = 8;
	wingmate_offsets[3][0] = 11;
	wingmate_offsets[3][1] = 16;
	wingmate_offsets[4][0] = 0;
	wingmate_offsets[4][1] = 16;
	wingmate_offsets[5][0] = 22;
	wingmate_offsets[5][1] = 16;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Left Background Filename:")) {
		stuff_string(fname_left, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Entry Background Filename:")) {
		stuff_string(fname_middle, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Right Background Filename:")) {
		stuff_string(fname_right, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Dot Filename:")) {
		stuff_string(fname_dots, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Header Offsets:")) {
		stuff_int_list(header_offsets, 2);
	}
	if(optional_string("Left Background Width:")) {
		stuff_int(&left_frame_end_x);
	}
	if(optional_string("Entry Width:")) {
		stuff_int(&wing_width);
	}
	if(optional_string("Single Wing Offsets:")) {
		stuff_int_list(single_wing_offsets, 2);
	}
	if(optional_string("Multiple Wing Start Offsets:")) {
		stuff_int_list(multiple_wing_offsets, 2);
	}
	if(optional_string("Wing Name Offsets:")) {
		stuff_int_list(wing_name_offsets, 2);
	}
	if(optional_string("Dot Offsets:")) {
		stuff_int_list(wingmate_offsets[0], 2);
		stuff_int_list(wingmate_offsets[1], 2);
		stuff_int_list(wingmate_offsets[2], 2);
		stuff_int_list(wingmate_offsets[3], 2);
		stuff_int_list(wingmate_offsets[4], 2);
		stuff_int_list(wingmate_offsets[5], 2);
	}
	
	int grow_mode = 0; //By default, expand the gauge to the left (in -x direction)

	if(optional_string("Expansion Mode:")) {
		if(required_string("Right")) 
			grow_mode = 1;
	}

	HudGaugeWingmanStatus* hud_gauge = new HudGaugeWingmanStatus();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname_left, fname_middle, fname_right, fname_dots);
	hud_gauge->initHeaderOffsets(header_offsets[0], header_offsets[1]);
	hud_gauge->initLeftFrameEndX(left_frame_end_x);
	hud_gauge->initMultipleWingOffsets(multiple_wing_offsets[0], multiple_wing_offsets[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initSingleWingOffsets(single_wing_offsets[0], single_wing_offsets[1]);
	hud_gauge->initWingmate1Offsets(wingmate_offsets[0][0], wingmate_offsets[0][1]);
	hud_gauge->initWingmate2Offsets(wingmate_offsets[1][0],	wingmate_offsets[1][1]);
	hud_gauge->initWingmate3Offsets(wingmate_offsets[2][0],	wingmate_offsets[2][1]);
	hud_gauge->initWingmate4Offsets(wingmate_offsets[3][0],	wingmate_offsets[3][1]);
	hud_gauge->initWingmate5Offsets(wingmate_offsets[4][0],	wingmate_offsets[4][1]);
	hud_gauge->initWingmate6Offsets(wingmate_offsets[5][0], wingmate_offsets[5][1]);
	hud_gauge->initWingNameOffsets(wing_name_offsets[0], wing_name_offsets[1]);
	hud_gauge->initWingWidth(wing_width);
	hud_gauge->initGrowMode(grow_mode);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_damage(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int header_offsets[2];
	int hull_integ_offsets[2];
	int hull_integ_val_offset_x;
	int middle_frame_start_offset_y;
	int subsys_integ_start_offsets[2];
	int subsys_integ_val_offset_x;
	int line_h;
	char fname_top[MAX_FILENAME_LEN] = "damage1";
	char fname_middle[MAX_FILENAME_LEN] = "damage2";
	char fname_bottom[MAX_FILENAME_LEN] = "damage3";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 245;
		coords[1] = 38;
		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 440;
		coords[1] = 61;
		base_res[0] = 1024;
		base_res[1] = 768;
	}
	header_offsets[0] = 3;
	header_offsets[1] = 2;
	hull_integ_offsets[0] = 4;
	hull_integ_offsets[1] = 15;
	hull_integ_val_offset_x = 142;
	middle_frame_start_offset_y = 25;
	subsys_integ_start_offsets[0] = 4;
	subsys_integ_start_offsets[1] = 27;
	subsys_integ_val_offset_x = 142;
	line_h = 9;

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Top Background Filename:")) {
		stuff_string(fname_top, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Entry Background Filename:")) {
		stuff_string(fname_middle, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Bottom Background Filename:")) {
		stuff_string(fname_bottom, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Header Offsets:")) {
		stuff_int_list(header_offsets, 2);
	}
	if(optional_string("Hull Integrity Offsets:")) {
		stuff_int_list(hull_integ_offsets, 2);
	}
	if(optional_string("Hull Integrity Value X-offset:")) {
		stuff_int(&hull_integ_val_offset_x);
	}
	if(optional_string("Top Background Height:")) {
		stuff_int(&middle_frame_start_offset_y);
	}
	if(optional_string("Subsystem Entry Height:")) {
		stuff_int(&line_h);
	}
	if(optional_string("Subsystem List Start Offsets:")) {
		stuff_int_list(subsys_integ_start_offsets, 2);
	}
	if(optional_string("Subsystem Entry Value X-offset:")) {
		stuff_int(&subsys_integ_val_offset_x);
	}

	HudGaugeDamage* hud_gauge = new HudGaugeDamage();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname_top, fname_middle, fname_bottom);
	hud_gauge->initHullIntegOffsets(hull_integ_offsets[0], hull_integ_offsets[1]);
	hud_gauge->initHullIntegValueOffsetX(hull_integ_val_offset_x);
	hud_gauge->initLineHeight(line_h);
	hud_gauge->initMiddleFrameStartOffsetY(middle_frame_start_offset_y);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initSubsysIntegStartOffsets(subsys_integ_start_offsets[0], subsys_integ_start_offsets[1]);
	hud_gauge->initSubsysIntegValueOffsetX(subsys_integ_val_offset_x);
	hud_gauge->initHeaderOffsets(header_offsets[0], header_offsets[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_support(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int header_offsets[2];
	int text_val_offset_y;
	int text_dock_offset_x;
	int text_dock_val_offset_x;
	char fname[MAX_FILENAME_LEN] = "support1";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 265;
		coords[1] = 334;

		base_res[0] = 640;
		base_res[1] = 480;

		header_offsets[0] = 2;
		header_offsets[1] = 1;
		text_val_offset_y = 14;
		text_dock_offset_x = 5;
		text_dock_val_offset_x = 63;
	} else {
		coords[0] = 459;
		coords[1] = 534;

		base_res[0] = 1024;
		base_res[1] = 768;

		header_offsets[0] = 3;
		header_offsets[1] = 2;
		text_val_offset_y = 12;
		text_dock_offset_x = 6;
		text_dock_val_offset_x = 65;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Header Offsets:")) {
		stuff_int_list(header_offsets, 2);
	}
	if(optional_string("Text Y-offset:")) {
		stuff_int(&text_val_offset_y);
	}
	if(optional_string("Dock Status X-offset:")) {
		stuff_int(&text_dock_offset_x);
	}
	if(optional_string("Dock Time X-offset:")) {
		stuff_int(&text_dock_val_offset_x);
	}

	HudGaugeSupport* hud_gauge = new HudGaugeSupport();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initHeaderOffsets(header_offsets[0], header_offsets[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initTextDockOffsetX(text_dock_offset_x);
	hud_gauge->initTextDockValueOffsetX(text_dock_val_offset_x);
	hud_gauge->initTextValueOffsetY(text_val_offset_y);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_training_messages(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int font_num = FONT1;

	if(gr_screen.res == GR_640) {
		coords[0] = 174;
		coords[1] = 40;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 379;
		coords[1] = 125;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
		
		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}

	HudGaugeTrainingMessages* hud_gauge = new HudGaugeTrainingMessages();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initFont(font_num);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_messages(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];

	int max_lines = 3;
	int max_width;
	int scroll_time = 30;
	int step_size = 3;
	int total_life = 14000;
	int line_height = 12;
	bool hidden_by_comms_menu = true;

	coords[0] = 5;
	coords[1] = 5;
	int font_num = FONT1;
	bool slew = false;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		base_res[0] = 640;
		base_res[1] = 480;

		max_width = 620;
	} else {
		base_res[0] = 1024;
		base_res[1] = 768;

		max_width = 1004;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}

	if ( optional_string("Slew:") ) {
		stuff_boolean(&slew);
	}
	if ( optional_string("Max Lines:") ) {
		stuff_int(&max_lines);
	}
	if ( optional_string("Max Width:") ) {
		stuff_int(&max_width);
	}
	if ( optional_string("Line Height:") ) {
		stuff_int(&line_height);
	}
	if ( optional_string("Total Lifetime:") ) {
		stuff_int(&total_life);
	}
	if ( optional_string("Scroll Time:") ) {
		stuff_int(&scroll_time);
	}
	if ( optional_string("Step Size:") ) {
		stuff_int(&step_size);
	}
	if ( optional_string("Hidden By Comms Menu:") ) {
		stuff_boolean(&hidden_by_comms_menu);
	}

	HudGaugeMessages* hud_gauge = new HudGaugeMessages();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initFont(font_num);
	hud_gauge->initSlew(slew);
	hud_gauge->initMaxLines(max_lines);
	hud_gauge->initMaxWidth(max_width);
	hud_gauge->initScrollTime(scroll_time);
	hud_gauge->initStepSize(step_size);
	hud_gauge->initTotalLife(total_life);
	hud_gauge->initLineHeight(line_height);
	hud_gauge->initHiddenByCommsMenu(hidden_by_comms_menu);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_fixed_messages(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];

	gr_set_font(FONT1);
	int h = gr_get_font_height();

	coords[0] = 0x8000; //Magic number, means "Center on X"
	coords[1] = 5 + (h * 3);
	int font_num = FONT1;

	if(gr_screen.res == GR_640) {
		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		base_res[0] = 1024;
		base_res[1] = 768;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}

	HudGaugeFixedMessages* hud_gauge = new HudGaugeFixedMessages();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initFont(font_num);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_weapon_linking(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Weapon_link_offsets[NUM_WEAPON_LINK_MODES][2];
	char fname_arc[MAX_FILENAME_LEN];
	char fname_primary_link_1[MAX_FILENAME_LEN];
	char fname_primary_link_2[MAX_FILENAME_LEN];
	char fname_secondary_link_1[MAX_FILENAME_LEN];
	char fname_secondary_link_2[MAX_FILENAME_LEN];
	char fname_secondary_link_3[MAX_FILENAME_LEN];
	bool slew = true;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 374;
		coords[1] = 242;

		base_res[0] = 640;
		base_res[1] = 480;

		Weapon_link_offsets[LINK_ONE_PRIMARY][0] = 32;
		Weapon_link_offsets[LINK_ONE_PRIMARY][1] = 11;
		Weapon_link_offsets[LINK_TWO_PRIMARY][0] = 32;
		Weapon_link_offsets[LINK_TWO_PRIMARY][1] = 11;
		Weapon_link_offsets[LINK_ONE_SECONDARY][0] = 17;
		Weapon_link_offsets[LINK_ONE_SECONDARY][1] = 34;
		Weapon_link_offsets[LINK_TWO_SECONDARY][0] = 17;
		Weapon_link_offsets[LINK_TWO_SECONDARY][1] = 34;
		Weapon_link_offsets[LINK_THREE_SECONDARY][0] = 17;
		Weapon_link_offsets[LINK_THREE_SECONDARY][1] = 34;

		strcpy_s(fname_arc, "rightarc1_fs1");
		strcpy_s(fname_primary_link_1, "rightarc2_fs1");
		strcpy_s(fname_primary_link_2, "rightarc3_fs1");
		strcpy_s(fname_secondary_link_1, "rightarc4_fs1");
		strcpy_s(fname_secondary_link_2, "rightarc5_fs1");
		strcpy_s(fname_secondary_link_3, "rightarc6_fs1");
	} else {
		coords[0] = 598;
		coords[1] = 387;

		base_res[0] = 1024;
		base_res[1] = 768;

		Weapon_link_offsets[LINK_ONE_PRIMARY][0] = 52;
		Weapon_link_offsets[LINK_ONE_PRIMARY][1] = 18;
		Weapon_link_offsets[LINK_TWO_PRIMARY][0] = 52;
		Weapon_link_offsets[LINK_TWO_PRIMARY][1] = 18;
		Weapon_link_offsets[LINK_ONE_SECONDARY][0] = 28;
		Weapon_link_offsets[LINK_ONE_SECONDARY][1] = 55;
		Weapon_link_offsets[LINK_TWO_SECONDARY][0] = 28;
		Weapon_link_offsets[LINK_TWO_SECONDARY][1] = 55;
		Weapon_link_offsets[LINK_THREE_SECONDARY][0] = 28;
		Weapon_link_offsets[LINK_THREE_SECONDARY][1] = 55;

		strcpy_s(fname_arc, "2_rightarc1_fs1");
		strcpy_s(fname_primary_link_1, "2_rightarc2_fs1");
		strcpy_s(fname_primary_link_2, "2_rightarc3_fs1");
		strcpy_s(fname_secondary_link_1, "2_rightarc4_fs1");
		strcpy_s(fname_secondary_link_2, "2_rightarc5_fs1");
		strcpy_s(fname_secondary_link_3, "2_rightarc6_fs1");
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Arc Filename:")) {
		stuff_string(fname_arc, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Single Primary Filename:")) {
		stuff_string(fname_primary_link_1, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Double Primary Filename:")) {
		stuff_string(fname_primary_link_2, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Single Secondary Filename:")) {
		stuff_string(fname_secondary_link_1, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Double Secondary Filename:")) {
		stuff_string(fname_secondary_link_2, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Triple Secondary Filename:")) {
		stuff_string(fname_secondary_link_3, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Single Primary Offsets:")) {
		stuff_int_list(Weapon_link_offsets[LINK_ONE_PRIMARY], 2);
	}
	if(optional_string("Double Primary Offsets:")) {
		stuff_int_list(Weapon_link_offsets[LINK_TWO_PRIMARY], 2);
	}
	if(optional_string("Single Secondary Offsets:")) {
		stuff_int_list(Weapon_link_offsets[LINK_ONE_SECONDARY], 2);
	}
	if(optional_string("Double Secondary Offsets:")) {
		stuff_int_list(Weapon_link_offsets[LINK_TWO_SECONDARY], 2);
	}
	if(optional_string("Triple Secondary Offsets:")) {
		stuff_int_list(Weapon_link_offsets[LINK_THREE_SECONDARY], 2);
	}

	HudGaugeWeaponLinking* hud_gauge = new HudGaugeWeaponLinking();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->init1PrimaryOffsets(Weapon_link_offsets[LINK_ONE_PRIMARY][0], Weapon_link_offsets[LINK_ONE_PRIMARY][1]);
	hud_gauge->init2PrimaryOffsets(Weapon_link_offsets[LINK_TWO_PRIMARY][0], Weapon_link_offsets[LINK_TWO_PRIMARY][1]);
	hud_gauge->init1SecondaryOffsets(Weapon_link_offsets[LINK_ONE_SECONDARY][0], Weapon_link_offsets[LINK_ONE_SECONDARY][1]);
	hud_gauge->init2SecondaryOffsets(Weapon_link_offsets[LINK_TWO_SECONDARY][0], Weapon_link_offsets[LINK_TWO_SECONDARY][1]);
	hud_gauge->init3SecondaryOffsets(Weapon_link_offsets[LINK_THREE_SECONDARY][0], Weapon_link_offsets[LINK_THREE_SECONDARY][1]);
	hud_gauge->initBitmaps(fname_arc, fname_primary_link_1, fname_primary_link_2, fname_secondary_link_1, fname_secondary_link_2, fname_secondary_link_3);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_multi_msg(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int font_num = FONT1;

	if(gr_screen.res == GR_640) {
		coords[0] = 5;
		coords[1] = 150;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 8;
		coords[1] = 240;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}

	HudGaugeMultiMsg* hud_gauge = new HudGaugeMultiMsg();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initFont(font_num);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_voice_status(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 5;
		coords[1] = 165;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 8;
		coords[1] = 255;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}

	HudGaugeVoiceStatus* hud_gauge = new HudGaugeVoiceStatus();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_ping(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 560;
		coords[1] = 3;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 896;
		coords[1] = 5;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}

	HudGaugePing* hud_gauge = new HudGaugePing();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_supernova(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 100;
		coords[1] = 100;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 170;
		coords[1] = 170;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}

	HudGaugeSupernova* hud_gauge = new HudGaugeSupernova();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_lock(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int base_res[2];
	int Lock_gauge_half_w;
	int Lock_gauge_half_h;
	int Lockspin_half_w;
	int Lockspin_half_h;
	float Lock_triangle_height;
	float Lock_triangle_base;
	int Lock_target_box_width;
	int Lock_target_box_height;
	bool loop_locked_anim;
	char fname_lock[MAX_FILENAME_LEN];
	char fname_spin[MAX_FILENAME_LEN];
	int font_num = FONT1;

	if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
		if(gr_screen.res == GR_640) {
			base_res[0] = 640;
			base_res[1] = 480;

			Lock_gauge_half_w = 15;
			Lock_gauge_half_h = 15;
			Lockspin_half_w = 16;
			Lockspin_half_h = 16;
			Lock_triangle_height = 4.0f;
			Lock_triangle_base = 4.0f;
			Lock_target_box_width = 19;
			Lock_target_box_height = 30;
			loop_locked_anim = true;

			strcpy_s(fname_lock, "lock1_fs1");
			strcpy_s(fname_spin, "lockspin_fs1");
		} else {
			base_res[0] = 1024;
			base_res[1] = 768;

			Lock_gauge_half_w = 24;
			Lock_gauge_half_h = 25;
			Lockspin_half_w = 26;
			Lockspin_half_h = 26;
			Lock_triangle_height = 6.5f;
			Lock_triangle_base = 6.5f;
			Lock_target_box_width = 19;
			Lock_target_box_height = 30;
			loop_locked_anim = true;

			strcpy_s(fname_lock, "2_lock1_fs1");
			strcpy_s(fname_spin, "2_lockspin_fs1");
		}
	} else {
		if(gr_screen.res == GR_640) {
			base_res[0] = 640;
			base_res[1] = 480;

			Lock_gauge_half_w = 17;
			Lock_gauge_half_h = 15;
			Lockspin_half_w = 31;
			Lockspin_half_h = 32;
			Lock_triangle_height = 4.0f;
			Lock_triangle_base = 4.0f;
			Lock_target_box_width = 19;
			Lock_target_box_height = 30;
			loop_locked_anim = false;

			strcpy_s(fname_lock, "lock1");
			strcpy_s(fname_spin, "lockspin");
		} else {
			base_res[0] = 1024;
			base_res[1] = 768;

			Lock_gauge_half_w = 28;
			Lock_gauge_half_h = 25;
			Lockspin_half_w = 50;
			Lockspin_half_h = 52;
			Lock_triangle_height = 6.5f;
			Lock_triangle_base = 6.5f;
			Lock_target_box_width = 19;
			Lock_target_box_height = 30;
			loop_locked_anim = false;

			strcpy_s(fname_lock, "2_lock1");
			strcpy_s(fname_spin, "2_lockspin");
		}
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Lock Filename:")) {
		stuff_string(fname_lock, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Locked Filename:")) {
		stuff_string(fname_spin, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Lock Center Offsets:")) {
		int temp[2];

		stuff_int_list(temp, 2);

		Lock_gauge_half_w = temp[0];
		Lock_gauge_half_h = temp[1];
	}
	if(optional_string("Locked Center Offsets:")) {
		int temp[2];

		stuff_int_list(temp, 2);

		Lockspin_half_w = temp[0];
		Lockspin_half_h = temp[1];
	}

	HudGaugeLock* hud_gauge = new HudGaugeLock();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname_lock, fname_spin);
	hud_gauge->initLoopLockedAnim(loop_locked_anim);
	hud_gauge->initGaugeHalfSize(Lock_gauge_half_w, Lock_gauge_half_h);
	hud_gauge->initSpinHalfSize(Lockspin_half_w, Lockspin_half_h);
	hud_gauge->initTriHeight(Lock_triangle_height);
	hud_gauge->initTriBase(Lock_triangle_base);
	hud_gauge->initTargetBoxSize(Lock_target_box_width,	Lock_target_box_height);
	hud_gauge->initFont(font_num);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_offscreen(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int base_res[2];
	float Max_offscreen_tri_seperation;
	float Max_front_seperation;
	float Offscreen_tri_base;
	float Offscreen_tri_height;
	int font_num = FONT1;

	if(gr_screen.res == GR_640) {
		base_res[0] = 640;
		base_res[1] = 480;

		Max_offscreen_tri_seperation = 10.0f;
		Max_front_seperation = 10.0f;
		Offscreen_tri_base = 6.0f;
		Offscreen_tri_height = 7.0f;
	} else {
		base_res[0] = 1024;
		base_res[1] = 768;

		Max_offscreen_tri_seperation = 16.0f;
		Max_front_seperation = 16.0f;
		Offscreen_tri_base = 9.5f;
		Offscreen_tri_height = 11.0f;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}

	HudGaugeOffscreen* hud_gauge = new HudGaugeOffscreen();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initMaxTriSeperation(Max_offscreen_tri_seperation);
	hud_gauge->initMaxFrontSeperation(Max_front_seperation);
	hud_gauge->initTriBase(Offscreen_tri_base);
	hud_gauge->initTriHeight(Offscreen_tri_height);
	hud_gauge->initFont(font_num);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_brackets(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int base_res[2];
	int min_target_box[2];
	int min_subtarget_box[2];
	char fname[MAX_FILENAME_LEN] = "attacker";
	int font_num = FONT1;

	if(gr_screen.res == GR_640) {
		base_res[0] = 640;
		base_res[1] = 480;
		min_target_box[0] = 20;
		min_target_box[1] = 20;
		min_subtarget_box[0] = 12;
		min_subtarget_box[1] = 12;
	} else {
		base_res[0] = 1024;
		base_res[1] = 768;
		min_target_box[0] = 30;
		min_target_box[1] = 30;
		min_subtarget_box[0] = 24;
		min_subtarget_box[1] = 24;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Dot Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}

	HudGaugeBrackets* hud_gauge = new HudGaugeBrackets();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initMinSubTargetBoxSizes(min_subtarget_box[0], min_subtarget_box[1]);
	hud_gauge->initMinTargetBoxSizes(min_target_box[0], min_target_box[1]);
	hud_gauge->initFont(font_num);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_hostile_tri(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Radius; 
	float Target_triangle_base;
	float Target_triangle_height;
	bool slew = true;
	int font_num = FONT1;

	if(gr_screen.res == GR_640) {
		coords[0] = 320;
		coords[1] = 242;

		base_res[0] = 640;
		base_res[1] = 480;

		Target_triangle_base = 6.0f;
		Target_triangle_height = 7.0f;
		Radius = 104;
	} else {
		coords[0] = 512;
		coords[1] = 387;

		base_res[0] = 1024;
		base_res[1] = 768;

		Target_triangle_base = 9.5f;
		Target_triangle_height = 11.0f;
		Radius = 166;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
		
		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Radius:")) {
		stuff_int(&Radius);
	}
	if(optional_string("Triangle Base:")) {
		stuff_float(&Target_triangle_base);
	}
	if(optional_string("Triangle Height:")) {
		stuff_float(&Target_triangle_height);
	}

	HudGaugeHostileTriangle* hud_gauge = new HudGaugeHostileTriangle();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initRadius(Radius);
	hud_gauge->initTriBase(Target_triangle_base);
	hud_gauge->initTriHeight(Target_triangle_height);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_target_tri(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Radius; 
	float Target_triangle_base;
	float Target_triangle_height;
	bool slew = true;
	int font_num = FONT1;

	if(gr_screen.res == GR_640) {
		coords[0] = 320;
		coords[1] = 242;

		base_res[0] = 640;
		base_res[1] = 480;

		Target_triangle_base = 6.0f;
		Target_triangle_height = 7.0f;
		Radius = 104;
	} else {
		coords[0] = 512;
		coords[1] = 387;

		base_res[0] = 1024;
		base_res[1] = 768;

		Target_triangle_base = 9.5f;
		Target_triangle_height = 11.0f;
		Radius = 166;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
		
		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Radius:")) {
		stuff_int(&Radius);
	}
	if(optional_string("Triangle Base:")) {
		stuff_float(&Target_triangle_base);
	}
	if(optional_string("Triangle Height:")) {
		stuff_float(&Target_triangle_height);
	}

	HudGaugeTargetTriangle* hud_gauge = new HudGaugeTargetTriangle();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initRadius(Radius);
	hud_gauge->initTriBase(Target_triangle_base);
	hud_gauge->initTriHeight(Target_triangle_height);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_missile_tri(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Radius; 
	float Target_triangle_base;
	float Target_triangle_height;
	bool slew = true;
	int font_num = FONT1;

	if(gr_screen.res == GR_640) {
		coords[0] = 320;
		coords[1] = 242;

		base_res[0] = 640;
		base_res[1] = 480;

		Target_triangle_base = 6.0f;
		Target_triangle_height = 7.0f;
		Radius = 104;
	} else {
		coords[0] = 512;
		coords[1] = 387;

		base_res[0] = 1024;
		base_res[1] = 768;

		Target_triangle_base = 9.5f;
		Target_triangle_height = 11.0f;
		Radius = 166;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
		
		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Radius:")) {
		stuff_int(&Radius);
	}
	if(optional_string("Triangle Base:")) {
		stuff_float(&Target_triangle_base);
	}
	if(optional_string("Triangle Height:")) {
		stuff_float(&Target_triangle_height);
	}

	HudGaugeMissileTriangles* hud_gauge = new HudGaugeMissileTriangles();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initRadius(Radius);
	hud_gauge->initTriBase(Target_triangle_base);
	hud_gauge->initTriHeight(Target_triangle_height);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_lead(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int base_res[2];
	float Lead_indicator_half[2];
	char fname[MAX_FILENAME_LEN];
	int font_num = FONT1;

	if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
		if(gr_screen.res == GR_640) {
			base_res[0] = 640;
			base_res[1] = 480;

			Lead_indicator_half[0] = 12.5f;
			Lead_indicator_half[1] = 12.5f;

			strcpy_s(fname, "lead1_fs1");
		} else {
			base_res[0] = 1024;
			base_res[1] = 768;

			Lead_indicator_half[0] = 20.0f;
			Lead_indicator_half[1] = 20.0f;

			strcpy_s(fname, "2_lead1_fs1");
		}
	} else {
		if(gr_screen.res == GR_640) {
			base_res[0] = 640;
			base_res[1] = 480;

			Lead_indicator_half[0] = 8.0f;
			Lead_indicator_half[1] = 8.0f;

			strcpy_s(fname, "lead1");
		} else {
			base_res[0] = 1024;
			base_res[1] = 768;

			Lead_indicator_half[0] = 13.0f;
			Lead_indicator_half[1] = 13.0f;

			strcpy_s(fname, "2_lead1");
		}
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Center Offsets:")) {
		int temp[2];

		stuff_int_list(temp, 2);

		Lead_indicator_half[0] = i2fl(temp[0]);
		Lead_indicator_half[1] = i2fl(temp[1]);
	}

	HudGaugeLeadIndicator* hud_gauge = new HudGaugeLeadIndicator();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initHalfSize(Lead_indicator_half[0], Lead_indicator_half[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initFont(font_num);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_orientation_tee(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int Radius;
	bool slew = true;
	int font_num = FONT1;

	if(gr_screen.res == GR_640) {
		coords[0] = 320;
		coords[1] = 242;

		base_res[0] = 640;
		base_res[1] = 480;

		Radius = 104;
	} else {
		coords[0] = 512;
		coords[1] = 387;

		base_res[0] = 1024;
		base_res[1] = 768;

		Radius = 166;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Radius:")) {
		stuff_int(&Radius);
	}

	HudGaugeOrientationTee* hud_gauge = new HudGaugeOrientationTee();
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initRadius(Radius);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_lead_sight(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	char fname[MAX_FILENAME_LEN] = "leadsight";
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 320;
		coords[1] = 242;

		base_res[0] = 640;
		base_res[1] = 480;
	} else {
		coords[0] = 512;
		coords[1] = 387;

		base_res[0] = 1024;
		base_res[1] = 768;
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}

	HudGaugeLeadSight* hud_gauge = new HudGaugeLeadSight();
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initFont(font_num);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}

void load_gauge_kills(int base_w, int base_h, int hud_font, int ship_index, color *use_clr)
{
	int coords[2];
	int base_res[2];
	int text_offsets[2] = {6, 4};
	int text_value_offsets[2] = {74, 4};
	char fname[MAX_FILENAME_LEN] = "kills1";
	bool slew = false;
	int font_num = FONT1;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	if(gr_screen.res == GR_640) {
		coords[0] = 497;
		coords[1] = 361;
		base_res[0] = 640;
		base_res[1] = 480;
		
		if(Lcl_gr) {
			text_value_offsets[0] = 118;
			text_value_offsets[1] = 4;
		}
	} else {
		coords[0] = 880;
		coords[1] = 624;
		base_res[0] = 1024;
		base_res[1] = 768;

		if(Lcl_gr) {
			text_value_offsets[0] = 104;
			text_value_offsets[1] = 4;
		}
	}

	if(check_base_res(base_w, base_h)) {
		base_res[0] = base_w;
		base_res[1] = base_h;

		if(optional_string("Position:")) {
			stuff_int_list(coords, 2);
		}
	}

	if ( use_clr != NULL ) {
		colors[0] = use_clr->red;
		colors[1] = use_clr->green;
		colors[2] = use_clr->blue;

		lock_color = true;
	} else if ( optional_string("Color:") ) {
		stuff_int_list(colors, 3);

		check_color(colors);

		lock_color = true;
	}

	if ( optional_string("Font:") ) {
		stuff_int(&font_num);
	} else {
		if ( hud_font >=0 ) {
			font_num = hud_font;
		}
	}
	if(optional_string("Slew:")) {
		stuff_boolean(&slew);
	}
	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Text Offsets:")) {
		stuff_int_list(text_offsets, 2);
	}
	if(optional_string("Value Offsets:")) {
		stuff_int_list(text_value_offsets, 2);
	}

	HudGaugeKills* hud_gauge = new HudGaugeKills();
	hud_gauge->initPosition(coords[0], coords[1]);
	hud_gauge->initBaseResolution(base_res[0], base_res[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initTextOffsets(text_offsets[0], text_offsets[1]);
	hud_gauge->initTextValueOffsets(text_value_offsets[0], text_value_offsets[1]);
	hud_gauge->initSlew(slew);
	hud_gauge->initFont(font_num);

	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);

	if(ship_index >= 0) {
		Ship_info[ship_index].hud_gauges.push_back(hud_gauge);
	} else {
		default_hud_gauges.push_back(hud_gauge);
	}
}
