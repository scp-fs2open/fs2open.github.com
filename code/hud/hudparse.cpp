/*
 * Created by WMCoolmon for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */

/*
 * $Logfile: /Freespace2/code/hud/hudparse.cpp $
 * $Revision: 2.28 $
 * $Date: 2005-04-24 08:27:17 $
 * $Author: Goober5000 $
 *
 * Contains code to parse hud gauge locations
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.27  2005/04/08 20:03:59  wmcoolmon
 * *crosses fingers*
 *
 * Revision 2.26  2005/03/06 11:23:45  wmcoolmon
 * RE-fixed stuff. Ogg support. Briefings.
 *
 * Revision 2.25  2005/02/27 23:27:50  wmcoolmon
 * Some HUD work
 *
 * Revision 2.24  2005/02/14 23:59:23  taylor
 * make hudparse GCC 3.4 friendly (WMCoolmon way want to check this with tbl)
 * fix OSX compile problem
 * debug message in weapons_page_in() should have been real debug message
 *
 * Revision 2.23  2005/02/04 23:29:32  taylor
 * merge with Linux/OSX tree - p0204-3
 *
 * Revision 2.22  2005/01/12 00:18:00  phreak
 * nude hud?  was this some kind of freudian slip of a sort or what?
 *
 * Revision 2.21  2005/01/01 19:37:05  wmcoolmon
 * Erg, always make sure CVS is completely up-to-date.
 *
 * Revision 2.20  2005/01/01 07:18:47  wmcoolmon
 * NEW_HUD stuff, turned off this time. :) It's in a state of disrepair at the moment, doesn't show anything.
 *
 * Revision 2.19  2004/12/25 09:27:41  wmcoolmon
 * Fix to modular tables workaround with Fs2NetD + Sync to current NEW_HUD code
 *
 * Revision 2.18  2004/12/24 05:07:05  wmcoolmon
 * NEW_HUD compiles now. :)
 *
 * Revision 2.17  2004/12/23 23:26:35  wmcoolmon
 * Proposed HUD system stuffs - within NEW_HUD defines.
 *
 * Revision 2.16  2004/12/22 09:09:38  wmcoolmon
 * Quick fix to hudparse.cpp; If this causes any problems, roll it back and I'll take another look at it.
 *
 * Revision 2.15  2004/11/27 10:50:32  taylor
 * correct positions for default graphics with no table
 * compiler warning fix
 *
 * Revision 2.14  2004/09/05 19:23:24  Goober5000
 * fixed a few warnings
 * --Goober5000
 *
 * Revision 2.13  2004/07/26 20:47:32  Kazan
 * remove MCD complete
 *
 * Revision 2.12  2004/07/17 09:25:59  taylor
 * add CF_SORT_REVERSE to real sort routine, makes CF_SORT_TIME work again
 *
 * Revision 2.11  2004/07/12 16:32:49  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.10  2004/06/26 03:19:53  wmcoolmon
 * Displayed escorts now settable up to MAX_COMPLETE_ESCORT_LIST via "$Max Escort Ships:" in hud_gauges.tbl
 * Escort list is now hud_gauges.tbl compatible.
 *
 * Revision 2.9  2004/06/18 04:59:54  wmcoolmon
 * Only used weapons paged in instead of all, fixed music box in FRED, sound quality settable with SoundSampleRate and SoundSampleBits registry values
 *
 * Revision 2.8  2004/06/08 01:12:58  wmcoolmon
 * Fix0red the current target's hud info appearing in the corner of the screen.
 *
 * Revision 2.7  2004/06/08 00:35:43  wmcoolmon
 * Added +Color: option for custom gauges.
 *
 * Revision 2.6  2004/06/05 23:31:12  wmcoolmon
 * More HUD update stuff. Coordinates are now floats, addparent is now obsolete.
 *
 * Revision 2.5  2004/06/01 07:31:56  wmcoolmon
 * Lotsa stuff. Custom gauges w/ ANIs support added, SEXPs to set gauge text, gauge image frames, and gauge coords. These SEXPs and toggle-hud reside in the Hud/change category.
 *
 * Revision 2.4  2004/05/31 08:32:25  wmcoolmon
 * Custom HUD support, better loading, etc etc.
 *
 * Revision 2.3  2004/05/30 08:04:49  wmcoolmon
 * Final draft of the HUD parsing system structure. May change how individual coord positions are specified in the TBL. -C
 *
 * Revision 2.2  2004/05/29 04:09:04  wmcoolmon
 * First bugfix in this!: Don't want Freespace exiting if it can't load a hud_gauge file.
 *
 * Revision 2.1  2004/05/29 03:02:53  wmcoolmon
 * Added HUD gauges placement table, "hud_gauges.tbl" or "*-hdg.tbm" table module
 *
 */ 

#include <cstddef>

#include "parse/parselo.h"
#include "graphics/2d.h"
#include "localization/localize.h"
#include "hud/hud.h"
#include "hud/hudescort.h"
//#include "weapon/emp.h"
#include "hud/hudparse.h" //Duh.
#include "ship/ship.h" //for ship struct
#include "mission/missionparse.h" //for MAX_SPECIES_NAMES
#include "graphics/font.h" //for gr_force_fit_string


//Global stuffs
#ifdef NEW_HUD
hud* current_hud = NULL;
//Storage for the default and ship huds
hud default_hud;
hud ship_huds[MAX_SHIP_TYPES];
hud species_huds[MAX_SPECIES_NAMES];
#else
hud_info* current_hud = NULL; //If not set, it's NULL. This should always be null outside of a mission.
hud_info default_hud;
hud_info ship_huds[MAX_SHIP_TYPES];
#endif
extern int ships_inited; //Need this

#ifndef NEW_HUD
//Set coord_x or coord_y to -1 to not change that value
//void resize_coords(int* values, float* factors);
//void set_coords_if_clear(int* dest_coords, int coord_x, int coord_y = -1);

//ADD YOUR VARIABLES HERE
//Gauges MUST come first, and all variables MUST be in the hud struct.

//Use this when setting gauge variables. It gets the OFFSET of the value in the hud_info struct
#define HUD_VAR(a) offsetof(hud_info, a)

gauge_info gauges[MAX_HUD_GAUGE_TYPES] = {
	{ NULL,			HUD_VAR(Player_shield_coords),	"$Player Shield:",			396, 379, 634, 670,	0, 0, 0, 0, 0, -1, -1 },
	{ NULL,			HUD_VAR(Target_shield_coords),	"$Target Shield:",			142, 379, 292, 670,	0, 0, 0, 0, 0, -1, -1 },
	{ NULL,			HUD_VAR(Shield_mini_coords),	"$Shield Mini:",			305, 291, 497, 470, 0, HUD_VAR(Shield_mini_fname), 0, 0, 0, -1, -1 },
	{ NULL,			HUD_VAR(Aburn_coords),			"$Afterburner Energy:",		171, 265, 274, 424, HUD_VAR(Aburn_size) ,HUD_VAR(Aburn_fname), 0, 0, 0, -1, -1 },
	{ NULL,			HUD_VAR(Wenergy_coords),		"$Weapons Energy:",			416, 265, 666, 424, HUD_VAR(Wenergy_size) ,HUD_VAR(Wenergy_fname), 0, 0, 0, -1, -1 },
	{ NULL,			HUD_VAR(Escort_coords),			"$Escort List:",			486, 206, 865, 330, 0, HUD_VAR(Escort_filename[0]), 0, HUD_VAR(Escort_htext), 0, -1, -1 },

	//Mini-gauges
	{ &gauges[2],	HUD_VAR(Hud_mini_3digit),		"$Text Base:",				310, 298, 502, 477,	0, 0, 0, 0, 0, -1, -1 },
	{ &gauges[2],	HUD_VAR(Hud_mini_1digit),		"$Text 1 digit:",			316, 298, 511, 477,	0, 0, 0, 0, 0, -1, -1 },
//	{ &gauges[2],	HUD_VAR(Hud_mini_2digit),		"$Text 2 digit:",			213, 298, 346, 477,	0, 0, 0, 0, 0, -1, -1 },
	{ &gauges[2],	HUD_VAR(Hud_mini_2digit),		"$Text 2 digit:",			313, 298, 506, 477,	0, 0, 0, 0, 0, -1, -1 },
	{ &gauges[5],	HUD_VAR(Escort_htext_coords),	"$Header Text:",			489, 208, 869, 331,			0, 0, 0, 0, 0, -1, -1 },
	{ &gauges[5],	HUD_VAR(Escort_list),			"$List:",					0, 12, 0, 13,		0, 0, 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[5],	HUD_VAR(Escort_entry),			"$Ship:",					0, 11, 0, 11,		0, HUD_VAR(Escort_filename[1]), 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[5],	HUD_VAR(Escort_entry_last),		"$Last Ship:",				0, 11, 0, 11,		0, HUD_VAR(Escort_filename[2]), 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[5],	HUD_VAR(Escort_name),			"$Ship Name:",				3, 0, 4, 0,			0, 0, 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[5],	HUD_VAR(Escort_integrity),		"$Ship Hull:",				128, 0, 116, 0,		0, 0, 0, 0, 0, HG_NOADD, -1 },
	{ &gauges[5],	HUD_VAR(Escort_status),			"$Ship Status:",			-12, 0, -11, 0,		0, 0, 0, 0, 0, HG_NOADD, -1 }
};

//Number of gauges
int Num_gauge_types = 16;
int Num_custom_gauges = 0;
#endif
#ifdef NEW_HUD
/*
int hud_escort_list(gauge_data* cg, ship* gauge_owner)
{
	if ( !Show_escort_view ) {
		return -1;
	}

	if ( !Num_escort_ships ) {
		return -1;
	}

	return 1;
}
int hud_escort_ship(gauge_data* cg, ship* gauge_owner)
{
	int curr_item = cg->get_update_num();
	if(curr_item < Num_escort_ships)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int hud_escort_ship_name()
{
	return 1;
}*/
/*
int hud_player_shield(gauge_data* cg, ship* gauge_owner)
{
	//Don't show if object isn't a ship
	if(Objects[gauge_owner->objnum].type != OBJ_SHIP)
	{
		return HG_RETURNNODRAW;
	}

	//Or current ship has primitive sensors
	if(gauge_owner->flags & SF2_PRIMITIVE_SENSORS)
	{
		return HG_RETURNNODRAW;
	}

	object* targetp = Objects[Ai_info[gauge_owner->ai_index].target_objnum];
	float max_shield = get_max_shield_quad(targetp);
	if(targetp->flags & OF_NO_SHIELDS)
	{
		return HG_RETURNLASTUPDATE;
	}
}*/

int hud_weapon_energy_text(gauge_data* cg, ship* gauge_owner)
{
	static float old_percent_left;
	static ship* old_owner;
	float percent_left;

	percent_left = gauge_owner->weapon_energy/Ship_info[gauge_owner->ship_info_index].max_weapon_reserve;

	if ( percent_left > 1 )
	{
		percent_left = 1.0f;
	}

	//If it's the same, don't bother updating info
	if(percent_left == old_percent_left && old_owner == gauge_owner)
	{
		//mprintf(("REDRAWING WEAPONS ENERGY LIKE A GOOD FUNCTION"));
		return 2;
	}

	old_percent_left = percent_left;
	old_owner = gauge_owner;

	if ( percent_left <= 0.3 )
	{
		if ( percent_left < 0.1 ) {
			gr_init_alphacolor(&cg->g_color, 200, 0, 0, 255);
		}
		sprintf(cg->text,XSTR( "%d%%", 326), fl2i(percent_left*100+0.5f));
		hud_num_make_mono(cg->text);
	}
	else
	{
		return -1;
	}

	return 1;
}

int hud_weapon_energy(gauge_data* cg, ship* gauge_owner)
{
	static float old_percent_left;
	static ship* old_owner;
	float percent_left;

	if ( Player_ship->weapons.num_primary_banks <= 0 )
	{
		return HG_RETURNNODRAW;
	}

	// also leave if no energy can be stored for weapons - Goober5000
	if (!ship_has_energy_weapons(gauge_owner))
		return HG_RETURNNODRAW;

	percent_left = gauge_owner->weapon_energy/Ship_info[gauge_owner->ship_info_index].max_weapon_reserve;
	if ( percent_left > 1 )
	{
		percent_left = 1.0f;
	}

	//If it's the same, don't bother updating info
	if(percent_left == old_percent_left && old_owner == gauge_owner)
	{
		//mprintf(("REDRAWING WEAPONS ENERGY LIKE A GOOD FUNCTION"));
		return HG_RETURNREDRAW;
	}

	old_percent_left = percent_left;
	old_owner = gauge_owner;
	
	//wtf is this??
/*	if ( percent_left <= 0.3 ) {
		char buf[32];
		if ( percent_left < 0.1 ) {
			gr_set_color_fast(&Color_bright_red);
		}
		sprintf(buf,XSTR( "%d%%", 326), fl2i(percent_left*100+0.5f));
		hud_num_make_mono(buf);
		gr_string(Weapon_energy_text_coords[gr_screen.res][0], Weapon_energy_text_coords[gr_screen.res][1], buf);
	}*/

	int clip_h = fl2i( (1.0f - percent_left) * cg->image_size[0] + 0.5f );

	//Background
	cg->frame[0] = 2;
	cg->frame_attrib[0][2] = cg->image_size[0] - clip_h;

	//The gauge
	cg->frame[1] = 3;
	cg->frame_attrib[1][0] = clip_h;

	return HG_RETURNLASTUPDATE;
}

int hud_afterburner_energy(gauge_data* cg, ship* gauge_owner)
{
	Assert(gauge_owner);
	static float old_percent_left = 0.0f;
	static ship* old_owner = NULL;

	//No afterburner? Don't draw it
	if ( !(Ship_info[gauge_owner->ship_info_index].flags & SIF_AFTERBURNER) )
	{
		//Don't draw the gauge at all
		return HG_RETURNNODRAW;
	}

	float percent_left;
	percent_left = gauge_owner->afterburner_fuel/Ship_info[gauge_owner->ship_info_index].afterburner_fuel_capacity;

	//Can't have over 100% displayed in gauge
	if ( percent_left > 1 )
	{
		percent_left = 1.0f;
	}

	//If it's the same, don't bother updating info
	if(percent_left == old_percent_left && old_owner == gauge_owner)
	{
		//mprintf(("REDRAWING AFTERBURNER LIKE A GOOD FUNCTION"));
		//Redraw the gauge with the current info
		return HG_RETURNREDRAW;
	}

	old_percent_left = percent_left;
	old_owner = gauge_owner;

	//How much do we want to clip off
	int clip_h = fl2i( (1.0f - percent_left) * cg->image_size[0] + 0.5f );

	//Set the clipping for the background
	cg->frame[0] = 0;
	cg->frame_attrib[0][2] = cg->image_size[0] - clip_h;	//From the bottom

	//Set the clipping for the actual energy stuff
	cg->frame[1] = 1;
	cg->frame_attrib[1][0] = clip_h;	//From the top

	//Only draw once
	return HG_RETURNLASTUPDATE;
}
/*
int hud_shield_mini(gauge_data* cg, ship* gauge_owner)
{
	int hud_shield_icon(gauge_data* cg, ship* gauge_owner)
{
	float max_shield;
	object* objp = &Objects[gauge_info->target_objnum];

	max_shield = get_max_shield_quad(objp);

	for ( int i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {

		if ( objp->flags & OF_NO_SHIELDS ) {
			break;
		}

		if ( objp->shield_quadrant[Quadrant_xlate[i]] < 0.1f ) {
			continue;
		}
				
		range = HUD_color_alpha;
		hud_color_index = fl2i( (objp->shield_quadrant[Quadrant_xlate[i]] / max_shield) * range + 0.5);
		Assert(hud_color_index >= 0 && hud_color_index <= range);
	
		if ( hud_color_index < 0 ) {
			hud_color_index = 0;
		}
		if ( hud_color_index >= HUD_NUM_COLOR_LEVELS ) {
			hud_color_index = HUD_NUM_COLOR_LEVELS - 1;
		}

		if ( hud_gauge_maybe_flash(HUD_TARGET_MINI_ICON) == 1) {
			// hud_set_bright_color();
			hud_set_gauge_color(HUD_TARGET_MINI_ICON, HUD_C_BRIGHT);
		} else {
			// gr_set_color_fast(&HUD_color_defaults[hud_color_index]);
			hud_set_gauge_color(HUD_TARGET_MINI_ICON, hud_color_index);
		}

		frames[i] = Shield_mini_gauge.first_frame + i;		
	}

	return 1;
}*/
#endif

//Loads defaults for if a hud isn't specified in the table
#ifdef NEW_HUD
static void load_hud_defaults(hud* hud)
{
/*	gauge_data* cgp = NULL;

	//Shield mini
	gauge_data *cg = hud->add_gauge("Shield Mini", 497, 470);
	cg->set_image("targhit1");

	cg = hud->add_gauge("Afterburner Energy", 274, 424);
	cg->set_image("2_energy2");
	cg->update_func = hud_afterburner_energy;

	cgp = cg = hud->add_gauge("Weapons Energy", 666, 424);
	cg->set_image("2_energy2");
	cg->update_func = hud_weapon_energy;

	//The cgp assigns the parent
	Assert(cgp != NULL);
	Assert(cg != NULL);
	cg = hud->add_gauge("Weapons Energy Text", 42, 85, cgp);
	cg->update_func = hud_weapon_energy_text;*/

	//Escort stuff
/*	cgp = cg = hud->add_gauge("Escort List", 865, 330);
	cg->set_image("escort1");
	cg->update_func = hud_escort_ship;

	cg = hud->add_gauge("Escort Header", 4, 1, cgp);
	strcpy(cg->text, XSTR( "monitoring", 285));

	cgp = hud->add_gauge("Escort Ships", 12, 13, cgp);

	cgp = cg = hud->add_gauge("Escort Ship", 0, 11, cgp);
	cg->set_image("escort2");

	hud->add_gauge("Escort Ship Name", 4, 0, cgp);
	hud->add_gauge("Escort Ship Integrity", 116, 0, cgp);
	hud->add_gauge("Escort Ship Status", -11, 0, cgp);*/
}
#else
static void load_hud_defaults(hud_info *hud)
{
	//X values
	if(gr_screen.max_w == 640)
	{
		//Size defaults
		hud->Aburn_size[0] = hud->Wenergy_size[0] = 60;

		//Image defaults
		strcpy(hud->Aburn_fname, "energy2");
		strcpy(hud->Wenergy_fname, "energy2");

		/**************************************************/
		//DO NOT CHANGE
		hud->resolution[0] = 640;
		hud->resolution[1] = 480;
		/**************************************************/
	}
	else
	{
		//Size defaults
		hud->Aburn_size[0] = hud->Wenergy_size[0] = 96;

		//Image defaults
		strcpy(hud->Aburn_fname, "2_energy2");
		strcpy(hud->Wenergy_fname, "2_energy2");

		/**************************************************/
		//DO NOT CHANGE
		hud->resolution[0] = 1024;
		hud->resolution[1] = 768;
		/**************************************************/
	}

	//Neither
	strcpy(hud->Shield_mini_fname, "targhit1");
	strcpy(hud->Escort_filename[0], "escort1");
	strcpy(hud->Escort_filename[1], "escort2");
	strcpy(hud->Escort_filename[2], "escort3");
	strcpy(hud->Escort_htext, XSTR( "monitoring", 285));

	/**************************************************/
	//NONE OF THIS NEEDS TO BE MODIFIED TO SETUP VARS
	gauge_info* cg;
	for(int i = 0; i < Num_gauge_types; i++)
	{
		cg = &gauges[i];
		if(gr_screen.max_w == 640)
		{
			HUD_INT(hud, cg->coord_dest)[0] = cg->defaultx_640;
			HUD_INT(hud, cg->coord_dest)[1] = cg->defaulty_480;
		}
		else
		{
			HUD_INT(hud, cg->coord_dest)[0] = cg->defaultx_1024;
			HUD_INT(hud, cg->coord_dest)[1] = cg->defaulty_768;
		}
	}
}


static void calculate_gauges(hud_info* dest_hud)
{
	//Put any post-loading calculation code after the beep. *BEEP*

	/**************************************************/
	//DO NOT MODIFY this stuff, unless you're changing the loading system.
	//Calculate parent gauge info
	gauge_info* cg;
	for(int i = 0; i < Num_gauge_types; i++)
	{
		cg = &gauges[i];
		if(cg->parent && !(cg->placement_flags & HG_NOADD))
		{
			HUD_INT(dest_hud, cg->coord_dest)[0] = HUD_INT(dest_hud, cg->coord_dest)[0] + HUD_INT(dest_hud, cg->parent->coord_dest)[0];
			HUD_INT(dest_hud, cg->coord_dest)[1] = HUD_INT(dest_hud, cg->coord_dest)[1] + HUD_INT(dest_hud, cg->parent->coord_dest)[1];
		}
	}
}

/****************************************************************************************************/
/* You shouldn't have to modify anything past here to add gauges */
/****************************************************************************************************/
//This doesn't belong in parse_lo, it's not really that low.
static int size_temp[2];
static float percentage_temp[2];
int stuff_coords(hud_info* dest_hud, gauge_info* cg, bool required = false)
{
	//Speed up calculations
	static hud_info* factor_for_hud;
	static float resize_factor[2];
	float fl_buffer[2];
	bool size_defined = false;

	if(required)
	{
		required_string(cg->fieldname);
	}
	if(!required && !optional_string(cg->fieldname))
	{
		return 0;
	}

	//stuff_int_list(HUD_INT(dest_hud, i), 2, RAW_INTEGER_TYPE);
	stuff_float_list(fl_buffer, 2);
	if(!cg->parent)
	{
		factor_for_hud = NULL;
		resize_factor[0] = 1;
		resize_factor[1] = 1;
	}
	else if(dest_hud != factor_for_hud)
	{
		resize_factor[0] = (float)gr_screen.max_w / (float)dest_hud->resolution[0];
		resize_factor[1] = (float)gr_screen.max_h / (float)dest_hud->resolution[1];
	}
	//Resize to current res
	HUD_INT(dest_hud, cg->coord_dest)[0] = fl2i(fl_buffer[0] * resize_factor[0]);
	HUD_INT(dest_hud, cg->coord_dest)[1] = fl2i(fl_buffer[1] * resize_factor[1]);

	if(optional_string("+Size:"))
	{
		stuff_int_list(size_temp, 2, RAW_INTEGER_TYPE);
		if(cg->size_dest)
		{
			HUD_INT(dest_hud, cg->size_dest)[0] = size_temp[0];
			HUD_INT(dest_hud, cg->size_dest)[1] = size_temp[1];
		}
		
		//For %
		size_defined = false;
	}

	if(optional_string("+Percentage:"))
	{
		stuff_float_list(percentage_temp, 2);
		percentage_temp[0] *= (gr_screen.max_w / 100.0f);
		percentage_temp[1] *= (gr_screen.max_h / 100.0f);

		//Bool true, size defined
		if(!size_defined)
		{
			if(percentage_temp[0])
			{
				percentage_temp[0] -= size_temp[0] / 2;
			}
			if(percentage_temp[1])
			{
				percentage_temp[1] -= size_temp[1] / 2;
			}
		}
		HUD_INT(dest_hud, cg->coord_dest)[0] += fl2i(percentage_temp[0]);
		HUD_INT(dest_hud, cg->coord_dest)[1] += fl2i(percentage_temp[1]);
	}

	char buffer[32];
	if(optional_string("+Image:"))
	{
		if(cg->image_dest)
		{
			stuff_string(HUD_CHAR(dest_hud, cg->image_dest), F_NAME, NULL);
		}
		else
		{
			stuff_string(buffer, F_NAME, NULL);
		}
	}
	if(optional_string("+Text:"))
	{
		if(cg->text_dest)
		{
			stuff_string(HUD_CHAR(dest_hud, cg->text_dest),  F_NAME, NULL);
		}
		else
		{
			stuff_string(buffer, F_NAME, NULL);
		}
	}
	if(optional_string("+Color:"))
	{
		if(cg->color_dest)
		{
			stuff_byte(&HUD_COLOR(dest_hud, cg->color_dest)->red);
			stuff_byte(&HUD_COLOR(dest_hud, cg->color_dest)->green);
			stuff_byte(&HUD_COLOR(dest_hud, cg->color_dest)->blue);
		}
		else
		{
			ubyte junk_byte;
			stuff_byte(&junk_byte);
			stuff_byte(&junk_byte);
			stuff_byte(&junk_byte);
		}
	}
	return 1;
}


static void parse_resolution(hud_info* dest_hud)
{
	//Parse it
	gauge_info* cg;
	for(int i = 0; i < Num_gauge_types; i++)
	{
		cg = &gauges[i];
		if(!cg->parent && cg->fieldname)
		{

			stuff_coords(dest_hud, cg);
		}
	}
}

static void parse_resolution_gauges(hud_info* dest_hud)
{
	char gaugename[32];
	gauge_info *cg, *parent;
	while(!required_string_3("$Gauge:","$Resolution:","#End"))
	{
		required_string("$Gauge:");
		stuff_string(gaugename, F_NAME, NULL);

		parent = NULL;

		for(int i = 0; i < Num_gauge_types; i++)
		{
			cg = &gauges[i];
			if(!parent)
			{
				if(!strnicmp(cg->fieldname + sizeof(char), gaugename, strlen(cg->fieldname) - 2))
				{
					parent = cg;
				}
			}
			else if(parent == cg->parent)
			{
				stuff_coords(dest_hud, cg);
			}
		}
	}

	dest_hud->loaded = true;
}


/*
void set_coords_if_clear(int* dest_coords, int coord_x, int coord_y)
{
	if(dest_coords[0] == -1 && coord_x != -1)
	{
		dest_coords[0] = coord_x;
	}
	if(dest_coords[1] == -1 && coord_y != -1)
	{
		dest_coords[1] = coord_y;
	}
}


void resize_coords(int* values, float* factors)
{
	values[0] = fl2i(values[0] * factors[0]);
	values[1] = fl2i(values[1] * factors[1]);
}*/

hud_info* parse_ship_start()
{
	hud_info* dest_hud = NULL;

	required_string("$Ship:");
	char shipname[NAME_LENGTH];
	int ship_index;
	stuff_string(shipname, F_NAME, NULL);
	ship_index = ship_info_lookup(shipname);

	if(ship_index == -1)
	{
		return NULL;
	}

	dest_hud = &ship_huds[ship_index];
	return dest_hud;
}

hud_info* parse_resolution_start(hud_info* dest_hud, int str_token)
{
	int buffer[2];

	if(str_token == 1)
	{
		required_string("$Default:");
		if(!dest_hud->loaded)
		{
			stuff_int_list(dest_hud->resolution, 2, RAW_INTEGER_TYPE);
			if(dest_hud->resolution[0] == 0 || dest_hud->resolution == 0)
			{
				dest_hud->resolution[0] = gr_screen.max_w;
				dest_hud->resolution[1] = gr_screen.max_h;
			}
			return dest_hud;
		}
	}
	else
	{
		required_string("$Resolution:");
		stuff_int_list(buffer, 2, RAW_INTEGER_TYPE);

		if(buffer[0] == gr_screen.max_w && buffer[1] == gr_screen.max_h)
		{
			//Get the ship HUD ready w/ defaults
			if(default_hud.loaded)
			{
				memcpy(dest_hud, &default_hud, sizeof(hud_info));
				//It's not really loaded
				dest_hud->loaded = false;
			}
			else
			{
				//Set the resolution
				memcpy(dest_hud->resolution, buffer, sizeof(buffer));
			}

			return dest_hud;
		}
	}

	return 0;
}


void parse_custom_gauge()
{
	if(Num_gauge_types < MAX_HUD_GAUGE_TYPES)
	{
		char buffer[32];

		gauge_info* cg = &gauges[Num_gauge_types];
		memset(cg, 0, sizeof(gauge_info));
		//Set all the ptrs (modified to work with GCC 3.4 - taylor)
		cg->coord_dest = HUD_VAR(custom_gauge_coords[0]) + (Num_custom_gauges * sizeof(int));
		cg->size_dest = HUD_VAR(custom_gauge_sizes[0]) + (Num_custom_gauges * sizeof(int));
		cg->image_dest = HUD_VAR(custom_gauge_images[0]) + (Num_custom_gauges * sizeof(int));
		cg->frame_dest = HUD_VAR(custom_gauge_frames[0]) + (Num_custom_gauges * sizeof(int));
		cg->text_dest = HUD_VAR(custom_gauge_text[0]) + (Num_custom_gauges * sizeof(int));
		cg->color_dest = HUD_VAR(custom_gauge_colors[0]) + (Num_custom_gauges * sizeof(int));

		required_string("$Name:");
		//Gotta make this a token
		cg->fieldname[0] = '$';
		stuff_string(cg->fieldname + 1, F_NAME, NULL);
		strcat(cg->fieldname, ":");

		if(optional_string("+Default640X:"))
		{
			stuff_int(&cg->defaultx_640);
		}
		if(optional_string("+Default640Y:"))
		{
			stuff_int(&cg->defaulty_480);
		}
		if(optional_string("+Default1024X:"))
		{
			stuff_int(&cg->defaultx_1024);
		}
		if(optional_string("+Default1024Y:"))
		{
			stuff_int(&cg->defaulty_768);
		}
		if(optional_string("+Parent:"))
		{
			stuff_string(buffer, F_NAME, NULL);
			cg->parent = hud_get_gauge(buffer);
		}

		Num_gauge_types++;
		Num_custom_gauges++;
	}
	else
	{
		skip_to_start_of_strings("$Name:", "#End");
	}
}

int parse_hud_gauges_tbl(char* longname)
{
	int rval;
	lcl_ext_open();
	if ((rval = setjmp(parse_abort)) != 0)
	{
		nprintf(("Warning", "Unable to parse %s!  Code = %i.\n", longname, rval));
		lcl_ext_close();
		return 0;
	}
	else
	{	
		read_file_text(longname);
		reset_parse();
	}

	read_file_text(longname);

	if(optional_string("$Max Escort Ships:"))
	{
		stuff_int(&Max_escort_ships);
	}
	
	if(optional_string("#Custom Gauges"))
	{
		while(required_string_either("#End", "$Name:"))
		{
			parse_custom_gauge();
		}
		required_string("#End");
	}

	//It's parsing time everehbody!
	hud_info* dest_hud = &default_hud;

	if(optional_string("#Main Gauges"))
	{
		while(rval = required_string_3("#End", "$Default:", "$Resolution:"), rval)
		{
			if(parse_resolution_start(dest_hud, rval))
			{
				parse_resolution(dest_hud);
			}
			else
			{
				skip_to_start_of_strings("$Resolution:", "$Default:", "#End");
			}
		}
		required_string("#End");
	}
	
	if(optional_string("#Gauges"))
	{
		while(rval = required_string_3("#End", "$Default:", "$Resolution:"), rval)
		{
			if(parse_resolution_start(dest_hud, rval))
			{
				parse_resolution_gauges(dest_hud);
			}
			else
			{
				skip_to_start_of_strings("$Resolution:", "$Default:", "#End");
			}
		}
		required_string("#End");
	}

	calculate_gauges(&default_hud);

	if(ships_inited)
	{
		//Parse main ship gauges
		if(optional_string("#Ship Main Gauges"))
		{
			while (required_string_either("#End","$Ship:"))
			{
				if(dest_hud = parse_ship_start(), dest_hud)
				{
					while(rval = required_string_3("#End", "$Default:", "$Resolution:"), rval)
					{
						if(parse_resolution_start(dest_hud, rval))
						{
							parse_resolution(dest_hud);
						}
						else
						{
							skip_to_start_of_strings("$Resolution:", "$Default:", "$Ship:");
						}
					}
				}
			}
			required_string("#End");
		}

		//Parse individual extra ship gauge info
		if(optional_string("#Ship Gauges"))
		{
			while (required_string_either("#End","$Ship:"))
			{
				if(dest_hud = parse_ship_start(), dest_hud)
				{
					while(rval = required_string_3("#End", "$Default:", "$Resolution:"), rval)
					{
						if(parse_resolution_start(dest_hud, rval))
						{
							parse_resolution_gauges(dest_hud);
						}
						else
						{
							skip_to_start_of_strings("$Resolution:", "$Default:", "$Ship:");
						}
					}
				}
			}
			required_string("#End");
		}

		for(int i = 0; i < MAX_SHIP_TYPES; i++)
		{
			if(ship_huds[i].loaded)
			{
				calculate_gauges(&ship_huds[i]);
			}
		}
	}
	lcl_ext_close();

	return 1;
}
#endif

extern int Ships_inited;
void hud_positions_init()
{
	if(!ships_inited)
	{
		Error(LOCATION, "Could not initialize hudparse.cpp as ships were not inited first.");
		return;
	}

	load_hud_defaults(&default_hud);
#ifndef NEW_HUD

	if(!parse_hud_gauges_tbl("hud_gauges.tbl"))
	{
		calculate_gauges(&default_hud);
	}

	char tbl_file_arr[MAX_TBL_PARTS][MAX_FILENAME_LEN];
	char *tbl_file_names[MAX_TBL_PARTS];

	int num_files = cf_get_file_list_preallocated(MAX_TBL_PARTS, tbl_file_arr, tbl_file_names, CF_TYPE_TABLES, "*-hdg.tbm", CF_SORT_REVERSE);
	for(int i = 0; i < num_files; i++)
	{
		//HACK HACK HACK
		modular_tables_loaded = true;
		strcat(tbl_file_names[i], ".tbm");
		parse_hud_gauges_tbl(tbl_file_names[i]);
	}

	set_current_hud(-1);
#endif
}

//Depending on the ship number specified, this copies either the default hud or a ship hud
//to a temporary hud_info struct and sets the current_hud pointer to it.
//This enables ship-specific huds.
//Note that this creates a temporary current hud, so changes aren't permanently saved.
#ifdef NEW_HUD
void set_current_hud(ship* owner)
#else
void set_current_hud(int player_ship_num)
#endif
{
#ifdef NEW_HUD
	if(!owner->ship_hud.is_loaded())
	{
		if(ship_huds[owner->ship_info_index].is_loaded() == true)
		{
			ship_huds[owner->ship_info_index].copy(&owner->ship_hud);
		}
		else if(species_huds[Ship_info[owner->ship_info_index].species].is_loaded() == true)
		{
			species_huds[Ship_info[owner->ship_info_index].species].copy(&owner->ship_hud);
		}
		else
		{
			default_hud.copy(&owner->ship_hud);
		}
		mprintf(("NEW_HUD: Hud created for ship %s", owner->ship_name));
	}
	current_hud = &owner->ship_hud;
	current_hud->owner = owner;
#else
	static hud_info real_current_hud;

	if(player_ship_num >= 0 && player_ship_num < MAX_SHIP_TYPES && ship_huds[player_ship_num].loaded)
	{
		memcpy(&real_current_hud, &ship_huds[player_ship_num], sizeof(hud_info));
	}
	else
	{
		memcpy(&real_current_hud, &default_hud, sizeof(hud_info));
	}

	current_hud = &real_current_hud;
#endif
}

#ifndef NEW_HUD

/* - not POD so GCC won't take it for offsetof - taylor
hud_info::hud_info()
{
	memset(this, 0, sizeof(hud_info));
}
*/

#else

/*
//Note: user range should be 50 to 150
ubyte hud_info::get_preset_alpha(int preset_id)
{
	if(!flags & HI_CONTRAST)
	{
		switch(preset_id)
		{
			case HI_DIM:
				return hud_color.alpha - 40;
			case HI_NORMAL:
				return hud_color.alpha;
			case HI_BRIGHT:
				return hud_color.alpha + 100;
		}
	}
	else
	{
		switch(preset_id)
		{
			case HI_DIM:
				return hud_color.alpha +10;
			case HI_NORMAL:
				return hud_color.alpha + 70;
			case HI_BRIGHT:
				return 255;
		}
	}
}*/
hud::hud()
{
	resolution[0] = resolution[1] = 0;
	num_gauges = 0;
	first_gauge = NULL;
	loaded = false;
}
hud::~hud()
{
	//Delete all gauges
	gauge_data *cg = first_gauge;
	while(cg != NULL)
	{
		first_gauge = cg->next;
		delete cg;
		cg = first_gauge;
	}
}

void hud::show()
{
	gauge_data *cg = first_gauge;
	while(cg != NULL)
	{
		if(cg->type == HG_MAINGAUGE)
		{
			cg->show(owner);
		}

		cg = cg->next;
	}
}

//Parents are responsible for setting child prev and next variables
//Remember dat.
int hud::copy(hud* dest_hud)
{
	memcpy(dest_hud->resolution, resolution, sizeof(resolution));
	dest_hud->num_gauges = num_gauges;
	memcpy(&dest_hud->hud_color, &hud_color, sizeof(color));

	gauge_data *cg = first_gauge;
	gauge_data *pg = NULL;
	gauge_data **dcg = &dest_hud->first_gauge;
	while(cg != NULL)
	{
		(*dcg) = new gauge_data;

		Assert((*dcg) != NULL);

		cg->copy((*dcg));
		(*dcg)->prev = pg;
		pg = (*dcg);
		dcg = &((*dcg)->next);
		cg = cg->next;
	}

	dest_hud->loaded = true;

	return 1;
}

//Gets a gauge by its name
//Names are unique and case-insensitive
//Returns NULL on failure
gauge_data *hud::get_gauge(char* name)
{
	gauge_data *cg = first_gauge;
	gauge_data *rv = NULL;

	while(cg != NULL)
	{
		if(!stricmp(cg->name, name))
		{
			return cg;
		}
		if(cg->first_child)
		{
			rv = cg->get_gauge(name);
			
			if(rv != NULL)
			{
				//You bastard...how dare you steal a Winnebago?
				return rv;
			}
		}

		cg = cg->next;
	}

	return NULL;
}

gauge_data *gauge_data::get_gauge(char *name)
{
	gauge_data *cg = first_child;
	while(cg != NULL)
	{
		if(!stricmp(cg->name, name))
		{
			return cg;
		}

		cg = cg->next;
	}

	return NULL;
}

//Adds a gauge to the end of the list. Will add priority crap later.
//Priority of -1 means place at the very front
//Priority of -2 means place at the very end
gauge_data *hud::add_gauge(char* name, int def_x_coord, int def_y_coord, gauge_data* new_gauge_parent, int priority)
{
	//If no gauges exist, add them.
	gauge_data *new_gauge = new gauge_data;
	if(new_gauge == NULL)
	{
		Warning(LOCATION, "Could not allocate memory for gauge \"%s\"", name);
		return NULL;
	}
	if(get_gauge(name) != NULL)
	{
		Warning(LOCATION, "Gauge with name \"%s\" already exists; new gauge was not added. Note that gauge names ARE case-sensitive.", name);
		return NULL;
	}

	//Make the fgp variable a double pointer,
	//so we can set the first-gauge as well if necessary.
	gauge_data **fgp;
	if(new_gauge_parent == NULL)
	{
		fgp = &first_gauge;
	}
	else
	{
		fgp = &new_gauge_parent->first_child;
	}

	if((*fgp) != NULL && priority != -1)
	{
		gauge_data *cg = (*fgp);
		while(cg != NULL)
		{
			if(cg->next != NULL)
			{
				if(cg->next->priority > priority)
				{
					break;
				}
				else
				{
					cg = cg->next;
				}
			}
			else
			{
				break;
			}
		}
		//We've found an empty spot, put it there.
		new_gauge->next = cg->next;
		new_gauge->prev = cg;
		cg->next = new_gauge;
	}
	else
	{
		if((*fgp) != NULL)
		{
			//Replace existing first gauge
			new_gauge->next = (*fgp);
			(*fgp)->prev = new_gauge;
			(*fgp) = new_gauge;
		}
		else
		{
			//Only one gauge, the new one
			(*fgp) = new_gauge;
		}
	}

	

	new_gauge->set_up(name, def_x_coord, def_y_coord, new_gauge_parent, priority);

	return new_gauge;
}

#endif

gauge_info* hud_get_gauge(char* name)
{
#ifndef NEW_HUD
	int id = hud_get_gauge_index(name);
	if(id > -1)
	{
		return &gauges[id];
	}
#endif

	return NULL;
}

int hud_get_gauge_index(char* name)
{
#ifndef NEW_HUD
	for(int i = 0; i < Num_gauge_types; i++)
	{
		if(!strnicmp(gauges[i].fieldname + sizeof(char), name, strlen(gauges[i].fieldname) - 2))
		{
			return i;
		}
	}
#endif

	return -1;
}


#ifdef NEW_HUD
int gauge_data::set_up(char* gauge_name, int def_x, int def_y, gauge_data* gauge_parent, int priority)
{
	//Don't set up the same gauge twice
	if(type != HG_UNUSED)
	{
		return 0;
	}

	//If a parent was given, set it
	if(gauge_parent)
	{
		type = HG_CHILDGAUGE;
	}
	else
	{
		type = HG_MAINGAUGE;
	}

	Assert(strlen(gauge_name) < sizeof(name));

	strcpy(name, gauge_name);
	coords[0] = def_x;
	coords[1] = def_y;
	priority = 100;

	return 1;
}

int gauge_data::set_image(char* image_name, int newframe, int sizex, int sizey)
{
	//Make sure we aren't trying to reset an image
	if(!stricmp(image_name, image))
	{
		return 1;
	}

	//Get the NEW model id
	int new_image_id = bm_load_animation(image_name, &num_frames);
	if(new_image_id == -1)
	{
		new_image_id = bm_load(image_name);
		//Make sure the new one is valid
		if(new_image_id == -1)
		{
			//Warning(("Warning","Could not load image %s for a HUD gauge", image_name));
			return 0;
		}
		num_frames = 1;
	}

	strcpy(image, image_name);

	//Give up the old one
	if(image_id != -1)
	{
		bm_unload(image_id);	//maybe unload?
	}

	//Set image_id
	image_id = new_image_id;
	Assert(newframe < num_frames);
	for(int i = 0; i < MAX_GAUGE_FRAMES; i++)
	{
		frame[i] = -1;
	}
	frame[0] = newframe;
	bm_get_info(image_id, &image_size[0], &image_size[1]);

	//Set size values
	if(sizex)
	{
		image_size[0] = sizex;
	}
	if(sizey)
	{
		image_size[1] = sizey;
	}

	return 1;
}

int gauge_data::reset()
{
	draw_coords[0] = draw_coords[1] = 0;
/*	if(image_id != -1)
	{
		bm_unload(image_id);
	}*/

	return 1;
}
int gauge_data::page_in()
{
	if(strlen(image))
	{
		if(image_id == -1)
		{
			image_id = bm_load_animation(image, &num_frames);
			if(image_id == -1)
			{
				image_id = bm_load(image);
				//Make sure the new one is valid
				if(image_id != -1)
				{
					num_frames = 1;
				}
				else
				{
					num_frames = 0;
				}
			}
		}
		bm_page_in_aabitmap(image_id, num_frames);
	}
	return 1;
}
gauge_data::gauge_data()
{
	int i;

	type = HG_UNUSED;
	priority = 100;
	coords[0] = coords[1] = 0;
	draw_coords[0] = draw_coords[1] = 0;

	//Color
	g_color.alpha = 0;
	user_color.alpha = 0;

	//Text
	text[0] = '\0';
	max_text_lines = 1;
	max_text_width = -1;

	//By default, show HUD gauges only if view is a cockpit
	show_flags = VM_OTHER_SHIP;

	//Shape
	for(i = 0; i < MAX_GAUGE_SHAPES; i++)
	{
		shape_id[i] = -1;
	}
	memset(shape_attrib, 0, sizeof(shape_attrib));

	//Image/animation
	image_id = -1;
	num_frames = 0;
	for(i = 0; i < MAX_GAUGE_FRAMES; i++)
	{
		frame[i] = -1;
	}
	memset(frame_attrib, 0, sizeof(frame_attrib));

	//Model
	model_id = -1;
	memset(model_attrib, 0, sizeof(model_attrib));

	prev = next = NULL;
	first_child = NULL;

	update_func = NULL;
}
gauge_data::~gauge_data()
{
	//deletes child gauges
	gauge_data *cg;
	cg = first_child;
	while(cg != NULL)
	{
		cg = cg->next;
		delete first_child;
		first_child = cg;
	}
}

int gauge_data::copy(gauge_data *dest_gauge)
{
	memcpy(dest_gauge, this, sizeof(gauge_data));

	//Just...no.
	dest_gauge->next = dest_gauge->prev = NULL;

	//Now clone the children. Fun with recursive functions.
	gauge_data *cg = first_child;				//Current Gauge
	gauge_data **cdgp = &dest_gauge->first_child;//Current Destination Gauge Pointer
	gauge_data *odg = NULL;						//Old Destination Gauge
	while(cg != NULL)
	{
		//Create the new child gauge (Also setting the next or first_child pointer)
		(*cdgp) = new gauge_data;

		//This is bad.
		Assert((*cdgp) != NULL);

		//Copy contents from the old child gauge to the new
		cg->copy((*cdgp));

		//Set prev for the current destination child-gauge to the last gauge added
		(*cdgp)->prev = odg;
		//Set old destination gauge var to the current gauge
		odg = (*cdgp);
		//Set cdgp to the next gauge's next pointer
		cdgp = &(*cdgp)->next;
		//Advance to the next source gauge
		cg = cg->next;
	}

	return 1;
}

void gauge_data::draw_text()
{
	if(max_text_width > 0 && max_text_lines <= 1)
	{
		char *short_str = new char[strlen(text) + 1];
		strcpy(short_str, text);
		gr_force_fit_string(short_str, sizeof(text), max_text_width);
		gr_string(draw_coords[0], draw_coords[1], short_str);
		delete[] short_str;
	}
	else
	{
		gr_string(draw_coords[0], draw_coords[1], text);
	}
}

void gauge_data::draw_image()
{
	color framecolor;
	for(int i = 0; i < MAX_GAUGE_FRAMES; i++)
	{
		if(frame_attrib[i][4])
		{
			gr_init_alphacolor(&framecolor, g_color.red, g_color.green, g_color.blue, frame_attrib[i][4]);
			gr_set_color_fast(&framecolor);
		}
		if(frame[i] != -1)
		{
			gr_set_bitmap(image_id + frame[i]);
			//clipping
			//0 top
			//1 left
			//2 bottom 
			//3 right
			gr_aabitmap_ex(draw_coords[0] + frame_attrib[i][1],						//X position
				draw_coords[1] + frame_attrib[i][0],						//Y position
				image_size[0] - frame_attrib[i][1] - frame_attrib[i][3],	//Width to show
				image_size[1] - frame_attrib[i][0] - frame_attrib[i][2],	//Height to show
				frame_attrib[i][1],										//Offset from left
				frame_attrib[i][0]);										//Offset from top
		}
	}
}

void gauge_data::draw_shape()
{
	//Notes on shape stuff
	//Where a bool is in the attributes, 0 is false, anything else is true
	//Values will likely not be checked for validity for speed
	for(int i = 0; i < MAX_GAUGE_SHAPES; i++)
	{
		switch(shape_id[i])
		{
			case HG_SHAPERECTANGLE:
				//0 - x offset
				//1 - y offset
				//2 - width
				//3 - height
				gr_rect(draw_coords[0] + shape_attrib[i][0],
						draw_coords[0] + shape_attrib[i][1],
						shape_attrib[i][2],
						shape_attrib[i][3]);
				break;
			case HG_SHAPECIRCLE:
				//0 - x offset
				//1 - y offset
				//2 - diameter
				gr_circle(draw_coords[0] + shape_attrib[i][0],
					draw_coords[0] + shape_attrib[i][1],
					shape_attrib[i][2]);
				break;
			case HG_SHAPELINE:
				//0 - 1st point x offset
				//1 - 1st point y offset
				//2 - 2nd point x offset
				//3 - 2nd point y offset
				//4 - Change line points so they aren't offscreen?
				gr_line(draw_coords[0] + shape_attrib[i][0],
					draw_coords[1] + shape_attrib[i][1],
					draw_coords[0] + shape_attrib[i][2],
					draw_coords[1] + shape_attrib[i][3],
					shape_attrib[i][4] == 0 ? false : true);
				break;
/*			case HG_SHAPEAALINE:
				//Draws anti-aliased line; slower than normal line
				//0 - 1st point x offset
				//1 - 1st point y offset
				//2 - 2nd point x offset
				//3 - 2nd point y offset
				//4 - Change line points so they aren't offscreen?
				gr_aaline(draw_coords[0] + shape_attrib[i][0],
					draw_coords[1] + shape_attrib[i][1],
					draw_coords[0] + shape_attrib[i][2],
					draw_coords[1] + shape_attrib[i][3],
					shape_attrib[i][4]);
				break;*/
			case HG_SHAPEGRADIENT:
				//Draws a line that slowly fades to 0 alpha
				//Auto-changes line points so they aren't offscreen
				//0 - 1st point x offset
				//1 - 1st point y offset
				//2 - 2nd point x offset
				//3 - 2nd point y offset
				gr_gradient(draw_coords[0] + shape_attrib[i][0],
					draw_coords[1] + shape_attrib[i][1],
					draw_coords[0] + shape_attrib[i][2],
					draw_coords[1] + shape_attrib[i][3]);
				break;
			default:
				break;
		}
	}
}

void gauge_data::draw_model()
{
	//A lot of this stuff is shamelessly ripped from hud_render_target_ship

//	model_render(model_id, 
}

int gauge_data::draw()
{
	//Color
	if(g_color.alpha && (!user_color.alpha || data_flags & HG_PRIORITYCOLOR))
	{
		gr_set_color_fast(&g_color);
	}
	else if(user_color.alpha)
	{
		gr_set_color_fast(&user_color);
	}
	else
	{
		hud_set_default_color();
	}

	//Shape
	//No if clause needed here, shapes are individually checked
	draw_shape();

	//Image
	if(image_id != -1)
	{
		draw_image();
	}

	//Model
	if(model_id != -1)
	{
		draw_model();
	}

	//Text
	if(text[0] != '\0')
	{
		draw_text();
	}

	return 1;
}

int gauge_data::show(ship* gauge_owner, gauge_data* cgp)
{
	//Don't execute if we can't see the gauge
	if(!(Viewer_mode & show_flags) && Viewer_mode != 0)
	{
		return 1;
	}


	update_num = 0;
	int rval = 0;

	//Increment by parent coords, if it has a parent
	if(cgp)
	{
		draw_coords[0] = cgp->coords[0];
		draw_coords[1] = cgp->coords[1];
	}

	do
	{
		if(update_func != NULL)
		{
			rval = update_func(this, gauge_owner);
		}
		else
		{
			rval = 1;
		}
		//rval -1:Don't draw
		//rval 0: update again
		//rval 1: this is the last update
		//all else: just draw the gauge
		if(rval == -1)
		{
			return 0;
		}

		//If we aren't redrawing and starting the loop, set to 0
		if(update_num ==0 && (rval == 1 || rval == 0))
		{
			if(cgp == NULL)
			{
				draw_coords[0] = draw_coords[1] = 0;
			}
			else
			{
				draw_coords[0] = cgp->coords[0];
				draw_coords[1] = cgp->coords[1];
			}

		}

		//If we aren't redrawing, set new draw coordinates
		if(rval == 1 || rval == 0)
		{
			draw_coords[0] += coords[0];
			draw_coords[1] += coords[1];
		}

		if(first_child != NULL)
		{
			gauge_data *cg = first_child;
			while(cg != NULL)
			{
				Assert(cg != this);

				cg->show(gauge_owner, this);
				cg = cg->next;
			}
		}

		draw();

		update_num++;
	} while(rval == 0);

	if(data_flags & HG_NEEDSUPDATE)
	{
		data_flags &= HG_NEEDSUPDATE;
	}

	return 1;
}

int gauge_var::Evaluate(int* result)
{
	(*result) = 0;
	gauge_var* cv = this;
	int temp_result;

	while(cv != NULL)
	{
		switch(data_type)
		{
			case GV_INTPTR:
			case GV_INTVAR:
				temp_result = (*int_variable);
				break;
			case GV_CHARPTR:
				temp_result = atoi(*char_pointer);
				break;
			case GV_CHARVAR:
				temp_result = atoi(char_variable);
				break;
			case GV_FLOATPTR:
			case GV_FLOATVAR:
				temp_result = fl2i(*float_variable);
				break;
			case GV_GAUGEVAR:
			case GV_GAUGEVARPTR:
				gv_variable->Evaluate(&temp_result);
				break;
			default:
				Error(LOCATION, "Unknown data type handed to gauge_var::Evaluate(int* result)!");
				return HG_RETURNNODRAW;
		}

		switch(var_operator)
		{
			case GV_NONE:
			case GV_ADD:
				(*result) += temp_result;
				break;
			case GV_SUBTRACT:
				(*result) -= temp_result;
				break;
			case GV_MULTIPLY:
				(*result) *= temp_result;
				break;
			case GV_DIVIDE:
				(*result) /= temp_result;
				break;
			default:
				Error(LOCATION, "Unknown operator handed to gauge_var::Evaluate(int* result)!");
				return HG_RETURNNODRAW;
		}

		cv = cv->next;
	}

	if(prev_data_type == GV_INTVAR && result == prev_int_result)
	{
		return HG_RETURNREDRAW;
	}
	else
	{
		if(prev_data_type != GV_INTVAR)
		{
			prev_data_type = GV_INTVAR;
			DeallocPrevVars();
			prev_int_result = new int;
		}
		(*prev_int_result) = (*result);

		return HG_RETURNLASTUPDATE;
	}
}

int gauge_var::Evaluate(float* result)
{
	(*result) = 0;
	gauge_var* cv = this;
	float temp_result;

	while(cv != NULL)
	{
		switch(data_type)
		{
			case GV_INTPTR:
			case GV_INTVAR:
				temp_result = i2fl(*int_variable);
				break;
			case GV_CHARPTR:
				temp_result = atof(*char_pointer);
				break;
			case GV_CHARVAR:
				temp_result = atof(char_variable);
				break;
			case GV_FLOATPTR:
			case GV_FLOATVAR:
				temp_result = (*float_variable);
				break;
			default:
				Error(LOCATION, "Unknown data type handed to gauge_var::Evaluate(float* result)!");
				return HG_RETURNNODRAW;
		}

		switch(var_operator)
		{
			case GV_NONE:
			case GV_ADD:
				(*result) += temp_result;
				break;
			case GV_SUBTRACT:
				(*result) -= temp_result;
				break;
			case GV_MULTIPLY:
				(*result) *= temp_result;
				break;
			case GV_DIVIDE:
				(*result) /= temp_result;
				break;
			default:
				Error(LOCATION, "Unknown operator handed to gauge_var::Evaluate(float* result)!");
				return HG_RETURNNODRAW;
		}

		cv = cv->next;
	}

	if(prev_data_type == GV_FLOATVAR && result == prev_float_result)
	{
		return HG_RETURNREDRAW;
	}
	else
	{
		if(prev_data_type != GV_FLOATVAR)
		{
			prev_data_type = GV_FLOATVAR;
			DeallocPrevVars();
			prev_float_result = new float;
		}
		(*prev_float_result) = (*result);

		return HG_RETURNLASTUPDATE;
	}
}

//This is going to be really, really hard...
//What it needs to do is add two number types if they're adjacent, but concatenate a string if it exists
//So that way, 1 + 2.5 + " to " + 4 * 2 gives a string of "3.5 to 8", not "12.5 to 4" and dies on an error.
//So...
//TODO: Implement this properly.
int gauge_var::Evaluate(char** result)
{
	//This should be big enough.
	(*result) = new char[32];
	char* temp_result = new char[32];

	gauge_var* cv = this;

	while(cv != NULL)
	{
		switch(data_type)
		{
			case GV_INTPTR:
			case GV_INTVAR:
				temp_result = itoa(*int_variable, temp_result, 10);
				break;
			case GV_CHARPTR:
				strcpy(temp_result, *char_pointer);
				break;
			case GV_CHARVAR:
				strcpy(temp_result, char_variable);
				break;
			case GV_FLOATPTR:
			case GV_FLOATVAR:
				sprintf(temp_result, "%d%%", *float_variable);
				break;
			default:
				Error(LOCATION, "Unknown data type handed to gauge_var::Evaluate(char** result)!");
				return HG_RETURNNODRAW;
		}

		switch(var_operator)
		{
			case GV_ADD:
				strcat(*result, temp_result);
				break;
			default:
				Error(LOCATION, "Unknown operator handed to gauge_var::Evaluate(char** result)!");
				return HG_RETURNNODRAW;
		}
		
		cv = cv->next;
	}

	if(prev_data_type == GV_CHARVAR && !strcmp(*result, prev_char_result))
	{
		return HG_RETURNREDRAW;
	}
	else
	{
		size_t new_char_size = strlen(*result) + 1;
		if(prev_data_type != GV_CHARVAR || prev_char_size < new_char_size)
		{
			prev_data_type = GV_CHARVAR;
			DeallocPrevVars();
			prev_char_result = new char[new_char_size];
		}
		strcpy(prev_char_result, (*result));

		return HG_RETURNLASTUPDATE;
	}
}
void gauge_var::DeallocVars()
{
	if(data_type < GV_PTRSTART)
	{
		//This may be unneccessary since this is a union
		//But I'm taking no chances.
		//TODO: Investigate so this isn't taking up CPU time
		switch(data_type)
		{
			case GV_NOVAR:
				break;
			case GV_INTVAR:
				delete int_variable;
				break;
			case GV_CHARVAR:
				delete[] char_variable;
				break;
			case GV_FLOATVAR:
				delete float_variable;
				break;
			case GV_GAUGEVAR:
				delete gv_variable;
				break;
			default:
				delete[] char_variable;
				break;
		}
	}
}

void gauge_var::DeallocPrevVars()
{
	switch(prev_data_type)
	{
		case GV_NOVAR:
			break;
		case GV_INTVAR:
			delete prev_int_result;
			break;
		case GV_CHARVAR:
			delete[] prev_char_result;
			break;
		case GV_FLOATVAR:
			delete prev_float_result;
			break;
		default:
			delete[] prev_char_result;
			break;
	}
}

//Functioning as normal storage space
void gauge_var::operator=(int* pointer)
{
	data_type = GV_INTPTR;
	DeallocVars();
	int_variable = pointer;
}

void gauge_var::operator=(char** pointer)
{
	data_type = GV_CHARPTR;
	DeallocVars();
	char_pointer = pointer;
}

void gauge_var::operator=(float* pointer)
{
	data_type = GV_FLOATPTR;
	DeallocVars();
	float_variable = pointer;
}

void gauge_var::operator=(int value)
{
	if(data_type != GV_INTVAR)
	{
		data_type = GV_INTVAR;
		DeallocVars();
		int_variable = new int;
	}
	
	(*int_variable) = value;
}

void gauge_var::operator=(char* value)
{
	size_t new_size = strlen(value) + 1;
	if(data_type != GV_CHARVAR || char_size < new_size)
	{
		data_type = GV_CHARVAR;
		DeallocVars();
		char_variable = new char[new_size];
		char_size = new_size;
	}

	strcpy(char_variable, value);
}

void gauge_var::operator=(float value)
{
	if(data_type != GV_FLOATVAR)
	{
		data_type = GV_FLOATVAR;
		DeallocVars();
		float_variable = new float;
	}
	
	(*float_variable) = value;
}

gauge_var::gauge_var()
{
	//I assume NULL is 0 for this code. Panic if it isn't.
	Assert(NULL == 0);

	memset(this, 0, sizeof(gauge_var));
}
gauge_var::~gauge_var()
{
	DeallocVars();
}

#define MAX_CHILD_OBJECTS
#define MAX_OFFSET_NUM	

struct gauge_offset
{
	size_t offset;
	int array_index;	//-1 if not an array. -2 if an array. Anything else if it's an indice into array
}

struct gauge_child
{
	gauge_object* child;

	unsigned int num_offsets;

	gauge_offset offsets[MAX_OFFSET_NUM];	//If this isn't NULL, offset is interpreted to indicate an index into this array
}

class gauge_object_instance
{
private:
	struct gauge_object_instance *next, *prev;
	char name[NAME_LENGTH];
	void *pointer;

	std::vector<gauge_object*> (*manual_get_object_types)();
	std::vector<gauge_object*> object_types;
	int flags;

public:
	gauge_object_instance(){name[0]='\0';pointer=NULL;get_object_types=NULL;flags=0;}
	void add(class gauge_object_instance* ngip);
	void set_type_func(std::vector<gauge_object*> (*manual_get_object_types)());
	std::vector<gauge_object*> (*get_object_types)();
};

gauge_object_instance::gauge_object_instance(char *in_name, void *in_pointer, int in_flags, gauge_object* in_object_types, ...)
{
	if(strlen(in_name) > sizeof(name))
		return;

	strcpy(name, in_name);

	pointer = in_pointer;
	flags = in_flags;

	//Add the types
	va_list marker;
	unsigned int count = 0;
	gauge_object* cotp = in_object_types;

	va_start(marker, in_offsets);
	while(cotp != -1)
	{
		object_types.push_back(cotp);
		count++;
		cotp = va_arg(marker, gauge_object*);
	}
	va_end(marker);
}

//Only call this from the main array
void gauge_object_instance::add(gauge_object_instance *ngip)
{
	list_append(this, ngip);
}

class gauge_object
{
private:
	struct gauge_object *next, *prev;

	char name[NAME_LENGTH];
	bool has_value;

	unsigned int num_children;
	gauge_child children[MAX_CHILD_OBJECTS];
	int flags;
public:
	gauge_object* add(char* in_name, bool in_has_value, int in_flags = 0);
	bool copy_children(gauge_object *in_copyee);
	bool add_child(gauge_object* in_child, gauge_offset* in_offsets, ...);
	~gauge_object();
};

//Call delete on the first in a list of gauge_objects, and they all go away. :)
gauge_object::~gauge_object()
{
	if(next != NULL)
		delete next;
}

//OK, so true object derivation and that crap is out. This lets you quickly copy children.
bool gauge_object::copy_children(gauge_object *in_copyee)
{
	for(unsigned int i = 0; i < in_copyee->num_children; i++)
	{
		if(j==MAX_CHILD_OBJECTS)
		{
			Warning(LOCATION, "Could only copy %d child objects to %s", name);
			return false;
		}

		in_copyee->children[num_children] = in_copyee->children[i];
	}

	return true;
}

bool gauge_object::add_child(gauge_object* in_child, gauge_offset* in_offsets, ...)
{
	Assert(in_child != NULL);
	if(num_children == MAX_CHILD_OBJECTS)
	{
		Warning(LOCATION, "Could not add child object to %s", name);
		return false;
	}

	gauge_child *gc = &children[num_children];

	gc->child = in_child;

	//DO Handle offset variables
	va_list marker;
	unsigned int count = 0;
	gauge_offset* curr_offset = in_offsets;

	va_start(marker, in_offsets);
	while(curr_offset != -1 && count < MAX_OFFSET_NUM)
	{
		gc->offsets[count] = curr_offset;
		count++;
		curr_offset = va_arg(marker, size_t);
	}
	va_end(marker);
	return true;
}

gauge_object* add(char* in_name, bool in_has_value, int in_flags)
{
	gauge_object* gop = this;	//sigh...

	//Look for entries before or at this entry for a common name
	while(gop != NULL)
	{
		if(!stricmp(gop->name, in_name))
			return NULL;
		else
			gop = gop->prev;
	}

	//Look for entries after this entry for a commmon name
	gop = next;
	while(gop != NULL)
	{
		if(!stricmp(gop->name, in_name))
			return NULL;
		else
			gop = gop->next;
	}

	//OK, apparently nothing has the same name...we're clear to go
	gop = new gauge_object;

	//Set the values
	gop->prev = this;
	gop->next = NULL;
	strcpy(gop->name, in_name);
	gop->has_value = in_has_value;
	gop->num_offsets = 0;
	memset(children, 0, sizeof(children));
	gop->flags = in_flags;

	//We're done, return a pointer to the new gauge object
	return gop;
}

gauge_object* Gauge_obj_list = NULL;
std::vector<gauge_object_instances> Gauge_obj_instances;
//To use:
//Gauge_obj_list = Gauge_obj_list->Add("object", false);
//then...
//delete Gauge_obj_list;

void init()
{
	if(!ships_inited)
	{
		Error(LOCATION, "Could not initialize hudparse.cpp as ships were not inited first.");
		return;
	}
	//Initialize variables
	gauge_offset goff;

	goff.offset = 0;
	goff.array_pointer = NULL;

	//"core" objects
	gauge_object *gol_object, *gol_escort;
	gol_object = Gauge_obj_list = Gauge_obj_list->add("object", false);
	gol_ship = Gauge_obj_list->add("ship", false);

	//"child" objects, ie variables
	gauge_object *gol_object_type;
	gol_object_type = Gauge_obj_list->add("type", true);

	//Assign child objects
	goff.offset = offsetof(object, type);
	gol_object->add_child(gol_object_type, &goff);

	//Instances

	Gauge_obj_instances.push_back
}

#endif
