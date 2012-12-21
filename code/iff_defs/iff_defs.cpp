/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */



#include "globalincs/def_files.h"
#include "iff_defs/iff_defs.h"
#include "parse/parselo.h"
#include "hud/hud.h"
#include "mission/missionparse.h"
#include "ship/ship.h"

extern int radar_target_id_flags;

int Num_iffs;
iff_info Iff_info[MAX_IFFS];

int Iff_traitor;

int radar_iff_color[5][2][4];
int iff_bright_delta;
int *iff_color_brightness = &iff_bright_delta;

// global only to file
color Iff_colors[MAX_IFF_COLORS][2];		// AL 1-2-97: Create two IFF colors, regular and bright

flag_def_list rti_flags[] = {
	{ "crosshairs",			RTIF_CROSSHAIRS,	0 },
	{ "blink",				RTIF_BLINK,			0 },
	{ "pulsate",			RTIF_PULSATE,		0 },
	{ "enlarge",			RTIF_ENLARGE,		0 }
};

int Num_rti_flags = sizeof(rti_flags)/sizeof(flag_def_list);

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
	typedef struct temp_color_t {
		int	r;
		int g;
		int b;
	} temp_color_t;

	int i, idx;
	temp_color_t *c;

	static int num_iff_colors = 0;
	static temp_color_t temp_colors[MAX_IFF_COLORS];

	Assert(r >= 0 && r <= 255);
	Assert(g >= 0 && g <= 255);
	Assert(b >= 0 && b <= 255);

	// make sure we're under the limit
	if (num_iff_colors >= MAX_IFF_COLORS)
	{
		Warning(LOCATION, "Too many iff colors!  Ignoring the rest...\n");
		return 0;
	}

	// find out if this color is in use
	for (i = 0; i < num_iff_colors; i++)
	{
		c = &temp_colors[i];

		if (c->r == r && c->g == g && c->b == b)
			return i;
	}

	// not in use, so add a new slot
	idx = num_iff_colors;
	num_iff_colors++;

	// save the values
	c = &temp_colors[idx];
	c->r = r;
	c->g = g;
	c->b = b;

	// init it
	gr_init_alphacolor(&Iff_colors[idx][0], r, g, b, iff_get_alpha_value(false));
	gr_init_alphacolor(&Iff_colors[idx][1], r, g, b, iff_get_alpha_value(true));

	// return the new slot
	return idx;
}

/**
 * Parse the table
 */
void iff_init()
{
	char traitor_name[NAME_LENGTH];
	char attack_names[MAX_IFFS][MAX_IFFS][NAME_LENGTH];
	struct {
		char iff_name[NAME_LENGTH];
		int color_index;
	} observed_color_table[MAX_IFFS][MAX_IFFS];

	int num_attack_names[MAX_IFFS];
	int num_observed_colors[MAX_IFFS];
	int i, j, k;
	int string_idx;

	int rval;
	if ((rval = setjmp(parse_abort)) != 0)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "iff_defs.tbl", rval));
		return;
	}

	// Goober5000 - if table doesn't exist, use the default table
	if (cf_exists_full("iff_defs.tbl", CF_TYPE_TABLES))
		read_file_text("iff_defs.tbl", CF_TYPE_TABLES);
	else
		read_file_text_from_array(defaults_get_file("iff_defs.tbl"));

	reset_parse();	

	// parse the table --------------------------------------------------------

	required_string("#IFFs");

	// get the traitor
	required_string("$Traitor IFF:");
	stuff_string(traitor_name, F_NAME, NAME_LENGTH);

	int rgb[3];

	// check if alternate colours are wanted to be used for these
	// Marks various stuff... like asteroids
	if ((optional_string("$Selection Color:")) || (optional_string("$Selection Colour:")))
	{
		stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
		iff_init_color(rgb[0], rgb[1], rgb[2]);
	}
	else
		iff_init_color(0xff, 0xff, 0xff);

	// Marks the ship currently saying something
	if ((optional_string("$Message Color:")) || (optional_string("$Message Colour:")))
	{
		stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
		iff_init_color(rgb[0], rgb[1], rgb[2]);
	}
	else
		iff_init_color(0x7f, 0x7f, 0x7f);

	// Marks the tagged ships
	if ((optional_string("$Tagged Color:")) || (optional_string("$Tagged Colour:")))
	{
		stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
		iff_init_color(rgb[0], rgb[1], rgb[2]);
	}
	else
		iff_init_color(0xff, 0xff, 0x00);

	// init radar blips colour table
	int a_bright,a_dim;
	bool alternate_blip_color = false;
	for (i=0;i<5;i++)
	{
		for (j=0;j<2;j++)
		{
			for (k=0;k<3;k++)
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
	if (optional_string("$Use Alternate Blip Coloring:"))
	{
		stuff_boolean(&alternate_blip_color);
	}

	// Parse blip colours, their order is hardcoded.
	if ((optional_string("$Missile Blip Color:")) || (optional_string("$Missile Blip Colour:")))
	{
		stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
		for (i=0;i<3;i++)
		{
			Assert(rgb[i] >= 0 && rgb[i] <= 255);
			radar_iff_color[0][1][i] = rgb[i];
			radar_iff_color[0][0][i] = rgb[i]/2;
		}
	}		

	if ((optional_string("$Navbuoy Blip Color:")) || (optional_string("$Navbuoy Blip Colour:")))
	{
		stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
		for (i=0;i<3;i++)
		{
			Assert(rgb[i] >= 0 && rgb[i] <= 255);
			radar_iff_color[1][1][i] = rgb[i];
			radar_iff_color[1][0][i] = rgb[i]/2;
		}
	}

	if ((optional_string("$Warping Blip Color:")) || (optional_string("$Warping Blip Colour:")))
	{
		stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
		for (i=0;i<3;i++)
		{
			Assert(rgb[i] >= 0 && rgb[i] <= 255);
			radar_iff_color[2][1][i] = rgb[i];
			radar_iff_color[2][0][i] = rgb[i]/2;
		}
	}

	if ((optional_string("$Node Blip Color:")) || (optional_string("$Node Blip Colour:")))
	{
		stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
		for (i=0;i<3;i++)
		{
			Assert(rgb[i] >= 0 && rgb[i] <= 255);
			radar_iff_color[3][1][i] = rgb[i];
			radar_iff_color[3][0][i] = rgb[i]/2;
		}
	}

	if ((optional_string("$Tagged Blip Color:")) || (optional_string("$Tagged Blip Colour:")))
	{
		stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
		for (i=0;i<3;i++)
		{
			Assert(rgb[i] >= 0 && rgb[i] <= 255);
			radar_iff_color[4][1][i] = rgb[i];
			radar_iff_color[4][0][i] = rgb[i]/2;
		}
	}

	if (alternate_blip_color == true)
	{
		a_bright = iff_get_alpha_value(true);
		a_dim = iff_get_alpha_value(false);
		for (i=0;i<5;i++)
		{
			if (radar_iff_color[i][0][0] >= 0)
			{
				for (j=0;j<3;j++)
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
		for (i=0;i<5;i++)
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
	Num_iffs = 0;
	while (required_string_either("#End","$IFF Name:"))
	{
		iff_info *iff;
		int cur_iff;
		
		// make sure we're under the limit
		if (Num_iffs >= MAX_IFFS)
		{
			Warning(LOCATION, "Too many iffs in iffs_defs.tbl!  Max is %d.\n", MAX_IFFS);
			skip_to_start_of_string("#End", NULL);
			break;
		}

		// add new IFF
		iff = &Iff_info[Num_iffs];
		cur_iff = Num_iffs;
		Num_iffs++;


		// get required IFF info ----------------------------------------------

		// get the iff name
		required_string("$IFF Name:");
		stuff_string(iff->iff_name, F_NAME, NAME_LENGTH);

		// get the iff color
		if (check_for_string("$Colour:"))
			required_string("$Colour:");
		else
			required_string("$Color:");
		stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
		iff->color_index = iff_init_color(rgb[0], rgb[1], rgb[2]);


		// get relationships between IFFs -------------------------------------

		// get the list of iffs attacked
		if (optional_string("$Attacks:"))
			num_attack_names[cur_iff] = stuff_string_list(attack_names[cur_iff], MAX_IFFS);
		else
			num_attack_names[cur_iff] = 0;

		// get the list of observed colors
		num_observed_colors[cur_iff] = 0;
		while (optional_string("+Sees"))
		{
			// get iff observed
			stuff_string_until(observed_color_table[cur_iff][num_observed_colors[cur_iff]].iff_name, "As:", NAME_LENGTH);
			required_string("As:");

			// get color observed
			stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
			observed_color_table[cur_iff][num_observed_colors[cur_iff]].color_index = iff_init_color(rgb[0], rgb[1], rgb[2]);

			// increment
			num_observed_colors[cur_iff]++;
		}


		// get flags ----------------------------------------------------------

		// get iff flags
		iff->flags = 0;
		if (optional_string("$Flags:"))
		{
			char flag_strings[MAX_IFF_FLAGS][NAME_LENGTH];

			int num_strings = stuff_string_list(flag_strings, MAX_IFF_FLAGS);
			for (string_idx = 0; string_idx < num_strings; string_idx++)
			{
				if (!stricmp(NOX("support allowed"), flag_strings[string_idx]))
					iff->flags |= IFFF_SUPPORT_ALLOWED;
				else if (!stricmp(NOX("exempt from all teams at war"), flag_strings[string_idx]))
					iff->flags |= IFFF_EXEMPT_FROM_ALL_TEAMS_AT_WAR;
				else if (!stricmp(NOX("orders hidden"), flag_strings[string_idx]))
					iff->flags |= IFFF_ORDERS_HIDDEN;
				else if (!stricmp(NOX("orders shown"), flag_strings[string_idx]))
					iff->flags |= IFFF_ORDERS_SHOWN;
				else if (!stricmp(NOX("wing name hidden"), flag_strings[string_idx]))
					iff->flags |= IFFF_WING_NAME_HIDDEN;
				else
					Warning(LOCATION, "Bogus string in iff flags: %s\n", flag_strings[string_idx]);
			}
		}

		// get default ship flags
		iff->default_parse_flags = 0;
		if (optional_string("$Default Ship Flags:"))
		{
			i = 0;
			j = 0;
			char flag_strings[MAX_PARSE_OBJECT_FLAGS][NAME_LENGTH];
			int num_strings = stuff_string_list(flag_strings, MAX_PARSE_OBJECT_FLAGS);
			for (i = 0; i < num_strings; i++)
			{
				for (j = 0; j < MAX_PARSE_OBJECT_FLAGS; j++)
				{
					if (!stricmp(flag_strings[i], Parse_object_flags[j]))
					{
						iff->default_parse_flags |= (1 << j);
						break;
					}
				}
			}

			if (j == MAX_PARSE_OBJECT_FLAGS)
				Warning(LOCATION, "Bogus string in iff default ship flags: %s\n", flag_strings[i]);
		}

		// again
		iff->default_parse_flags2 = 0;
		if (optional_string("$Default Ship Flags2:"))
		{
			i = 0;
			j = 0;
			char flag_strings[MAX_PARSE_OBJECT_FLAGS_2][NAME_LENGTH];
			int num_strings = stuff_string_list(flag_strings, MAX_PARSE_OBJECT_FLAGS_2);
			for (i = 0; i < num_strings; i++)
			{
				for (j = 0; j < MAX_PARSE_OBJECT_FLAGS_2; j++)
				{
					if (!stricmp(flag_strings[i], Parse_object_flags_2[j]))
					{
						iff->default_parse_flags2 |= (1 << j);
						break;
					}
				}
			}

			if (j == MAX_PARSE_OBJECT_FLAGS_2)
				Warning(LOCATION, "Bogus string in iff default ship flags2: %s\n", flag_strings[i]);
		}
	}
	
	required_string("#End");


	// now resolve the relationships ------------------------------------------

	// first get the traitor
	Iff_traitor = iff_lookup(traitor_name);
	if (Iff_traitor < 0)
	{
		Iff_traitor = 0;
		Warning(LOCATION, "Traitor IFF %s not found in iff_defs.tbl!  Defaulting to %s.\n", traitor_name, Iff_info[Iff_traitor].iff_name);
	}

	// next get the attackees and colors
	for (int cur_iff = 0; cur_iff < Num_iffs; cur_iff++)
	{
		iff_info *iff = &Iff_info[cur_iff];

		// clear the iffs to be attacked
		iff->attackee_bitmask = 0;
		iff->attackee_bitmask_all_teams_at_war = 0;

		// clear the observed colors
		for (j = 0; j < MAX_IFFS; j++)
			iff->observed_color_index[j] = -1;

		// resolve the list names
		for (int list_index = 0; list_index < MAX_IFFS; list_index++)
		{
			// are we within the number of attackees listed?
			if (list_index < num_attack_names[cur_iff])
			{
				// find out who
				int target_iff = iff_lookup(attack_names[cur_iff][list_index]);

				// valid?
				if (target_iff >= 0)
					iff->attackee_bitmask |= iff_get_mask(target_iff);
				else
					Warning(LOCATION, "Attack target IFF %s not found for IFF %s in iff_defs.tbl!\n", attack_names[cur_iff][list_index], iff->iff_name);
			}

			// are we within the number of colors listed?
			if (list_index < num_observed_colors[cur_iff])
			{
				// find out who
				int target_iff = iff_lookup(observed_color_table[cur_iff][list_index].iff_name);

				// valid?
				if (target_iff >= 0)
					iff->observed_color_index[target_iff] = observed_color_table[cur_iff][list_index].color_index;
				else
					Warning(LOCATION, "Observed color IFF %s not found for IFF %s in iff_defs.tbl!\n", observed_color_table[cur_iff][list_index].iff_name, iff->iff_name);
			}
		}

		// resolve the all teams at war relationships
		if (iff->flags & IFFF_EXEMPT_FROM_ALL_TEAMS_AT_WAR)
		{
			// exempt, so use standard attacks
			iff->attackee_bitmask_all_teams_at_war = iff->attackee_bitmask;
		}
		else
		{
			// nonexempt, so build bitmask of all other nonexempt teams
			for (int other_iff = 0; other_iff < Num_iffs; other_iff++)
			{
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

	// add tbl/tbm to multiplayer validation list
	extern void fs2netd_add_table_validation(char *tblname);
	fs2netd_add_table_validation("iff_defs.tbl");
}

/**
 * Find the iff name
 *
 * @param iff_name Pointer to name as a string
 * @return Index into ::Iff_info array
 */
int iff_lookup(char *iff_name)
{
	// bogus
	Assert(iff_name);

	if(iff_name == NULL)
		return -1;

	for (int i = 0; i < Num_iffs; i++)
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
	Assert(attacker_team >= 0 && attacker_team < Num_iffs);

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
	Assert(attackee_team >= 0 && attackee_team < Num_iffs);

	int i, attacker_bitmask = 0;
	for (i = 0; i < Num_iffs; i++)
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
 *
 * @return >0 if true, 0 if false
 */
int iff_x_attacks_y(int team_x, int team_y)
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
 *
 * @return 1 if matches, 0 if does not match
 */
int iff_matches_mask(int team, int mask)
{
	return (iff_get_mask(team) & mask) ? 1 : 0;
}

/**
 * Get the color from the color index
 */
color *iff_get_color(int color_index, int is_bright)
{
	return &Iff_colors[color_index][is_bright];
}

/**
 * Get the color index, taking objective vs. subjective into account
 */
color *iff_get_color_by_team(int team, int seen_from_team, int is_bright)
{
	Assert(team >= 0 && team < Num_iffs);
	Assert(seen_from_team < Num_iffs);
	Assert(is_bright == 0 || is_bright == 1);


	// is this guy being seen by anyone?
	if (seen_from_team < 0)
		return &Iff_colors[Iff_info[team].color_index][is_bright];

	// Goober5000 - base the following on "sees X as" from iff code
	// c.f. AL's comment:

	// AL 12-26-97:	it seems IFF color needs to be set relative to the player team.  If
	//						the team in question is the same as the player, then it should be 
	//						drawn friendly.  If the team is different than the player's, then draw the
	//						appropriate IFF.


	// assume an observed color is defined; if not, use normal color
	int color_index = Iff_info[seen_from_team].observed_color_index[team];
	if (color_index < 0)
		color_index = Iff_info[team].color_index;


	return &Iff_colors[color_index][is_bright];
}

/**
 * Get the color index, taking objective vs. subjective into account
 * 
 * this one for the function calls that include some - any - of object
 */
color *iff_get_color_by_team_and_object(int team, int seen_from_team, int is_bright, object *objp)
{
	Assert(team >= 0 && team < Num_iffs);
	Assert(seen_from_team < Num_iffs);
	Assert(is_bright == 0 || is_bright == 1);

	int alt_color_index = -1;

	// is this guy being seen by anyone?
	if (seen_from_team < 0)
		return &Iff_colors[Iff_info[team].color_index][is_bright];

	int color_index = Iff_info[seen_from_team].observed_color_index[team];

	// switch incase some sort of parent iff color inheritance for example for bombs is wanted...
	switch(objp->type)
	{
		case OBJ_SHIP:
			if (Ships[objp->instance].ship_iff_color[seen_from_team][team] >= 0)
			{
				alt_color_index = Ships[objp->instance].ship_iff_color[seen_from_team][team];
			}
			else
			{
				alt_color_index = Ship_info[Ships[objp->instance].ship_info_index].ship_iff_info[seen_from_team][team];
			}
			break;
		default:
			break;
	}

	// temporary solution.... 
	if (alt_color_index >= 0)
		color_index = alt_color_index;
	if (color_index < 0)
		color_index = Iff_info[team].color_index;


	return &Iff_colors[color_index][is_bright];
}
