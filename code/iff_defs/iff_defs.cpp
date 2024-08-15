/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */



#include "def_files/def_files.h"
#include "hud/hud.h"
#include "iff_defs/iff_defs.h"
#include "io/timer.h"
#include "mission/missionparse.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "options/Option.h"

extern int radar_target_id_flags;

SCP_vector<iff_info> Iff_info;

int Iff_traitor;

int radar_iff_color[5][2][4];
int iff_bright_delta;
int *iff_color_brightness = &iff_bright_delta;

int IFF_COLOR_SELECTION = 0;
int IFF_COLOR_MESSAGE   = 1;
int IFF_COLOR_TAGGED    = 2;

// global only to file
SCP_vector <std::pair<color, color>> Iff_colors;		// AL 1-2-97: Create two IFF colors, regular and bright

flag_def_list rti_flags[] = {
	{ "crosshairs",			RTIF_CROSSHAIRS,	0 },
	{ "blink",				RTIF_BLINK,			0 },
	{ "pulsate",			RTIF_PULSATE,		0 },
	{ "enlarge",			RTIF_ENLARGE,		0 }
};

int Num_rti_flags = sizeof(rti_flags)/sizeof(flag_def_list);

static bool AccessiblitySupported = false;

// used by In-Game Options menu
static bool AccessibilityEnabled = false;

static auto AccessibilityOption = options::OptionBuilder<bool>("Game.IffAccessibility",
	std::pair<const char*, int>{"Accessibility IFF Colors", 1855},
	std::pair<const char*, int>{"Enables or disables IFF Accessibility color overrides", 1856})
										 .category(std::make_pair("Game", 1824))
										 .default_val(false)
										 .level(options::ExpertLevel::Advanced)
										 .bind_to_once(&AccessibilityEnabled)
										 .importance(60)
										 .finish();

/**
 * Borrowed from ship.cpp, ship_iff_init_colors
 *
 * @param is_bright Whether set to bright
 */
int iff_get_alpha_value(bool is_bright)
{
	if (is_bright == false)
		return (HUD_COLOR_ALPHA_MAX - iff_bright_delta) * 16;
	else 
		return HUD_COLOR_ALPHA_MAX * 16;
}

/**
 * Init a color and add it to the ::Iff_colors array
 *
 * @param r Red
 * @param g Green
 * @param b Blue
 *
 * @return The new IFF colour slot in ::Iff_colors array
 */
int iff_init_color(int r, int g, int b)
{
	typedef struct {
		int r;
		int g;
		int b;
	} temp_color_t;

	int i, idx;

	static SCP_vector<temp_color_t> temp_colors;

	Assert(r >= 0 && r <= 255);
	Assert(g >= 0 && g <= 255);
	Assert(b >= 0 && b <= 255);

	// find out if this color is in use
	for (i = 0; i < (int) temp_colors.size(); i++)
	{
		const temp_color_t& c = temp_colors[i];

		if (c.r == r && c.g == g && c.b == b)
			return i;
	}

	// not in use, so add a new slot
	// save the values
	temp_colors.push_back({ r, g, b });

	idx = (int) temp_colors.size() - 1;

	// init it
	Iff_colors.push_back({ color(), color() });
	gr_init_alphacolor(&Iff_colors[idx].first, r, g, b, iff_get_alpha_value(false));
	gr_init_alphacolor(&Iff_colors[idx].second, r, g, b, iff_get_alpha_value(true));

	// return the new slot
	return idx;
}

//Initialize this as the retail default - Mjn
static char traitor_name[NAME_LENGTH] = "Traitor";
static SCP_vector<SCP_vector<SCP_string>> attack_names;

struct observed_color_t {
	char iff_name[NAME_LENGTH];
	int color_index;
};

static SCP_vector<SCP_vector<observed_color_t>> observed_color_table;

static SCP_vector<SCP_vector<observed_color_t>> accessibility_observed_color_table;

void resolve_iff_data()
{
	// first get the traitor
	Iff_traitor = iff_lookup(traitor_name);
	if (Iff_traitor < 0) {
		Iff_traitor = 0;
		Warning(LOCATION,
			"Traitor IFF %s not found in iff_defs.tbl!  Defaulting to %s.\n",
			traitor_name,
			Iff_info[Iff_traitor].iff_name);
	}

	// next get the attackees and colors
	for (int cur_iff = 0; cur_iff < (int)Iff_info.size(); cur_iff++) {
		iff_info* iff = &Iff_info[cur_iff];

		// clear the iffs to be attacked
		iff->attackee_bitmask = 0;
		iff->attackee_bitmask_all_teams_at_war = 0;

		// clear the observed colors
		iff->observed_color_map.clear();

		// resolve the attacking list names
		for (const SCP_string& attacks : attack_names[cur_iff]) {
			// find out who
			int target_iff = iff_lookup(attacks.c_str());

			// valid?
			if (target_iff >= 0)
				iff->attackee_bitmask |= iff_get_mask(target_iff);
			else
				Warning(LOCATION,
					"Attack target IFF %s not found for IFF %s in iff_defs.tbl!\n",
					attacks.c_str(),
					iff->iff_name);
		}

		// resolve the observed color list names
		for (const auto& observed_color : observed_color_table[cur_iff]) {
			// find out who
			int target_iff = iff_lookup(observed_color.iff_name);

			// valid?
			if (target_iff >= 0) {
				iff->observed_color_map[target_iff] = observed_color.color_index;
			} else{
				Warning(LOCATION,
					"Observed color IFF %s not found for IFF %s in iff_defs.tbl!\n",
					observed_color.iff_name,
					iff->iff_name);
			}
		}

		// resolve the accessibility observed color list names
		for (const auto& observed_color : accessibility_observed_color_table[cur_iff]) {
			// find out who
			int target_iff = iff_lookup(observed_color.iff_name);

			// valid?
			if (target_iff >= 0) {
				iff->accessibility_observed_color_map[target_iff] = observed_color.color_index;
			} else {
				Warning(LOCATION,
					"Observed color IFF %s not found for IFF %s in iff_defs.tbl!\n",
					observed_color.iff_name,
					iff->iff_name);
			}
		}

		// resolve the all teams at war relationships
		if (iff->flags & IFFF_EXEMPT_FROM_ALL_TEAMS_AT_WAR) {
			// exempt, so use standard attacks
			iff->attackee_bitmask_all_teams_at_war = iff->attackee_bitmask;
		} else {
			// nonexempt, so build bitmask of all other nonexempt teams
			for (int other_iff = 0; other_iff < (int)Iff_info.size(); other_iff++) {
				// skip myself (unless I attack myself normally)
				if ((other_iff == cur_iff) && !iff_x_attacks_y(cur_iff, cur_iff))
					continue;

				// skip anyone exempt
				if (Iff_info[other_iff].flags & IFFF_EXEMPT_FROM_ALL_TEAMS_AT_WAR)
					continue;

				// add everyone else
				iff->attackee_bitmask_all_teams_at_war |= iff_get_mask(other_iff);
			}
		}
	}
}

static iff_info* get_iff_pointer(const char* name)
{
	for (int i = 0; i < (int)Iff_info.size(); i++) {
		if (!stricmp(name, Iff_info[i].iff_name)) {
			return &Iff_info[i];
		}
	}

	// Didn't find anything.
	return nullptr;
}

static int get_iff_position(const char* name)
{
	for (int i = 0; i < (int)Iff_info.size(); i++) {
		if (!stricmp(name, Iff_info[i].iff_name)) {
			return i;
		}
	}

	return -1;
}

/**
 * Parse the table
 */
void parse_iff_table(const char* filename)
{
	int i, j, k;

	iff_info ifft;
	iff_info* iffp;
	int cur_iff;
	bool create_if_not_found = true;

	try
	{
		if (!Parsing_modular_table) {
			// Goober5000 - if table doesn't exist, use the default table
			if (cf_exists_full(filename, CF_TYPE_TABLES))
				read_file_text(filename, CF_TYPE_TABLES);
			else
				read_file_text_from_default(defaults_get_file("iff_defs.tbl"));
		} else {
			read_file_text(filename, CF_TYPE_TABLES);
		}

		reset_parse();

		// parse the table --------------------------------------------------------

		required_string("#IFFs");

		// get the traitor
		if (optional_string("$Traitor IFF:")) {
			stuff_string(traitor_name, F_NAME, NAME_LENGTH);
		}

		if (optional_string("$Accessibility Supported:")) {
			stuff_boolean(&AccessiblitySupported);
		}

		int rgb[3];

		// check if alternate colours are wanted to be used for these
		// Marks various stuff... like asteroids
		if ((optional_string("$Selection Color:")) || (optional_string("$Selection Colour:")))
		{
			stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
			IFF_COLOR_SELECTION = iff_init_color(rgb[0], rgb[1], rgb[2]);
		}
		else if (!Parsing_modular_table)
			IFF_COLOR_SELECTION = iff_init_color(0xff, 0xff, 0xff);

		// Marks the ship currently saying something
		if ((optional_string("$Message Color:")) || (optional_string("$Message Colour:")))
		{
			stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
			IFF_COLOR_MESSAGE = iff_init_color(rgb[0], rgb[1], rgb[2]);
		}
		else if (!Parsing_modular_table)
			IFF_COLOR_MESSAGE = iff_init_color(0x7f, 0x7f, 0x7f);

		// Marks the tagged ships
		if ((optional_string("$Tagged Color:")) || (optional_string("$Tagged Colour:")))
		{
			stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
			IFF_COLOR_TAGGED = iff_init_color(rgb[0], rgb[1], rgb[2]);
		}
		else if (!Parsing_modular_table)
			IFF_COLOR_TAGGED = iff_init_color(0xff, 0xff, 0x00);

		// init radar blips colour table
		int a_bright, a_dim;
		bool alternate_blip_color = false;
		for (i = 0; i < 5; i++)
		{
			for (j = 0; j < 2; j++)
			{
				for (k = 0; k < 3; k++)
				{
					radar_iff_color[i][j][k] = -1;
				}
			}
		}

		// if the bright/dim scaling is wanted to be changed
		if (optional_string("$Dimmed IFF brightness:"))
		{
			int dim_iff_brightness;
			stuff_int(&dim_iff_brightness);
			Assert(dim_iff_brightness >= 0 && dim_iff_brightness <= HUD_COLOR_ALPHA_MAX);
			*iff_color_brightness = dim_iff_brightness;
		}
		else
			*iff_color_brightness = 4;

		// alternate = use same method as with ship blips
		// retail = use 1/2 intensities
		if (optional_string("$Use Alternate Blip Coloring:") || optional_string("$Use Alternate Blip Colouring:"))
		{
			stuff_boolean(&alternate_blip_color);
		}

		// Parse blip colours, their order is hardcoded.
		if ((optional_string("$Missile Blip Color:")) || (optional_string("$Missile Blip Colour:")))
		{
			stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
			for (i = 0; i < 3; i++)
			{
				Assert(rgb[i] >= 0 && rgb[i] <= 255);
				radar_iff_color[0][1][i] = rgb[i];
				radar_iff_color[0][0][i] = rgb[i] / 2;
			}
		}

		if ((optional_string("$Navbuoy Blip Color:")) || (optional_string("$Navbuoy Blip Colour:")))
		{
			stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
			for (i = 0; i < 3; i++)
			{
				Assert(rgb[i] >= 0 && rgb[i] <= 255);
				radar_iff_color[1][1][i] = rgb[i];
				radar_iff_color[1][0][i] = rgb[i] / 2;
			}
		}

		if ((optional_string("$Warping Blip Color:")) || (optional_string("$Warping Blip Colour:")))
		{
			stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
			for (i = 0; i < 3; i++)
			{
				Assert(rgb[i] >= 0 && rgb[i] <= 255);
				radar_iff_color[2][1][i] = rgb[i];
				radar_iff_color[2][0][i] = rgb[i] / 2;
			}
		}

		if ((optional_string("$Node Blip Color:")) || (optional_string("$Node Blip Colour:")))
		{
			stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
			for (i = 0; i < 3; i++)
			{
				Assert(rgb[i] >= 0 && rgb[i] <= 255);
				radar_iff_color[3][1][i] = rgb[i];
				radar_iff_color[3][0][i] = rgb[i] / 2;
			}
		}

		if ((optional_string("$Tagged Blip Color:")) || (optional_string("$Tagged Blip Colour:")))
		{
			stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
			for (i = 0; i < 3; i++)
			{
				Assert(rgb[i] >= 0 && rgb[i] <= 255);
				radar_iff_color[4][1][i] = rgb[i];
				radar_iff_color[4][0][i] = rgb[i] / 2;
			}
		}

		if (alternate_blip_color == true)
		{
			a_bright = iff_get_alpha_value(true);
			a_dim = iff_get_alpha_value(false);
			for (i = 0; i < 5; i++)
			{
				if (radar_iff_color[i][0][0] >= 0)
				{
					for (j = 0; j < 3; j++)
					{
						radar_iff_color[i][0][j] = radar_iff_color[i][1][j];
					}

					radar_iff_color[i][1][3] = a_bright;
					radar_iff_color[i][0][3] = a_dim;
				}
			}
		}
		else
		{
			for (i = 0; i < 5; i++)
			{
				if (radar_iff_color[i][0][0] >= 0)
				{
					radar_iff_color[i][0][3] = 255;
					radar_iff_color[i][1][3] = 255;
				}
			}
		}

		if (optional_string("$Radar Target ID Flags:")) {
			parse_string_flag_list((int*)&radar_target_id_flags, rti_flags, Num_rti_flags);
			if (optional_string("+reset"))
				radar_target_id_flags = 0;
		}

		// begin reading data
		while (required_string_either("#End", "$IFF Name:"))
		{

			// get the iff name
			required_string("$IFF Name:");
			stuff_string(ifft.iff_name, F_NAME, NAME_LENGTH);
			mprintf(("IFFs got to name %s\n", ifft.iff_name));
			if (optional_string("+nocreate")) {
				if (!Parsing_modular_table) {
					error_display(0, "+nocreate flag used for iff in non-modular table\n");
				}
				create_if_not_found = false;
			}

			// Does this iff exist already?
			// If so, load this new info into it
			iffp = get_iff_pointer(ifft.iff_name);
			if (iffp != nullptr) {
				if (!Parsing_modular_table) {
					error_display(1,
						"Error:  IFF %s already exists.  All IFF names must be unique.",
						ifft.iff_name);
				}
			} else {
				// Don't create iff if it has +nocreate and is in a modular table.
				if (!create_if_not_found && Parsing_modular_table) {
					if (!skip_to_start_of_string_either("$IFF Name:", "#End")) {
						error_display(1, "Missing [#End] or [$IFF Name] after IFF %s", ifft.iff_name);
					}
					continue;
				}
				Iff_info.push_back(ifft);
				attack_names.emplace_back();
				observed_color_table.emplace_back();
				accessibility_observed_color_table.emplace_back();
				iffp = &Iff_info[Iff_info.size() - 1];
				//Initialize this to white for new IFFs
				iffp->color_index = iff_init_color(255, 255, 255);
				// Initialize this to white for new IFFs
				iffp->accessibility_color_index = iff_init_color(255, 255, 255);
				//Make sure flags are reset for new IFFs
				iffp->default_parse_flags.reset();
			}

			cur_iff = get_iff_position(ifft.iff_name);

			// get the iff color
			if (optional_string_either("$Colour:", "$Color:") != -1) {
				stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
				iffp->color_index = iff_init_color(rgb[0], rgb[1], rgb[2]);
			}

			// get the accessiblity iff color
			if (optional_string_either("$Accessibility Colour:", "$Accessibility Color:") != -1) {
				stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
				iffp->accessibility_color_index = iff_init_color(rgb[0], rgb[1], rgb[2]);
			}


			// get relationships between IFFs -------------------------------------

			// get the list of iffs attacked
			if (optional_string("$Attacks:"))
				stuff_string_list(attack_names[cur_iff]);

			// get the list of observed colors
			while (optional_string("+Sees"))
			{
				observed_color_table[cur_iff].emplace_back();
				// get iff observed
				stuff_string_until(observed_color_table[cur_iff].back().iff_name, "As:", NAME_LENGTH);
				required_string("As:");

				// get color observed
				stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
				observed_color_table[cur_iff].back().color_index = iff_init_color(rgb[0], rgb[1], rgb[2]);
			}

			// get the list of accessibility observed colors
			while (optional_string("+Accessibility Sees")) {
				accessibility_observed_color_table[cur_iff].emplace_back();
				// get iff observed
				stuff_string_until(accessibility_observed_color_table[cur_iff].back().iff_name, "As:", NAME_LENGTH);
				required_string("As:");

				// get color observed
				stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
				accessibility_observed_color_table[cur_iff].back().color_index = iff_init_color(rgb[0], rgb[1], rgb[2]);
			}

			// get F3 override
			iffp->hotkey_team = IFF_hotkey_team::Default;
			if (optional_string("+Hotkey Team:"))
			{
				char temp[NAME_LENGTH];
				stuff_string(temp, F_NAME, NAME_LENGTH);

				if (!stricmp(temp, "Friendly"))
					iffp->hotkey_team = IFF_hotkey_team::Friendly;
				else if (!stricmp(temp, "Hostile") || !stricmp(temp, "Enemy"))
					iffp->hotkey_team = IFF_hotkey_team::Hostile;
				else if (!stricmp(temp, "None"))
					iffp->hotkey_team = IFF_hotkey_team::None;
				else
					Warning(LOCATION, "Unrecognized +Hotkey Tean: %s\n", temp);
			}


			// get flags ----------------------------------------------------------

			// get iff flags
			iffp->flags = 0;
			if (optional_string("$Flags:"))
			{
				SCP_vector<SCP_string> flag_strings;
				stuff_string_list(flag_strings);

				for (auto &item: flag_strings)
				{
					auto flag_string = item.c_str();

					if (!stricmp(NOX("support allowed"), flag_string))
						iffp->flags |= IFFF_SUPPORT_ALLOWED;
					else if (!stricmp(NOX("exempt from all teams at war"), flag_string))
						iffp->flags |= IFFF_EXEMPT_FROM_ALL_TEAMS_AT_WAR;
					else if (!stricmp(NOX("orders hidden"), flag_string))
						iffp->flags |= IFFF_ORDERS_HIDDEN;
					else if (!stricmp(NOX("orders shown"), flag_string))
						iffp->flags |= IFFF_ORDERS_SHOWN;
					else if (!stricmp(NOX("wing name hidden"), flag_string))
						iffp->flags |= IFFF_WING_NAME_HIDDEN;
					else
						Warning(LOCATION, "Bogus string in iff flags: %s\n", flag_string);
				}
			}

            // get default ship flags
            if (optional_string("$Default Ship Flags:"))
            {
                parse_string_flag_list(iffp->default_parse_flags, Parse_object_flags, Num_parse_object_flags, nullptr);
            }

            // again, for compatibility reasons
            if (optional_string("$Default Ship Flags2:"))
            {
                parse_string_flag_list(iffp->default_parse_flags, Parse_object_flags, Num_parse_object_flags, nullptr);
            }

			// this is cleared between each level but let's just set it here for thoroughness
			iffp->ai_good_rearm_timestamp = TIMESTAMP::invalid();
			iffp->ai_bad_rearm_timestamp = TIMESTAMP::invalid();
		}

		required_string("#End");
		
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", "iff_defs.tbl", e.what()));
		return;
	}
}

void iff_init()
{
	// first parse the default table
	parse_iff_table("iff_defs.tbl");

	// parse any modular tables
	parse_modular_table("*-iff.tbm", parse_iff_table);

	if (!AccessiblitySupported) {
		options::OptionsManager::instance()->removeOption(AccessibilityOption);
		AccessibilityEnabled = false;
	}

	// now resolve the relationships
	resolve_iff_data();
}

/**
 * Find the iff name
 *
 * @param iff_name Pointer to name as a string
 * @return Index into ::Iff_info array
 */
int iff_lookup(const char *iff_name)
{
	// bogus
	Assert(iff_name);

	if(iff_name == NULL)
		return -1;

	for (int i = 0; i < (int) Iff_info.size(); i++)
		if (!stricmp(iff_name, Iff_info[i].iff_name))
			return i;

	return -1;
}

/**
 * Get the mask, taking All Teams At War into account
 *
 * @param attacker_team Team of attacker
 * @return Bitmask
 */
int iff_get_attackee_mask(int attacker_team)
{
	Assertion(SCP_vector_inbounds(Iff_info, attacker_team), "Attempted to get the attackee mask of an attacker team that is out of bounds");
	if (!SCP_vector_inbounds(Iff_info, attacker_team))
		return 0;

	//	All teams attack all other teams.
	if (Mission_all_attack)
	{
		return Iff_info[attacker_team].attackee_bitmask_all_teams_at_war;
	}
	// normal
	else
	{
		return Iff_info[attacker_team].attackee_bitmask;
	}
}

/**
 * Rather slower, since it has to construct a mask
 *
 * @param attackee_team Team of attacker
 * @return Bitmask
 */
int iff_get_attacker_mask(int attackee_team)
{
	Assert(attackee_team >= 0 && attackee_team < (int)Iff_info.size());

	int i, attacker_bitmask = 0;
	for (i = 0; i < (int) Iff_info.size(); i++)
	{
		if (iff_x_attacks_y(i, attackee_team))
			attacker_bitmask |= iff_get_mask(i);
	}

	return attacker_bitmask;
}

/**
 * Similar to above
 *
 * @param team_x Team of attacker
 * @param team_y Team of attackee
 */
bool iff_x_attacks_y(int team_x, int team_y)
{
	return iff_matches_mask(team_y, iff_get_attackee_mask(team_x));
}

/**
 * Generate a mask for a team
 *
 * @param team Team to generate mask for
 */
int iff_get_mask(int team)
{
	return (1 << team);
}

/**
 * See if the mask contains the team
 *
 * @param team Team to test
 * @param mask Mask
 */
bool iff_matches_mask(int team, int mask)
{
	return (iff_get_mask(team) & mask) != 0;
}

/**
 * Get the color from the color index
 */
color *iff_get_color(int color_index, int is_bright)
{
	return is_bright == 0 ? &Iff_colors[color_index].first : &Iff_colors[color_index].second;
}

/**
 * Get the color index, taking objective vs. subjective into account
 */
color *iff_get_color_by_team(int team, int seen_from_team, int is_bright)
{
	Assertion(SCP_vector_inbounds(Iff_info, team), "Cannot get color by team because team is invalid. Get a coder!");
	Assertion((is_bright == 0 || is_bright == 1), "Error with brightness selection when getting color. Get a coder!");

	int color_index;
	SCP_map<int, int> color_map;

	if (AccessibilityEnabled) {
		color_index = Iff_info[team].accessibility_color_index;
		if (SCP_vector_inbounds(Iff_info, seen_from_team)) {
			color_map = Iff_info[seen_from_team].accessibility_observed_color_map;
		}
	} else {
		color_index = Iff_info[team].color_index;
		if (SCP_vector_inbounds(Iff_info, seen_from_team)) {
			color_map = Iff_info[seen_from_team].observed_color_map;
		}
	}


	// is this guy being seen by anyone?
	if (seen_from_team < 0) {
		return is_bright == 0 ? &Iff_colors[color_index].first : &Iff_colors[color_index].second;
	}

	// Goober5000 - base the following on "sees X as" from iff code
	// c.f. AL's comment:

	// AL 12-26-97:	it seems IFF color needs to be set relative to the player team.  If
	//						the team in question is the same as the player, then it should be 
	//						drawn friendly.  If the team is different than the player's, then draw the
	//						appropriate IFF.

	// This goes here because seen_from_team can validly be a negative number. In that case color_map is undefined
	// but that doesn't matter because we return in the statement above here. Below here, color_map needs to be defined
	// which will happen if seen_from_team is valid.
	Assertion(SCP_vector_inbounds(Iff_info, seen_from_team), "Cannot get color because seen_from_team is invalid. Get a coder!");

	// assume an observed color is defined; if not, use normal color
	auto it = color_map.find(team);

	if (it != color_map.end()) {
		color_index = it->second;
	}
	
	return is_bright == 0 ? &Iff_colors[color_index].first : &Iff_colors[color_index].second;
}

/**
 * Get the color index, taking objective vs. subjective into account
 * 
 * this one for the function calls that include some - any - of object
 */
color *iff_get_color_by_team_and_object(int team, int seen_from_team, int is_bright, object *objp)
{
	Assertion(SCP_vector_inbounds(Iff_info, team), "Cannot get color by team because team is invalid. Get a coder!");
	Assertion((is_bright == 0 || is_bright == 1), "Error with brightness selection when getting color. Get a coder!");

	int color_index;
	SCP_map<int, int> color_map;

	if (AccessibilityEnabled) {
		color_index = Iff_info[team].accessibility_color_index;
		if (SCP_vector_inbounds(Iff_info, seen_from_team)) {
			color_map = Iff_info[seen_from_team].accessibility_observed_color_map;
		}
	} else {
		color_index = Iff_info[team].color_index;
		if (SCP_vector_inbounds(Iff_info, seen_from_team)) {
			color_map = Iff_info[seen_from_team].observed_color_map;
		}
	}

	// is this guy being seen by anyone?
	if (seen_from_team < 0) {
		return is_bright == 0 ? &Iff_colors[color_index].first : &Iff_colors[color_index].second;
	}

	// This goes here because seen_from_team can validly be a negative number. In that case color_map is undefined
	// but that doesn't matter because we return in the statement above here. Below here, color_map needs to be defined
	// which will happen if seen_from_team is valid.
	Assertion(SCP_vector_inbounds(Iff_info, seen_from_team), "Cannot get color because seen_from_team is invalid. Get a coder!");

	int this_color_index = -1;
	{
		auto it = color_map.find(team);
		if (it != color_map.end())
			this_color_index = it->second;
	}

	int alt_color_index = -1;

	// switch in case some sort of parent iff color inheritance for example for bombs is wanted...
	switch(objp->type)
	{
		case OBJ_SHIP:
		{
			auto it = Ships[objp->instance].ship_iff_color.find({ seen_from_team, team });
			if (it != Ships[objp->instance].ship_iff_color.end())
			{
				alt_color_index = it->second;
				break;
			}
			it = Ship_info[Ships[objp->instance].ship_info_index].ship_iff_info.find({ seen_from_team, team });
			if (it != Ship_info[Ships[objp->instance].ship_info_index].ship_iff_info.end())
			{
				alt_color_index = it->second;
			}
			break;
		}
		default:
			break;
	}

	// temporary solution.... 
	if (alt_color_index >= 0) {
		this_color_index = alt_color_index;
	} else {
		this_color_index = color_index;
	}


	return is_bright == 0 ? &Iff_colors[this_color_index].first : &Iff_colors[this_color_index].second;
}
