#include "mission/missionparse.h"
#include "missionui/redalert.h"
#include "nebula/neb.h"
#include "parse/parselo.h"
#include "species_defs/species_defs.h"
#include "starfield/starfield.h"

#include "xwingbrflib.h"
#include "xwinglib.h"
#include "xwingmissionparse.h"

// vazor222
void parse_xwi_mission_info(mission *pm, XWingMission *xwim, bool basic)
{
	// XWI file version is checked on load, just use latest here
	pm->version = MISSION_VERSION;

	// XWI missions will be assigned names in the briefing
	strcpy_s(pm->name, "XWI Mission");

	strcpy_s(pm->author, "XWIConverter");

	// XWI missions don't track these timestamps, just use now 
	char time_string[DATE_TIME_LENGTH];
	time_t rawtime;
	time(&rawtime);
	strftime(time_string, DATE_TIME_LENGTH, "%D %T", localtime(&rawtime));
	strcpy_s(pm->created, time_string);

	strcpy_s(pm->modified, time_string);

	strcpy_s(pm->notes, "This is a FRED2_OPEN created mission, imported from XWI.");

	strcpy_s(pm->mission_desc, NOX("No description\n"));

	pm->game_type = MISSION_TYPE_SINGLE;  // assume all XWI missions are single player

	pm->flags.reset();

	// nebula mission stuff
	Neb2_awacs = -1.0f;
	Neb2_fog_near_mult = 1.0f;
	Neb2_fog_far_mult = 1.0f;
	
	// Goober5000 - ship contrail speed threshold
	pm->contrail_threshold = CONTRAIL_THRESHOLD_DEFAULT;

	pm->num_players = 1;
	pm->num_respawns = 0;
	The_mission.max_respawn_delay = -1;

	red_alert_invalidate_timestamp();

	// if we are just requesting basic info then skip everything else.  the reason
	// for this is to make sure that we don't modify things outside of the mission struct
	// that might not get reset afterwards (like what can happen in the techroom) - taylor
	//
	// NOTE: this can be dangerous so be sure that any get_mission_info() call (defaults to basic info) will
	//       only reference data parsed before this point!! (like current FRED2 and game code does)
	if (basic)
		return;

	// this is part of mission info but derived from the campaign file rather than parsed
	extern int debrief_find_persona_index();
	pm->debriefing_persona = debrief_find_persona_index();

	// set up support ships
	pm->support_ships.arrival_location = ARRIVE_AT_LOCATION;
	pm->support_ships.arrival_anchor = -1;
	pm->support_ships.departure_location = DEPART_AT_LOCATION;
	pm->support_ships.departure_anchor = -1;
	pm->support_ships.max_hull_repair_val = 0.0f;
	pm->support_ships.max_subsys_repair_val = 100.0f;	//ASSUMPTION: full repair capabilities
	pm->support_ships.max_support_ships = -1;	// infinite
	pm->support_ships.max_concurrent_ships = 1;
	pm->support_ships.ship_class = -1;
	pm->support_ships.tally = 0;
	pm->support_ships.support_available_for_species = 0;

	// for each species, store whether support is available
	for (int species = 0; species < (int)Species_info.size(); species++) {
		if (Species_info[species].support_ship_index >= 0) {
			pm->support_ships.support_available_for_species |= (1 << species);
		}
	}

	// disallow support in XWI missions unless specifically enabled
	pm->support_ships.max_support_ships = 0;

	Mission_all_attack = 0;

	//	Maybe delay the player's entry.
	// TODO find player's Flight Group and use arrival delay value from that?
	// TODO convert from 0 = 0?, 1 = one minute, 2D = 2 minutes, 30 seconds.. hmm, strange encoding
	if (xwim->flightgroups[0]->arrivalDelay > 0) {
		Entry_delay_time = xwim->flightgroups[0]->arrivalDelay;
	}
	else
	{
		Entry_delay_time = 0;
	}

	// player is always flight group 0
	Parse_viewer_pos.xyz.x = xwim->flightgroups[0]->start1_x;
	Parse_viewer_pos.xyz.y = xwim->flightgroups[0]->start1_y + 100;
	Parse_viewer_pos.xyz.z = xwim->flightgroups[0]->start1_z - 100;
	
	// possible squadron reassignment
	strcpy_s(pm->squad_name, "");
	strcpy_s(pm->squad_filename, "");

	// XWI missions are always single player
	Num_teams = 1;				// assume 1

	// TODO load deathstar surface skybox model when specified?
	strcpy_s(pm->skybox_model, "");

	vm_set_identity(&pm->skybox_orientation);
	pm->skybox_flags = DEFAULT_NMODEL_FLAGS;

	// Goober5000 - AI on a per-mission basis
	The_mission.ai_profile = &Ai_profiles[Default_ai_profile];

	pm->sound_environment.id = -1;
}

void parse_xwi_mission(mission *pm, XWingMission *xwim)
{
}

/**
* Set up xwi briefing based on assumed .brf file in the same folder. If .brf is not there, 
* just use minimal xwi briefing. 
*
* NOTE: This updates the global Briefing struct with all the data necessary to drive the briefing
*/
void parse_xwi_briefing(mission *pm, XWingBriefing *xwBrief)
{
	brief_stage *bs;
	briefing *bp;

	brief_reset();

	bp = &Briefings[0];
	bp->num_stages = 1;  // xwing briefings only have one stage
	bs = &bp->stages[0];

	if (xwBrief != NULL)
	{
		bs->text = xwBrief->message1;  // this?
		bs->text = xwBrief->ships[2].designation;  // or this?
		bs->num_icons = xwBrief->header.icon_count;  // VZTODO is this the right place to store this?
	}
	else
	{
		bs->text = "Prepare for the next xwing mission!";
		strcpy_s(bs->voice, "none.wav");
		vm_vec_zero(&bs->camera_pos);
		bs->camera_orient = SCALE_IDENTITY_VECTOR;
		bs->camera_time = 500;
		bs->num_lines = 0;
		bs->num_icons = 0;
		bs->flags = 0;
		bs->formula = Locked_sexp_true;
	}
}

