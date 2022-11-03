#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <assert.h>
#include "xwinglib.h"

XWingMission::XWingMission()
{
}

#pragma pack(push, 1)
struct xwi_header {
	short version;
	short mission_time_limit;
	short end_event;
	short unknown1;
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
	short unknown1;
	short unknown2;
	short unknown3;
	short unknown4;
	short unknown5;
	short unknown6;
	short unknown7;
	short formation;
	short player_pos;
	short craft_ai;
	short order;
	short dock_time_or_throttle;
	short craft_color;
	short unknown8;
	short objective;
	short primary_target;
	short secondary_target;
};
#pragma pack(pop)

XWingMission *XWingMission::load(const char *data)
{
	struct xwi_header *h = (struct xwi_header *)data;
	if (h->version != 2)
		return NULL;

	XWingMission *m = new XWingMission();

	m->missionTimeLimit = h->mission_time_limit;

	switch (h->end_event) {
		case 0:
			m->endEvent = XWingMission::ev_rescued;
			break;
		case 1:
			m->endEvent = XWingMission::ev_captured;
			break;
		case 5:
			m->endEvent = XWingMission::ev_hit_exhaust_port;
			break;
		default:
			assert(false);
	}

	m->unknown1 = h->unknown1;
	
	switch(h->mission_location) {
		case 0:
			m->missionLocation = XWingMission::ml_deep_space;
			break;
		case 1:
			m->missionLocation = XWingMission::ml_death_star;
			if (m->endEvent == XWingMission::ev_captured)
				m->endEvent = XWingMission::ev_cleared_laser_turrets;
			break;
		default:
			assert(false);
	}

	m->completionMsg1 = h->completion_msg_1;
	m->completionMsg2 = h->completion_msg_2;
	m->completionMsg3 = h->completion_msg_3;

	for (int n = 0; n < h->number_of_flight_groups; n++) {
		struct xwi_flightgroup *fg = (struct xwi_flightgroup *)(data + sizeof(struct xwi_header) + sizeof(struct xwi_flightgroup) * n);
		XWMFlightGroup *nfg = new XWMFlightGroup();

		nfg->designation = fg->designation;
		nfg->cargo = fg->cargo;
		nfg->specialCargo = fg->special_cargo;
		nfg->specialShipNumber = fg->special_ship_number;

		switch(fg->flight_group_type) {
			case 0:
				nfg->flightGroupType = XWMFlightGroup::fg_None;
				break;
			case 1:
				nfg->flightGroupType = XWMFlightGroup::fg_X_Wing;
				break;
			case 2:
				nfg->flightGroupType = XWMFlightGroup::fg_Y_Wing;
				break;
			case 3:
				nfg->flightGroupType = XWMFlightGroup::fg_A_Wing;
				break;
			case 4:
				nfg->flightGroupType = XWMFlightGroup::fg_TIE_Fighter;
				break;
			case 5:
				nfg->flightGroupType = XWMFlightGroup::fg_TIE_Interceptor;
				break;
			case 6:
				nfg->flightGroupType = XWMFlightGroup::fg_TIE_Bomber;
				break;
			case 7:
				nfg->flightGroupType = XWMFlightGroup::fg_Gunboat;
				break;
			case 8:
				nfg->flightGroupType = XWMFlightGroup::fg_Transport;
				break;
			case 9:
				nfg->flightGroupType = XWMFlightGroup::fg_Shuttle;
				break;
			case 10:
				nfg->flightGroupType = XWMFlightGroup::fg_Tug;
				break;
			case 11:
				nfg->flightGroupType = XWMFlightGroup::fg_Container;
				break;
			case 12:
				nfg->flightGroupType = XWMFlightGroup::fg_Frieghter;
				break;
			case 13:
				nfg->flightGroupType = XWMFlightGroup::fg_Calamari_Cruiser;
				break;
			case 14:
				nfg->flightGroupType = XWMFlightGroup::fg_Nebulon_B_Frigate;
				break;
			case 15:
				nfg->flightGroupType = XWMFlightGroup::fg_Corellian_Corvette;
				break;
			case 16:
				nfg->flightGroupType = XWMFlightGroup::fg_Imperial_Star_Destroyer;
				break;
			case 17:
				nfg->flightGroupType = XWMFlightGroup::fg_TIE_Advanced;
				break;
			case 18:
				nfg->flightGroupType = XWMFlightGroup::fg_B_Wing;
				break;
			default:
				assert(false);
		}

		switch(fg->craft_iff) {
			case 0:
				nfg->craftIFF = XWMFlightGroup::iff_default;
				break;
			case 1:
				nfg->craftIFF = XWMFlightGroup::iff_rebel;
				break;
			case 2:
				nfg->craftIFF = XWMFlightGroup::iff_imperial;
				break;
			case 3:
				nfg->craftIFF = XWMFlightGroup::iff_neutral;
				break;
		}

		switch(fg->craft_status) {
			case 0:
				nfg->craftStatus = XWMFlightGroup::cs_normal;
				break;
			case 1:
				nfg->craftStatus = XWMFlightGroup::cs_no_missiles;
				break;
			case 2:
				nfg->craftStatus = XWMFlightGroup::cs_half_missiles;
				break;
			case 3:
				nfg->craftStatus = XWMFlightGroup::cs_no_shields;
				break;
			case 4:
				// XXX Used by CENTURY 1 in CONVOY2.XWI
				nfg->craftStatus = XWMFlightGroup::cs_normal;
				break;
			case 10:
				// XXX Used by Unnammed B-Wing group in DESUPPLY.XWI
				nfg->craftStatus = XWMFlightGroup::cs_normal;
				break;
			case 11:
				// XXX Used by PROTOTYPE 2 in T5H1WB.XWI
				nfg->craftStatus = XWMFlightGroup::cs_no_missiles;
				break;
			case 12:
				// XXX Used by RED 1 in T5M19MB.XWI
				nfg->craftStatus = XWMFlightGroup::cs_half_missiles;
				break;
			default:
				assert(false);
		}

		nfg->numberInWave = fg->number_in_wave;
		nfg->numberOfWaves = fg->number_of_waves + 1;

		switch(fg->arrival_event) {
			case 0:
				nfg->arrivalEvent = XWMFlightGroup::ae_mission_start;
				break;
			case 1:
				nfg->arrivalEvent = XWMFlightGroup::ae_afg_arrives;
				break;
			case 2:
				nfg->arrivalEvent = XWMFlightGroup::ae_afg_destroyed;
				break;
			case 3:
				nfg->arrivalEvent = XWMFlightGroup::ae_afg_attacked;
				break;
			case 4:
				nfg->arrivalEvent = XWMFlightGroup::ae_afg_boarded;
				break;
			case 5:
				nfg->arrivalEvent = XWMFlightGroup::ae_afg_identified;
				break;
			case 6:
				nfg->arrivalEvent = XWMFlightGroup::ae_afg_disabled;
				break;
			default:
				assert(false);
		}

		// TODO: this strange encoding
		nfg->arrivalDelay = fg->arrival_delay;

		nfg->arrivalFlightGroup = fg->arrival_flight_group;
		nfg->mothership = fg->mothership;
		assert(nfg->mothership == -1 || nfg->mothership < h->number_of_flight_groups);
		nfg->arriveByHyperspace = fg->arrive_by_hyperspace == 0 ? false : true;
		nfg->departByHyperspace = fg->depart_by_hyperspace == 0 ? false : true;

		nfg->start1_x = fg->start1_x / 160.0f;
		nfg->start2_x = fg->start2_x / 160.0f;
		nfg->start3_x = fg->start3_x / 160.0f;
		nfg->start1_y = fg->start1_y / 160.0f;
		nfg->start2_y = fg->start2_y / 160.0f;
		nfg->start3_y = fg->start3_y / 160.0f;
		nfg->start1_z = fg->start1_z / 160.0f;
		nfg->start2_z = fg->start2_z / 160.0f;
		nfg->start3_z = fg->start3_z / 160.0f;

		nfg->wp1_x = fg->wp1_x / 160.0f;
		nfg->wp2_x = fg->wp2_x / 160.0f;
		nfg->wp3_x = fg->wp3_x / 160.0f;
		nfg->wp1_y = fg->wp1_y / 160.0f;
		nfg->wp2_y = fg->wp2_y / 160.0f;
		nfg->wp3_y = fg->wp3_y / 160.0f;
		nfg->wp1_z = fg->wp1_z / 160.0f;
		nfg->wp2_z = fg->wp2_z / 160.0f;
		nfg->wp3_z = fg->wp3_z / 160.0f;

		nfg->hyperspace_x = fg->hyp_x / 160.0f;
		nfg->hyperspace_y = fg->hyp_y / 160.0f;
		nfg->hyperspace_z = fg->hyp_z / 160.0f;

		nfg->unknown1 = fg->unknown1;
		nfg->unknown2 = fg->unknown2;
		nfg->unknown3 = fg->unknown3;
		nfg->unknown4 = fg->unknown4;
		nfg->unknown5 = fg->unknown5;
		nfg->unknown6 = fg->unknown6;
		nfg->unknown7 = fg->unknown7;

		switch(fg->formation) {
			case 0:
				nfg->formation = XWMFlightGroup::f_Vic;
				break;
			case 1:
				nfg->formation = XWMFlightGroup::f_Finger_Four;
				break;
			case 2:
				nfg->formation = XWMFlightGroup::f_Line_Astern;
				break;
			case 3:
				nfg->formation = XWMFlightGroup::f_Line_Abreast;
				break;
			case 4:
				nfg->formation = XWMFlightGroup::f_Echelon_Right;
				break;
			case 5:
				nfg->formation = XWMFlightGroup::f_Echelon_Left;
				break;
			case 6:
				nfg->formation = XWMFlightGroup::f_Double_Astern;
				break;
			case 7:
				nfg->formation = XWMFlightGroup::f_Diamond;
				break;
			case 8:
				nfg->formation = XWMFlightGroup::f_Stacked;
				break;
			case 9:
				nfg->formation = XWMFlightGroup::f_Spread;
				break;
			case 10:
				nfg->formation = XWMFlightGroup::f_Hi_Lo;
				break;
			case 11:
				nfg->formation = XWMFlightGroup::f_Spiral;
				break;
			default:
				assert(false);
		}

		nfg->playerPos = fg->player_pos;

		switch(fg->craft_ai) {
			case 0:
				nfg->craftAI = XWMFlightGroup::ai_Rookie;
				break;
			case 1:
				nfg->craftAI = XWMFlightGroup::ai_Officer;
				break;
			case 2:
				nfg->craftAI = XWMFlightGroup::ai_Veteran;
				break;
			case 3:
				nfg->craftAI = XWMFlightGroup::ai_Ace;
				break;
			case 4:
				nfg->craftAI = XWMFlightGroup::ai_Top_Ace;
				break;
			default:
				assert(false);
		}

		switch(fg->order) {
			case 0:
				nfg->order = XWMFlightGroup::o_Hold_Steady;
				break;
			case 1:
				nfg->order = XWMFlightGroup::o_Fly_Home;
				break;
			case 2:
				nfg->order = XWMFlightGroup::o_Circle_And_Ignore;
				break;
			case 3:
				nfg->order = XWMFlightGroup::o_Fly_Once_And_Ignore;
				break;
			case 4:
				nfg->order = XWMFlightGroup::o_Circle_And_Evade;
				break;
			case 5:
				nfg->order = XWMFlightGroup::o_Fly_Once_And_Evade;
				break;
			case 6:
				nfg->order = XWMFlightGroup::o_Close_Escort;
				break;
			case 7:
				nfg->order = XWMFlightGroup::o_Loose_Escort;
				break;
			case 8:
				nfg->order = XWMFlightGroup::o_Attack_Escorts;
				break;
			case 9:
				nfg->order = XWMFlightGroup::o_Attack_Pri_And_Sec_Targets;
				break;
			case 10:
				nfg->order = XWMFlightGroup::o_Attack_Enemies;
				break;
			case 11:
				nfg->order = XWMFlightGroup::o_Rendezvous;
				break;
			case 12:
				nfg->order = XWMFlightGroup::o_Disabled;
				break;
			case 13:
				nfg->order = XWMFlightGroup::o_Board_To_Deliver;
				break;
			case 14:
				nfg->order = XWMFlightGroup::o_Board_To_Take;
				break;
			case 15:
				nfg->order = XWMFlightGroup::o_Board_To_Exchange;
				break;
			case 16:
				nfg->order = XWMFlightGroup::o_Board_To_Capture;
				break;
			case 17:
				nfg->order = XWMFlightGroup::o_Board_To_Destroy;
				break;
			case 18:
				nfg->order = XWMFlightGroup::o_Disable_Pri_And_Sec_Targets;
				break;
			case 19:
				nfg->order = XWMFlightGroup::o_Disable_All;
				break;
			case 20:
				nfg->order = XWMFlightGroup::o_Attack_Transports;
				break;
			case 21:
				nfg->order = XWMFlightGroup::o_Attack_Freighters;
				break;
			case 22:
				nfg->order = XWMFlightGroup::o_Attack_Starships;
				break;
			case 23:
				nfg->order = XWMFlightGroup::o_Attack_Satelites_And_Mines;
				break;
			case 24:
				nfg->order = XWMFlightGroup::o_Disable_Frieghters;
				break;
			case 25:
				nfg->order = XWMFlightGroup::o_Disable_Starships;
				break;
			case 26:
				nfg->order = XWMFlightGroup::o_Starship_Sit_And_Fire;
				break;
			case 27:
				nfg->order = XWMFlightGroup::o_Starship_Fly_Dance;
				break;
			case 28:
				nfg->order = XWMFlightGroup::o_Starship_Circle;
				break;
			case 29:
				nfg->order = XWMFlightGroup::o_Starship_Await_Return;
				break;
			case 30:
				nfg->order = XWMFlightGroup::o_Starship_Await_Launch;
				break;
			case 31:
				nfg->order = XWMFlightGroup::o_Starship_Await_Boarding;
				break;
			case 32:
				// XXX Used by T-FORCE 1 in LEIA.XWI
				nfg->order = XWMFlightGroup::o_Attack_Enemies;
				break;
			default:
				assert(false);
		}

		nfg->dockTime = fg->dock_time_or_throttle;
		nfg->Throttle = fg->dock_time_or_throttle;

		switch(fg->craft_color) {
			case 0:
				nfg->craftColor = XWMFlightGroup::c_Red;
				break;
			case 1:
				nfg->craftColor = XWMFlightGroup::c_Gold;
				break;
			case 2:
				nfg->craftColor = XWMFlightGroup::c_Blue;
				break;
			default:
				assert(false);
		}

		nfg->unknown8 = fg->unknown8;

		switch(fg->objective) {
			case 0:
				nfg->objective = XWMFlightGroup::o_None;
				break;
			case 1:
				nfg->objective = XWMFlightGroup::o_All_Destroyed;
				break;
			case 2:
				nfg->objective = XWMFlightGroup::o_All_Survive;
				break;
			case 3:
				nfg->objective = XWMFlightGroup::o_All_Captured;
				break;
			case 4:
				nfg->objective = XWMFlightGroup::o_All_Docked;
				break;
			case 5:
				nfg->objective = XWMFlightGroup::o_Special_Craft_Destroyed;
				break;
			case 6:
				nfg->objective = XWMFlightGroup::o_Special_Craft_Survive;
				break;
			case 7:
				nfg->objective = XWMFlightGroup::o_Special_Craft_Captured;
				break;
			case 8:
				nfg->objective = XWMFlightGroup::o_Special_Craft_Docked;
				break;
			case 9:
				nfg->objective = XWMFlightGroup::o_50_Percent_Destroyed;
				break;
			case 10:
				nfg->objective = XWMFlightGroup::o_50_Percent_Survive;
				break;
			case 11:
				nfg->objective = XWMFlightGroup::o_50_Percent_Captured;
				break;
			case 12:
				nfg->objective = XWMFlightGroup::o_50_Percent_Docked;
				break;
			case 13:
				nfg->objective = XWMFlightGroup::o_All_Identified;
				break;
			case 14:
				nfg->objective = XWMFlightGroup::o_Special_Craft_Identifed;
				break;
			case 15:
				nfg->objective = XWMFlightGroup::o_50_Percent_Identified;
				break;
			case 16:
				nfg->objective = XWMFlightGroup::o_Arrive;
				break;
			default:
				assert(false);
		}

		// XXX LEVEL1.XWI seems to set primaryTarget to junk
		if (nfg->objective == XWMFlightGroup::o_None) {
			nfg->primaryTarget = -1;
			nfg->secondaryTarget = -1;
		} else {
			nfg->primaryTarget = fg->primary_target;
			nfg->secondaryTarget = fg->secondary_target;
		}

		assert(nfg->primaryTarget == -1 || nfg->primaryTarget < h->number_of_flight_groups);
		assert(nfg->secondaryTarget == -1 || nfg->secondaryTarget < h->number_of_flight_groups);

		m->flightgroups.push_back(nfg);
	}

	return m;
}

#ifdef TEST_XWINGLIB
int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2) {
		printf("usage: xwinglib <xwi file>\n");
		return 1;
	}

	XWingMission *m = XWingMission::load(argv[1]);

	return 0;
}
#endif

