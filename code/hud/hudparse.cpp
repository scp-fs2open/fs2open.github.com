/*
 * Created by WMCoolmon for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

 

#include <cstddef>

#include "cmdline/cmdline.h"
#include "graphics/font.h" //for gr_force_fit_string
#include "hud/hud.h"
#include "hud/hudbrackets.h"
#include "hud/hudconfig.h" // for retrieving user's hud config
#include "hud/hudescort.h"
#include "hud/hudets.h"
#include "hud/hudlock.h"
#include "hud/hudmessage.h"
#include "hud/hudparse.h" //Duh.
#include "hud/hudreticle.h"
#include "hud/hudscripting.h"
#include "hud/hudshield.h"
#include "hud/hudsquadmsg.h"
#include "hud/hudtarget.h"
#include "hud/hudtargetbox.h" 
#include "hud/hudwingmanstatus.h"
#include "localization/localize.h"
#include "mission/missionmessage.h"
#include "mission/missiontraining.h"
#include "parse/parselo.h"
#include "radar/radar.h"
#include "radar/radardradis.h"
#include "radar/radarngon.h"
#include "radar/radarorb.h"
#include "radar/radarsetup.h"
#include "ship/ship.h" //for ship struct

//Global stuffs
extern bool Ships_inited; //Need this

float Hud_unit_multiplier = 1.0f;	//Backslash
float Hud_speed_multiplier = 1.0f;	//The E

// Goober5000
int Hud_reticle_style = HUD_RETICLE_STYLE_FS2;

bool Hud_retail = true;
bool Scale_retail_gauges = true;
int Force_scaling_above_res_global[2] = {-1, -1};

int Hud_font = -1;

bool Chase_view_only_ex = false;

//WARNING: If you add gauges to this array, make sure to bump Num_builtin_gauges!
const int Num_builtin_gauges = 42;
static int Builtin_gauges[Num_builtin_gauges] = {
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

Legacy_HUD_gauge_pair Legacy_HUD_gauges[NUM_HUD_GAUGES] =
{
	{ "LEAD_INDICATOR", HUD_OBJECT_LEAD },
	{ "ORIENTATION_TEE", HUD_OBJECT_ORIENTATION_TEE },
	{ "HOSTILE_TRIANGLE", HUD_OBJECT_HOSTILE_TRI },
	{ "TARGET_TRIANGLE", HUD_OBJECT_TARGET_TRI },
	{ "MISSION_TIME", HUD_OBJECT_MISSION_TIME },
	{ "RETICLE_CIRCLE", -1 },
	{ "THROTTLE_GAUGE", HUD_OBJECT_THROTTLE },
	{ "RADAR", HUD_OBJECT_RADAR_STD },
	{ "TARGET_MONITOR", HUD_OBJECT_TARGET_MONITOR },
	{ "CENTER_RETICLE", HUD_OBJECT_CENTER_RETICLE },
	{ "TARGET_MONITOR_EXTRA_DATA", HUD_OBJECT_EXTRA_TARGET_DATA },
	{ "TARGET_SHIELD_ICON", HUD_OBJECT_TARGET_SHIELD },
	{ "PLAYER_SHIELD_ICON", HUD_OBJECT_PLAYER_SHIELD },
	{ "ETS_GAUGE", HUD_OBJECT_ETS_RETAIL },
	{ "AUTO_TARGET", HUD_OBJECT_AUTO_TARGET },
	{ "AUTO_SPEED", HUD_OBJECT_AUTO_SPEED },
	{ "WEAPONS_GAUGE", HUD_OBJECT_WEAPONS },
	{ "ESCORT_VIEW", HUD_OBJECT_ESCORT },
	{ "DIRECTIVES_VIEW", HUD_OBJECT_DIRECTIVES },
	{ "THREAT_GAUGE", HUD_OBJECT_THREAT },
	{ "AFTERBURNER_ENERGY", HUD_OBJECT_AFTERBURNER },
	{ "WEAPONS_ENERGY", HUD_OBJECT_WEAPON_ENERGY },
	{ "WEAPON_LINKING_GAUGE", HUD_OBJECT_WEAPON_LINKING },
	{ "TARGER_MINI_ICON", HUD_OBJECT_MINI_SHIELD },
	{ "OFFSCREEN_INDICATOR", HUD_OBJECT_OFFSCREEN },
	{ "TALKING_HEAD", HUD_OBJECT_TALKING_HEAD },
	{ "DAMAGE_GAUGE", HUD_OBJECT_DAMAGE },
	{ "MESSAGE_LINES", HUD_OBJECT_MESSAGES },
	{ "MISSILE_WARNING_ARROW", HUD_OBJECT_MISSILE_TRI },
	{ "CMEASURE_GAUGE", HUD_OBJECT_CMEASURES },
	{ "OBJECTIVES_NOTIFY_GAUGE", HUD_OBJECT_OBJ_NOTIFY },
	{ "WINGMEN_STATUS", HUD_OBJECT_WINGMAN_STATUS },
	{ "OFFSCREEN RANGE", -1 },
	{ "KILLS GAUGE", HUD_OBJECT_KILLS },
	{ "ATTACKING TARGET COUNT", -1 },
	{ "TEXT FLASH", HUD_OBJECT_TEXT_WARNINGS },
	{ "MESSAGE BOX", HUD_OBJECT_SQUAD_MSG },
	{ "SUPPORT GUAGE", HUD_OBJECT_SUPPORT },
	{ "LAG GUAGE", HUD_OBJECT_LAG },
};

flag_def_list Hud_gauge_types[] = {
	{ "Messages",			HUD_OBJECT_MESSAGES,			0},
	{ "Training messages",	HUD_OBJECT_TRAINING_MESSAGES,	0},		// Not in legacy list
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
	{ "Ets weapons",		HUD_OBJECT_ETS_WEAPONS,			0},		// Not in legacy list
	{ "Ets shields",		HUD_OBJECT_ETS_SHIELDS,			0},		// Not in legacy list
	{ "Ets engines",		HUD_OBJECT_ETS_ENGINES,			0},		// Not in legacy list
	{ "Target monitor",		HUD_OBJECT_TARGET_MONITOR,		0},
	{ "Extra target data",	HUD_OBJECT_EXTRA_TARGET_DATA,	0},
	{ "Radar",				HUD_OBJECT_RADAR_STD,			0},
	{ "Radar orb",			HUD_OBJECT_RADAR_ORB,			0},		// Not in legacy list
	{ "Radar BSG",			HUD_OBJECT_RADAR_BSG,			0},		// Not in legacy list
	{ "Afterburner energy",	HUD_OBJECT_AFTERBURNER,			0},
	{ "Weapon energy",		HUD_OBJECT_WEAPON_ENERGY,		0},
	{ "Text warnings",		HUD_OBJECT_TEXT_WARNINGS,		0},
	{ "Center reticle",		HUD_OBJECT_CENTER_RETICLE,		0},
	{ "Throttle",			HUD_OBJECT_THROTTLE,			0},
	{ "Threat indicator",	HUD_OBJECT_THREAT,				0},
	{ "Lead indicator",		HUD_OBJECT_LEAD,				0},
	{ "Lead sight",			HUD_OBJECT_LEAD_SIGHT,			0},
	{ "Lock indicator",		HUD_OBJECT_LOCK,				0},		// Not in legacy list
	{ "Weapon linking",		HUD_OBJECT_WEAPON_LINKING,		0},
	{ "Multiplayer messages",	HUD_OBJECT_MULTI_MSG,			0},	// Not in legacy list
	{ "Voice status",		HUD_OBJECT_VOICE_STATUS,		0},		// Not in legacy list
	{ "Ping",				HUD_OBJECT_PING,				0},		// Not in legacy list
	{ "Supernova",			HUD_OBJECT_SUPERNOVA,			0},		// Not in legacy list
	{ "Offscreen indicator",	HUD_OBJECT_OFFSCREEN,			0},
	{ "Targeting brackets",	HUD_OBJECT_BRACKETS,			0},
	{ "Orientation",		HUD_OBJECT_ORIENTATION_TEE,		0},
	{ "Hostile direction",	HUD_OBJECT_HOSTILE_TRI,			0},
	{ "Target direction",	HUD_OBJECT_TARGET_TRI,			0},
	{ "Missile indicator",	HUD_OBJECT_MISSILE_TRI,			0},
	{ "Kills",				HUD_OBJECT_KILLS,				0},
	{ "Fixed messages",		HUD_OBJECT_FIXED_MESSAGES,		0},		// Not in legacy list
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

void parse_hud_gauges_tbl(const char *filename)
{
	int i;
	char *saved_Mp = NULL;

	int colors[3] = {255, 255, 255};
	color hud_color;
	color ship_color;
	color *hud_clr_p = NULL;
	color *ship_clr_p = NULL;
	bool scale_gauge = true;
	bool chase_view_only = false;

	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		if (optional_string("$Load Retail Configuration:")) {
			stuff_boolean(&Hud_retail);
		}

		if (optional_string("$Color:")) {
			stuff_int_list(colors, 3);

			check_color(colors);
			gr_init_alphacolor(&hud_color, colors[0], colors[1], colors[2], 255);
			hud_clr_p = &hud_color;
		}

		if (optional_string("$Font:")) {
			Hud_font = font::parse_font();
		}
	
		if (optional_string("$Chase View Only:")) {
			stuff_boolean(&chase_view_only);
			Chase_view_only_ex = chase_view_only;
		}

		if(optional_string("$Max Directives:")) {
			stuff_int(&Max_directives);
		}

		if (optional_string("$Max Escort Ships:")) {
			stuff_int(&Max_escort_ships);
		}

		if (optional_string("$Length Unit Multiplier:"))	{
			stuff_float(&Hud_unit_multiplier);

			if (Hud_unit_multiplier <= 0.0f) {
				Warning(LOCATION, "\"$Length Unit Multiplier:\" value of \"%f\" is invalid!  Resetting to default.", Hud_unit_multiplier);
				Hud_unit_multiplier = 1.0f;
			}
		}

		if (optional_string("$Speed Unit Multiplier:")) {
			stuff_float(&Hud_speed_multiplier);

			if (Hud_speed_multiplier <= 0.0f) {
				Warning(LOCATION, "\"$Speed Unit Multiplier:\" value of \"%f\" is invalid!  Resetting to default.", Hud_speed_multiplier);
				Hud_speed_multiplier = 1.0f;
			}
		}
		else {
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

		if (optional_string("$Scale Gauges:")) {
			stuff_boolean(&scale_gauge);
			Scale_retail_gauges = scale_gauge;
		}

		if (optional_string("$Force Scaling Above:")) {
			stuff_int_list(Force_scaling_above_res_global, 2);
		}

		if (optional_string("$Reticle Style:")) {
			int temp = required_string_either("FS1", "FS2");

			// using require_string_either won't advance the Mp pointer to the next token
			Mp += 3;

			if (temp < 0)
				Warning(LOCATION, "Undefined reticle style in hud_gauges.tbl!");
			else
				Hud_reticle_style = temp;
		}

		int base_res[2];
		int force_scaling_above_res[2];
		int ship_idx = -1;
		int ship_font = -1;
		int gauge_type = -1;
		int use_font = -1;
		color *use_clr_p = NULL;
		SCP_vector<int> ship_classes;
		bool retail_config = false;
		int n_ships = 0;

		while (optional_string("#Gauge Config")) {
			ship_classes.clear();
			switch (optional_string_either("$Ship:", "$Ships:")) {
			case 0:
				mprintf(("$Ship in hud_gauges.tbl and -hdg.tbms is deprecated. Use \"$Ships: (\"Some ship class\") instead.\n"));

				if (!Ships_inited) {
					// just in case ship info has not been initialized.
					skip_to_start_of_string("#Gauge Config");
					continue;
				}

				// get the ship number for this HUD configuration.	
				ship_idx = parse_ship_start();
				ship_classes.push_back(ship_idx);

				if (ship_idx >= 0) {
					Ship_info[ship_idx].hud_enabled = true;

					// see if we need to load defaults for this configuration
					if (optional_string("$Load Retail Configuration:")) {
						stuff_boolean(&Ship_info[ship_idx].hud_retail);
					}

					if (optional_string("$Color:")) {
						stuff_int_list(colors, 3);

						check_color(colors);
						gr_init_alphacolor(&ship_color, colors[0], colors[1], colors[2], 255);
						ship_clr_p = &ship_color;
					}

					if (optional_string("$Font:")) {
						ship_font = font::parse_font();
					}
					
					if (optional_string("$Chase View Only:")) {
						stuff_boolean(&chase_view_only);
					}
				}
				else {
					// can't find ship class. move on.
					ship_classes.push_back(-1);
					skip_to_start_of_string("#Gauge Config");
					//skip_to_start_of_string_either("#Gauge Config", "#End");
					continue;
				}
				break;
			case 1:
				int shiparray[256];

				n_ships = (int)stuff_int_list(shiparray, 256, SHIP_INFO_TYPE);

				if (optional_string("$Load Retail Configuration:")) {
					stuff_boolean(&retail_config);
				}

				for (i = 0; i < n_ships; ++i) {
					ship_classes.push_back(shiparray[i]);
					Ship_info[shiparray[i]].hud_enabled = true;
					Ship_info[shiparray[i]].hud_retail = retail_config;
				}

				if (optional_string("$Color:")) {
					stuff_int_list(colors, 3);

					check_color(colors);
					gr_init_alphacolor(&ship_color, colors[0], colors[1], colors[2], 255);
					ship_clr_p = &ship_color;
				}

				if (optional_string("$Font:")) {
					ship_font = font::parse_font();
				}
				if (optional_string("$Chase View Only:")) {
					stuff_boolean(&chase_view_only);
				}
				break;
			default:
				// No particular ship. -1 for default HUD configuration.
				ship_classes.push_back(-1);
				ship_font = -1;
				ship_clr_p = NULL;
				break;
			}

			if (ship_clr_p != NULL) {
				use_clr_p = ship_clr_p;
			}
			else {
				use_clr_p = hud_clr_p;
			}

			if (ship_font >= 0) {
				use_font = ship_font;
			}
			else {
				use_font = Hud_font;
			}

			// Now start going through resolution info for this HUD layout
			required_string("$Base:");

			// get the base width and height describing this HUD
			stuff_int_list(base_res, 2, RAW_INTEGER_TYPE);

			// gauge scaling for this base res?
			if (optional_string("$Scale Gauges:")) {
				stuff_boolean(&scale_gauge);
			}

			// maximum res at which scaling can be disabled
			if (optional_string("$Force Scaling Above:")) {
				stuff_int_list(force_scaling_above_res, 2);
			} else {
				memcpy(force_scaling_above_res, Force_scaling_above_res_global, sizeof(force_scaling_above_res));
			}

			// Pruning time. Let's see if the current resolution defined by the user matches the conditions set by this entry
			bool prune_config = false;
			if (optional_string("$Required Aspect:")) {
				// filter aspect ratio.
				if (optional_string("Full Screen")) {
					if ((float)gr_screen.center_w / (float)gr_screen.center_h > 1.5f) {
						prune_config = true;
					}
				}
				else if (optional_string("Wide Screen")) {
					if ((float)gr_screen.center_w / (float)gr_screen.center_h <= 1.5f) {
						prune_config = true;
					}
				}
			}

			// check minimum resolution
			if (optional_string("$Min:")) {
				int min_res[2];
				stuff_int_list(min_res, 2, RAW_INTEGER_TYPE);

				if (min_res[0] > gr_screen.max_w) {
					prune_config = true;
				}
				else if (min_res[0] == gr_screen.center_w) {
					if (min_res[1] > gr_screen.center_h) {
						prune_config = true;
					}
				}
			}

			// check maximum resolution
			if (optional_string("$Max:")) {
				int max_res[2];
				stuff_int_list(max_res, 2, RAW_INTEGER_TYPE);

				if (max_res[0] < gr_screen.max_w) {
					prune_config = true;
				}
				else if (max_res[0] == gr_screen.center_w) {
					if (max_res[1] < gr_screen.center_h) {
						prune_config = true;
					}
				}
			}

			// don't prune resolutions in FRED
			if (!Fred_running && prune_config) {
				skip_to_start_of_string("#Gauge Config");
				continue;
			}

			// let's start parsing for gauges.
			required_string("$Gauges:");

			while ((gauge_type = parse_gauge_type()) >= 0) {
				// change some of the default gauge settings to the appropriate values.
				gauge_settings settings;
				settings.font_num = use_font;
				settings.scale_gauge = scale_gauge;
				memcpy(settings.force_scaling_above_res, force_scaling_above_res, sizeof(settings.force_scaling_above_res));
				settings.ship_idx = &ship_classes;
				settings.use_clr = use_clr_p;
				settings.chase_view_only = chase_view_only;

				// if "default" is specified, then the base resolution is {-1, -1},
				// indicating GR_640 or GR_1024 to the handlers. otherwise, change it
				// to the given base resolution.
				if (!optional_string("default")) {
					memcpy(settings.base_res, base_res, sizeof(settings.base_res));
				}

				// then call the specific gauge load handler function for this gauge type.
				load_gauge(gauge_type, &settings);

				if (saved_Mp && (saved_Mp == Mp)) {
					Mp++;
				}

				// HACK: The previous code simply skipped invalid entries but now we try to generate a warning for that.
				// If we don't see either $ or + then it means that there was an invalid token somewhere in between
				if (!check_for_string("$") && !check_for_string("+")) {
					error_display(0, "Detected invalid tokens while parsing HUD gauges: [%.32s]", next_tokens());
					skip_to_start_of_string_either("$", "+");
				}

				// stolened from AI_profiles
				// if we've been through once already and are at the same place, force a move
				saved_Mp = Mp;
			}

			required_string("$End Gauges");
			required_string("#End");
		}
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

void hud_positions_init()
{
	if(!Ships_inited)
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

		for(int i = 0; i < Num_builtin_gauges; i++) {
			retail_gauge_loaded = false;

			for(int j = 0; j < num_loaded_gauges; j++) {
				if(Builtin_gauges[i] == default_hud_gauges[j]->getObjectType()) {
					retail_gauge_loaded = true;
					break;
				}
			}

			if(!retail_gauge_loaded) {
				gauge_settings settings;
				load_gauge(Builtin_gauges[i], &settings);
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
			gauge_settings settings;
			load_gauge((Cmdline_orb_radar ? HUD_OBJECT_RADAR_ORB : HUD_OBJECT_RADAR_STD), &settings);
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
				gauge_settings settings;
				load_gauge(HUD_OBJECT_WEAPON_LINKING, &settings);
			}
		}
	}

	// for each ship class, check if their specific HUD config is enabled
	int k = 0;
	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); k++, ++it) {
		SCP_vector<int> sindex;
		sindex.push_back(k);
		if(it->hud_enabled && it->hud_retail) {
			int num_loaded_gauges = (int)it->hud_gauges.size();

			for(int i = 0; i < Num_builtin_gauges; i++) {
				for(int j = 0; j < num_loaded_gauges; j++) {
					if(Builtin_gauges[i] == it->hud_gauges[j]->getObjectType()) {
						retail_gauge_loaded = true;
					}
				}

				if(!retail_gauge_loaded) {
					gauge_settings settings;
					settings.ship_idx = &sindex;
					load_gauge(Builtin_gauges[i], &settings);
				}
			}

			// if we're missing a radar gauge, load either orb or standard
			retail_gauge_loaded = false;
			for(int j = 0; j < num_loaded_gauges; j++) {
				if(HUD_OBJECT_RADAR_ORB == it->hud_gauges[j]->getObjectType() || 
					HUD_OBJECT_RADAR_STD == it->hud_gauges[j]->getObjectType() ||
					HUD_OBJECT_RADAR_BSG == it->hud_gauges[j]->getObjectType()) {
					retail_gauge_loaded = true;
				}
			}

			// load radar gauge if not loaded.
			if(!retail_gauge_loaded) {
				gauge_settings settings;
				settings.ship_idx = &sindex;
				load_gauge((Cmdline_orb_radar ? HUD_OBJECT_RADAR_ORB : HUD_OBJECT_RADAR_STD), &settings);
			}

			// Throw in the weapon linking reticle gauge if using FS1 defaults
			retail_gauge_loaded = false;
			if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
				for(int j = 0; j < num_loaded_gauges; j++) {
					if(HUD_OBJECT_WEAPON_LINKING == it->hud_gauges[j]->getObjectType()) {
						retail_gauge_loaded = true;
					}
				}

				if(!retail_gauge_loaded) {
					gauge_settings settings;
					settings.ship_idx = &sindex;
					load_gauge(HUD_OBJECT_WEAPON_LINKING, &settings);
				}
			}
		}
	}
}

// Called once after mission load is complete. Sets initial gauge activity states.
void init_hud() {
	int config_type;
	size_t i, num_gauges;

	if(!Ship_info[Player_ship->ship_info_index].hud_gauges.empty()) {
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

extern void hud_init_ballistic_index();

void set_current_hud()
{
	int config_type;
	size_t i, num_gauges;

	// before we load any hud gauges, see whether we're carring a ballistic weapon (Mantis #2962)
	hud_init_ballistic_index();

	// go through all HUD gauges. Load gauge properties defined in the HUD config if gauge is not customized.
	if(!Ship_info[Player_ship->ship_info_index].hud_gauges.empty()) {
		num_gauges = Ship_info[Player_ship->ship_info_index].hud_gauges.size();

		for(i = 0; i < num_gauges; i++) {
			HudGauge* hgp = Ship_info[Player_ship->ship_info_index].hud_gauges[i].get();
			config_type = hgp->getConfigType();

			if ( ( (!hgp->isOffbyDefault() || hgp->isActive()) && hud_config_show_flag_is_set(config_type)) )
				hgp->updateActive(true);
			else
				hgp->updateActive(false);

			//hgp->updateActive(hud_config_show_flag_is_set(config_type) ? true : false);
			hgp->updatePopUp(hud_config_popup_flag_is_set(config_type) ? true : false);
			hgp->updateColor(
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

	if(optional_string("+Flight Path Marker:"))
		return HUD_OBJECT_FLIGHT_PATH;

	if ( optional_string("+Warhead Count:") )
		return HUD_OBJECT_WARHEAD_COUNT;

	if ( optional_string("+Hardpoints:") )
		return HUD_OBJECT_HARDPOINTS;

	if ( optional_string("+Primary Weapons:") )
		return HUD_OBJECT_PRIMARY_WEAPONS;

	if ( optional_string("+Secondary Weapons:") )
		return HUD_OBJECT_SECONDARY_WEAPONS;

	if ( optional_string("+Scripted Gauge:") )
		return HUD_OBJECT_SCRIPTING;
	
	return -1;
}

void load_gauge(int gauge, gauge_settings* settings)
{
	switch(gauge) {
	case HUD_OBJECT_CUSTOM:
		load_gauge_custom(settings);
		break;
	case HUD_OBJECT_MESSAGES:
		load_gauge_messages(settings);
		break;
	case HUD_OBJECT_TRAINING_MESSAGES:
		load_gauge_training_messages(settings);
		break;
	case HUD_OBJECT_SUPPORT:
		load_gauge_support(settings);
		break;
	case HUD_OBJECT_DAMAGE:
		load_gauge_damage(settings);
		break;
	case HUD_OBJECT_WINGMAN_STATUS:
		load_gauge_wingman_status(settings);
		break;
	case HUD_OBJECT_AUTO_SPEED:
		load_gauge_auto_speed(settings);
		break;
	case HUD_OBJECT_AUTO_TARGET:
		load_gauge_auto_target(settings);
		break;
	case HUD_OBJECT_CMEASURES:
		load_gauge_countermeasures(settings);
		break;
	case HUD_OBJECT_TALKING_HEAD:
		load_gauge_talking_head(settings);
		break;
	case HUD_OBJECT_DIRECTIVES:
		load_gauge_directives(settings);
		break;
	case HUD_OBJECT_WEAPONS:
		load_gauge_weapons(settings);
		break;
	case HUD_OBJECT_OBJ_NOTIFY:
		load_gauge_objective_notify(settings);
		break;
	case HUD_OBJECT_SQUAD_MSG:
		load_gauge_squad_message(settings);
		break;
	case HUD_OBJECT_LAG:
		load_gauge_lag(settings);
		break;
	case HUD_OBJECT_MINI_SHIELD:
		load_gauge_mini_shields(settings);
		break;
	case HUD_OBJECT_PLAYER_SHIELD:
		load_gauge_player_shields(settings);
		break;
	case HUD_OBJECT_TARGET_SHIELD:
		load_gauge_target_shields(settings);
		break;
	case HUD_OBJECT_ESCORT:
		load_gauge_escort_view(settings);
		break;
	case HUD_OBJECT_MISSION_TIME:
		load_gauge_mission_time(settings);
		break;
	case HUD_OBJECT_ETS_WEAPONS:
		load_gauge_ets_weapons(settings);
		break;
	case HUD_OBJECT_ETS_SHIELDS:
		load_gauge_ets_shields(settings);
		break;
	case HUD_OBJECT_ETS_ENGINES:
		load_gauge_ets_engines(settings);
		break;
	case HUD_OBJECT_ETS_RETAIL:
		load_gauge_ets_retail(settings);
		break;
	case HUD_OBJECT_TARGET_MONITOR:
		load_gauge_target_monitor(settings);
		break;
	case HUD_OBJECT_EXTRA_TARGET_DATA:
		load_gauge_extra_target_data(settings);
		break;
	case HUD_OBJECT_RADAR_STD:
		load_gauge_radar_std(settings);
		break;
	case HUD_OBJECT_RADAR_ORB:
		load_gauge_radar_orb(settings);
		break;
	case HUD_OBJECT_RADAR_BSG:
		load_gauge_radar_dradis(settings);
		break;
	case HUD_OBJECT_AFTERBURNER:
		load_gauge_afterburner(settings);
		break;
	case HUD_OBJECT_WEAPON_ENERGY:
		load_gauge_weapon_energy(settings);
		break;
	case HUD_OBJECT_TEXT_WARNINGS:
		load_gauge_text_warnings(settings);
		break;
	case HUD_OBJECT_CENTER_RETICLE:
		load_gauge_center_reticle(settings);
		break;
	case HUD_OBJECT_THROTTLE:
		load_gauge_throttle(settings);
		break;
	case HUD_OBJECT_THREAT:
		load_gauge_threat_indicator(settings);
		break;
	case HUD_OBJECT_LEAD:
		load_gauge_lead(settings);
		break;
	case HUD_OBJECT_LEAD_SIGHT:
		load_gauge_lead_sight(settings);
		break;
	case HUD_OBJECT_LOCK:
		load_gauge_lock(settings);
		break;
	case HUD_OBJECT_WEAPON_LINKING:
		load_gauge_weapon_linking(settings);
		break;
	case HUD_OBJECT_MULTI_MSG:
		load_gauge_multi_msg(settings);
		break;
	case HUD_OBJECT_VOICE_STATUS:
		load_gauge_voice_status(settings);
		break;
	case HUD_OBJECT_PING:
		load_gauge_ping(settings);
		break;
	case HUD_OBJECT_SUPERNOVA:
		load_gauge_supernova(settings);
		break;
	case HUD_OBJECT_OFFSCREEN:
		load_gauge_offscreen(settings);
		break;
	case HUD_OBJECT_BRACKETS:
		load_gauge_brackets(settings);
		break;
	case HUD_OBJECT_ORIENTATION_TEE:
		load_gauge_orientation_tee(settings);
		break;
	case HUD_OBJECT_HOSTILE_TRI:
		load_gauge_hostile_tri(settings);
		break;
	case HUD_OBJECT_TARGET_TRI:
		load_gauge_target_tri(settings);
		break;
	case HUD_OBJECT_MISSILE_TRI:
		load_gauge_missile_tri(settings);
		break;
	case HUD_OBJECT_KILLS:
		load_gauge_kills(settings);
		break;
	case HUD_OBJECT_FIXED_MESSAGES:
		load_gauge_fixed_messages(settings);
		break;
	case HUD_OBJECT_FLIGHT_PATH:
		load_gauge_flight_path(settings);
		break;
	case HUD_OBJECT_WARHEAD_COUNT:
		load_gauge_warhead_count(settings);
		break;
	case HUD_OBJECT_HARDPOINTS:
		load_gauge_hardpoints(settings);
		break;
	case HUD_OBJECT_PRIMARY_WEAPONS:
		load_gauge_primary_weapons(settings);
		break;
	case HUD_OBJECT_SECONDARY_WEAPONS:
		load_gauge_secondary_weapons(settings);
		break;
	case HUD_OBJECT_SCRIPTING:
		load_gauge_scripting(settings);
		break;
	default:
		// It's either -1, indicating we're ignoring a parse error, or it's a coding error.
		Assertion(gauge == -1, "Invalid value '%d' passed to load_gauge(); get a coder!\n", gauge);
		break;
	}
}

inline bool check_base_res(int *res)
{
	return (res[0] >= 640) && (res[1] >= 480);
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

void adjust_base_res(int *base_res, const int *force_scaling_above_res, float *aspect_quotient, bool scaling = true)
{
	// Don't scale gauge if:
	// no scaling is set and current res is between base res and "force scaling above res"
	// Avoid HUD blurring caused solely by rounding errors
	if ((!scaling && gr_screen.center_w >= base_res[0] && gr_screen.center_h >= base_res[1] &&
			(force_scaling_above_res[0] <= 0 || force_scaling_above_res[1] <= 0 ||
			gr_screen.center_w <= force_scaling_above_res[0] || gr_screen.center_h <= force_scaling_above_res[1])) ||
			(gr_screen.center_w >= base_res[0] && gr_screen.center_h == base_res[1]) ||
			(gr_screen.center_w == base_res[0] && gr_screen.center_h >= base_res[1])) {
		*aspect_quotient = 1.0f;
		base_res[0] = gr_screen.center_w;
		base_res[1] = gr_screen.center_h;
		return;
	}

	if (!scaling && force_scaling_above_res[0] > base_res[0] && force_scaling_above_res[1] > base_res[1] &&
			gr_screen.center_w > force_scaling_above_res[0] && gr_screen.center_h > force_scaling_above_res[1]) {
		base_res[0] = force_scaling_above_res[0];
		base_res[1] = force_scaling_above_res[1];
	}

	*aspect_quotient = ((float)gr_screen.center_w / (float)gr_screen.center_h) / ((float)base_res[0] / (float)base_res[1]);
	if (*aspect_quotient >= 1.0f) {
		base_res[0] = (int)(base_res[0] * *aspect_quotient);
	} else {
		base_res[1] = (int)(base_res[1] / *aspect_quotient);
	}
}

void adjust_for_multimonitor(int *base_res, bool set_position, int *coords)
{
	float scale_w = (float)gr_screen.center_w / (float)base_res[0];
	float scale_h = (float)gr_screen.center_h / (float)base_res[1];

	base_res[0] = fl2ir(base_res[0] * ((float)gr_screen.max_w / (float)gr_screen.center_w));
	base_res[1] = fl2ir(base_res[1] * ((float)gr_screen.max_h / (float)gr_screen.center_h));

	if (set_position) {
		coords[0] += fl2ir(gr_screen.center_offset_x / scale_w);
		coords[1] += fl2ir(gr_screen.center_offset_y / scale_h);
	}
}

template<class T>
std::unique_ptr<T> gauge_load_common(gauge_settings* settings, T* preAllocated = NULL)
{
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	// render to texture parameters
	char display_name[MAX_FILENAME_LEN] = "";
	int display_size[2] = {0, 0};
	int display_offset[2] = {0, 0};
	int canvas_size[2] = {0, 0};

	if(check_base_res(settings->base_res)) {
		if (settings->set_position) {
			if(optional_string("Position:")) {
				stuff_int_list(settings->coords, 2);
				settings->use_coords = true;
			} else {
				if(optional_string("Scale Gauge:")) {
					stuff_boolean(&settings->scale_gauge);;
				}

				if(optional_string("Force Scaling Above:")) {
					stuff_int_list(settings->force_scaling_above_res, 2);
				}

				adjust_base_res(settings->base_res, settings->force_scaling_above_res, &settings->aspect_quotient, settings->scale_gauge);

				// If no positioning information is specified, use the default position
				bool use_default_pos = true;

				if(optional_string("Origin:")) {
					stuff_float_list(settings->origin, 2);
					use_default_pos = false;

					required_string("Offset:");
					stuff_int_list(settings->offset, 2);
				}

				if(optional_string("Offset:")) {
					Error(LOCATION, "HUD gauges table: Offset must also have Origin defined");
				}

				if ( !(settings->use_coords && use_default_pos) ) {
					settings->coords[0] = (int)(settings->base_res[0] * settings->origin[0]) + settings->offset[0];
					settings->coords[1] = (int)(settings->base_res[1] * settings->origin[1]) + settings->offset[1];
					settings->use_coords = true;
				}
			}
		} else {
			adjust_base_res(settings->base_res, settings->force_scaling_above_res, &settings->aspect_quotient, settings->scale_gauge);
		}
	} else {
		if(gr_screen.res == GR_640) {
			settings->base_res[0] = 640;
			settings->base_res[1] = 480;
		} else {
			settings->base_res[0] = 1024;
			settings->base_res[1] = 768;
		}

		adjust_base_res(settings->base_res, settings->force_scaling_above_res, &settings->aspect_quotient, settings->scale_gauge);

		if (settings->set_position && !settings->use_coords) {
			settings->coords[0] = (int)(settings->base_res[0] * settings->origin[0]) + settings->offset[0];
			settings->coords[1] = (int)(settings->base_res[1] * settings->origin[1]) + settings->offset[1];
			settings->use_coords = true;
		}
	}

	if (settings->set_position) {
		if ( optional_string("Cockpit Target:") && settings->ship_idx->at(0) >= 0 ) {
			stuff_string(display_name, F_NAME, MAX_FILENAME_LEN);

			if ( optional_string("Canvas Size:") ) {
				stuff_int_list(canvas_size, 2);
			}

			if ( optional_string("Display Offset:") ) {
				stuff_int_list(display_offset, 2);
			}

			required_string("Display Size:");
			stuff_int_list(display_size, 2);
		} else {
			// adjust for multimonitor setups ONLY if not rendering gauge to a texture
			adjust_for_multimonitor(settings->base_res, true, settings->coords);
		}
	} else {
		adjust_for_multimonitor(settings->base_res, true, settings->coords);
	}

	if (settings->set_colour) {
		if ( settings->use_clr != NULL ) {
			colors[0] = settings->use_clr->red;
			colors[1] = settings->use_clr->green;
			colors[2] = settings->use_clr->blue;

			lock_color = true;
		} else if ( optional_string("Color:") ) {
			stuff_int_list(colors, 3);

			check_color(colors);

			lock_color = true;
		}
	}

	if (optional_string("Font:")) {
		settings->font_num = font::parse_font();
	} else {
		if ( settings->font_num < 0 ) {
			settings->font_num = font::FONT1;
		}
	}

	if (optional_string("Chase View Only:")) {
		stuff_boolean(&settings->chase_view_only);
	}

	if (settings->set_position) {
		if(optional_string("Slew:")) {
			stuff_boolean(&settings->slew);
		}
	}

	std::unique_ptr<T> instance(preAllocated);

	if (instance == NULL)
	{
		instance.reset(new T());
	}

	instance->initBaseResolution(settings->base_res[0], settings->base_res[1], settings->aspect_quotient);
	instance->initFont(settings->font_num);
	instance->initOriginAndOffset(settings->origin[0], settings->origin[1], settings->offset[0], settings->offset[1]);
	instance->initCoords(settings->use_coords, settings->coords[0], settings->coords[1]);

	instance->initChase_view_only(settings->chase_view_only);
	if (settings->set_position) {
		instance->initPosition(settings->coords[0], settings->coords[1]);
		instance->initSlew(settings->slew);
		instance->initCockpitTarget(display_name, display_offset[0], display_offset[1], display_size[0], display_size[1], canvas_size[0], canvas_size[1]);
	}
	if (settings->set_colour) {
		instance->updateColor(colors[0], colors[1], colors[2]);
		instance->lockConfigColor(lock_color);
	}

	return instance;
}

template<typename T>
void gauge_assign_common(const gauge_settings* settings, std::unique_ptr<T>&& hud_gauge) {
	if(settings->ship_idx && settings->ship_idx->at(0) >= 0) {
		for (auto ship_index = settings->ship_idx->begin(); ship_index != settings->ship_idx->end(); ++ship_index) {
			std::unique_ptr<T> instance(new T());
			*instance = *hud_gauge;
			Ship_info[*ship_index].hud_gauges.push_back(move(instance));
		}
		// Previous instance goes out of scope here and is destructed
	} else {
		default_hud_gauges.push_back(move(hud_gauge));
	}
}

void load_gauge_custom(gauge_settings* settings)
{
	int i;
	char gauge_string[MAX_FILENAME_LEN];
	char name[MAX_FILENAME_LEN];
	char text[MAX_FILENAME_LEN];
	char filename[MAX_FILENAME_LEN];
	int gauge_type = HUD_CENTER_RETICLE;
	bool active_by_default = true;
	int txtoffset_x = 0, txtoffset_y = 0;
	ubyte r = 255, g = 255, b = 255;
	int colors[3] = {255, 255, 255};
	bool lock_color = false;

	// render to texture parameters
	char display_name[MAX_FILENAME_LEN] = "";
	int display_size[2] = {0, 0};
	int display_offset[2] = {0, 0};
	int canvas_size[2] = {0, 0};

	if(check_base_res(settings->base_res)) {
		if(optional_string("Position:")) {
			stuff_int_list(settings->coords, 2);
			settings->use_coords = true;
		} else {
			if(optional_string("Scale Gauge:")) {
				stuff_boolean(&settings->scale_gauge);;
			}

			if(optional_string("Force Scaling Above:")) {
				stuff_int_list(settings->force_scaling_above_res, 2);
			}

			adjust_base_res(settings->base_res, settings->force_scaling_above_res, &settings->aspect_quotient, settings->scale_gauge);

			// If no positioning information is specified, use the default position
			bool use_default_pos = true;

			if(optional_string("Origin:")) {
				stuff_float_list(settings->origin, 2);
				use_default_pos = false;

				required_string("Offset:");
				stuff_int_list(settings->offset, 2);
			}

			if(optional_string("Offset:")) {
				Error(LOCATION, "HUD gauges table: Offset must also have Origin defined");
			}

			if (!use_default_pos) {
				settings->coords[0] = (int)(settings->base_res[0] * settings->origin[0]) + settings->offset[0];
				settings->coords[1] = (int)(settings->base_res[1] * settings->origin[1]) + settings->offset[1];
				settings->use_coords = true;
			}
		}

		if ( optional_string("Cockpit Target:") && settings->ship_idx->at(0) >= 0 ) {
			stuff_string(display_name, F_NAME, MAX_FILENAME_LEN);

			if ( optional_string("Canvas Size:") ) {
				stuff_int_list(canvas_size, 2);
			}

			if ( optional_string("Display Offset:") ) {
				stuff_int_list(display_offset, 2);
			}

			required_string("Display Size:");
			stuff_int_list(display_size, 2);
		} else {
			// adjust for multimonitor setups ONLY if not rendering gauge to a texture
			adjust_for_multimonitor(settings->base_res, true, settings->coords);
		}

		if ( settings->use_clr != NULL ) {
			colors[0] = settings->use_clr->red;
			colors[1] = settings->use_clr->green;
			colors[2] = settings->use_clr->blue;

			lock_color = true;
		} else if ( optional_string("Color:") ) {
			stuff_int_list(colors, 3);

			check_color(colors);

			lock_color = true;
		}

		if (optional_string("Font:")) {
			settings->font_num = font::parse_font();
		} else {
			if ( settings->font_num < 0 ) {
				settings->font_num = font::FONT1;
			}
		}

		if (optional_string("Chase View Only:")) {
			stuff_boolean(&settings->chase_view_only);
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
			if(!strcmp(Legacy_HUD_gauges[i].hud_gauge_text, gauge_string)) {
				gauge_type = i;
				break;
			}
		}

		if(optional_string("Slew:")) {
			stuff_boolean(&settings->slew);
		}

		if(optional_string("Active by default:")) {
			stuff_boolean(&active_by_default);
		}

		required_string("Filename:");
		stuff_string(filename, F_NAME, MAX_FILENAME_LEN);
	} else {
		settings->base_res[0] = 640;
		settings->base_res[1] = 480;

		if ( settings->font_num < 0 ) {
			settings->font_num = font::FONT1;
		}
	}

	std::unique_ptr<HudGauge> hud_gauge(new HudGauge(gauge_type, settings->slew, r, g, b, name, text, filename, txtoffset_x, txtoffset_y));

	hud_gauge->initBaseResolution(settings->base_res[0], settings->base_res[1], settings->aspect_quotient);
	hud_gauge->initPosition(settings->coords[0], settings->coords[1]);
	hud_gauge->initFont(settings->font_num);
	hud_gauge->initOriginAndOffset(settings->origin[0], settings->origin[1], settings->offset[0], settings->offset[1]);
	hud_gauge->initCoords(settings->use_coords, settings->coords[0], settings->coords[1]);

	hud_gauge->initRenderStatus(active_by_default);
	hud_gauge->updateColor(colors[0], colors[1], colors[2]);
	hud_gauge->lockConfigColor(lock_color);
	hud_gauge->initCockpitTarget(display_name, display_offset[0], display_offset[1], display_size[0], display_size[1], canvas_size[0], canvas_size[1]);
	hud_gauge->initChase_view_only(settings->chase_view_only);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_lag(gauge_settings* settings)
{
	char fname[MAX_FILENAME_LEN] = "netlag1";
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 66;
		settings->offset[1] = 91;
	} else {
		settings->offset[0] = 115;
		settings->offset[1] = 145;
	}

	auto hud_gauge = gauge_load_common<HudGaugeLag>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}

	hud_gauge->initBitmaps(fname);

	gauge_assign_common(settings, std::move(hud_gauge));
}
void load_gauge_mini_shields(gauge_settings* settings)
{
	int Mini_3digit_offsets[2];
	int Mini_1digit_offsets[2];
	int Mini_2digit_offsets[2];
	char fname[MAX_FILENAME_LEN] = "targhit1";
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -15;
		settings->offset[1] = 51;

		Mini_3digit_offsets[0] = 5;
		Mini_3digit_offsets[1] = 7;
		Mini_1digit_offsets[0] = 11;
		Mini_1digit_offsets[1] = 7;
		Mini_2digit_offsets[0] = 8;
		Mini_2digit_offsets[1] = 7;
	} else {
		settings->offset[0] = -15;
		settings->offset[1] = 86;

		Mini_3digit_offsets[0] = 5;
		Mini_3digit_offsets[1] = 7;
		Mini_1digit_offsets[0] = 14;
		Mini_1digit_offsets[1] = 7;
		Mini_2digit_offsets[0] = 9;
		Mini_2digit_offsets[1] = 7;
	}

	auto hud_gauge = gauge_load_common<HudGaugeShieldMini>(settings);

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

	hud_gauge->init1DigitOffsets(Mini_1digit_offsets[0], Mini_1digit_offsets[1]);
	hud_gauge->init2DigitOffsets(Mini_2digit_offsets[0], Mini_2digit_offsets[1]);
	hud_gauge->init3DigitOffsets(Mini_3digit_offsets[0], Mini_3digit_offsets[1]);
	hud_gauge->initBitmaps(fname);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_weapon_energy(gauge_settings* settings)
{
	int Wenergy_text_offsets[2];
	int Wenergy_h;
	HudAlignment text_alignment = HudAlignment::NONE;
	bool always_show_text = false;
	bool show_ballistic = false;
	bool moving_text = false;
	int armed_weapon_offsets[2] = {0, 0};
	int armed_weapon_h = 12;
	HudAlignment weapon_alignment = HudAlignment::NONE;
	bool show_weapons = false;
	char fname[MAX_FILENAME_LEN];
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

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
		settings->offset[0] = 96;
		settings->offset[1] = 25;

		Wenergy_text_offsets[0] = 23;
		Wenergy_text_offsets[1] = 53;

		Wenergy_h = 60;
	} else {
		settings->offset[0] = 154;
		settings->offset[1] = 40;

		Wenergy_text_offsets[0] = 43;
		Wenergy_text_offsets[1] = 85;

		Wenergy_h = 96;
	}

	auto hud_gauge = gauge_load_common<HudGaugeWeaponEnergy>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Foreground Clip Height:")) {
		stuff_int(&Wenergy_h);
	}
	if(optional_string("Text Offsets:")) {
		stuff_int_list(Wenergy_text_offsets, 2);

		if(optional_string("Text Alignment:")) {
			char temp[NAME_LENGTH];
			stuff_string(temp, F_NAME, NAME_LENGTH);
			text_alignment = hud_alignment_lookup(temp);
		}
	}
	if(optional_string("Always Show Text:")) {
		stuff_boolean(&always_show_text);
	}
	if(optional_string("Text Follows:")) {
		stuff_boolean(&moving_text);
	}
	if(optional_string("Show Ballistic Ammo:")) {
		stuff_boolean(&show_ballistic);
	}
	if(optional_string("Armed Guns List Offsets:")) {
		stuff_int_list(armed_weapon_offsets, 2);
		show_weapons = true;

		if(optional_string("Armed Guns List Alignment:")) {
			char temp[NAME_LENGTH];
			stuff_string(temp, F_NAME, NAME_LENGTH);
			weapon_alignment = hud_alignment_lookup(temp);
		}

		if(optional_string("Armed Guns List Entry Height:")) {
			stuff_int(&armed_weapon_h);
		}
	}

	hud_gauge->initBitmaps(fname);
	hud_gauge->initEnergyHeight(Wenergy_h);
	hud_gauge->initTextOffsets(Wenergy_text_offsets[0], Wenergy_text_offsets[1]);
	hud_gauge->initAlignments(text_alignment, weapon_alignment);
	hud_gauge->initAlwaysShowText(always_show_text);
	hud_gauge->initMoveText(moving_text);
	hud_gauge->initShowBallistics(show_ballistic);
	hud_gauge->initArmedOffsets(armed_weapon_offsets[0], armed_weapon_offsets[1], armed_weapon_h, show_weapons);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_target_shields(gauge_settings* settings)
{
	settings->origin[0] = 0.5f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -178;
		settings->offset[1] = -101;
	} else {
		settings->offset[0] = -220;
		settings->offset[1] = -98;
	}

	auto hud_gauge = gauge_load_common<HudGaugeShieldTarget>(settings);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_player_shields(gauge_settings* settings)
{
	settings->origin[0] = 0.5f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 76;
		settings->offset[1] = -101;
	} else {
		settings->offset[0] = 122;
		settings->offset[1] = -98;
	}

	auto hud_gauge = gauge_load_common<HudGaugeShieldPlayer>(settings);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_escort_view(gauge_settings* settings)
{
	int header_text_offsets[2];
	int list_start_offsets[2];
	int entry_h;
	int entry_stagger_w;
	int bottom_bg_offset = 0;
	int ship_name_offsets[2];
	int ship_name_max_w = 100;
	int ship_integrity_offsets[2];
	int ship_status_offsets[2];
	bool right_align_names = false;
	char header_text[MAX_FILENAME_LEN] = "";
	char fname_top[MAX_FILENAME_LEN] = "escort1";
	char fname_middle[MAX_FILENAME_LEN] = "escort2";
	char fname_bottom[MAX_FILENAME_LEN] = "escort3";
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 0.5f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -154;
		settings->offset[1] = -40;

		header_text_offsets[0] = 3;
		header_text_offsets[1] = 2;
		list_start_offsets[0] = 0;
		list_start_offsets[1] = 13;
		entry_h = 11;
		entry_stagger_w = 0;
		ship_name_offsets[0] = 3;
		ship_name_offsets[1] = 0;
		ship_integrity_offsets[0] = 118;
		ship_integrity_offsets[1] = 0;
		ship_status_offsets[0] = -12;
		ship_status_offsets[1] = 0;
	} else {
		settings->offset[0] = -159;
		settings->offset[1] = -54;

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

	auto hud_gauge = gauge_load_common<HudGaugeEscort>(settings);

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
	if(optional_string("Bottom Background Offset:")) {
		stuff_int(&bottom_bg_offset);
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

	if ( optional_string("Ship Name Max Width:") ) {
		stuff_int(&ship_name_max_w);
	}

	if ( optional_string("Right-Align Ship Names:") ) {
		stuff_boolean(&right_align_names);
	}

	if (header_text[0] == '\0') {
		strcpy_s(header_text, XSTR("monitoring", 285));
	}

	hud_gauge->initBitmaps(fname_top, fname_middle, fname_bottom);
	hud_gauge->initEntryHeight(entry_h);
	hud_gauge->initEntryStaggerWidth(entry_stagger_w);
	hud_gauge->initBottomBgOffset(bottom_bg_offset);
	hud_gauge->initHeaderText(header_text);
	hud_gauge->initHeaderTextOffsets(header_text_offsets[0], header_text_offsets[1]);
	hud_gauge->initListStartOffsets(list_start_offsets[0], list_start_offsets[1]);
	hud_gauge->initShipIntegrityOffsets(ship_integrity_offsets[0], ship_integrity_offsets[1]);
	hud_gauge->initShipNameOffsets(ship_name_offsets[0], ship_name_offsets[1]);
	hud_gauge->initShipStatusOffsets(ship_status_offsets[0], ship_status_offsets[1]);
	hud_gauge->initShipNameMaxWidth(ship_name_max_w);
	hud_gauge->initRightAlignNames(right_align_names);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_afterburner(gauge_settings* settings)
{
	int energy_h;
	char fname[MAX_FILENAME_LEN];
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

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
		settings->offset[0] = -149;
		settings->offset[1] = 25;

		energy_h = 60;
	} else {
		settings->offset[0] = -238;
		settings->offset[1] = 40;

		energy_h = 96;
	}

	auto hud_gauge = gauge_load_common<HudGaugeAfterburner>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Foreground Clip Height:")) {
		stuff_int(&energy_h);
	}

	hud_gauge->initEnergyHeight(energy_h);
	hud_gauge->initBitmaps(fname);

	gauge_assign_common(settings, std::move(hud_gauge));
}


void load_gauge_mission_time(gauge_settings* settings)
{
	int time_text_offsets[2];
	int time_val_offsets[2];
	char fname[MAX_FILENAME_LEN] = "time1";
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -53;
		settings->offset[1] = -32;
	} else {
		settings->offset[0] = -55;
		settings->offset[1] = -52;
	}
	time_text_offsets[0] = 4;
	time_text_offsets[1] = 4;

	time_val_offsets[0] = 26;
	time_val_offsets[1] = 12;

	auto hud_gauge = gauge_load_common<HudGaugeMissionTime>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Text Offsets:")) {
		stuff_int_list(time_text_offsets, 2);
	}
	if(optional_string("Value Offsets:")) {
		stuff_int_list(time_val_offsets, 2);
	}

	hud_gauge->initTextOffsets(time_text_offsets[0], time_text_offsets[1]);
	hud_gauge->initValueOffsets(time_val_offsets[0], time_val_offsets[1]);
	hud_gauge->initBitmaps(fname);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_threat_indicator(gauge_settings* settings)
{
	int Laser_warn_offsets[2];
	int Lock_warn_offsets[2];
	char fname_arc[MAX_FILENAME_LEN];
	char fname_laser[MAX_FILENAME_LEN];
	char fname_lock[MAX_FILENAME_LEN];

	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

	if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
		if(gr_screen.res == GR_640) {
			settings->offset[0] = -79;
			settings->offset[1] = -103;

			Laser_warn_offsets[0] = 59;
			Laser_warn_offsets[1] = 0;

			Lock_warn_offsets[0] = 79;
			Lock_warn_offsets[1] = 0;

			strcpy_s(fname_arc, "toparc1_fs1");
			strcpy_s(fname_laser, "toparc2_fs1");
			strcpy_s(fname_lock, "toparc3_fs1");
		} else {
			settings->offset[0] = -126;
			settings->offset[1] = -165;

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
			settings->offset[0] = 39;
			settings->offset[1] = -72;

			Laser_warn_offsets[0] = 41;
			Laser_warn_offsets[1] = 77;

			Lock_warn_offsets[0] = 35;
			Lock_warn_offsets[1] = 93;

			strcpy_s(fname_arc, "rightarc1");
			strcpy_s(fname_laser, "toparc2");
			strcpy_s(fname_lock, "toparc3");
		} else {
			settings->offset[0] = 62;
			settings->offset[1] = -115;

			Laser_warn_offsets[0] = 66;
			Laser_warn_offsets[1] = 124;

			Lock_warn_offsets[0] = 57;
			Lock_warn_offsets[1] = 150;

			strcpy_s(fname_arc, "2_rightarc1");
			strcpy_s(fname_laser, "2_toparc2");
			strcpy_s(fname_lock, "2_toparc3");
		}
	}

	auto hud_gauge = gauge_load_common<HudGaugeThreatIndicator>(settings);

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

	hud_gauge->initBitmaps(fname_arc, fname_laser, fname_lock);
	hud_gauge->initLaserWarnOffsets(Laser_warn_offsets[0], Laser_warn_offsets[1]);
	hud_gauge->initLockWarnOffsets(Lock_warn_offsets[0], Lock_warn_offsets[1]);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_center_reticle(gauge_settings* settings)
{
	char fname[MAX_FILENAME_LEN];
	bool firepoints = false;
	int scaleX = 15;
	int scaleY = 10;
	int size = 5;
	int autoaim_frame = -1;
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

	if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
		if(gr_screen.res == GR_640) {
			settings->offset[0] = -12;
			settings->offset[1] = -5;

			strcpy_s(fname, "reticle1_fs1");
		} else {
			settings->offset[0] = -19;
			settings->offset[1] = -8;

			strcpy_s(fname, "2_reticle1_fs1");
		}
	} else {
		if(gr_screen.res == GR_640) {
			settings->offset[0] = -12;
			settings->offset[1] = -5;

			strcpy_s(fname, "reticle1");
		} else {
			settings->offset[0] = -19;
			settings->offset[1] = -14;

			strcpy_s(fname, "2_reticle1");
		}
	}

	auto hud_gauge = gauge_load_common<HudGaugeReticle>(settings);

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

	if(optional_string("Autoaim Frame:"))
		stuff_int(&autoaim_frame);

	hud_gauge->initBitmaps(fname);
	hud_gauge->initFirepointDisplay(firepoints, scaleX, scaleY, size);
	hud_gauge->setAutoaimFrame(autoaim_frame);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_throttle(gauge_settings* settings)
{
	int bottom_offset_y;
	int throttle_h, throttle_w;
	int throttle_aburn_h;
	int max_speed_offset[2];
	bool show_max_speed = true;
	int zero_speed_offset[2];
	bool show_min_speed = true;
	bool orbit = true;
	int orbit_center_offset[2];
	int orbit_radius;
	int target_speed_offset[2] = {0, 0};
	bool show_target_speed = false;
	bool show_target_speed_percent = false;
	int glide_offset[2] = {0, 0};
	bool custom_glide = false;
	int match_speed_offset[2] = {0, 0};
	bool custom_match = false;
	char fname[MAX_FILENAME_LEN];
	bool show_background = false;
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

	// default values for the throttle
	if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
		if(gr_screen.res == GR_640) {
			settings->offset[0] = -103;
			settings->offset[1] = 4;

			bottom_offset_y = 65;
			throttle_h = 50;
			throttle_w = 49;
			throttle_aburn_h = 17;
			max_speed_offset[0] = 14;
			max_speed_offset[1] = 14;
			zero_speed_offset[0] = 33;
			zero_speed_offset[1] = 63;
			orbit_center_offset[0] = 103;
			orbit_center_offset[1] = -1;
			orbit_radius = 104;
			strcpy_s(fname, "leftarc_fs1");
		} else {
			settings->offset[0] = -165;
			settings->offset[1] = 6;

			bottom_offset_y = 104;
			throttle_h = 80;
			throttle_w = 78;
			throttle_aburn_h = 27;
			max_speed_offset[0] = 22;
			max_speed_offset[1] = 22;
			zero_speed_offset[0] = 53;
			zero_speed_offset[1] = 101;
			orbit_center_offset[0] = 165;
			orbit_center_offset[1] = -1;
			orbit_radius = 166;
			strcpy_s(fname, "2_leftarc_fs1");
		}
		show_background = true;
	} else {
		if(gr_screen.res == GR_640) {
			settings->offset[0] = -104;
			settings->offset[1] = -72;

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
			settings->offset[0] = -166;
			settings->offset[1] = -115;

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

	auto hud_gauge = gauge_load_common<HudGaugeThrottle>(settings);

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
	if(optional_string("Show Max Speed Label:")) {
		stuff_boolean(&show_max_speed);
	}
	if(optional_string("Min Speed Label Offsets:")) {
		stuff_int_list(zero_speed_offset, 2);
	}
	if(optional_string("Show Min Speed Label:")) {
		stuff_boolean(&show_min_speed);
	}
	if(optional_string("Orbit Center Offsets:")) {
		stuff_int_list(orbit_center_offset, 2);
	}
	if(optional_string("Orbit Radius:")) {
		stuff_int(&orbit_radius);
	}
	if(optional_string("Current Speed Offsets:")) {
		stuff_int_list(orbit_center_offset, 2);
		orbit = false;
	}
	if(optional_string("Target Speed Offsets:")) {
		stuff_int_list(target_speed_offset, 2);
		show_target_speed = true;
	}
	if ( optional_string("Show Percentage:") ) {
		stuff_boolean(&show_target_speed_percent);
	}
	if(optional_string("Glide Status Offsets:")) {
		stuff_int_list(glide_offset, 2);
		custom_glide = true;
	}
	if(optional_string("Match Speed Status Offsets:")) {
		stuff_int_list(match_speed_offset, 2);
		custom_match = true;
	}

	hud_gauge->initThrottleStartY(bottom_offset_y);
	hud_gauge->initThrottleSizes(throttle_w, throttle_h);
	hud_gauge->initAburnHeight(throttle_aburn_h);
	hud_gauge->initMaxSpeedOffsets(max_speed_offset[0], max_speed_offset[1], show_max_speed);
	hud_gauge->initZeroSpeedOffsets(zero_speed_offset[0], zero_speed_offset[1], show_min_speed);
	hud_gauge->initOrbitCenterOffsets(orbit_center_offset[0], orbit_center_offset[1], orbit);
	hud_gauge->initOrbitRadius(orbit_radius);
	hud_gauge->initTargetSpeedOffsets(target_speed_offset[0], target_speed_offset[1], show_target_speed, show_target_speed_percent);
	hud_gauge->initGlideOffsets(glide_offset[0], glide_offset[1], custom_glide);
	hud_gauge->initMatchSpeedOffsets(match_speed_offset[0], match_speed_offset[1], custom_match);
	hud_gauge->initBitmaps(fname);
	hud_gauge->showBackground(show_background);

	gauge_assign_common(settings, std::move(hud_gauge));
}

/**
 * Load retail style ETS gauge
 * i.e. treats weapons, shields & engines gauges as a single gauge
 */
void load_gauge_ets_retail(gauge_settings* settings)
{
	int bar_h;
	int letter_offsets[2];
	int top_offsets[2];
	int bottom_offsets[2];
	char ets_letters[num_retail_ets_gauges];
	char fname[MAX_FILENAME_LEN] = "energy1";
	int gauge_offset; // distance between micro gauges
	int i;
	int gauge_positions[num_retail_ets_gauges];
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 1.0f;

	if (Lcl_gr) {
		ets_letters[0] = 'G'; ets_letters[1] = 'S'; ets_letters[2] = 'A'; // German
	} else if (Lcl_fr) {
		ets_letters[0] = 'C'; ets_letters[1] = 'B'; ets_letters[2] = 'M'; // French
	} else {
		ets_letters[0] = 'G'; ets_letters[1] = 'S'; ets_letters[2] = 'E'; // English
	}

	// default values which may be overwritten by .tbl
	if(gr_screen.res == GR_640) {
		settings->offset[0] = -117;
		settings->offset[1] = -100;

		gauge_offset = 17;
	} else {
		settings->offset[0] = -144;
		settings->offset[1] = -120;

		gauge_offset = 18;
	}
	bar_h = 41;
	letter_offsets[0] = 2;
	letter_offsets[1] = 42;
	top_offsets[0] = 0;
	top_offsets[1] = 0;
	bottom_offsets[0] = 0;
	bottom_offsets[1] = 50;

	auto hud_gauge = gauge_load_common<HudGaugeEtsRetail>(settings);

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

	// calculate offsets for the three gauges from settings->coords[0], which was set by gauge_load_common
	for (i = 0; i < num_retail_ets_gauges; ++i) {
		gauge_positions[i] = settings->coords[0] + gauge_offset * i;
	}

	hud_gauge->initLetters(ets_letters);
	hud_gauge->initLetterOffsets(letter_offsets[0], letter_offsets[1]);
	hud_gauge->initTopOffsets(top_offsets[0], top_offsets[1]);
	hud_gauge->initBottomOffsets(bottom_offsets[0], bottom_offsets[1]);
	hud_gauge->initBarHeight(bar_h);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initGaugePositions(gauge_positions);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_ets_weapons(gauge_settings* settings)
{
	int bar_h;
	int letter_offsets[2];
	int top_offsets[2];
	int bottom_offsets[2];
	char letter;
	char fname[MAX_FILENAME_LEN] = "energy1";
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 1.0f;

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
		settings->offset[0] = -117;
		settings->offset[1] = -100;
	} else {
		settings->offset[0] = -144;
		settings->offset[1] = -120;
	}
	bar_h = 41;
	letter_offsets[0] = 2;
	letter_offsets[1] = 42;
	top_offsets[0] = 0;
	top_offsets[1] = 0;
	bottom_offsets[0] = 0;
	bottom_offsets[1] = 50;

	auto hud_gauge = gauge_load_common<HudGaugeEtsWeapons>(settings);

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

	hud_gauge->initLetter(letter);
	hud_gauge->initLetterOffsets(letter_offsets[0], letter_offsets[1]);
	hud_gauge->initTopOffsets(top_offsets[0], top_offsets[1]);
	hud_gauge->initBottomOffsets(bottom_offsets[0], bottom_offsets[1]);
	hud_gauge->initBarHeight(bar_h);
	hud_gauge->initBitmaps(fname);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_ets_shields(gauge_settings* settings)
{
	int bar_h;
	int letter_offsets[2];
	int top_offsets[2];
	int bottom_offsets[2];
	char letter;
	char fname[MAX_FILENAME_LEN] = "energy1";
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 1.0f;

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
		settings->offset[0] = -100;
		settings->offset[1] = -100;
	} else {
		settings->offset[0] = -126;
		settings->offset[1] = -120;
	}
	bar_h = 41;

	letter_offsets[0] = 2;
	letter_offsets[1] = 42;
	top_offsets[0] = 0;
	top_offsets[1] = 0;
	bottom_offsets[0] = 0;
	bottom_offsets[1] = 50;

	auto hud_gauge = gauge_load_common<HudGaugeEtsShields>(settings);

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

	hud_gauge->initBarHeight(bar_h);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initBottomOffsets(bottom_offsets[0], bottom_offsets[1]);
	hud_gauge->initLetter(letter);
	hud_gauge->initLetterOffsets(letter_offsets[0], letter_offsets[1]);
	hud_gauge->initTopOffsets(top_offsets[0], top_offsets[1]);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_ets_engines(gauge_settings* settings)
{
	int bar_h;
	int letter_offsets[2];
	int top_offsets[2];
	int bottom_offsets[2];
	char letter;
	char fname[MAX_FILENAME_LEN] = "energy1";
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 1.0f;

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
		settings->offset[0] = -83;
		settings->offset[1] = -100;
	} else {
		settings->offset[0] = -108;
		settings->offset[1] = -120;
	}

	bar_h = 41;

	letter_offsets[0] = 2;
	letter_offsets[1] = 42;
	top_offsets[0] = 0;
	top_offsets[1] = 0;
	bottom_offsets[0] = 0;
	bottom_offsets[1] = 50;

	auto hud_gauge = gauge_load_common<HudGaugeEtsEngines>(settings);

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

	hud_gauge->initBarHeight(bar_h);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initBottomOffsets(bottom_offsets[0], bottom_offsets[1]);
	hud_gauge->initLetter(letter);
	hud_gauge->initLetterOffsets(letter_offsets[0], letter_offsets[1]);
	hud_gauge->initTopOffsets(top_offsets[0], top_offsets[1]);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_extra_target_data(gauge_settings* settings)
{
	int dock_offsets[2];
	int dock_max_w;
	int time_offsets[2];
	int bracket_offsets[2];
	int order_offsets[2];
	int order_max_w;
	char fname[MAX_FILENAME_LEN] = "targetview3";
	
	settings->origin[0] = 0.0f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 5;
		settings->offset[1] = -200;

		dock_offsets[0] = 8;
		dock_offsets[1] = 19;

		time_offsets[0] = 8;
		time_offsets[1] = 10;
	} else {
		settings->offset[0] = 5;
		settings->offset[1] = -216;

		dock_offsets[0] = 8;
		dock_offsets[1] = 18;

		time_offsets[0] = 8;
		time_offsets[1] = 9;
	}

	bracket_offsets[0] = 0;
	bracket_offsets[1] = 3;

	order_offsets[0] = 8;
	order_offsets[1] = 0;

	dock_max_w = 173;
	order_max_w = 162;

	auto hud_gauge = gauge_load_common<HudGaugeExtraTargetData>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Bracket Offsets:")) {
		stuff_int_list(bracket_offsets, 2);
	}
	if(optional_string("Dock Offsets:")) {
		stuff_int_list(dock_offsets, 2);
	}
	if(optional_string("Dock Max Width:")) {
		stuff_int(&dock_max_w);
	}
	if(optional_string("Order Offsets:")) {
		stuff_int_list(order_offsets, 2);
	}
	if(optional_string("Order Max Width:")) {
		stuff_int(&order_max_w);
	}
	if(optional_string("Time Offsets:")) {
		stuff_int_list(time_offsets, 2);
	}

	hud_gauge->initBitmaps(fname);
	hud_gauge->initBracketOffsets(bracket_offsets[0], bracket_offsets[1]);
	hud_gauge->initDockOffsets(dock_offsets[0], dock_offsets[1]);
	hud_gauge->initDockMaxWidth(dock_max_w);
	hud_gauge->initOrderOffsets(order_offsets[0], order_offsets[1]);
	hud_gauge->initOrderMaxWidth(order_max_w);
	hud_gauge->initTimeOffsets(time_offsets[0], time_offsets[1]);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_radar_std(gauge_settings* settings)
{
	int Radar_blip_radius_normal;
	int Radar_blip_radius_target;
	int Radar_radius[2];
	int Radar_dist_offsets[RR_MAX_RANGES][2];
	float Radar_center_offsets[2];
	char fname[MAX_FILENAME_LEN];
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -63;
		settings->offset[1] = -111;

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

		Radar_dist_offsets[2][0] = 111;
		Radar_dist_offsets[2][1] = 92;

		strcpy_s(fname, "radar1");
	} else {
		settings->offset[0] = -101;
		settings->offset[1] = -178;

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

	// Ngon radar -- Cyborg17 - to fit this into gauge_load_common, we have to have this right before origin and Offset
	int num_sides = 0;
	float offset  = 0.0f;

	if (optional_string("Ngon Sides:")) {
		stuff_int(&num_sides);
		if ((num_sides < RADAR_NGON_MIN_SIDES)) {
			Warning(LOCATION,
			        "Invalid value for 'Ngon Sides'! Value must be greater than %i.\n Using standard radar instead.\n",
			        RADAR_NGON_MIN_SIDES);
			num_sides = 0;
		}

		if (optional_string("Ngon Offset:")) {
			stuff_float(&offset);
		}
	}

	//=======================================================================================================
	// Cyborg17 - not using auto here is a huge pain to get exactly right and to read. If we put this in a
	// normal if block, either auto will break or hud_gauge will go out of scope before the last block below.
	// The ternery operator lets us circumvent this problem by allowing us to use auto, while also loading
	// the radar, creating the unique_ptr, and staying in scope.

	// did we have a normal radar? If so, load the regular radar.
	auto hud_gauge = (num_sides == 0)
	                     ? gauge_load_common<HudGaugeRadarStd>(settings)
	                     // if not *construct* and then load the ngon radar
	                     : gauge_load_common<HudGaugeRadarNgon>(settings, new HudGaugeRadarNgon(num_sides, offset));

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Radar Center Offsets:")) {
		stuff_float_list(Radar_center_offsets, 2);
	}
	if(optional_string("Radar Size:")) {
		stuff_int_list(Radar_radius, 2);
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
		hud_gauge->initBitmaps(fname);
		hud_gauge->initBlipRadius(Radar_blip_radius_normal, Radar_blip_radius_target);
		hud_gauge->initCenterOffsets(Radar_center_offsets[0], Radar_center_offsets[1]);
		hud_gauge->initDistanceInfinityOffsets(Radar_dist_offsets[2][0], Radar_dist_offsets[2][1]);
		hud_gauge->initDistanceLongOffsets(Radar_dist_offsets[1][0], Radar_dist_offsets[1][1]);
		hud_gauge->initDistanceShortOffsets(Radar_dist_offsets[0][0], Radar_dist_offsets[0][1]);
		hud_gauge->initRadius(Radar_radius[0], Radar_radius[1]);
		hud_gauge->initInfinityIcon();

		gauge_assign_common(settings, std::move(hud_gauge));
	}
}

void load_gauge_radar_orb(gauge_settings* settings)
{
	int Radar_blip_radius_normal;
	int Radar_blip_radius_target;
	int Radar_radius[2];
	int Radar_dist_offsets[RR_MAX_RANGES][2];
	float Radar_center_offsets[2];
	char fname[MAX_FILENAME_LEN];
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -63;
		settings->offset[1] = -111;

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
		settings->offset[0] = -101;
		settings->offset[1] = -178;

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

	auto hud_gauge = gauge_load_common<HudGaugeRadarOrb>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Radar Center Offsets:")) {
		stuff_float_list(Radar_center_offsets, 2);
	}
	if(optional_string("Radar Size:")) {
		stuff_int_list(Radar_radius, 2);
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
		hud_gauge->initBitmaps(fname);
		hud_gauge->initBlipRadius(Radar_blip_radius_normal, Radar_blip_radius_target);
		hud_gauge->initCenterOffsets(Radar_center_offsets[0], Radar_center_offsets[1]);
		hud_gauge->initDistanceInfinityOffsets(Radar_dist_offsets[2][0], Radar_dist_offsets[2][1]);
		hud_gauge->initDistanceLongOffsets(Radar_dist_offsets[1][0], Radar_dist_offsets[1][1]);
		hud_gauge->initDistanceShortOffsets(Radar_dist_offsets[0][0], Radar_dist_offsets[0][1]);
		hud_gauge->initRadius(Radar_radius[0], Radar_radius[1]);
		hud_gauge->initInfinityIcon();

		gauge_assign_common(settings, std::move(hud_gauge));
	}
}

/**
 * BSG style DRADIS as used by Diaspora
 * Unfortunately, I can't see how to make this fit the gauge_load_common function
 */
void load_gauge_radar_dradis(gauge_settings* settings)
{
	// basic radar gauge info
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

	gamesnd_id loop_snd;
	float loop_snd_volume = 1.0f;

	gamesnd_id arrival_beep_snd;
	gamesnd_id departure_beep_snd;

	gamesnd_id stealth_arrival_snd;
	gamesnd_id stealth_departure_snd;

	float arrival_beep_delay = 0.0f;
	float departure_beep_delay = 0.0f;
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -89;
		settings->offset[1] = -148;
	} else {
		settings->offset[0] = -143;
		settings->offset[1] = -237;
	}

	Radar_radius[0] = 281;
	Radar_radius[1] = 233;

	if(check_base_res(settings->base_res)) {
		if(optional_string("Position:")) {
			stuff_int_list(settings->coords, 2);
			settings->use_coords = true;
		} else {
			if (optional_string("Scale Gauge:")) {
				stuff_boolean(&settings->scale_gauge);
			}

			if (optional_string("Force Scaling Above:")) {
				stuff_int_list(settings->force_scaling_above_res, 2);
			}

			adjust_base_res(settings->base_res, settings->force_scaling_above_res, &settings->aspect_quotient, settings->scale_gauge);

			if(optional_string("Origin:")) {
				stuff_float_list(settings->origin, 2);

				required_string("Offset:");
				stuff_int_list(settings->offset, 2);
			}

			if(optional_string("Offset:")) {
				Error(LOCATION, "HUD gauges table: Offset must also have Origin defined");
			}

			settings->coords[0] = (int)(settings->base_res[0] * settings->origin[0]) + settings->offset[0];
			settings->coords[1] = (int)(settings->base_res[1] * settings->origin[1]) + settings->offset[1];
			settings->use_coords = true;
		}
	} else {
		if(gr_screen.res == GR_640) {
			settings->base_res[0] = 640;
			settings->base_res[1] = 480;
		} else {
			settings->base_res[0] = 1024;
			settings->base_res[1] = 768;
		}

		adjust_base_res(settings->base_res, settings->force_scaling_above_res, &settings->aspect_quotient, settings->scale_gauge);

		settings->coords[0] = (int)(settings->base_res[0] * settings->origin[0]) + settings->offset[0];
		settings->coords[1] = (int)(settings->base_res[1] * settings->origin[1]) + settings->offset[1];
		settings->use_coords = true;
	}

	if (optional_string("Font:")) {
		settings->font_num = font::parse_font();
	} else {
		if ( settings->font_num < 0 ) {
			settings->font_num = font::FONT1;
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
	if(optional_string("Cockpit Target:") && settings->ship_idx && settings->ship_idx->at(0) >= 0) {
		stuff_string(display_name, F_NAME, MAX_FILENAME_LEN);

		if(optional_string("Canvas Size:")) {
			stuff_int_list(canvas_size, 2);
		}

		if ( optional_string("Display Offset:") ) {
			stuff_int_list(display_offset, 2);
		}

		required_string("Display Size:");
		stuff_int_list(display_size, 2);
	} else {
		// adjust for multimonitor setups ONLY if not rendering gauge to a texture
		adjust_for_multimonitor(settings->base_res, true, settings->coords);
	}

	parse_game_sound("Loop Sound:", &loop_snd);

	if (optional_string("Loop Volume:"))
	{
		stuff_float(&loop_snd_volume);

		if (loop_snd_volume <= 0.0f)
		{
			Warning(LOCATION, "\"Loop Volume:\" value of \"%f\" is invalid! Must be more than zero! Resetting to default.", arrival_beep_delay);
			loop_snd_volume = 1.0f;
		}
	}

	parse_game_sound("Arrival Beep Sound:", &arrival_beep_snd);
	parse_game_sound("Stealth arrival Beep Sound:", &stealth_arrival_snd);

	if (optional_string("Minimum Beep Delay:"))
	{
		stuff_float(&arrival_beep_delay);

		if (arrival_beep_delay < 0.0f)
		{
			Warning(LOCATION, "\"Minimum Beep Delay:\" value of \"%f\" is invalid! Must be more than or equal to zero! Resetting to default.", arrival_beep_delay);
			arrival_beep_delay = 0.0f;
		}
	}

	parse_game_sound("Departure Beep Sound:", &departure_beep_snd);
	parse_game_sound("Stealth departure Beep Sound:", &stealth_departure_snd);

	if (optional_string("Minimum Beep Delay:"))
	{
		stuff_float(&departure_beep_delay);

		if (departure_beep_delay < 0.0f)
		{
			Warning(LOCATION, "\"Minimum Beep Delay:\" value of \"%f\" is invalid! Must be more than or equal to zero! Resetting to default.", departure_beep_delay);
			departure_beep_delay = 0.0f;
		}
	}

	std::unique_ptr<HudGaugeRadarDradis> hud_gauge(new HudGaugeRadarDradis());
	hud_gauge->initBaseResolution(settings->base_res[0], settings->base_res[1], settings->aspect_quotient);
	hud_gauge->initPosition(settings->coords[0], settings->coords[1]);
	hud_gauge->initFont(settings->font_num);
	hud_gauge->initOriginAndOffset(settings->origin[0], settings->origin[1], settings->offset[0], settings->offset[1]);
	hud_gauge->initCoords(settings->use_coords, settings->coords[0], settings->coords[1]);

	hud_gauge->initRadius(Radar_radius[0], Radar_radius[1]);
	hud_gauge->initBitmaps(xy_fname, xz_yz_fname, sweep_fname, target_fname, unknown_fname);
	hud_gauge->initCockpitTarget(display_name, display_offset[0], display_offset[1], display_size[0], display_size[1], canvas_size[0], canvas_size[1]);
	hud_gauge->initSound(loop_snd, loop_snd_volume, arrival_beep_snd, departure_beep_snd, stealth_arrival_snd, stealth_departure_snd, arrival_beep_delay, departure_beep_delay);
	hud_gauge->initChase_view_only(settings->chase_view_only);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_text_warnings(gauge_settings* settings)
{
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 0;
		settings->offset[1] = -68;
	} else {
		settings->offset[0] = 0;
		settings->offset[1] = -109;
	}

	auto hud_gauge = gauge_load_common<HudGaugeTextWarnings>(settings);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_target_monitor(gauge_settings* settings)
{
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
	CargoScanType Cargo_scan_type;
	int Cargo_scan_start_offsets[2];
	int Cargo_scan_size[2];

	int Subsys_name_offsets[2] = {0, 0};
	bool Use_subsys_name_offsets = false;
	
	int Subsys_integrity_offsets[2] = {0, 0};
	bool Use_subsys_integrity_offsets = false;

	int Disabled_status_offsets[2] = {0, 0};
	bool Use_disabled_status_offsets = false;

	bool desaturate = false;

	char fname_monitor[MAX_FILENAME_LEN] = "targetview1";
	char fname_integrity[MAX_FILENAME_LEN] = "targetview2";
	char fname_static[MAX_FILENAME_LEN] = "TargetStatic";
	char fname_monitor_mask[MAX_FILENAME_LEN] = "";
	
	settings->origin[0] = 0.0f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 5;
		settings->offset[1] = -161;
	} else {
		settings->offset[0] = 5;
		settings->offset[1] = -178;
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

	Cargo_scan_type = Cmdline_dualscanlines ? CargoScanType::DUAL_SCAN_LINES : CargoScanType::DEFAULT;
	Cargo_scan_start_offsets[0] = 2;
	Cargo_scan_start_offsets[1] = 45;
	Cargo_scan_size[0] = 130;
	Cargo_scan_size[1] = 109;

	auto hud_gauge = gauge_load_common<HudGaugeTargetBox>(settings);

	if(optional_string("Monitor Filename:")) {
		stuff_string(fname_monitor, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Monitor Alpha Mask Filename:")) {
		stuff_string(fname_monitor_mask, F_NAME, MAX_FILENAME_LEN);
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
	if (optional_string("Cargo Scan Type:")) {
		int type = required_string_one_of(3, "default", "dualscanlines", "discoscanlines");
		if (type == 0) {
			Cargo_scan_type = CargoScanType::DEFAULT;
		} else if (type == 1) {
			Cargo_scan_type = CargoScanType::DUAL_SCAN_LINES;
		} else {
			Cargo_scan_type = CargoScanType::DISCO_SCAN_LINES;
		}
	}
	if(optional_string("Cargo Scan Start Offsets:")) {
		stuff_int_list(Cargo_scan_start_offsets, 2);
	}
	if(optional_string("Cargo Scan Size:")) {
		stuff_int_list(Cargo_scan_size, 2);
	}
	if ( optional_string("Subsystem Name Offsets:") ) {
		stuff_int_list(Subsys_name_offsets, 2);
		Use_subsys_name_offsets = true;
	}
	if ( optional_string("Subsystem Integrity Offsets:") ) {
		stuff_int_list(Subsys_integrity_offsets, 2);
		Use_subsys_integrity_offsets = true;
	}
	if ( optional_string("Disabled Status Offsets:") ) {
		stuff_int_list(Disabled_status_offsets, 2);
		Use_disabled_status_offsets = true;
	}
	if ( optional_string("Desaturate:") ) {
		stuff_boolean(&desaturate);
	}

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
	hud_gauge->initCargoScanType(Cargo_scan_type);
	hud_gauge->initCargoScanStartOffsets(Cargo_scan_start_offsets[0], Cargo_scan_start_offsets[1]);
	hud_gauge->initCargoScanSize(Cargo_scan_size[0], Cargo_scan_size[1]);
	hud_gauge->initSubsysNameOffsets(Subsys_name_offsets[0], Subsys_name_offsets[1], Use_subsys_name_offsets);
	hud_gauge->initSubsysIntegrityOffsets(Subsys_integrity_offsets[0], Subsys_integrity_offsets[1], Use_subsys_integrity_offsets);
	hud_gauge->initDisabledStatusOffsets(Disabled_status_offsets[0], Disabled_status_offsets[1], Use_disabled_status_offsets);
	hud_gauge->initDesaturate(desaturate);
	hud_gauge->initBitmaps(fname_monitor, fname_monitor_mask, fname_integrity, fname_static);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_squad_message(gauge_settings* settings)
{
	int Pgup_offsets[2];
	int Pgdn_offsets[2];
	int Header_offsets[2];
	int Item_start_offsets[2];
	int Middle_frame_start_offset_y;
	int bottom_bg_offset = 0;
	int Item_h;
	int Item_offset_x;
	char fname_top[MAX_FILENAME_LEN] = "message1";
	char fname_middle[MAX_FILENAME_LEN] = "message2";
	char fname_bottom[MAX_FILENAME_LEN] = "message3";
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 0.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -195;
		settings->offset[1] = 5;

		Pgup_offsets[0] = 145;
		Pgup_offsets[1] = 4;
		Pgdn_offsets[0] = 145;
		Pgdn_offsets[1] = 115;
	} else {
		settings->offset[0] = -197;
		settings->offset[1] = 5;

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

	auto hud_gauge = gauge_load_common<HudGaugeSquadMessage>(settings);

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
	if(optional_string("Bottom Background Offset:")) {
		stuff_int(&bottom_bg_offset);
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

	hud_gauge->initBitmaps(fname_top, fname_middle, fname_bottom);
	hud_gauge->initHeaderOffsets(Header_offsets[0], Header_offsets[1]);
	hud_gauge->initItemStartOffsets(Item_start_offsets[0], Item_start_offsets[1]);
	hud_gauge->initMiddleFrameStartOffsetY(Middle_frame_start_offset_y);
	hud_gauge->initBottomBgOffset(bottom_bg_offset);
	hud_gauge->initItemHeight(Item_h);
	hud_gauge->initItemOffsetX(Item_offset_x);
	hud_gauge->initPgUpOffsets(Pgup_offsets[0], Pgup_offsets[1]);
	hud_gauge->initPgDnOffsets(Pgdn_offsets[0], Pgdn_offsets[1]);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_objective_notify(gauge_settings* settings)
{
	int Objective_text_offset_y;
	int Objective_text_val_offset_y;
	int Subspace_text_offset_y;
	int Subspace_text_val_offset_y;
	int Red_text_offset_y;
	int Red_text_val_offset_y;
	char fname[MAX_FILENAME_LEN] = "objective1";
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -75;
		settings->offset[1] = -126;

		Objective_text_offset_y = 2;
		Objective_text_val_offset_y = 11;
		Subspace_text_offset_y = 2;
		Subspace_text_val_offset_y = 10;
		Red_text_offset_y = 2;
		Red_text_val_offset_y = 10;
	} else {
		settings->offset[0] = -76;
		settings->offset[1] = -200;

		Objective_text_offset_y = 2;
		Objective_text_val_offset_y = 11;
		Subspace_text_offset_y = 2;
		Subspace_text_val_offset_y = 10;
		Red_text_offset_y = 2;
		Red_text_val_offset_y = 10;
	}

	auto hud_gauge = gauge_load_common<HudGaugeObjectiveNotify>(settings);

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

	hud_gauge->initBitmaps(fname);
	hud_gauge->initObjTextOffsetY(Objective_text_offset_y);
	hud_gauge->initObjValueOffsetY(Objective_text_val_offset_y);
	hud_gauge->initSubspaceTextOffsetY(Subspace_text_offset_y);
	hud_gauge->initSubspaceValueOffsetY(Subspace_text_val_offset_y);
	hud_gauge->initRedAlertTextOffsetY(Red_text_offset_y);
	hud_gauge->initRedAlertValueOffsetY(Red_text_val_offset_y);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_weapons(gauge_settings* settings)
{
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
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -143;
		settings->offset[1] = -228;
	} else {
		settings->offset[0] = -144;
		settings->offset[1] = -257;
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

	auto hud_gauge = gauge_load_common<HudGaugeWeapons>(settings);

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
	hud_gauge->initLinkIcon();

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_directives(gauge_settings* settings)
{
	int header_offsets[2];
	int middle_frame_offset_y;
	int text_start_offsets[2];
	int text_h;
	int max_line_width = 167;
	char fname_top[MAX_FILENAME_LEN] = "directives1";
	char fname_middle[MAX_FILENAME_LEN] = "directives2";
	char fname_bottom[MAX_FILENAME_LEN] = "directives3";
	int bottom_bg_offset = 0;
	
	settings->origin[0] = 0.0f;
	settings->origin[1] = 0.5f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 5;
		settings->offset[1] = -62;
	} else {
		settings->offset[0] = 5;
		settings->offset[1] = -106;
	}

	header_offsets[0] = 2;
	header_offsets[1] = 2;
	middle_frame_offset_y = 12;
	text_start_offsets[0] = 3;
	text_start_offsets[1] = 14;
	text_h = 9;

	auto hud_gauge = gauge_load_common<HudGaugeDirectives>(settings);

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
	if(optional_string("Bottom Background Offset:")) {
		stuff_int(&bottom_bg_offset);
	}
	if ( optional_string("Max Line Width:") ) {
		stuff_int(&max_line_width);
	}

	hud_gauge->initBitmaps(fname_top, fname_middle, fname_bottom);
	hud_gauge->initMiddleFrameOffsetY(middle_frame_offset_y);
	hud_gauge->initTextHeight(text_h);
	hud_gauge->initBottomBgOffset(bottom_bg_offset);
	hud_gauge->initTextStartOffsets(text_start_offsets[0], text_start_offsets[1]);
	hud_gauge->initHeaderOffsets(header_offsets[0], header_offsets[1]);
	hud_gauge->initMaxLineWidth(max_line_width);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_talking_head(gauge_settings* settings)
{
	int Header_offsets[2];
	int Anim_offsets[2];
	int Anim_size[2];
	char fname[MAX_FILENAME_LEN] = "head1";
	
	settings->origin[0] = 0.0f;
	settings->origin[1] = 0.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 5;
		settings->offset[1] = 35;
	} else {
		settings->offset[0] = 5;
		settings->offset[1] = 56;
	}

	Header_offsets[0] = 2;
	Header_offsets[1] = 2;	
	Anim_offsets[0] = 2;
	Anim_offsets[1] = 10;
	Anim_size[0] = 160;
	Anim_size[1] = 120;

	auto hud_gauge = gauge_load_common<HudGaugeTalkingHead>(settings);

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
		mprintf(("Animation Background Size in hud_gauges.tbl and -hdg.tbms is deprecated. Use \"Animation Size\" instead.\n"));
		stuff_int_list(Anim_size, 2);
	}
	if(optional_string("Animation Size:")) {
		stuff_int_list(Anim_size, 2);
	}

	hud_gauge->initAnimOffsets(Anim_offsets[0], Anim_offsets[1]);
	hud_gauge->initAnimSizes(Anim_size[0], Anim_size[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initHeaderOffsets(Header_offsets[0], Header_offsets[1]);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_countermeasures(gauge_settings* settings)
{
	int cm_text_offset[2];
	int cm_text_val_offset[2];
	char fname[MAX_FILENAME_LEN] = "countermeasure1";
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -143;
		settings->offset[1] = -137;
	} else {
		settings->offset[0] = -144;
		settings->offset[1] = -166;
	}
	cm_text_offset[0] = 36;
	cm_text_offset[1] = 4;
	cm_text_val_offset[0] = 9;
	cm_text_val_offset[1] = 4;

	auto hud_gauge = gauge_load_common<HudGaugeCmeasures>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Text Offsets:")) {
		stuff_int_list(cm_text_offset, 2);
	}
	if(optional_string("Value Offsets:")) {
		stuff_int_list(cm_text_val_offset, 2);
	}

	hud_gauge->initBitmaps(fname);
	hud_gauge->initCountTextOffsets(cm_text_offset[0], cm_text_offset[1]);
	hud_gauge->initCountValueOffsets(cm_text_val_offset[0], cm_text_val_offset[1]);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_auto_target(gauge_settings* settings)
{
	int auto_text_offset[2];
	int target_text_offset[2];
	char fname[MAX_FILENAME_LEN] = "toggle1";

	int on_color[4] = {0, 0, 0, 255};
	int off_color[4] = {-1, -1, -1, -1};
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -63;
		settings->offset[1] = -100;
	} else {
		settings->offset[0] = -64;
		settings->offset[1] = -120;
	}

	auto_text_offset[0] = 13;
	auto_text_offset[1] = 2;
	if (Lcl_pl) {
		target_text_offset[0] = 2;
		target_text_offset[1] = 10;
	} else {
		target_text_offset[0] = 7;
		target_text_offset[1] = 10;
	}
	
	auto hud_gauge = gauge_load_common<HudGaugeAutoTarget>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Auto Offsets:")) {
		stuff_int_list(auto_text_offset, 2);
	}
	if(optional_string("Target Offsets:")) {
		stuff_int_list(target_text_offset, 2);
	}

	if ( optional_string("On Text Color:") ) {
		stuff_int_list(on_color, 4);
	}

	if ( optional_string("Off Text Color:") ) {
		stuff_int_list(off_color, 4);
	}

	hud_gauge->initAutoTextOffsets(auto_text_offset[0], auto_text_offset[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initTargetTextOffsets(target_text_offset[0], target_text_offset[1]);
	hud_gauge->initOnColor(on_color[0], on_color[1], on_color[2], on_color[3]);
	hud_gauge->initOffColor(off_color[0], off_color[1], off_color[2], off_color[3]);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_auto_speed(gauge_settings* settings)
{
	int auto_text_offset[2];
	int speed_text_offset[2];
	char fname[MAX_FILENAME_LEN] = "toggle1";
	int on_color[4] = {0, 0, 0, 255};
	int off_color[4] = {-1, -1, -1, -1};
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -63;
		settings->offset[1] = -76;
	} else {
		settings->offset[0] = -64;
		settings->offset[1] = -96;
	}

	auto_text_offset[0] = 13;
	auto_text_offset[1] = 2;
	if (Lcl_pl) {
		speed_text_offset[0] = 9;
		speed_text_offset[1] = 10;
	} else {
		speed_text_offset[0] = 10;
		speed_text_offset[1] = 10;
	}
	
	auto hud_gauge = gauge_load_common<HudGaugeAutoSpeed>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Auto Offsets:")) {
		stuff_int_list(auto_text_offset, 2);
	}
	if(optional_string("Speed Offsets:")) {
		stuff_int_list(speed_text_offset, 2);
	}

	if ( optional_string("On Text Color:") ) {
		stuff_int_list(on_color, 4);
	}

	if ( optional_string("Off Text Color:") ) {
		stuff_int_list(off_color, 4);
	}

	hud_gauge->initAutoTextOffsets(auto_text_offset[0], auto_text_offset[1]);
	hud_gauge->initBitmaps(fname);
	hud_gauge->initSpeedTextOffsets(speed_text_offset[0], speed_text_offset[1]);
	hud_gauge->initOnColor(on_color[0], on_color[1], on_color[2], on_color[3]);
	hud_gauge->initOffColor(off_color[0], off_color[1], off_color[2], off_color[3]);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_wingman_status(gauge_settings* settings)
{
	int header_offsets[2];
	bool fixed_header_position;
	int left_frame_end_x;
	
	int single_wing_offsets[2];
	int multiple_wing_offsets[2];
	int wing_width;
	int right_bg_offset = 0;
	int wing_name_offsets[2];

	int wingmate_offsets[MAX_SHIPS_PER_WING][2];
	char fname_left[MAX_FILENAME_LEN] = "wingman1";
	char fname_middle[MAX_FILENAME_LEN] = "wingman2";
	char fname_right[MAX_FILENAME_LEN] = "wingman3";
	char fname_dots[MAX_FILENAME_LEN] = "wingman4";
	// "wingman5" isn't used anymore since Goober implemented string based wing names
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 0.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -90;
		settings->offset[1] = 144;
	} else {
		settings->offset[0] = -92;
		settings->offset[1] = 144;
	}

	header_offsets[0] = 2;
	header_offsets[1] = 2;
	fixed_header_position = false;
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

	auto hud_gauge = gauge_load_common<HudGaugeWingmanStatus>(settings);

	if(optional_string("Left Background Filename:")) {
		stuff_string(fname_left, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("First Background Filename:")) {
		stuff_string(fname_left, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Entry Background Filename:")) {
		stuff_string(fname_middle, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Right Background Filename:") || optional_string("Last Background Filename:")) {
		stuff_string(fname_right, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Dot Filename:")) {
		stuff_string(fname_dots, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Header Offsets:")) {
		stuff_int_list(header_offsets, 2);
	}
	if(optional_string("Fixed Header Position:")) {
		stuff_boolean(&fixed_header_position);
	}
	if(optional_string("Left Background Width:")) {
		stuff_int(&left_frame_end_x);
	}
	if(optional_string("First Background Size:")) {
		stuff_int(&left_frame_end_x);
	}
	if(optional_string("Entry Width:")) {
		stuff_int(&wing_width);
	}
	if(optional_string("Entry Size:")) {
		stuff_int(&wing_width);
	}
	if(optional_string("Right Background Offset:") || optional_string("Last Background Offset:")) {
		stuff_int(&right_bg_offset);
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
		if(optional_string("Right")) 
			grow_mode = 1;
		else if(optional_string("Down"))
			grow_mode = 2;
	}

	bool use_full_wingnames = false;
	if (optional_string("Use Full Wing Names:")) {
		stuff_boolean(&use_full_wingnames);
	}

	bool use_expanded_colors = false;
	if (optional_string("Use Expanded Colors:")) {
		stuff_boolean(&use_expanded_colors);
	}

	hud_gauge->initBitmaps(fname_left, fname_middle, fname_right, fname_dots);
	hud_gauge->initHeaderOffsets(header_offsets[0], header_offsets[1]);
	hud_gauge->initFixedHeaderPosition(fixed_header_position);
	hud_gauge->initLeftFrameEndX(left_frame_end_x);
	hud_gauge->initMultipleWingOffsets(multiple_wing_offsets[0], multiple_wing_offsets[1]);
	hud_gauge->initSingleWingOffsets(single_wing_offsets[0], single_wing_offsets[1]);
	hud_gauge->initWingmate1Offsets(wingmate_offsets[0][0], wingmate_offsets[0][1]);
	hud_gauge->initWingmate2Offsets(wingmate_offsets[1][0],	wingmate_offsets[1][1]);
	hud_gauge->initWingmate3Offsets(wingmate_offsets[2][0],	wingmate_offsets[2][1]);
	hud_gauge->initWingmate4Offsets(wingmate_offsets[3][0],	wingmate_offsets[3][1]);
	hud_gauge->initWingmate5Offsets(wingmate_offsets[4][0],	wingmate_offsets[4][1]);
	hud_gauge->initWingmate6Offsets(wingmate_offsets[5][0], wingmate_offsets[5][1]);
	hud_gauge->initWingNameOffsets(wing_name_offsets[0], wing_name_offsets[1]);
	hud_gauge->initWingWidth(wing_width);
	hud_gauge->initRightBgOffset(right_bg_offset);
	hud_gauge->initGrowMode(grow_mode);
	hud_gauge->initUseFullWingnames(use_full_wingnames);
	hud_gauge->initUseExpandedColors(use_expanded_colors);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_damage(gauge_settings* settings)
{
	int header_offsets[2];
	int hull_integ_offsets[2];
	int hull_integ_val_offset_x;
	int middle_frame_start_offset_y;
	int subsys_integ_start_offsets[2];
	int subsys_integ_val_offset_x;
	int bottom_bg_offset = 0;
	int line_h;
	char fname_top[MAX_FILENAME_LEN] = "damage1";
	char fname_middle[MAX_FILENAME_LEN] = "damage2";
	char fname_bottom[MAX_FILENAME_LEN] = "damage3";
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -75;
		settings->offset[1] = 38;
	} else {
		settings->offset[0] = -72;
		settings->offset[1] = 61;
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

	auto hud_gauge = gauge_load_common<HudGaugeDamage>(settings);

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
	if(optional_string("Bottom Background Offset:")) {
		stuff_int(&bottom_bg_offset);
	}

	hud_gauge->initBitmaps(fname_top, fname_middle, fname_bottom);
	hud_gauge->initHullIntegOffsets(hull_integ_offsets[0], hull_integ_offsets[1]);
	hud_gauge->initHullIntegValueOffsetX(hull_integ_val_offset_x);
	hud_gauge->initLineHeight(line_h);
	hud_gauge->initMiddleFrameStartOffsetY(middle_frame_start_offset_y);
	hud_gauge->initSubsysIntegStartOffsets(subsys_integ_start_offsets[0], subsys_integ_start_offsets[1]);
	hud_gauge->initSubsysIntegValueOffsetX(subsys_integ_val_offset_x);
	hud_gauge->initBottomBgOffset(bottom_bg_offset);
	hud_gauge->initHeaderOffsets(header_offsets[0], header_offsets[1]);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_support(gauge_settings* settings)
{
	int header_offsets[2];
	int text_val_offset_y;
	int text_dock_offset_x;
	int text_dock_val_offset_x;
	char fname[MAX_FILENAME_LEN] = "support1";
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -55;
		settings->offset[1] = 94;

		header_offsets[0] = 2;
		header_offsets[1] = 1;
		text_val_offset_y = 14;
		text_dock_offset_x = 5;
		text_dock_val_offset_x = 63;
	} else {
		settings->offset[0] = -53;
		settings->offset[1] = 150;

		header_offsets[0] = 3;
		header_offsets[1] = 2;
		text_val_offset_y = 12;
		text_dock_offset_x = 6;
		text_dock_val_offset_x = 65;
	}

	auto hud_gauge = gauge_load_common<HudGaugeSupport>(settings);

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

	hud_gauge->initBitmaps(fname);
	hud_gauge->initHeaderOffsets(header_offsets[0], header_offsets[1]);
	hud_gauge->initTextDockOffsetX(text_dock_offset_x);
	hud_gauge->initTextDockValueOffsetX(text_dock_val_offset_x);
	hud_gauge->initTextValueOffsetY(text_val_offset_y);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_training_messages(gauge_settings* settings)
{
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -146;
		settings->offset[1] = -200;
	} else {
		settings->offset[0] = -133;
		settings->offset[1] = -259;
	}

	auto hud_gauge = gauge_load_common<HudGaugeTrainingMessages>(settings);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_messages(gauge_settings* settings)
{
	int max_lines = 3;
	int max_width;
	int scroll_time = 30;
	int step_size = 3;
	int total_life = 14000;
	int line_height = 9;
	bool hidden_by_comms_menu = true;

	settings->origin[0] = 0.0f;
	settings->origin[1] = 0.0f;
	settings->offset[0] = 8;
	settings->offset[1] = 5;

	if(gr_screen.res == GR_640) {
		max_width = 620;
	} else {
		max_width = 1004;
	}

	auto hud_gauge = gauge_load_common<HudGaugeMessages>(settings);

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

	hud_gauge->initMaxLines(max_lines);
	hud_gauge->initMaxWidth(max_width);
	hud_gauge->initScrollTime(scroll_time);
	hud_gauge->initStepSize(step_size);
	hud_gauge->initTotalLife(total_life);
	hud_gauge->initLineHeight(line_height);
	hud_gauge->initHiddenByCommsMenu(hidden_by_comms_menu);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_fixed_messages(gauge_settings* settings)
{
	font::set_font(font::FONT1);
	
	int h = gr_get_font_height();

	bool center_text;

	settings->use_coords = true;
	// set both coords to INT_MIN so that it's possible to check whether a HUD table has overridden them
	// (this is necessary to determine the default value for center_text)
	settings->coords[0] = INT_MIN;
	settings->coords[1] = INT_MIN;

	auto hud_gauge = gauge_load_common<HudGaugeFixedMessages>(settings);

	if(settings->coords[0] == INT_MIN && settings->coords[1] == INT_MIN) {
		// coords have almost certainly not been overridden; set them to their true defaults and set center_text to true
		settings->coords[0] = settings->base_res[0] / 2;
		settings->coords[1] = 5 + (h * 3);
		hud_gauge->initPosition(settings->coords[0], settings->coords[1]);
		center_text = true;
	} else {
		// coords have been overridden; set center_text to false
		center_text = false;
	}

	if(optional_string("Center Text On Gauge X-Position:")) {
		stuff_boolean(&center_text);
	}

	hud_gauge->initCenterText(center_text);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_weapon_linking(gauge_settings* settings)
{
	int Weapon_link_offsets[NUM_WEAPON_LINK_MODES][2];
	char fname_arc[MAX_FILENAME_LEN];
	char fname_primary_link_1[MAX_FILENAME_LEN];
	char fname_primary_link_2[MAX_FILENAME_LEN];
	char fname_secondary_link_1[MAX_FILENAME_LEN];
	char fname_secondary_link_2[MAX_FILENAME_LEN];
	char fname_secondary_link_3[MAX_FILENAME_LEN];
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 54;
		settings->offset[1] = 2;

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
		settings->offset[0] = 86;
		settings->offset[1] = 3;

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

	auto hud_gauge = gauge_load_common<HudGaugeWeaponLinking>(settings);

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

	hud_gauge->init1PrimaryOffsets(Weapon_link_offsets[LINK_ONE_PRIMARY][0], Weapon_link_offsets[LINK_ONE_PRIMARY][1]);
	hud_gauge->init2PrimaryOffsets(Weapon_link_offsets[LINK_TWO_PRIMARY][0], Weapon_link_offsets[LINK_TWO_PRIMARY][1]);
	hud_gauge->init1SecondaryOffsets(Weapon_link_offsets[LINK_ONE_SECONDARY][0], Weapon_link_offsets[LINK_ONE_SECONDARY][1]);
	hud_gauge->init2SecondaryOffsets(Weapon_link_offsets[LINK_TWO_SECONDARY][0], Weapon_link_offsets[LINK_TWO_SECONDARY][1]);
	hud_gauge->init3SecondaryOffsets(Weapon_link_offsets[LINK_THREE_SECONDARY][0], Weapon_link_offsets[LINK_THREE_SECONDARY][1]);
	hud_gauge->initBitmaps(fname_arc, fname_primary_link_1, fname_primary_link_2, fname_secondary_link_1, fname_secondary_link_2, fname_secondary_link_3);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_multi_msg(gauge_settings* settings)
{
	settings->origin[0] = 0.0f;
	settings->origin[1] = 0.5f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 5;
		settings->offset[1] = -90;
	} else {
		settings->offset[0] = 8;
		settings->offset[1] = -144;
	}

	auto hud_gauge = gauge_load_common<HudGaugeMultiMsg>(settings);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_voice_status(gauge_settings* settings)
{
	settings->origin[0] = 0.0f;
	settings->origin[1] = 0.5f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 5;
		settings->offset[1] = -75;
	} else {
		settings->offset[0] = 8;
		settings->offset[1] = -129;
	}

	auto hud_gauge = gauge_load_common<HudGaugeVoiceStatus>(settings);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_ping(gauge_settings* settings)
{
	settings->origin[0] = 1.0f;
	settings->origin[1] = 0.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -80;
		settings->offset[1] = 3;
	} else {
		settings->offset[0] = -128;
		settings->offset[1] = 5;
	}

	auto hud_gauge = gauge_load_common<HudGaugePing>(settings);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_supernova(gauge_settings* settings)
{
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -220;
		settings->offset[1] = -140;
	} else {
		settings->offset[0] = -342;
		settings->offset[1] = -214;
	}

	auto hud_gauge = gauge_load_common<HudGaugeSupernova>(settings);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_lock(gauge_settings* settings)
{
	int Lock_gauge_half_w;
	int Lock_gauge_half_h;
	int Lockspin_half_w;
	int Lockspin_half_h;
	float Lock_triangle_height;
	float Lock_triangle_base;
	int Lock_target_box_width;
	int Lock_target_box_height;
	bool loop_locked_anim;
	bool blink_locked_anim;
	char fname_lock[MAX_FILENAME_LEN];
	char fname_spin[MAX_FILENAME_LEN];

	settings->set_position = false;
	settings->set_colour = false;

	if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
		if(gr_screen.res == GR_640) {
			Lock_gauge_half_w = 15;
			Lock_gauge_half_h = 15;
			Lockspin_half_w = 16;
			Lockspin_half_h = 16;
			Lock_triangle_height = 4.0f;
			Lock_triangle_base = 4.0f;
			Lock_target_box_width = 19;
			Lock_target_box_height = 30;
			loop_locked_anim = true;
			blink_locked_anim = false;

			strcpy_s(fname_lock, "lock1_fs1");
			strcpy_s(fname_spin, "lockspin_fs1");
		} else {
			Lock_gauge_half_w = 24;
			Lock_gauge_half_h = 25;
			Lockspin_half_w = 26;
			Lockspin_half_h = 26;
			Lock_triangle_height = 6.5f;
			Lock_triangle_base = 6.5f;
			Lock_target_box_width = 19;
			Lock_target_box_height = 30;
			loop_locked_anim = true;
			blink_locked_anim = false;

			strcpy_s(fname_lock, "2_lock1_fs1");
			strcpy_s(fname_spin, "2_lockspin_fs1");
		}
	} else {
		if(gr_screen.res == GR_640) {
			Lock_gauge_half_w = 17;
			Lock_gauge_half_h = 15;
			Lockspin_half_w = 31;
			Lockspin_half_h = 32;
			Lock_triangle_height = 4.0f;
			Lock_triangle_base = 4.0f;
			Lock_target_box_width = 19;
			Lock_target_box_height = 30;
			loop_locked_anim = false;
			blink_locked_anim = true;

			strcpy_s(fname_lock, "lock1");
			strcpy_s(fname_spin, "lockspin");
		} else {
			Lock_gauge_half_w = 28;
			Lock_gauge_half_h = 25;
			Lockspin_half_w = 50;
			Lockspin_half_h = 52;
			Lock_triangle_height = 6.5f;
			Lock_triangle_base = 6.5f;
			Lock_target_box_width = 19;
			Lock_target_box_height = 30;
			loop_locked_anim = false;
			blink_locked_anim = true;

			strcpy_s(fname_lock, "2_lock1");
			strcpy_s(fname_spin, "2_lockspin");
		}
	}

	auto hud_gauge = gauge_load_common<HudGaugeLock>(settings);

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
	if(optional_string("Loop Locked Animation:")) {
		stuff_boolean(&loop_locked_anim);
	}
	if(optional_string("Blink Locked Animation:")) {
		stuff_boolean(&loop_locked_anim);
	}

	hud_gauge->initBitmaps(fname_lock, fname_spin);
	hud_gauge->initLoopLockedAnim(loop_locked_anim);
	hud_gauge->initBlinkLockedAnim(blink_locked_anim);
	hud_gauge->initGaugeHalfSize(Lock_gauge_half_w, Lock_gauge_half_h);
	hud_gauge->initSpinHalfSize(Lockspin_half_w, Lockspin_half_h);
	hud_gauge->initTriHeight(Lock_triangle_height);
	hud_gauge->initTriBase(Lock_triangle_base);
	hud_gauge->initTargetBoxSize(Lock_target_box_width,	Lock_target_box_height);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_offscreen(gauge_settings* settings)
{
	float Max_offscreen_tri_seperation;
	float Max_front_seperation;
	float Offscreen_tri_base;
	float Offscreen_tri_height;

	settings->set_position = false;
	settings->set_colour = false;

	if(gr_screen.res == GR_640) {
		Max_offscreen_tri_seperation = 10.0f;
		Max_front_seperation = 10.0f;
		Offscreen_tri_base = 6.0f;
		Offscreen_tri_height = 7.0f;
	} else {
		Max_offscreen_tri_seperation = 16.0f;
		Max_front_seperation = 16.0f;
		Offscreen_tri_base = 9.5f;
		Offscreen_tri_height = 11.0f;
	}

	auto hud_gauge = gauge_load_common<HudGaugeOffscreen>(settings);

	hud_gauge->initMaxTriSeperation(Max_offscreen_tri_seperation);
	hud_gauge->initMaxFrontSeperation(Max_front_seperation);
	hud_gauge->initTriBase(Offscreen_tri_base);
	hud_gauge->initTriHeight(Offscreen_tri_height);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_brackets(gauge_settings* settings)
{
	int min_target_box[2];
	int min_subtarget_box[2];
	char fname[MAX_FILENAME_LEN] = "attacker";

	settings->set_position = false;
	settings->set_colour = false;

	if(gr_screen.res == GR_640) {
		min_target_box[0] = 20;
		min_target_box[1] = 20;
		min_subtarget_box[0] = 12;
		min_subtarget_box[1] = 12;
	} else {
		min_target_box[0] = 30;
		min_target_box[1] = 30;
		min_subtarget_box[0] = 24;
		min_subtarget_box[1] = 24;
	}

	auto hud_gauge = gauge_load_common<HudGaugeBrackets>(settings);

	if(optional_string("Dot Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}

	hud_gauge->initBitmaps(fname);
	hud_gauge->initMinSubTargetBoxSizes(min_subtarget_box[0], min_subtarget_box[1]);
	hud_gauge->initMinTargetBoxSizes(min_target_box[0], min_target_box[1]);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_hostile_tri(gauge_settings* settings)
{
	int Radius; 
	float Target_triangle_base;
	float Target_triangle_height;
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 0;
		settings->offset[1] = 2;

		Target_triangle_base = 6.0f;
		Target_triangle_height = 7.0f;
		Radius = 104;
	} else {
		settings->offset[0] = 0;
		settings->offset[1] = 3;

		Target_triangle_base = 9.5f;
		Target_triangle_height = 11.0f;
		Radius = 166;
	}

	auto hud_gauge = gauge_load_common<HudGaugeHostileTriangle>(settings);

	if(optional_string("Radius:")) {
		stuff_int(&Radius);
	}
	if(optional_string("Triangle Base:")) {
		stuff_float(&Target_triangle_base);
	}
	if(optional_string("Triangle Height:")) {
		stuff_float(&Target_triangle_height);
	}

	hud_gauge->initRadius(Radius);
	hud_gauge->initTriBase(Target_triangle_base);
	hud_gauge->initTriHeight(Target_triangle_height);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_target_tri(gauge_settings* settings)
{
	int Radius; 
	float Target_triangle_base;
	float Target_triangle_height;
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 0;
		settings->offset[1] = 2;

		Target_triangle_base = 6.0f;
		Target_triangle_height = 7.0f;
		Radius = 104;
	} else {
		settings->offset[0] = 0;
		settings->offset[1] = 3;

		Target_triangle_base = 9.5f;
		Target_triangle_height = 11.0f;
		Radius = 166;
	}

	auto hud_gauge = gauge_load_common<HudGaugeTargetTriangle>(settings);

	if(optional_string("Radius:")) {
		stuff_int(&Radius);
	}
	if(optional_string("Triangle Base:")) {
		stuff_float(&Target_triangle_base);
	}
	if(optional_string("Triangle Height:")) {
		stuff_float(&Target_triangle_height);
	}

	hud_gauge->initRadius(Radius);
	hud_gauge->initTriBase(Target_triangle_base);
	hud_gauge->initTriHeight(Target_triangle_height);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_missile_tri(gauge_settings* settings)
{
	int Radius; 
	float Target_triangle_base;
	float Target_triangle_height;
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 0;
		settings->offset[1] = 2;

		Target_triangle_base = 6.0f;
		Target_triangle_height = 7.0f;
		Radius = 104;
	} else {
		settings->offset[0] = 0;
		settings->offset[1] = 3;

		Target_triangle_base = 9.5f;
		Target_triangle_height = 11.0f;
		Radius = 166;
	}

	auto hud_gauge = gauge_load_common<HudGaugeMissileTriangles>(settings);

	if(optional_string("Radius:")) {
		stuff_int(&Radius);
	}
	if(optional_string("Triangle Base:")) {
		stuff_float(&Target_triangle_base);
	}
	if(optional_string("Triangle Height:")) {
		stuff_float(&Target_triangle_height);
	}

	hud_gauge->initRadius(Radius);
	hud_gauge->initTriBase(Target_triangle_base);
	hud_gauge->initTriHeight(Target_triangle_height);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_lead(gauge_settings* settings)
{
	float Lead_indicator_half[2];
	char fname[MAX_FILENAME_LEN];

	settings->set_position = false;
	settings->set_colour = false;

	if(Hud_reticle_style == HUD_RETICLE_STYLE_FS1) {
		if(gr_screen.res == GR_640) {
			Lead_indicator_half[0] = 12.5f;
			Lead_indicator_half[1] = 12.5f;

			strcpy_s(fname, "lead1_fs1");
		} else {
			Lead_indicator_half[0] = 20.0f;
			Lead_indicator_half[1] = 20.0f;

			strcpy_s(fname, "2_lead1_fs1");
		}
	} else {
		if(gr_screen.res == GR_640) {
			Lead_indicator_half[0] = 8.0f;
			Lead_indicator_half[1] = 8.0f;

			strcpy_s(fname, "lead1");
		} else {
			Lead_indicator_half[0] = 13.0f;
			Lead_indicator_half[1] = 13.0f;

			strcpy_s(fname, "2_lead1");
		}
	}

	auto hud_gauge = gauge_load_common<HudGaugeLeadIndicator>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Center Offsets:")) {
		int temp[2];

		stuff_int_list(temp, 2);

		Lead_indicator_half[0] = i2fl(temp[0]);
		Lead_indicator_half[1] = i2fl(temp[1]);
	}

	hud_gauge->initHalfSize(Lead_indicator_half[0], Lead_indicator_half[1]);
	hud_gauge->initBitmaps(fname);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_orientation_tee(gauge_settings* settings)
{
	int Radius;
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 0;
		settings->offset[1] = 2;

		Radius = 104;
	} else {
		settings->offset[0] = 0;
		settings->offset[1] = 3;

		Radius = 166;
	}

	auto hud_gauge = gauge_load_common<HudGaugeOrientationTee>(settings);

	if(optional_string("Radius:")) {
		stuff_int(&Radius);
	}

	hud_gauge->initRadius(Radius);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_lead_sight(gauge_settings* settings)
{
	char fname[MAX_FILENAME_LEN] = "leadsight";
	
	settings->origin[0] = 0.5f;
	settings->origin[1] = 0.5f;
	settings->slew = true;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = 0;
		settings->offset[1] = 2;
	} else {
		settings->offset[0] = 0;
		settings->offset[1] = 3;
	}

	auto hud_gauge = gauge_load_common<HudGaugeLeadSight>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}

	hud_gauge->initBitmaps(fname);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_kills(gauge_settings* settings)
{
	int text_offsets[2] = {6, 4};
	int text_value_offsets[2] = {74, 4};
	char fname[MAX_FILENAME_LEN] = "kills1";
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -143;
		settings->offset[1] = -119;
		
		if(Lcl_gr) {
			text_value_offsets[0] = 118;
			text_value_offsets[1] = 4;
		}
	} else {
		settings->offset[0] = -144;
		settings->offset[1] = -144;

		if(Lcl_gr) {
			text_value_offsets[0] = 104;
			text_value_offsets[1] = 4;
		}
	}

	auto hud_gauge = gauge_load_common<HudGaugeKills>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Text Offsets:")) {
		stuff_int_list(text_offsets, 2);
	}
	if(optional_string("Value Offsets:")) {
		stuff_int_list(text_value_offsets, 2);
	}

	hud_gauge->initBitmaps(fname);
	hud_gauge->initTextOffsets(text_offsets[0], text_offsets[1]);
	hud_gauge->initTextValueOffsets(text_value_offsets[0], text_value_offsets[1]);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_flight_path(gauge_settings* settings)
{
	int Marker_half[2];
	char fname[MAX_FILENAME_LEN] = "flight_path";

	Marker_half[0] = 21;
	Marker_half[1] = 21;

	settings->set_position = false;

	auto hud_gauge = gauge_load_common<HudGaugeFlightPath>(settings);

	if(optional_string("Filename:")) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}
	if(optional_string("Center Offsets:")) {
		stuff_int_list(Marker_half, 2);
	}

	hud_gauge->initHalfSize(Marker_half[0], Marker_half[1]);
	hud_gauge->initBitmap(fname);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_warhead_count(gauge_settings* settings)
{
	int warhead_name_offsets[2] = {6, 4};
	int warhead_count_offsets[2] = {74, 4};
	int icon_width = 0;
	int icon_height = 0;
	int max_icons = 0;
	int max_columns = 0;
	HudAlignment alignment = HudAlignment::NONE;
	char fname[MAX_FILENAME_LEN] = "warhead_icon";
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 1.0f;
	settings->slew = true;

	if ( gr_screen.res == GR_640 ) {
		settings->offset[0] = -143;
		settings->offset[1] = -119;
	} else {
		settings->offset[0] = -144;
		settings->offset[1] = -144;
	}

	auto hud_gauge = gauge_load_common<HudGaugeWarheadCount>(settings);

	if ( optional_string("Filename:") ) {
		stuff_string(fname, F_NAME, MAX_FILENAME_LEN);
	}

	if ( optional_string("Name Offsets:") ) {
		stuff_int_list(warhead_name_offsets, 2);
	}

	if ( optional_string("Count Offsets:") ) {
		stuff_int_list(warhead_count_offsets, 2);
	}

	if ( optional_string("Icon Width:") ) {
		stuff_int(&icon_width);
	}

	if ( optional_string("Icon Height:") ) {
		stuff_int(&icon_height);
	}

	if ( optional_string("Max Icons:") ) {
		stuff_int(&max_icons);
	}

	if ( optional_string("Max Columns:") ) {
		stuff_int(&max_columns);
	}

	if ( optional_string("Name Alignment:") ) {
		char temp[NAME_LENGTH];
		stuff_string(temp, F_NAME, NAME_LENGTH);
		alignment = hud_alignment_lookup(temp);
	}

	hud_gauge->initBitmap(fname);
	hud_gauge->initNameOffsets(warhead_name_offsets[0], warhead_name_offsets[1]);
	hud_gauge->initCountOffsets(warhead_count_offsets[0], warhead_count_offsets[1]);
	hud_gauge->initCountSizes(icon_width, icon_height);
	hud_gauge->initMaxSymbols(max_icons);
	hud_gauge->initMaxColumns(max_columns);
	hud_gauge->initTextAlign(alignment);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_hardpoints(gauge_settings* settings)
{
	int sizes[2] = {150, 150};
	float line_width = 1.0f;
	int view_dir = HudGaugeHardpoints::TOP;
	bool show_primary = false;
	bool show_secondary = true;
	
	settings->origin[0] = 1.0f;
	settings->origin[1] = 1.0f;

	if(gr_screen.res == GR_640) {
		settings->offset[0] = -244;
		settings->offset[1] = -101;
	} else {
		settings->offset[0] = -390;
		settings->offset[1] = -98;
	}

	auto hud_gauge = gauge_load_common<HudGaugeHardpoints>(settings);

	if ( optional_string("Size:") ) {
		stuff_int_list(sizes, 2);
	}

	if ( optional_string("Line Width:") ) {
		stuff_float(&line_width);
	}

	if ( optional_string("View Direction:") ) {
		if ( optional_string("Top") ) {
			view_dir = HudGaugeHardpoints::TOP;
		} else if ( optional_string("Front") ) {
			view_dir = HudGaugeHardpoints::FRONT;
		}
	}

	if ( optional_string("Show Primary Weapons:") ) {
		stuff_boolean(&show_primary);
	}

	if ( optional_string("Show Secondary Weapons:") ) {
		stuff_boolean(&show_secondary);
	}

	hud_gauge->initSizes(sizes[0], sizes[1]);
	hud_gauge->initLineWidth(line_width);
	hud_gauge->initViewDir(view_dir);
	hud_gauge->initDrawOptions(show_primary, show_secondary);

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_primary_weapons(gauge_settings* settings)
{
	char fname_first[MAX_FILENAME_LEN] = "weapon_list1";
	char fname_entry[MAX_FILENAME_LEN] = "weapon_list2";
	char fname_last[MAX_FILENAME_LEN] = "weapon_list3";
	int header_offsets[2] = {2, 2};
	char header_text[NAME_LENGTH] = "Primary Weapons";
	int first_bg_h = 12;
	int first_bg_offset_x = 0;
	int entry_bg_h = 12;
	int entry_bg_offset_x = 0;
	int last_bg_offset_x = 0;
	int last_bg_offset_y = 0;
	int entry_h = 10;
	int entry_start_offset_y = 12;
	int ammo_x = 28;
	int link_x = 33;
	int name_x = 35;

	auto hud_gauge = gauge_load_common<HudGaugePrimaryWeapons>(settings);

	if ( optional_string("Header Offsets:") ) {
		stuff_int_list(header_offsets, 2);
	}

	if ( optional_string("Header Text:") ) {
		stuff_string(header_text, F_NAME, NAME_LENGTH);
	}

	if ( optional_string("First Background Filename:") ) {
		stuff_string(fname_first, F_NAME, MAX_FILENAME_LEN);
	}

	if ( optional_string("First Background Height:") ) {
		stuff_int(&first_bg_h);
	}

	if ( optional_string("First Background X-offset:") ) {
		stuff_int(&first_bg_offset_x);
	}

	if ( optional_string("Entry Background Filename:") ) {
		stuff_string(fname_entry, F_NAME, MAX_FILENAME_LEN);
	}

	if ( optional_string("Entry Background Height:") ) {
		stuff_int(&entry_bg_h);
	}

	if ( optional_string("Entry Background X-offset:") ) {
		stuff_int(&entry_bg_offset_x);
	}

	if ( optional_string("Last Background Filename:") ) {
		stuff_string(fname_last, F_NAME, MAX_FILENAME_LEN);
	}

	if ( optional_string("Last Background X-offset:") ) {
		stuff_int(&last_bg_offset_x);
	}

	if ( optional_string("Last Background Y-offset:") ) {
		stuff_int(&last_bg_offset_y);
	}

	if ( optional_string("Entry Height:") ) {
		stuff_int(&entry_h);
	}

	if ( optional_string("List Start Y-offset:") ) {
		stuff_int(&entry_start_offset_y);
	}

	if ( optional_string("Ammo X-offset:") ) {
		stuff_int(&ammo_x);
	}

	if ( optional_string("Link X-offset:") ) {
		stuff_int(&link_x);
	}

	if ( optional_string("Name X-offset:") ) {
		stuff_int(&name_x);
	}
	
	hud_gauge->initBitmaps(fname_first, fname_entry, fname_last);
	hud_gauge->initHeaderOffsets(header_offsets[0], header_offsets[1]);
	hud_gauge->initHeaderText(header_text);
	hud_gauge->initBgFirstHeight(first_bg_h);
	hud_gauge->initBgFirstOffsetX(first_bg_offset_x);
	hud_gauge->initBgEntryHeight(entry_bg_h);
	hud_gauge->initBgEntryOffsetX(entry_bg_offset_x);
	hud_gauge->initBgLastOffsetX(last_bg_offset_x);
	hud_gauge->initBgLastOffsetY(last_bg_offset_y);
	hud_gauge->initEntryHeight(entry_h);
	hud_gauge->initEntryStartY(entry_start_offset_y);
	hud_gauge->initPrimaryAmmoOffsetX(ammo_x);
	hud_gauge->initPrimaryLinkOffsetX(link_x);
	hud_gauge->initPrimaryNameOffsetX(name_x);
	hud_gauge->initLinkIcon();

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_secondary_weapons(gauge_settings* settings)
{
	char fname_first[MAX_FILENAME_LEN] = "weapon_list1";
	char fname_entry[MAX_FILENAME_LEN] = "weapon_list2";
	char fname_last[MAX_FILENAME_LEN] = "weapon_list3";
	int header_offsets[2] = {2, 2};
	char header_text[NAME_LENGTH] = "Secondary Weapons";
	int first_bg_h = 12;
	int first_bg_offset_x = 0;
	int entry_bg_h = 12;
	int entry_bg_offset_x = 0;
	int last_bg_offset_x = 0;
	int last_bg_offset_y = 0;
	int entry_h = 10;
	int entry_start_offset_y = 12;
	int ammo_x = 28;
	int link_x = 28;
	int name_x = 39;
	int reload_x = 118;
	int unlink_x = 33;

	auto hud_gauge = gauge_load_common<HudGaugeSecondaryWeapons>(settings);

	if ( optional_string("Header Offsets:") ) {
		stuff_int_list(header_offsets, 2);
	}

	if ( optional_string("Header Text:") ) {
		stuff_string(header_text, F_NAME, NAME_LENGTH);
	}

	if ( optional_string("First Background Filename:") ) {
		stuff_string(fname_first, F_NAME, MAX_FILENAME_LEN);
	}

	if ( optional_string("First Background Height:") ) {
		stuff_int(&first_bg_h);
	}

	if ( optional_string("First Background X-offset:") ) {
		stuff_int(&first_bg_offset_x);
	}

	if ( optional_string("Entry Background Filename:") ) {
		stuff_string(fname_entry, F_NAME, MAX_FILENAME_LEN);
	}

	if ( optional_string("Entry Background Height:") ) {
		stuff_int(&entry_bg_h);
	}

	if ( optional_string("Entry Background X-offset:") ) {
		stuff_int(&entry_bg_offset_x);
	}

	if ( optional_string("Last Background Filename:") ) {
		stuff_string(fname_last, F_NAME, MAX_FILENAME_LEN);
	}

	if ( optional_string("Last Background X-offset:") ) {
		stuff_int(&last_bg_offset_x);
	}

	if ( optional_string("Last Background Y-offset:") ) {
		stuff_int(&last_bg_offset_y);
	}

	if ( optional_string("Entry Height:") ) {
		stuff_int(&entry_h);
	}

	if ( optional_string("List Start Y-offset:") ) {
		stuff_int(&entry_start_offset_y);
	}

	if ( optional_string("Ammo X-offset:") ) {
		stuff_int(&ammo_x);
	}

	if ( optional_string("Link X-offset:") ) {
		stuff_int(&link_x);
	}

	if ( optional_string("Name X-offset:") ) {
		stuff_int(&name_x);
	}

	if ( optional_string("Unlink X-offset:") ) {
		stuff_int(&unlink_x);
	}

	if ( optional_string("Reload X-offset:") ) {
		stuff_int(&reload_x);
	}

	hud_gauge->initBitmaps(fname_first, fname_entry, fname_last);
	hud_gauge->initHeaderOffsets(header_offsets[0], header_offsets[1]);
	hud_gauge->initHeaderText(header_text);
	hud_gauge->initBgFirstHeight(first_bg_h);
	hud_gauge->initBgFirstOffsetX(first_bg_offset_x);
	hud_gauge->initBgEntryHeight(entry_bg_h);
	hud_gauge->initBgEntryOffsetX(entry_bg_offset_x);
	hud_gauge->initBgLastOffsetX(last_bg_offset_x);
	hud_gauge->initBgLastOffsetY(last_bg_offset_y);
	hud_gauge->initEntryHeight(entry_h);
	hud_gauge->initEntryStartY(entry_start_offset_y);
	hud_gauge->initSecondaryAmmoOffsetX(ammo_x);
	hud_gauge->initSecondaryLinkedOffsetX(link_x);
	hud_gauge->initSecondaryNameOffsetX(name_x);
	hud_gauge->initSecondaryReloadOffsetX(reload_x);
	hud_gauge->initSecondaryUnlinkedOffsetX(unlink_x);
	hud_gauge->initLinkIcon();

	gauge_assign_common(settings, std::move(hud_gauge));
}

void load_gauge_scripting(gauge_settings* settings) {
	auto hud_gauge = gauge_load_common<HudGaugeScripting>(settings);

	required_string("Name:");
	SCP_string name;
	stuff_string(name, F_NAME);

	hud_gauge->initName(name);

	gauge_assign_common(settings, std::move(hud_gauge));
}
