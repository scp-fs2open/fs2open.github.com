#include "iff_defs/iff_defs.h"
#include "mission/missionparse.h"
#include "mission/missiongoals.h"
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
void parse_xwi_mission_info(mission *pm, const XWingMission *xwim)
{
	pm->author = "X-Wing";
	strcpy_s(pm->created, "00/00/00 at 00:00:00");

	// NOTE: Y and Z are swapped and the units are in km
	Parse_viewer_pos.xyz.x = 1000 * xwim->flightgroups[Player_flight_group].start1_x;
	Parse_viewer_pos.xyz.y = 1000 * xwim->flightgroups[Player_flight_group].start1_z + 100;
	Parse_viewer_pos.xyz.z = 1000 * xwim->flightgroups[Player_flight_group].start1_y - 100;
	vm_angle_2_matrix(&Parse_viewer_orient, PI_4, 0);
}

bool is_fighter_or_bomber(const XWMFlightGroup *fg)
{
	switch (fg->flightGroupType)
	{
		case XWMFlightGroupType::fg_X_Wing:
		case XWMFlightGroupType::fg_Y_Wing:
		case XWMFlightGroupType::fg_A_Wing:
		case XWMFlightGroupType::fg_B_Wing:
		case XWMFlightGroupType::fg_TIE_Fighter:
		case XWMFlightGroupType::fg_TIE_Interceptor:
		case XWMFlightGroupType::fg_TIE_Bomber:
		case XWMFlightGroupType::fg_Gunboat:
		case XWMFlightGroupType::fg_TIE_Advanced:
			return true;
		default:
			break;
	}
	return false;
}

bool is_wing(const XWMFlightGroup *fg)
{
	return (fg->numberInWave > 1 || fg->numberOfWaves > 1 || is_fighter_or_bomber(fg));
}

int xwi_flightgroup_lookup(const XWingMission *xwim, const XWMFlightGroup *fg)
{
	for (size_t i = 0; i < xwim->flightgroups.size(); i++)
	{
		if (xwim->flightgroups[i].designation == fg->designation)
			return (int)i;
	}
	return -1;
}

void xwi_add_attack_check(const XWingMission *xwim, const XWMFlightGroup *fg)
{
	char fg_name[NAME_LENGTH] = "";
	char event_name[NAME_LENGTH];
	char sexp_buf[NAME_LENGTH + 50];

	int fg_index = xwi_flightgroup_lookup(xwim, fg);
	Assertion(fg_index >= 0, "Flight Group index must be valid");

	strcpy_s(fg_name, fg->designation.c_str());
	SCP_totitle(fg_name);

	sprintf(event_name, "FG %d Attack Check", fg_index);

	if (mission_event_lookup(event_name) >= 0)
		return;

	Mission_events.emplace_back();
	auto event = &Mission_events.back();
	event->name = event_name;

	if (is_wing(fg))
		sprintf(sexp_buf, "( when ( true ) ( fotg-wing-attacked-init \"%s\" ) )", fg_name);
	else
		sprintf(sexp_buf, "( when ( true ) ( fotg-ship-attacked-init \"%s\" ) )", fg_name);
	Mp = sexp_buf;
	event->formula = get_sexp_main();
}

int xwi_determine_arrival_cue(const XWingMission *xwim, const XWMFlightGroup *fg)
{
	const XWMFlightGroup *arrival_fg = nullptr;
	char arrival_fg_name[NAME_LENGTH] = "";
	char sexp_buf[NAME_LENGTH * 2 + 80];

	bool check_wing = false;
	if (fg->arrivalFlightGroup >= 0)
	{
		arrival_fg = &xwim->flightgroups[fg->arrivalFlightGroup];
		check_wing = is_wing(arrival_fg);
		strcpy_s(arrival_fg_name, arrival_fg->designation.c_str());
		SCP_totitle(arrival_fg_name);
	}
	else
		return Locked_sexp_true;

	if (fg->arrivalEvent == XWMArrivalEvent::ae_afg_arrived)
	{
		sprintf(sexp_buf, "( has-arrived-delay 0 \"%s\" )", arrival_fg_name);
		Mp = sexp_buf;
		return get_sexp_main();
	}

	if (fg->arrivalEvent == XWMArrivalEvent::ae_afg_attacked)
	{
		xwi_add_attack_check(xwim, arrival_fg);

		if (check_wing)
			sprintf(sexp_buf, "( fotg-is-wing-attacked \"%s\" )", arrival_fg_name);
		else
			sprintf(sexp_buf, "( fotg-is-ship-attacked \"%s\" )", arrival_fg_name);
		Mp = sexp_buf;
		return get_sexp_main();
	}

	if (fg->arrivalEvent == XWMArrivalEvent::ae_afg_captured)
	{
		if (check_wing)
			sprintf(sexp_buf, "( fotg-is-wing-captured \"%s\" )", arrival_fg_name);
		else
			sprintf(sexp_buf, "( fotg-is-ship-captured \"%s\" )", arrival_fg_name);
		Mp = sexp_buf;
		return get_sexp_main();
	}

	if (fg->arrivalEvent == XWMArrivalEvent::ae_afg_destroyed)
	{
		// X-Wing treats destruction for arrivals slightly differently
		if (check_wing)
			sprintf(sexp_buf, "( and ( percent-ships-destroyed 1 \"%s\" ) ( destroyed-or-departed-delay 0 \"%s\" ) )", arrival_fg_name, arrival_fg_name);
		else
			sprintf(sexp_buf, "( is-destroyed-delay 0 \"%s\" )", arrival_fg_name);
		Mp = sexp_buf;
		return get_sexp_main();
	}

	if (fg->arrivalEvent == XWMArrivalEvent::ae_afg_disabled)
	{
		if (check_wing)
			sprintf(sexp_buf, "( fotg-is-wing-disabled \"%s\" )", arrival_fg_name);
		else
			sprintf(sexp_buf, "( fotg-is-ship-disabled \"%s\" )", arrival_fg_name);
		Mp = sexp_buf;
		return get_sexp_main();
	}

	if (fg->arrivalEvent == XWMArrivalEvent::ae_afg_identified)
	{
		if (check_wing)
			sprintf(sexp_buf, "( fotg-is-wing-identified \"%s\" )", arrival_fg_name);
		else
			sprintf(sexp_buf, "( fotg-is-ship-identified \"%s\" )", arrival_fg_name);
		Mp = sexp_buf;
		return get_sexp_main();
	}

	return Locked_sexp_true;
}

int xwi_determine_anchor(const XWingMission *xwim, const XWMFlightGroup *fg)
{
	int mothership_number = fg->mothership;

	if (mothership_number >= 0)
	{
		if (mothership_number < (int)xwim->flightgroups.size())
			return get_parse_name_index(xwim->flightgroups[mothership_number].designation.c_str());
		else
			Warning(LOCATION, "Mothership number %d is out of range for Flight Group %s", mothership_number, fg->designation.c_str());
	}

	return -1;
}

const char *xwi_determine_formation(const XWMFlightGroup *fg)
{
	switch (fg->formation)
	{
		case XWMFormation::f_Vic:
			return "Double Vic";
		case XWMFormation::f_Finger_Four:
			return "Finger Four";
		case XWMFormation::f_Line_Astern:
			return "Line Astern";
		case XWMFormation::f_Line_Abreast:
			return "Line Abreast";
		case XWMFormation::f_Echelon_Right:
			return "Echelon Right";
		case XWMFormation::f_Echelon_Left:
			return "Echelon Left";
		case XWMFormation::f_Double_Astern:
			return "Double Astern";
		case XWMFormation::f_Diamond:
			return "Diamond";
		case XWMFormation::f_Stacked:
			return "Stacked";
		case XWMFormation::f_Spread:
			return "Spread";
		case XWMFormation::f_Hi_Lo:
			return "Hi-Lo";
		case XWMFormation::f_Spiral:
			return "Spiral";
	}

	return nullptr;
}

const char *xwi_determine_base_ship_class(const XWMFlightGroup *fg)
{
	switch (fg->flightGroupType)
	{
		case XWMFlightGroupType::fg_X_Wing:
			return "T-65c X-wing";
		case XWMFlightGroupType::fg_Y_Wing:
			return "BTL-A4 Y-wing";
		case XWMFlightGroupType::fg_A_Wing:
			return "RZ-1 A-wing";
		case XWMFlightGroupType::fg_TIE_Fighter:
			return "TIE/ln Fighter";
		case XWMFlightGroupType::fg_TIE_Interceptor:
			return "TIE/In Interceptor";
		case XWMFlightGroupType::fg_TIE_Bomber:
			return "TIE/sa Bomber";
		case XWMFlightGroupType::fg_Gunboat:
			return nullptr;
		case XWMFlightGroupType::fg_Transport:
			return "DX-9 Stormtrooper Transport";
		case XWMFlightGroupType::fg_Shuttle:
			return "Lambda-class T-4a Shuttle";
		case XWMFlightGroupType::fg_Tug:
			return nullptr;
		case XWMFlightGroupType::fg_Container:
			return nullptr;
		case XWMFlightGroupType::fg_Freighter:
			return "BFF-1 Bulk Freighter";
		case XWMFlightGroupType::fg_Calamari_Cruiser:
			return "Liberty Type Star Cruiser";
		case XWMFlightGroupType::fg_Nebulon_B_Frigate:
			return "EF76 Nebulon-B Escort Frigate";
		case XWMFlightGroupType::fg_Corellian_Corvette:
			return "CR90 Corvette";
		case XWMFlightGroupType::fg_Imperial_Star_Destroyer:
			return "Imperial Star Destroyer";
		case XWMFlightGroupType::fg_TIE_Advanced:
			return nullptr;
		case XWMFlightGroupType::fg_B_Wing:
			return "B-wing Starfighter";
		default:
			break;
	}

	return nullptr;
}

int xwi_determine_ship_class(const XWMFlightGroup *fg)
{
	// base ship class must exist
	auto class_name = xwi_determine_base_ship_class(fg);
	if (class_name == nullptr)
		return -1;

	SCP_string variant_class = class_name;
	bool variant = false;

	// now see if we have any variants
	if (fg->craftColor == XWMCraftColor::c_Red)
	{
		variant_class += "#red";
		variant = true;
	}
	else if (fg->craftColor == XWMCraftColor::c_Gold)
	{
		variant_class += "#gold";
		variant = true;
	}
	else if (fg->craftColor == XWMCraftColor::c_Blue)
	{
		variant_class += "#blue";
		variant = true;
	}

	if (variant)
	{
		int ship_class = ship_info_lookup(variant_class.c_str());
		if (ship_class >= 0)
			return ship_class;

		Warning(LOCATION, "Could not find variant ship class %s for Flight Group %s.  Using base class instead.", variant_class.c_str(), fg->designation.c_str());
	}

	// no variant, or we're just going with the base class
	return ship_info_lookup(class_name);
}

const char *xwi_determine_team(const XWingMission *xwim, const XWMFlightGroup *fg, const ship_info *sip)
{
	SCP_UNUSED(sip);

	auto player_iff = xwim->flightgroups[Player_flight_group].craftIFF;

	if (fg->craftIFF == XWMCraftIFF::iff_imperial)
	{
		if (player_iff == XWMCraftIFF::iff_imperial)
			return "Friendly";
		if (player_iff == XWMCraftIFF::iff_rebel)
			return "Hostile";
	}

	if (fg->craftIFF == XWMCraftIFF::iff_rebel)
	{
		if (player_iff == XWMCraftIFF::iff_imperial)
			return "Hostile";
		if (player_iff == XWMCraftIFF::iff_rebel)
			return "Friendly";
	}

	if (fg->craftIFF == XWMCraftIFF::iff_neutral)
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
		SCP_totitle(Cargo_names[index]);
	}
	return index;
}

const char *xwi_determine_ai_class(const XWMFlightGroup *fg)
{
	// Rookie = Cadet
	// Officer = Officer
	// Veteran = Captain
	// Ace = Commander
	// Top Ace = General

	switch (fg->craftAI)
	{
		case XWMCraftAI::ai_Rookie:
			return "Cadet";
		case XWMCraftAI::ai_Officer:
			return "Officer";
		case XWMCraftAI::ai_Veteran:
			return "Captain";
		case XWMCraftAI::ai_Ace:
			return "Commander";
		case XWMCraftAI::ai_Top_Ace:
			return "General";
	}

	return nullptr;
}

void xwi_determine_orientation(matrix *orient, const XWMFlightGroup *fg, const vec3d *start1, const vec3d *start2, const vec3d *start3,
	const vec3d *waypoint1, const vec3d *waypoint2, const vec3d *waypoint3, const vec3d *hyperspace)
{
	SCP_UNUSED(start2);
	SCP_UNUSED(start3);
	SCP_UNUSED(waypoint2);
	SCP_UNUSED(waypoint3);
	SCP_UNUSED(hyperspace);
	vec3d fvec;

	// RandomStarfighter says:
	// If WP1 is disabled, it has 45 degree pitch and yaw.
	if (!fg->waypoint1_enabled)
	{
		angles a;
		a.p = PI_4;
		a.b = 0;
		a.h = PI_4;
		vm_angles_2_matrix(orient, &a);
		return;
	}

	// RandomStarfighter says:
	// It arrives from start point and points toward waypoint 1, if waypoint 1 is enabled.
	// This also matches FG Red orientation in STARSNDB
	vm_vec_normalized_dir(&fvec, waypoint1, start1);
	vm_vector_2_matrix_norm(orient, &fvec);
}

void parse_xwi_flightgroup(mission *pm, const XWingMission *xwim, const XWMFlightGroup *fg)
{
	SCP_UNUSED(pm);

	int arrival_cue = xwi_determine_arrival_cue(xwim, fg);

	int number_in_wave = fg->numberInWave;
	if (number_in_wave > MAX_SHIPS_PER_WING)
	{
		Warning(LOCATION, "Too many ships in Flight Group %s.  FreeSpace supports up to a maximum of %d.", fg->designation.c_str(), MAX_SHIPS_PER_WING);
		number_in_wave = MAX_SHIPS_PER_WING;
	}

	// see if this flight group is what FreeSpace would treat as a wing
	wing *wingp = nullptr;
	int wingnum = -1;
	if (is_wing(fg))
	{
		wingnum = Num_wings++;
		wingp = &Wings[wingnum];

		strcpy_s(wingp->name, fg->designation.c_str());
		SCP_totitle(wingp->name);
		wingp->num_waves = fg->numberOfWaves;

		auto formation_name = xwi_determine_formation(fg);
		if (formation_name)
		{
			wingp->formation = wing_formation_lookup(formation_name);
			if (wingp->formation < 0)
				Warning(LOCATION, "Formation %s from Flight Group %s was not found", formation_name, fg->designation.c_str());
		}
		if (wingp->formation >= 0 && !is_fighter_or_bomber(fg))
			wingp->formation_scale = 4.0f;

		wingp->arrival_cue = arrival_cue;
		wingp->arrival_delay = fg->arrivalDelay;
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

		wingp->wave_count = number_in_wave;
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

	// similarly for the AI
	int ai_index = sip->ai_class;
	auto ai_name = xwi_determine_ai_class(fg);
	if (ai_name)
	{
		int index = string_lookup(ai_name, Ai_class_names, Num_ai_classes);
		if (index >= 0)
			ai_index = index;
		else
			Warning(LOCATION, "Could not find AI class %s", ai_name);
	}

	// similarly for any waypoints
	// NOTE: Y and Z are swapped
	auto start1 = vm_vec_new(fg->start1_x, fg->start1_z, fg->start1_y);
	auto start2 = vm_vec_new(fg->start2_x, fg->start2_z, fg->start2_y);
	auto start3 = vm_vec_new(fg->start3_x, fg->start3_z, fg->start3_y);
	auto waypoint1 = vm_vec_new(fg->waypoint1_x, fg->waypoint1_z, fg->waypoint1_y);
	auto waypoint2 = vm_vec_new(fg->waypoint2_x, fg->waypoint2_z, fg->waypoint2_y);
	auto waypoint3 = vm_vec_new(fg->waypoint3_x, fg->waypoint3_z, fg->waypoint3_y);
	auto hyperspace = vm_vec_new(fg->hyperspace_x, fg->hyperspace_z, fg->hyperspace_y);

	// waypoint units are in kilometers (after processing by xwinglib which handles the factor of 160), so scale them up
	vm_vec_scale(&start1, 1000);
	vm_vec_scale(&start2, 1000);
	vm_vec_scale(&start3, 1000);
	vm_vec_scale(&waypoint1, 1000);
	vm_vec_scale(&waypoint2, 1000);
	vm_vec_scale(&waypoint3, 1000);
	vm_vec_scale(&hyperspace, 1000);

	matrix orient;
	xwi_determine_orientation(&orient, fg, &start1, &start2, &start3, &waypoint1, &waypoint2, &waypoint3, &hyperspace);

	// now configure each ship in the flight group
	for (int wing_index = 0; wing_index < number_in_wave; wing_index++)
	{
		p_object pobj;

		if (wingp)
		{
			wing_bash_ship_name(pobj.name, wingp->name, wing_index + 1, nullptr);
			pobj.wingnum = wingnum;
			pobj.pos_in_wing = wing_index;
			pobj.arrival_cue = Locked_sexp_false;
		}
		else
		{
			strcpy_s(pobj.name, fg->designation.c_str());
			SCP_totitle(pobj.name);

			pobj.arrival_cue = arrival_cue;
			pobj.arrival_delay = fg->arrivalDelay;
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
		pobj.ai_class = ai_index;
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
		pobj.orient = orient;

		if (wingp && wing_index == fg->specialShipNumber)
			pobj.cargo1 = (char)xwi_lookup_cargo(fg->specialCargo.c_str());
		else
			pobj.cargo1 = (char)xwi_lookup_cargo(fg->cargo.c_str());

		if (fg->craftOrder != XWMCraftOrder::o_Hold_Steady && fg->craftOrder != XWMCraftOrder::o_Starship_Sit_And_Fire)
			pobj.initial_velocity = 100;

		if (fg->playerPos == wing_index + 1)
		{
			// undo any previously set player
			if (Player_starts > 0)
			{
				auto prev_player_pobjp = mission_parse_get_parse_object(Player_start_shipname);
				if (prev_player_pobjp)
				{
					Warning(LOCATION, "This mission specifies multiple player starting ships.  Skipping %s.", Player_start_shipname);
					prev_player_pobjp->flags.remove(Mission::Parse_Object_Flags::OF_Player_start);
					prev_player_pobjp->flags.remove(Mission::Parse_Object_Flags::SF_Cargo_known);
					prev_player_pobjp->flags.remove(Mission::Parse_Object_Flags::SF_Cannot_perform_scan);
					Player_starts--;
				}
				else
					Warning(LOCATION, "Multiple player starts specified, but previous player start %s couldn't be found!", Player_start_shipname);
			}

			strcpy_s(Player_start_shipname, pobj.name);
			pobj.flags.set(Mission::Parse_Object_Flags::OF_Player_start);
			pobj.flags.set(Mission::Parse_Object_Flags::SF_Cargo_known);
			pobj.flags.set(Mission::Parse_Object_Flags::SF_Cannot_perform_scan);
			Player_starts++;
		}

		if (fg->craftStatus == XWMCraftStatus::cs_no_shields)
			pobj.flags.set(Mission::Parse_Object_Flags::OF_No_shields);

		if (fg->craftStatus == XWMCraftStatus::cs_no_missiles || fg->craftStatus == XWMCraftStatus::cs_half_missiles)
		{
			// the only subsystem we actually need is Pilot, because everything else uses defaults
			pobj.subsys_index = Subsys_index;
			int this_subsys = allocate_subsys_status();
			pobj.subsys_count++;
			strcpy_s(Subsys_status[this_subsys].name, NOX("Pilot"));

			for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
			{
				Subsys_status[this_subsys].secondary_banks[bank] = SUBSYS_STATUS_NO_CHANGE;
				Subsys_status[this_subsys].secondary_ammo[bank] = (fg->craftStatus == XWMCraftStatus::cs_no_missiles) ? 0 : 50;
			}
		}

		Parse_objects.push_back(pobj);
	}
}

void parse_xwi_mission(mission *pm, const XWingMission *xwim)
{
	int index = -1;
	char sexp_buf[35];

	// find player flight group
	for (int i = 0; i < (int)xwim->flightgroups.size(); i++)
	{
		if (xwim->flightgroups[i].playerPos > 0)
		{
			index = i;
			// don't break in case multiple FGs set a player - we will use the last one assigned
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
	strcpy_s(Starting_wing_names[0], xwim->flightgroups[Player_flight_group].designation.c_str());
	SCP_totitle(Starting_wing_names[0]);
	strcpy_s(Squadron_wing_names[0], Starting_wing_names[0]);
	strcpy_s(TVT_wing_names[0], Starting_wing_names[0]);

	// indicate we are using X-Wing options
	Mission_events.emplace_back();
	auto config_event = &Mission_events.back();
	config_event->name = "XWI Import";

	sprintf(sexp_buf, "( when ( true ) ( do-nothing ) )");
	Mp = sexp_buf;
	config_event->formula = get_sexp_main();

	// load flight groups
	for (const auto &fg : xwim->flightgroups)
		parse_xwi_flightgroup(pm, xwim, &fg);
}

void post_process_xwi_mission(mission *pm, const XWingMission *xwim)
{
	SCP_UNUSED(pm);
	SCP_UNUSED(xwim);

	for (int wingnum = 0; wingnum < Num_wings; wingnum++)
	{
		auto wingp = &Wings[wingnum];
		auto leader_objp = &Objects[Ships[wingp->ship_index[0]].objnum];

		// we need to arrange all the flight groups into their formations, but this can't be done until the FRED objects are created from the parse objects
		for (int i = 1; i < wingp->wave_count; i++)
		{
			auto objp = &Objects[Ships[wingp->ship_index[i]].objnum];

			get_absolute_wing_pos(&objp->pos, leader_objp, wingnum, i, false);
			objp->orient = leader_objp->orient;
		}

		// set the hotkeys for the starting wings
		for (int i = 0; i < MAX_STARTING_WINGS; i++)
		{
			if (!stricmp(wingp->name, Starting_wing_names[i]))
			{
				wingp->hotkey = i;
				break;
			}
		}
	}
}

/**
* Set up xwi briefing based on assumed .brf file in the same folder. If .brf is not there, 
* just use minimal xwi briefing. 
*
* NOTE: This updates the global Briefing struct with all the data necessary to drive the briefing
*/
void parse_xwi_briefing(mission *pm, const XWingBriefing *xwBrief)
{
	SCP_UNUSED(pm);
	SCP_UNUSED(xwBrief);

	auto bp = &Briefings[0];
	bp->num_stages = 1;  // xwing briefings only have one stage
	auto bs = &bp->stages[0];

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
		bs->camera_orient = IDENTITY_MATRIX;
		bs->camera_time = 500;
		bs->num_lines = 0;
		bs->num_icons = 0;
		bs->flags = 0;
		bs->formula = Locked_sexp_true;
	}
}

