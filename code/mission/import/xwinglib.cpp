
#include <vector>
#include <string>
#include "xwinglib.h"

#pragma pack(push, 1)
struct xwi_header {
	short version;
	short mission_time_limit;
	short end_event;
	short rnd_seed;
	short mission_location;
	char completion_msg_1[64];
	char completion_msg_2[64];
	char completion_msg_3[64];
	short number_of_flight_groups;
	short number_of_objects;
};

struct xwi_flightgroup {
	char designation[16];
	char cargo[16];
	char special_cargo[16];
	short special_ship_number;
	short flight_group_type;
	short craft_iff;
	short craft_status;
	short number_in_wave;
	short number_of_waves;
	short arrival_event;
	short arrival_delay;
	short arrival_flight_group;
	short mothership;
	short arrive_by_hyperspace;
	short depart_by_hyperspace;
	short start1_x;
	short wp1_x;
	short wp2_x;
	short wp3_x;
	short start2_x;
	short start3_x;
	short hyp_x;
	short start1_y;
	short wp1_y;
	short wp2_y;
	short wp3_y;
	short start2_y;
	short start3_y;
	short hyp_y;
	short start1_z;
	short wp1_z;
	short wp2_z;
	short wp3_z;
	short start2_z;
	short start3_z;
	short hyp_z;
	short start1_enabled;
	short wp1_enabled;
	short wp2_enabled;
	short wp3_enabled;
	short start2_enabled;
	short start3_enabled;
	short hyp_enabled;
	short formation;
	short player_pos;
	short craft_ai;
	short order;
	short dock_time_or_throttle;
	short craft_markings1;
	short craft_markings2;
	short objective;
	short primary_target;
	short secondary_target;
};
#pragma pack(pop)

int XWingMission::arrival_delay_to_seconds(int delay)
{
	// If the arrival delay is less than or equal to 20, it's in minutes. If it's over 20, it's in 6 second blocks. So 21 is 6 seconds, for example
	if (delay <= 20)
		return delay * 60;
	return (delay - 20) * 6;
}

bool XWingMission::load(XWingMission *m, const char *data)
{
	xwi_header *h = (xwi_header *)data;
	if (h->version != 2)
		return false;

	m->missionTimeLimit = h->mission_time_limit;

	switch (h->end_event) {
		case 0:
			m->endEvent = XWMEndEvent::ev_rescued;
			break;
		case 1:
			m->endEvent = XWMEndEvent::ev_captured;
			break;
		case 5:
			m->endEvent = XWMEndEvent::ev_hit_exhaust_port;
			break;
		default:
			return false;
	}

	m->rnd_seed = h->rnd_seed;
	
	switch(h->mission_location) {
		case 0:
			m->missionLocation = XWMMissionLocation::ml_deep_space;
			break;
		case 1:
			m->missionLocation = XWMMissionLocation::ml_death_star;
			if (m->endEvent == XWMEndEvent::ev_captured)
				m->endEvent = XWMEndEvent::ev_cleared_laser_turrets;
			break;
		default:
			return false;
	}

	m->completionMsg1 = h->completion_msg_1;
	m->completionMsg2 = h->completion_msg_2;
	m->completionMsg3 = h->completion_msg_3;

	for (int n = 0; n < h->number_of_flight_groups; n++) {
		xwi_flightgroup *fg = (xwi_flightgroup *)(data + sizeof(xwi_header) + sizeof(xwi_flightgroup) * n);
		XWMFlightGroup nfg_buf;
		XWMFlightGroup *nfg = &nfg_buf;

		nfg->designation = fg->designation;
		nfg->cargo = fg->cargo;
		nfg->specialCargo = fg->special_cargo;
		nfg->specialShipNumber = fg->special_ship_number;

		switch(fg->flight_group_type) {
			case 0:
				nfg->flightGroupType = XWMFlightGroupType::fg_None;
				break;
			case 1:
				nfg->flightGroupType = XWMFlightGroupType::fg_X_Wing;
				break;
			case 2:
				nfg->flightGroupType = XWMFlightGroupType::fg_Y_Wing;

				// There is a special case for Y-wings.  If
				// the status is 10 (decimal) or higher, the FG is interpreted as a B-wing
				// CraftType instead.  The status list repeats in the same order.  For example, a
				// Y-Wing with a status of 1 has no warheads, but with a status of 11 becomes a
				// B-Wing with no warheads.
				if (fg->craft_status >= 10)
				{
					fg->flight_group_type = 18;	// B-Wing
					fg->craft_status -= 10;
					nfg->flightGroupType = XWMFlightGroupType::fg_B_Wing;
				}

				break;
			case 3:
				nfg->flightGroupType = XWMFlightGroupType::fg_A_Wing;
				break;
			case 4:
				nfg->flightGroupType = XWMFlightGroupType::fg_TIE_Fighter;
				break;
			case 5:
				nfg->flightGroupType = XWMFlightGroupType::fg_TIE_Interceptor;
				break;
			case 6:
				nfg->flightGroupType = XWMFlightGroupType::fg_TIE_Bomber;
				break;
			case 7:
				nfg->flightGroupType = XWMFlightGroupType::fg_Gunboat;
				break;
			case 8:
				nfg->flightGroupType = XWMFlightGroupType::fg_Transport;
				break;
			case 9:
				nfg->flightGroupType = XWMFlightGroupType::fg_Shuttle;
				break;
			case 10:
				nfg->flightGroupType = XWMFlightGroupType::fg_Tug;
				break;
			case 11:
				nfg->flightGroupType = XWMFlightGroupType::fg_Container;
				break;
			case 12:
				nfg->flightGroupType = XWMFlightGroupType::fg_Freighter;
				break;
			case 13:
				nfg->flightGroupType = XWMFlightGroupType::fg_Calamari_Cruiser;
				break;
			case 14:
				nfg->flightGroupType = XWMFlightGroupType::fg_Nebulon_B_Frigate;
				break;
			case 15:
				nfg->flightGroupType = XWMFlightGroupType::fg_Corellian_Corvette;
				break;
			case 16:
				nfg->flightGroupType = XWMFlightGroupType::fg_Imperial_Star_Destroyer;
				break;
			case 17:
				nfg->flightGroupType = XWMFlightGroupType::fg_TIE_Advanced;
				break;
			case 18:
				nfg->flightGroupType = XWMFlightGroupType::fg_B_Wing;
				break;
			default:
				return false;
		}

		switch(fg->craft_iff) {
			case 0:
				nfg->craftIFF = XWMCraftIFF::iff_default;
				break;
			case 1:
				nfg->craftIFF = XWMCraftIFF::iff_rebel;
				break;
			case 2:
				nfg->craftIFF = XWMCraftIFF::iff_imperial;
				break;
			case 3:
				nfg->craftIFF = XWMCraftIFF::iff_neutral;
				break;
		}

		switch(fg->craft_status) {
			case 0:
				nfg->craftStatus = XWMCraftStatus::cs_normal;
				break;
			case 1:
				nfg->craftStatus = XWMCraftStatus::cs_no_missiles;
				break;
			case 2:
				nfg->craftStatus = XWMCraftStatus::cs_half_missiles;
				break;
			case 3:
				nfg->craftStatus = XWMCraftStatus::cs_no_shields;
				break;
			case 4:
				// XXX Used by CENTURY 1 in CONVOY2.XWI
				nfg->craftStatus = XWMCraftStatus::cs_normal;
				break;
			case 10:
				// XXX Used by Unnammed B-Wing group in DESUPPLY.XWI
				nfg->craftStatus = XWMCraftStatus::cs_normal;
				break;
			case 11:
				// XXX Used by PROTOTYPE 2 in T5H1WB.XWI
				nfg->craftStatus = XWMCraftStatus::cs_no_missiles;
				break;
			case 12:
				// XXX Used by RED 1 in T5M19MB.XWI
				nfg->craftStatus = XWMCraftStatus::cs_half_missiles;
				break;
			default:
				return false;
		}

		nfg->numberInWave = fg->number_in_wave;
		nfg->numberOfWaves = fg->number_of_waves + 1;

		switch(fg->arrival_event) {
			case 0:
				nfg->arrivalEvent = XWMArrivalEvent::ae_mission_start;
				break;
			case 1:
				nfg->arrivalEvent = XWMArrivalEvent::ae_afg_arrived;
				break;
			case 2:
				nfg->arrivalEvent = XWMArrivalEvent::ae_afg_destroyed;
				break;
			case 3:
				nfg->arrivalEvent = XWMArrivalEvent::ae_afg_attacked;
				break;
			case 4:
				nfg->arrivalEvent = XWMArrivalEvent::ae_afg_captured;
				break;
			case 5:
				nfg->arrivalEvent = XWMArrivalEvent::ae_afg_identified;
				break;
			case 6:
				nfg->arrivalEvent = XWMArrivalEvent::ae_afg_disabled;
				break;
			default:
				return false;
		}

		nfg->arrivalDelay = arrival_delay_to_seconds(fg->arrival_delay);

		nfg->arrivalFlightGroup = fg->arrival_flight_group;
		nfg->mothership = fg->mothership;
		nfg->arriveByHyperspace = (fg->arrive_by_hyperspace != 0);
		nfg->departByHyperspace = (fg->depart_by_hyperspace != 0);

		nfg->start1_x = fg->start1_x / 160.0f;
		nfg->start2_x = fg->start2_x / 160.0f;
		nfg->start3_x = fg->start3_x / 160.0f;
		nfg->start1_y = fg->start1_y / 160.0f;
		nfg->start2_y = fg->start2_y / 160.0f;
		nfg->start3_y = fg->start3_y / 160.0f;
		nfg->start1_z = fg->start1_z / 160.0f;
		nfg->start2_z = fg->start2_z / 160.0f;
		nfg->start3_z = fg->start3_z / 160.0f;

		nfg->waypoint1_x = fg->wp1_x / 160.0f;
		nfg->waypoint2_x = fg->wp2_x / 160.0f;
		nfg->waypoint3_x = fg->wp3_x / 160.0f;
		nfg->waypoint1_y = fg->wp1_y / 160.0f;
		nfg->waypoint2_y = fg->wp2_y / 160.0f;
		nfg->waypoint3_y = fg->wp3_y / 160.0f;
		nfg->waypoint1_z = fg->wp1_z / 160.0f;
		nfg->waypoint2_z = fg->wp2_z / 160.0f;
		nfg->waypoint3_z = fg->wp3_z / 160.0f;

		nfg->hyperspace_x = fg->hyp_x / 160.0f;
		nfg->hyperspace_y = fg->hyp_y / 160.0f;
		nfg->hyperspace_z = fg->hyp_z / 160.0f;

		nfg->start1_enabled = (fg->start1_enabled != 0);
		nfg->start2_enabled = (fg->start2_enabled != 0);
		nfg->start3_enabled = (fg->start3_enabled != 0);
		nfg->waypoint1_enabled = (fg->wp1_enabled != 0);
		nfg->waypoint2_enabled = (fg->wp2_enabled != 0);
		nfg->waypoint3_enabled = (fg->wp3_enabled != 0);
		nfg->hyperspace_enabled = (fg->hyp_enabled != 0);

		switch(fg->formation) {
			case 0:
				nfg->formation = XWMFormation::f_Vic;
				break;
			case 1:
				nfg->formation = XWMFormation::f_Finger_Four;
				break;
			case 2:
				nfg->formation = XWMFormation::f_Line_Astern;
				break;
			case 3:
				nfg->formation = XWMFormation::f_Line_Abreast;
				break;
			case 4:
				nfg->formation = XWMFormation::f_Echelon_Right;
				break;
			case 5:
				nfg->formation = XWMFormation::f_Echelon_Left;
				break;
			case 6:
				nfg->formation = XWMFormation::f_Double_Astern;
				break;
			case 7:
				nfg->formation = XWMFormation::f_Diamond;
				break;
			case 8:
				nfg->formation = XWMFormation::f_Stacked;
				break;
			case 9:
				nfg->formation = XWMFormation::f_Spread;
				break;
			case 10:
				nfg->formation = XWMFormation::f_Hi_Lo;
				break;
			case 11:
				nfg->formation = XWMFormation::f_Spiral;
				break;
			default:
				return false;
		}

		nfg->playerPos = fg->player_pos;

		switch(fg->craft_ai) {
			case 0:
				nfg->craftAI = XWMCraftAI::ai_Rookie;
				break;
			case 1:
				nfg->craftAI = XWMCraftAI::ai_Officer;
				break;
			case 2:
				nfg->craftAI = XWMCraftAI::ai_Veteran;
				break;
			case 3:
				nfg->craftAI = XWMCraftAI::ai_Ace;
				break;
			case 4:
				nfg->craftAI = XWMCraftAI::ai_Top_Ace;
				break;
			default:
				return false;
		}

		switch(fg->order) {
			case 0:
				nfg->craftOrder = XWMCraftOrder::o_Hold_Steady;
				break;
			case 1:
				nfg->craftOrder = XWMCraftOrder::o_Fly_Home;
				break;
			case 2:
				nfg->craftOrder = XWMCraftOrder::o_Circle_And_Ignore;
				break;
			case 3:
				nfg->craftOrder = XWMCraftOrder::o_Fly_Once_And_Ignore;
				break;
			case 4:
				nfg->craftOrder = XWMCraftOrder::o_Circle_And_Evade;
				break;
			case 5:
				nfg->craftOrder = XWMCraftOrder::o_Fly_Once_And_Evade;
				break;
			case 6:
				nfg->craftOrder = XWMCraftOrder::o_Close_Escort;
				break;
			case 7:
				nfg->craftOrder = XWMCraftOrder::o_Loose_Escort;
				break;
			case 8:
				nfg->craftOrder = XWMCraftOrder::o_Attack_Escorts;
				break;
			case 9:
				nfg->craftOrder = XWMCraftOrder::o_Attack_Pri_And_Sec_Targets;
				break;
			case 10:
				nfg->craftOrder = XWMCraftOrder::o_Attack_Enemies;
				break;
			case 11:
				nfg->craftOrder = XWMCraftOrder::o_Rendezvous;
				break;
			case 12:
				nfg->craftOrder = XWMCraftOrder::o_Disabled;
				break;
			case 13:
				nfg->craftOrder = XWMCraftOrder::o_Board_To_Deliver;
				break;
			case 14:
				nfg->craftOrder = XWMCraftOrder::o_Board_To_Take;
				break;
			case 15:
				nfg->craftOrder = XWMCraftOrder::o_Board_To_Exchange;
				break;
			case 16:
				nfg->craftOrder = XWMCraftOrder::o_Board_To_Capture;
				break;
			case 17:
				nfg->craftOrder = XWMCraftOrder::o_Board_To_Destroy;
				break;
			case 18:
				nfg->craftOrder = XWMCraftOrder::o_Disable_Pri_And_Sec_Targets;
				break;
			case 19:
				nfg->craftOrder = XWMCraftOrder::o_Disable_All;
				break;
			case 20:
				nfg->craftOrder = XWMCraftOrder::o_Attack_Transports;
				break;
			case 21:
				nfg->craftOrder = XWMCraftOrder::o_Attack_Freighters;
				break;
			case 22:
				nfg->craftOrder = XWMCraftOrder::o_Attack_Starships;
				break;
			case 23:
				nfg->craftOrder = XWMCraftOrder::o_Attack_Satellites_And_Mines;
				break;
			case 24:
				nfg->craftOrder = XWMCraftOrder::o_Disable_Freighters;
				break;
			case 25:
				nfg->craftOrder = XWMCraftOrder::o_Disable_Starships;
				break;
			case 26:
				nfg->craftOrder = XWMCraftOrder::o_Starship_Sit_And_Fire;
				break;
			case 27:
				nfg->craftOrder = XWMCraftOrder::o_Starship_Fly_Dance;
				break;
			case 28:
				nfg->craftOrder = XWMCraftOrder::o_Starship_Circle;
				break;
			case 29:
				nfg->craftOrder = XWMCraftOrder::o_Starship_Await_Return;
				break;
			case 30:
				nfg->craftOrder = XWMCraftOrder::o_Starship_Await_Launch;
				break;
			case 31:
				nfg->craftOrder = XWMCraftOrder::o_Starship_Await_Boarding;
				break;
			case 32:
				// XXX Used by T-FORCE 1 in LEIA.XWI
				nfg->craftOrder = XWMCraftOrder::o_Attack_Enemies;
				break;
			default:
				return false;
		}

		nfg->dockTime = fg->dock_time_or_throttle;
		nfg->Throttle = fg->dock_time_or_throttle;

		switch(fg->craft_markings1) {
			case 0:
				nfg->craftColor = XWMCraftColor::c_Red;
				break;
			case 1:
				nfg->craftColor = XWMCraftColor::c_Gold;
				break;
			case 2:
				nfg->craftColor = XWMCraftColor::c_Blue;
				break;
			default:
				return false;
		}

		nfg->craftMarkings = fg->craft_markings2;

		switch(fg->objective) {
			case 0:
				nfg->objective = XWMObjective::o_None;
				break;
			case 1:
				nfg->objective = XWMObjective::o_All_Destroyed;
				break;
			case 2:
				nfg->objective = XWMObjective::o_All_Survive;
				break;
			case 3:
				nfg->objective = XWMObjective::o_All_Captured;
				break;
			case 4:
				nfg->objective = XWMObjective::o_All_Docked;
				break;
			case 5:
				nfg->objective = XWMObjective::o_Special_Craft_Destroyed;
				break;
			case 6:
				nfg->objective = XWMObjective::o_Special_Craft_Survive;
				break;
			case 7:
				nfg->objective = XWMObjective::o_Special_Craft_Captured;
				break;
			case 8:
				nfg->objective = XWMObjective::o_Special_Craft_Docked;
				break;
			case 9:
				nfg->objective = XWMObjective::o_50_Percent_Destroyed;
				break;
			case 10:
				nfg->objective = XWMObjective::o_50_Percent_Survive;
				break;
			case 11:
				nfg->objective = XWMObjective::o_50_Percent_Captured;
				break;
			case 12:
				nfg->objective = XWMObjective::o_50_Percent_Docked;
				break;
			case 13:
				nfg->objective = XWMObjective::o_All_Identified;
				break;
			case 14:
				nfg->objective = XWMObjective::o_Special_Craft_Identifed;
				break;
			case 15:
				nfg->objective = XWMObjective::o_50_Percent_Identified;
				break;
			case 16:
				nfg->objective = XWMObjective::o_Arrive;
				break;
			default:
				return false;
		}

		// XXX LEVEL1.XWI seems to set primaryTarget to junk
		if (nfg->objective == XWMObjective::o_None) {
			nfg->primaryTarget = -1;
			nfg->secondaryTarget = -1;
		} else {
			nfg->primaryTarget = fg->primary_target;
			nfg->secondaryTarget = fg->secondary_target;
		}

		assert(nfg->primaryTarget == -1 || nfg->primaryTarget < h->number_of_flight_groups);
		assert(nfg->secondaryTarget == -1 || nfg->secondaryTarget < h->number_of_flight_groups);

		m->flightgroups.push_back(*nfg);
	}

	return true;
}
