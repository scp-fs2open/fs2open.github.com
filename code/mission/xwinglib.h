
enum class XWMFlightGroupType : short
{
	fg_None = 0,
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
	fg_Freighter,
	fg_Calamari_Cruiser,
	fg_Nebulon_B_Frigate,
	fg_Corellian_Corvette,
	fg_Imperial_Star_Destroyer,
	fg_TIE_Advanced,
	fg_B_Wing
};

enum class XWMCraftStatus : short
{
	cs_normal = 0,
	cs_no_missiles,
	cs_half_missiles,
	cs_no_shields
};

enum class XWMCraftIFF : short
{
	iff_default = 0,
	iff_rebel,
	iff_imperial,
	iff_neutral
};

enum class XWMArrivalEvent : short
{
	ae_mission_start = 0,
	ae_afg_arrived,
	ae_afg_destroyed,
	ae_afg_attacked,
	ae_afg_captured,
	ae_afg_identified,
	ae_afg_disabled
};

enum class XWMFormation : short
{
	f_Vic = 0,
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
};

enum class XWMCraftAI : short
{
	ai_Rookie = 0,
	ai_Officer,
	ai_Veteran,
	ai_Ace,
	ai_Top_Ace
};

enum class XWMCraftOrder : short
{
	o_Hold_Steady = 0,
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
	o_Attack_Satellites_And_Mines,
	o_Disable_Freighters,
	o_Disable_Starships,
	o_Starship_Sit_And_Fire,
	o_Starship_Fly_Dance,
	o_Starship_Circle,
	o_Starship_Await_Return,
	o_Starship_Await_Launch,
	o_Starship_Await_Boarding
};

enum class XWMCraftColor : short
{
	c_Red = 0,
	c_Gold,
	c_Blue
};

enum class XWMObjective : short
{
	o_None = 0,
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
};


class XWMFlightGroup
{
public:

	std::string designation;
	std::string cargo;
	std::string specialCargo;

	int specialShipNumber;

	XWMFlightGroupType flightGroupType;

	XWMCraftIFF craftIFF;

	XWMCraftStatus craftStatus;

	int numberInWave;
	int numberOfWaves;

	XWMArrivalEvent arrivalEvent;

	int arrivalDelay;

	int arrivalFlightGroup;

	int mothership;

	bool arriveByHyperspace;
	bool departByHyperspace;

	float start1_x, start1_y, start1_z;
	float start2_x, start2_y, start2_z;
	float start3_x, start3_y, start3_z;
	float waypoint1_x, waypoint1_y, waypoint1_z;
	float waypoint2_x, waypoint2_y, waypoint2_z;
	float waypoint3_x, waypoint3_y, waypoint3_z;
	float hyperspace_x, hyperspace_y, hyperspace_z;

	bool start1_enabled;
	bool start2_enabled;
	bool start3_enabled;
	bool waypoint1_enabled;
	bool waypoint2_enabled;
	bool waypoint3_enabled;
	bool hyperspace_enabled;

	XWMFormation formation;

	int playerPos;

	XWMCraftAI craftAI;

	XWMCraftOrder craftOrder;

	int dockTime;
	int Throttle;

	XWMCraftColor craftColor;

	short craftMarkings;

	XWMObjective objective;

	int primaryTarget;
	int secondaryTarget;
};

class XWMObject
{
public:

};


enum class XWMEndEvent : short
{
	ev_rescued = 0,
	ev_captured,
	ev_cleared_laser_turrets,
	ev_hit_exhaust_port
};

enum class XWMMissionLocation : short
{
	ml_deep_space = 0,
	ml_death_star
};

class XWingMission
{
public:

	static int arrival_delay_to_seconds(int delay);
	static bool load(XWingMission *xwim, const char *data);

	int missionTimeLimit;
	XWMEndEvent endEvent;
	short rnd_seed;   // Not used
	XWMMissionLocation missionLocation;

	std::string completionMsg1;
	std::string completionMsg2;
	std::string completionMsg3;

	std::vector<XWMFlightGroup> flightgroups;
	std::vector<XWMObject> objects;
};
