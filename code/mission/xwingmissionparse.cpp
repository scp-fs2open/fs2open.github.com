#include "iff_defs/iff_defs.h"
#include "mission/missionparse.h"
#include "missionui/redalert.h"
#include "nebula/neb.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "species_defs/species_defs.h"
#include "starfield/starfield.h"

#include "xwingbrflib.h"
#include "xwinglib.h"
#include "xwingmissionparse.h"

extern int allocate_subsys_status();

static int Player_flight_group = 0;

// vazor222
void parse_xwi_mission_info(mission *pm, XWingMission *xwim, bool basic)
{
	strcpy_s(pm->author, "X-Wing");
	strcpy_s(pm->created, "00/00/00 at 00:00:00");

	Parse_viewer_pos.xyz.x = 1000 * xwim->flightgroups[Player_flight_group]->start1_x;
	Parse_viewer_pos.xyz.y = 1000 * xwim->flightgroups[Player_flight_group]->start1_y + 100;
	Parse_viewer_pos.xyz.z = 1000 * xwim->flightgroups[Player_flight_group]->start1_z - 100;
	vm_angle_2_matrix(&Parse_viewer_orient, PI_4, 0);
}

bool is_fighter_or_bomber(XWMFlightGroup *fg)
{
	switch (fg->flightGroupType)
	{
		case XWMFlightGroup::fg_X_Wing:
		case XWMFlightGroup::fg_Y_Wing:
		case XWMFlightGroup::fg_A_Wing:
		case XWMFlightGroup::fg_B_Wing:
		case XWMFlightGroup::fg_TIE_Fighter:
		case XWMFlightGroup::fg_TIE_Interceptor:
		case XWMFlightGroup::fg_TIE_Bomber:
		case XWMFlightGroup::fg_Gunboat:
		case XWMFlightGroup::fg_TIE_Advanced:
			return true;
	}
	return false;
}

int xwi_determine_anchor(XWingMission *xwim, XWMFlightGroup *fg)
{
	int mothership_number = fg->mothership;

	if (mothership_number >= 0)
	{
		if (mothership_number < (int)xwim->flightgroups.size())
			return get_parse_name_index(xwim->flightgroups[mothership_number]->designation.c_str());
		else
			Warning(LOCATION, "Mothership number %d is out of range for Flight Group %s", mothership_number, fg->designation.c_str());
	}

	return -1;
}

const char *xwi_determine_base_ship_class(XWMFlightGroup *fg)
{
	int flightGroupType = fg->flightGroupType;

	switch (flightGroupType)
	{
		case XWMFlightGroup::fg_X_Wing:
			return "T-65c X-wing";
		case XWMFlightGroup::fg_Y_Wing:
			return "BTL-A4 Y-wing";
		case XWMFlightGroup::fg_A_Wing:
			return "RZ-1 A-wing";
		case XWMFlightGroup::fg_TIE_Fighter:
			return "TIE/ln Fighter";
		case XWMFlightGroup::fg_TIE_Interceptor:
			return "TIE/In Interceptor";
		case XWMFlightGroup::fg_TIE_Bomber:
			return "TIE/sa Bomber";
		case XWMFlightGroup::fg_Gunboat:
			return nullptr;
		case XWMFlightGroup::fg_Transport:
			return "DX-9 Stormtrooper Transport";
		case XWMFlightGroup::fg_Shuttle:
			return "Lambda-class T-4a Shuttle";
		case XWMFlightGroup::fg_Tug:
			return nullptr;
		case XWMFlightGroup::fg_Container:
			return nullptr;
		case XWMFlightGroup::fg_Frieghter:
			return "BFF-1 Bulk Freighter";
		case XWMFlightGroup::fg_Calamari_Cruiser:
			return "Liberty Type Star Cruiser";
		case XWMFlightGroup::fg_Nebulon_B_Frigate:
			return "EF76 Nebulon-B Escort Frigate";
		case XWMFlightGroup::fg_Corellian_Corvette:
			return "CR90 Corvette";
		case XWMFlightGroup::fg_Imperial_Star_Destroyer:
			return "Imperial Star Destroyer";
		case XWMFlightGroup::fg_TIE_Advanced:
			return nullptr;
		case XWMFlightGroup::fg_B_Wing:
			return "B-wing Starfighter";
	}

	return nullptr;
}

int xwi_determine_ship_class(XWMFlightGroup *fg)
{
	// base ship class must exist
	auto class_name = xwi_determine_base_ship_class(fg);
	if (class_name == nullptr)
		return -1;

	SCP_string variant_class = class_name;
	bool variant = false;

	// now see if we have any variants
	if (fg->craftColor == XWMFlightGroup::c_Red)
	{
		variant_class += "#red";
		variant = true;
	}
	else if (fg->craftColor == XWMFlightGroup::c_Gold)
	{
		variant_class += "#gold";
		variant = true;
	}
	else if (fg->craftColor == XWMFlightGroup::c_Blue)
	{
		variant_class += "#blue";
		variant = true;
	}

	if (variant)
	{
		int ship_class = ship_info_lookup(variant_class.c_str());
		if (ship_class >= 0)
			return ship_class;

		Warning(LOCATION, "Could not find variant ship class %s for flight group %s.  Using base class instead.", variant_class.c_str(), fg->designation.c_str());
	}

	// no variant, or we're just going with the base class
	return ship_info_lookup(class_name);
}

const char *xwi_determine_team(XWingMission *xwim, XWMFlightGroup *fg, ship_info *sip)
{
	auto player_iff = xwim->flightgroups[Player_flight_group]->craftIFF;

	if (fg->craftIFF == XWMFlightGroup::iff_imperial)
	{
		if (player_iff == XWMFlightGroup::iff_imperial)
			return "Friendly";
		if (player_iff == XWMFlightGroup::iff_rebel)
			return "Hostile";
	}

	if (fg->craftIFF == XWMFlightGroup::iff_rebel)
	{
		if (player_iff == XWMFlightGroup::iff_imperial)
			return "Hostile";
		if (player_iff == XWMFlightGroup::iff_rebel)
			return "Friendly";
	}

	if (fg->craftIFF == XWMFlightGroup::iff_neutral)
		return "Civilian";

	return nullptr;
}

int xwi_lookup_cargo(const char *cargo_name)
{
	// empty cargo is the same as Nothing
	if (!*cargo_name)
		return 0;

	int index = string_lookup(cargo_name, Cargo_names, Num_cargo);
	if (index < 0)
	{
		if (Num_cargo == MAX_CARGO)
		{
			Warning(LOCATION, "Can't add any more cargo!");
			return 0;
		}

		index = Num_cargo++;
		strcpy(Cargo_names[index], cargo_name);
	}
	return index;
}

void parse_xwi_flightgroup(mission *pm, XWingMission *xwim, XWMFlightGroup *fg)
{
	// see if this flight group is what FreeSpace would treat as a wing
	wing *wingp = nullptr;
	int wingnum = -1;
	if (fg->numberInWave > 1 || fg->numberOfWaves > 1 || is_fighter_or_bomber(fg))
	{
		wingnum = Num_wings++;
		wingp = &Wings[wingnum];

		strcpy_s(wingp->name, fg->designation.c_str());
		SCP_totitle(wingp->name);
		wingp->num_waves = fg->numberOfWaves;
		wingp->formation = -1;	// TODO

		wingp->arrival_location = fg->arriveByHyperspace ? ARRIVE_AT_LOCATION : ARRIVE_FROM_DOCK_BAY;
		wingp->arrival_anchor = xwi_determine_anchor(xwim, fg);
		wingp->departure_location = fg->departByHyperspace ? DEPART_AT_LOCATION : DEPART_AT_DOCK_BAY;
		wingp->departure_anchor = wingp->arrival_anchor;

		// if a wing doesn't have an anchor, make sure it is at-location
		// (flight groups present at mission start will have arriveByHyperspace set to false)
		if (wingp->arrival_anchor < 0)
			wingp->arrival_location = ARRIVE_AT_LOCATION;
		if (wingp->departure_anchor < 0)
			wingp->departure_location = DEPART_AT_LOCATION;

		wingp->wave_count = fg->numberInWave;
	}

	// all ships in the flight group share a class, so determine that here
	int ship_class = xwi_determine_ship_class(fg);
	if (ship_class < 0)
	{
		Warning(LOCATION, "Unable to determine ship class for Flight Group %s", fg->designation.c_str());
		ship_class = 0;
	}
	auto sip = &Ship_info[ship_class];

	// similarly for the team
	auto team_name = xwi_determine_team(xwim, fg, sip);
	int team = Species_info[sip->species].default_iff;
	if (team_name)
	{
		int index = iff_lookup(team_name);
		if (index >= 0)
			team = index;
		else
			Warning(LOCATION, "Could not find iff %s", team_name);
	}

	// similarly for any waypoints
	auto start1 = vm_vec_new(fg->start1_x, fg->start1_y, fg->start1_z);
	auto start2 = vm_vec_new(fg->start2_x, fg->start2_y, fg->start2_z);
	auto start3 = vm_vec_new(fg->start3_x, fg->start3_y, fg->start3_z);
	auto waypoint1 = vm_vec_new(fg->wp1_x, fg->wp1_y, fg->wp1_z);
	auto waypoint2 = vm_vec_new(fg->wp2_x, fg->wp2_y, fg->wp2_z);
	auto waypoint3 = vm_vec_new(fg->wp3_x, fg->wp3_y, fg->wp3_z);
	auto hyp = vm_vec_new(fg->hyperspace_x, fg->hyperspace_y, fg->hyperspace_z);

	// waypoint units are in kilometers (after processing by xwinglib which handles the factor of 160), so scale them up
	vm_vec_scale(&start1, 1000);
	vm_vec_scale(&start2, 1000);
	vm_vec_scale(&start3, 1000);
	vm_vec_scale(&waypoint1, 1000);
	vm_vec_scale(&waypoint2, 1000);
	vm_vec_scale(&waypoint3, 1000);
	vm_vec_scale(&hyp, 1000);

	// now configure each ship in the flight group
	for (int wave_index = 0; wave_index < fg->numberInWave; wave_index++)
	{
		p_object pobj;

		if (wingp)
		{
			wing_bash_ship_name(pobj.name, wingp->name, wave_index + 1, nullptr);
			pobj.wingnum = wingnum;
			pobj.pos_in_wing = wave_index;
		}
		else
		{
			strcpy_s(pobj.name, fg->designation.c_str());
			SCP_totitle(pobj.name);

			pobj.arrival_location = fg->arriveByHyperspace ? ARRIVE_AT_LOCATION : ARRIVE_FROM_DOCK_BAY;
			pobj.arrival_anchor = xwi_determine_anchor(xwim, fg);
			pobj.departure_location = fg->departByHyperspace ? DEPART_AT_LOCATION : DEPART_AT_DOCK_BAY;
			pobj.departure_anchor = pobj.arrival_anchor;

			// if a ship doesn't have an anchor, make sure it is at-location
			// (flight groups present at mission start will have arriveByHyperspace set to false)
			if (pobj.arrival_anchor < 0)
				pobj.arrival_location = ARRIVE_AT_LOCATION;
			if (pobj.departure_anchor < 0)
				pobj.departure_location = DEPART_AT_LOCATION;
		}

		pobj.ship_class = ship_class;

		// initialize class-specific fields
		pobj.ai_class = sip->ai_class;
		pobj.warpin_params_index = sip->warpin_params_index;
		pobj.warpout_params_index = sip->warpout_params_index;
		pobj.ship_max_shield_strength = sip->max_shield_strength;
		pobj.ship_max_hull_strength = sip->max_hull_strength;
		Assert(pobj.ship_max_hull_strength > 0.0f);	// Goober5000: div-0 check (not shield because we might not have one)
		pobj.max_shield_recharge = sip->max_shield_recharge;
		pobj.replacement_textures = sip->replacement_textures;	// initialize our set with the ship class set, which may be empty
		pobj.score = sip->score;

		pobj.team = team;

		pobj.pos = start1;

		if (fg->arriveByHyperspace)
		{
			// YOGEME: When a craft arrives via hyperspace, it will be oriented such that it will pass through HYP and be pointing towards SP1
			vec3d fvec;
			vm_vec_normalized_dir(&fvec, &start1, &hyp);
			vm_vector_2_matrix_norm(&pobj.orient, &fvec);
		}
		else
		{
			vec3d fvec;
			vm_vec_normalized_dir(&fvec, &hyp, &start1);
			vm_vector_2_matrix_norm(&pobj.orient, &fvec);
		}

		if (wingp && wave_index == fg->specialShipNumber)
			pobj.cargo1 = (char)xwi_lookup_cargo(fg->specialCargo.c_str());
		else
			pobj.cargo1 = (char)xwi_lookup_cargo(fg->cargo.c_str());

		if (fg->order != XWMFlightGroup::o_Hold_Steady && fg->order != XWMFlightGroup::o_Starship_Sit_And_Fire)
			pobj.initial_velocity = 100;

		if (fg->playerPos == wave_index + 1)
		{
			strcpy_s(Player_start_shipname, pobj.name);
			pobj.flags.set(Mission::Parse_Object_Flags::OF_Player_start);
			pobj.flags.set(Mission::Parse_Object_Flags::SF_Cargo_known);
			Player_starts++;
		}

		if (fg->craftStatus == XWMFlightGroup::cs_no_shields)
			pobj.flags.set(Mission::Parse_Object_Flags::OF_No_shields);

		if (fg->craftStatus == XWMFlightGroup::cs_no_missiles || fg->craftStatus == XWMFlightGroup::cs_half_missiles)
		{
			// the only subsystem we actually need is Pilot, because everything else uses defaults
			pobj.subsys_index = Subsys_index;
			int this_subsys = allocate_subsys_status();
			pobj.subsys_count++;
			strcpy_s(Subsys_status[this_subsys].name, NOX("Pilot"));

			for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
			{
				Subsys_status[this_subsys].secondary_banks[bank] = SUBSYS_STATUS_NO_CHANGE;
				Subsys_status[this_subsys].secondary_ammo[bank] = (fg->craftStatus == XWMFlightGroup::cs_no_missiles) ? 0 : 50;
			}
		}

		Parse_objects.push_back(pobj);
	}
}

void parse_xwi_mission(mission *pm, XWingMission *xwim)
{
	int index = -1;

	// find player flight group
	for (int i = 0; i < xwim->flightgroups.size(); i++)
	{
		if (xwim->flightgroups[i]->playerPos > 0)
		{
			index = i;
			break;
		}
	}
	if (index >= 0)
		Player_flight_group = index;
	else
	{
		Warning(LOCATION, "Player flight group not found?");
		Player_flight_group = 0;
	}

	// clear out wings by default
	for (int i = 0; i < MAX_STARTING_WINGS; i++)
		sprintf(Starting_wing_names[i], "Hidden %d", i);
	for (int i = 0; i < MAX_SQUADRON_WINGS; i++)
		sprintf(Squadron_wing_names[i], "Hidden %d", i);
	for (int i = 0; i < MAX_TVT_WINGS; i++)
		sprintf(TVT_wing_names[i], "Hidden %d", i);

	// put the player's flight group in the default spot
	strcpy_s(Starting_wing_names[0], xwim->flightgroups[Player_flight_group]->designation.c_str());
	SCP_totitle(Starting_wing_names[0]);
	strcpy_s(Squadron_wing_names[0], Starting_wing_names[0]);
	strcpy_s(TVT_wing_names[0], Starting_wing_names[0]);

	// load flight groups
	for (auto fg : xwim->flightgroups)
		parse_xwi_flightgroup(pm, xwim, fg);
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

	bp = &Briefings[0];
	bp->num_stages = 1;  // xwing briefings only have one stage
	bs = &bp->stages[0];

	/*
	if (xwBrief != NULL)
	{
		bs->text = xwBrief->message1;  // this?
		bs->text = xwBrief->ships[2].designation;  // or this?
		bs->num_icons = xwBrief->header.icon_count;  // VZTODO is this the right place to store this?
	}
	else
	*/
	{
		bs->text = "Prepare for the next X-Wing mission!";
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

