
class XWMFlightGroup
{
public:

	std::string designation;
	std::string cargo;
	std::string specialCargo;

	int specialShipNumber;

	enum {
		fg_None,
		fg_X_Wing,
		fg_Y_Wing,
		fg_A_Wing,
		fg_TIE_Fighter,
		fg_TIE_Interceptor,
		fg_TIE_Bomber,
		fg_Gunboat,
		fg_Transport,
		fg_Shuttle,
		fg_Tug,
		fg_Container,
		fg_Frieghter,
		fg_Calamari_Cruiser,
		fg_Nebulon_B_Frigate,
		fg_Corellian_Corvette,
		fg_Imperial_Star_Destroyer,
		fg_TIE_Advanced,
		fg_B_Wing
	} flightGroupType;

	enum {
		iff_default,
		iff_rebel,
		iff_imperial,
		iff_neutral
	} craftIFF;

	enum {
		cs_normal,
		cs_no_missiles,
		cs_half_missiles,
		cs_no_shields
	} craftStatus;

	int numberInWave;
	int numberOfWaves;

	enum {
		ae_mission_start,
		ae_afg_arrives,
		ae_afg_destroyed,
		ae_afg_attacked,
		ae_afg_boarded,
		ae_afg_identified,
		ae_afg_disabled
	} arrivalEvent;

	int arrivalDelay;

	int arrivalFlightGroup;

	int mothership;

	bool arriveByHyperspace;
	bool departByHyperspace;

	// TODO: use vector class
	float start1_x, start1_y, start1_z;
	float start2_x, start2_y, start2_z;
	float start3_x, start3_y, start3_z;
	float wp1_x, wp1_y, wp1_z;
	float wp2_x, wp2_y, wp2_z;
	float wp3_x, wp3_y, wp3_z;
	float hyperspace_x, hyperspace_y, hyperspace_z;

	short unknown1;
	short unknown2;
	short unknown3;
	short unknown4;
	short unknown5;
	short unknown6;
	short unknown7;

	enum {
		f_Vic,
		f_Finger_Four,
		f_Line_Astern,
		f_Line_Abreast,
		f_Echelon_Right,
		f_Echelon_Left,
		f_Double_Astern,
		f_Diamond,
		f_Stacked,
		f_Spread,
		f_Hi_Lo,
		f_Spiral
	} formation;

	int playerPos;

	enum {
		ai_Rookie, 
		ai_Officer, 
		ai_Veteran, 
		ai_Ace, 
		ai_Top_Ace
	} craftAI;

	enum {
		o_Hold_Steady,
		o_Fly_Home,
		o_Circle_And_Ignore,
		o_Fly_Once_And_Ignore,
		o_Circle_And_Evade,
		o_Fly_Once_And_Evade,
		o_Close_Escort,
		o_Loose_Escort,
		o_Attack_Escorts,
		o_Attack_Pri_And_Sec_Targets,
		o_Attack_Enemies,
		o_Rendezvous,
		o_Disabled,
		o_Board_To_Deliver,
		o_Board_To_Take,
		o_Board_To_Exchange,
		o_Board_To_Capture,
		o_Board_To_Destroy,
		o_Disable_Pri_And_Sec_Targets,
		o_Disable_All,
		o_Attack_Transports,
		o_Attack_Freighters,
		o_Attack_Starships,
		o_Attack_Satelites_And_Mines,
		o_Disable_Frieghters,
		o_Disable_Starships,
		o_Starship_Sit_And_Fire,
		o_Starship_Fly_Dance,
		o_Starship_Circle,
		o_Starship_Await_Return,
		o_Starship_Await_Launch,
		o_Starship_Await_Boarding
	} order;

	int dockTime;
	int Throttle;

	enum {
		c_Red, 
		c_Gold, 
		c_Blue
	} craftColor;

	short unknown8;

	enum {
		o_None,
		o_All_Destroyed,
		o_All_Survive,
		o_All_Captured,
		o_All_Docked,
		o_Special_Craft_Destroyed,
		o_Special_Craft_Survive,
		o_Special_Craft_Captured,
		o_Special_Craft_Docked,
		o_50_Percent_Destroyed,
		o_50_Percent_Survive,
		o_50_Percent_Captured,
		o_50_Percent_Docked,
		o_All_Identified,
		o_Special_Craft_Identifed,
		o_50_Percent_Identified,
		o_Arrive
	} objective;

	int primaryTarget;
	int secondaryTarget;
};

class XWMObject
{
public:

};

class XWingMission
{
protected:
	XWingMission();

public:

	static 	XWingMission *load(const char *data);

	int missionTimeLimit;
	enum { ev_rescued, ev_captured, ev_cleared_laser_turrets, ev_hit_exhaust_port } endEvent;
	short unknown1;   // XXX
	enum { ml_deep_space, ml_death_star } missionLocation;
	std::string completionMsg1;
	std::string completionMsg2;
	std::string completionMsg3;
	std::vector<XWMFlightGroup*> flightgroups;
	std::vector<XWMObject*> objects;
};
